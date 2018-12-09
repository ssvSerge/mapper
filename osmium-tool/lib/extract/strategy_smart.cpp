/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium-tool/

Copyright (C) 2013-2018  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "strategy_smart.hpp"
#include "../util.hpp"

#include <osmium/handler/check_order.hpp>
#include <osmium/util/string.hpp>

namespace strategy_smart {

    void Data::add_relation(const osmium::Relation& relation) {
        for (const auto& member : relation.members()) {
            const auto ref = member.positive_ref();
            switch (member.type()) {
                case osmium::item_type::node:
                    extra_node_ids.set(ref);
                    break;
                case osmium::item_type::way:
                    extra_way_ids.set(ref);
                    break;
                default:
                    break;
            }
        }
    }

    void Data::add_relation_parents(osmium::unsigned_object_id_type id, const osmium::index::RelationsMapIndex& map) {
        map.for_each_parent(id, [&](osmium::unsigned_object_id_type parent_id) {
            if (!relation_ids.get(parent_id) &&
                !extra_relation_ids.get(parent_id)) {
                relation_ids.set(parent_id);
                add_relation_parents(parent_id, map);
            }
        });
    }

    Strategy::Strategy(const std::vector<std::unique_ptr<Extract>>& extracts, const osmium::util::Options& options) {
        m_extracts.reserve(extracts.size());
        for (const auto& extract : extracts) {
            m_extracts.emplace_back(*extract);
        }

        for (const auto& option : options) {
            if (std::string{"types"} != option.first) {
                warning(std::string{"Ignoring unknown option '"} + option.first + "' for 'smart' strategy.\n");
            }
        }

        const auto types = options.get("types");
        if (types.empty()) {
            m_types = {"multipolygon"};
        } else if (types != "any") {
            m_types = osmium::split_string(types, ',', true);
        }
    }

    const char* Strategy::name() const noexcept {
        return "smart";
    }

    bool Strategy::check_type(const osmium::Relation& relation) const noexcept {
        if (m_types.empty()) {
            return true;
        }

        const char* type = relation.tags()["type"];
        if (!type) {
            return false;
        }
        const auto it = std::find(m_types.begin(), m_types.end(), type);
        return it != m_types.end();
    }

    void Strategy::show_arguments(osmium::util::VerboseOutput& vout) {
        vout << "Additional strategy options:\n";
        if (m_types.empty()) {
            vout << "  types: any\n";
        } else {
            vout << "  types:\n";
            for (const auto& type : m_types) {
                vout << "      " << type << '\n';
            }
        }
        vout << '\n';
    }

    class Pass1 : public Pass<Strategy, Pass1> {

        osmium::handler::CheckOrder m_check_order;
        osmium::index::RelationsMapStash m_relations_map_stash;

    public:

        explicit Pass1(Strategy& strategy) :
            Pass(strategy) {
        }

        void node(const osmium::Node& node) {
            m_check_order.node(node);
        }

        void enode(extract_data& e, const osmium::Node& node) {
            if (e.contains(node.location())) {
                e.node_ids.set(node.positive_id());
            }
        }

        void way(const osmium::Way& way) {
            m_check_order.way(way);
        }

        void eway(extract_data& e, const osmium::Way& way) {
            for (const auto& nr : way.nodes()) {
                if (e.node_ids.get(nr.positive_ref())) {
                    e.way_ids.set(way.positive_id());
                    return;
                }
            }
        }

        void relation(const osmium::Relation& relation) {
            m_check_order.relation(relation);
            m_relations_map_stash.add_members(relation);
        }

        void erelation(extract_data& e, const osmium::Relation& relation) {
            for (const auto& member : relation.members()) {
                switch (member.type()) {
                    case osmium::item_type::node:
                        if (e.node_ids.get(member.positive_ref())) {
                            e.relation_ids.set(relation.positive_id());
                            if (strategy().check_type(relation)) {
                                e.add_relation(relation);
                            }
                            return;
                        }
                        break;
                    case osmium::item_type::way:
                        if (e.way_ids.get(member.positive_ref())) {
                            e.relation_ids.set(relation.positive_id());
                            if (strategy().check_type(relation)) {
                                e.add_relation(relation);
                            }
                            return;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        osmium::index::RelationsMapStash& relations_map_stash() noexcept {
            return m_relations_map_stash;
        }

    }; // class Pass1

    class Pass2 : public Pass<Strategy, Pass2> {

    public:

        explicit Pass2(Strategy& strategy) :
            Pass(strategy) {
        }

        void eway(extract_data& e, const osmium::Way& way) {
            if (e.way_ids.get(way.positive_id()) ||
                e.extra_way_ids.get(way.positive_id())) {
                for (const auto& nr : way.nodes()) {
                    e.extra_node_ids.set(nr.ref());
                }
            }
        }

    }; // class Pass2

    class Pass3 : public Pass<Strategy, Pass3> {

    public:

        explicit Pass3(Strategy& strategy) :
            Pass(strategy) {
        }

        void enode(extract_data& e, const osmium::Node& node) {
            if (e.node_ids.get(node.positive_id()) ||
                e.extra_node_ids.get(node.positive_id())) {
                e.write(node);
            }
        }

        void eway(extract_data& e, const osmium::Way& way) {
            if (e.way_ids.get(way.positive_id()) ||
                e.extra_way_ids.get(way.positive_id())) {
                e.write(way);
            }
        }

        void erelation(extract_data& e, const osmium::Relation& relation) {
            if (e.relation_ids.get(relation.positive_id()) ||
                e.extra_relation_ids.get(relation.positive_id())) {
                e.write(relation);
            }
        }

    }; // class Pass3

    void Strategy::run(osmium::util::VerboseOutput& vout, bool display_progress, const osmium::io::File& input_file) {
        if (input_file.filename().empty()) {
            throw osmium::io_error{"Can not read from STDIN when using 'smart' strategy."};
        }

        vout << "Running 'smart' strategy in three passes...\n";
        const std::size_t file_size = osmium::util::file_size(input_file.filename());
        osmium::ProgressBar progress_bar{file_size * 3, display_progress};

        vout << "First pass (of three)...\n";
        Pass1 pass1{*this};
        pass1.run(progress_bar, input_file, osmium::io::read_meta::no);
        progress_bar.file_done(file_size);

        // recursively get parents of all relations that are in an extract
        const auto relations_map = pass1.relations_map_stash().build_member_to_parent_index();
        for (auto& e : m_extracts) {
            for (osmium::unsigned_object_id_type id : e.relation_ids) {
                e.add_relation_parents(id, relations_map);
            }
        }

        progress_bar.remove();
        vout << "Second pass (of three)...\n";
        Pass2 pass2{*this};
        pass2.run(progress_bar, input_file, osmium::osm_entity_bits::way, osmium::io::read_meta::no);
        progress_bar.file_done(file_size);

        progress_bar.remove();
        vout << "Third pass (of three)...\n";
        Pass3 pass3{*this};
        pass3.run(progress_bar, input_file);

        progress_bar.done();
    }

} // namespace strategy_smart

