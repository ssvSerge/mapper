#include <iostream>
#include <string>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <list>
#include <deque>
#include <sstream>
#include <iomanip>
#include <functional> 
#include <fstream>

#include "..\_tools\XmlInspector.hpp"

using namespace std;

#define GEO_COORDINATE_ERROR                    (-1000)
#define GEO_ID_ERROR                            (-1)

#define ARRAY_SIZE(x)                           ( sizeof(x) / sizeof(x[0]) )
#define SET_FALSE(var)                          { var=false; cout << "Error ar: " << __LINE__ << endl; }
#define RET_FALSE                               { cout << "Error at: " << __LINE__ << endl; return false; }

#define CFG_CMD_PARAM                           "Param"
#define CFG_CMD_ROAD                            "Road"
#define CFG_CMD_NAME                            "Name"
#define CFG_CMD_BUILDING                        "Building"

#define CFG_CMD_DELETE                          "Delete"

#define CFG_TEXT_TRANSPORT_METRO_ENTRANCE       "MetroEntrance"
#define CFG_TEXT_TRANSPORT                      "Transport"
#define CFG_TEXT_TRANSPORT_BUS                  "Bus"
#define CFG_TEXT_TRANSPORT_TRAM                 "Tram"
#define CFG_TEXT_TRANSPORT_TRAIN                "Train"
#define CFG_TEXT_TRANSPORT_TROLLEY              "Trolley"
#define CFG_TEXT_TRANSPORT_TAXI                 "Taxi"
#define CFG_TEXT_TRANSPORT_METRO                "Metro"

#define CFG_TEXT_HIGHWAY_MOTORWAY               "Motorway"
#define CFG_TEXT_HIGHWAY_PRIMARY                "Primary"
#define CFG_TEXT_HIGHWAY_SECONDARY              "Secondary"
#define CFG_TEXT_HIGHWAY_RESIDENTIAL            "Residential"
#define CFG_TEXT_HIGHWAY_STREET                 "Street"
#define CFG_TEXT_HIGHWAY_PATH                   "Path"
#define CFG_TEXT_HIGHWAY_METRO                  "Metro"

#define  POINT_ID_DEFAULT                       (0x00000000)
#define  POINT_ID_TRANSPORT                     (0x00000001)
#define  POINT_ID_TRANSPORT_BUS                 (0x00000002)
#define  POINT_ID_TRANSPORT_TRAM                (0x00000004)
#define  POINT_ID_TRANSPORT_TRAIN               (0x00000008)
#define  POINT_ID_TRANSPORT_TROLLEY             (0x00000010)
#define  POINT_ID_TRANSPORT_TAXI                (0x00000020)
#define  POINT_ID_TRANSPORT_METRO               (0x00000040)
#define  POINT_ID_TRANSPORT_METRO_ENTRANCE      (0x00000080)

#define  ROAD_TYPE_DEFAULT                      (0x00000000)
#define  ROAD_TYPE_MOTORWAY                     (0x00000001)
#define  ROAD_TYPE_PRIMARY                      (0x00000002)
#define  ROAD_TYPE_SECONDARY                    (0x00000004)
#define  ROAD_TYPE_RESIDENTIAL                  (0x00000008)
#define  ROAD_TYPE_STREET                       (0x00000010)
#define  ROAD_TYPE_PATH                         (0x00000020)
#define  ROAD_TYPE_METRO                        (0x00000040)

#define  OPTION_ID_LANGUAGE                     (0x00000001)
#define  OPTION_ID_DELETE                       (0x00000002)


typedef struct tag_xml_lex_info {
    string                  k;
    string                  v;
    uint64_t                t;
}   XML_LEX_INFO;


typedef vector<XML_LEX_INFO>        CFG_LEX_LIST;


typedef struct tag_xml_params_list {
    string                  op;
    string                  p [32];
    XML_LEX_INFO            info;
    bool                    del;
}   XML_PARAMS_LIST;


typedef struct tag_xml_mapper {
    string                  param;
    uint64_t                key;
}   XML_MAPPER;


typedef struct tag_osm_param {
    string                  k;
    string                  v;
}   OSM_PARAM;


typedef list<OSM_PARAM>   OSM_PARAMS_LIST;


typedef struct tag_osm_text_info {
    string                  type;
    uint64_t                id;
    string                  lang;
    string                  text;
}   OSM_TEXT_INFO;


typedef vector<OSM_TEXT_INFO>       OSM_TEXT_LISTS;


typedef struct tag_geo_coord {
    double                  lat;
    double                  lon;
}   GEO_COORD;


typedef list<GEO_COORD>     GEO_CORD_LIST;


typedef vector<uint64_t>    REF_LIST;


#pragma pack (1)


typedef struct tag_osm_node_cache {
    uint64_t                id;                 // Номер нода
    uint64_t                type;               // Тип объекта
    double                  lat;                // Широта 
    double                  lon;                // Долгота
}   OSM_NODE_CACHE;


typedef struct tag_osm_data_cache {
    uint64_t                id;                 // id нода или дороги
    uint64_t                off;                // смещение в файле
}   OSM_DATA_CACHE;


#pragma pack ()


class OsmWayInfo {

    public:
        OsmWayInfo () {
            clear ();
        }

        void clear () {
            id    = 0;
            type  = 0;
            lanes = 1;
            nodes.clear();
        }

    public:
        uint64_t                id;
        uint64_t                type;
        uint64_t                lanes;
        GEO_CORD_LIST           nodes;
};



class OsmObject {

    public:
        OsmObject () {
            Clear ();
        }

    public:
        void Clear () {
            used        = false;
            deleted     = false;
            node.id     = GEO_ID_ERROR;
            node.lat    = GEO_COORDINATE_ERROR;
            node.lon    = GEO_COORDINATE_ERROR;
            node_name.clear ();
            params.clear ();
            names.clear ();
            refs.clear ();
        }

    public:
        string              node_name;          // XML имя объекта (node, way, relation, nd, tag, member)

    public:
        OSM_NODE_CACHE      node;               // Данные для записи в файл.
        OSM_PARAM           tmp;                // текущий TAG.
        uint64_t            obj_ref;            // 

    public:
        bool                used;               // TRUE в том случае, если объект используется в карте
        bool                deleted;            // TRUE если объект необходимо удалить (опции proposed, completted)
        OSM_PARAMS_LIST     params;             // Список тэгов k="" и v=""
        OSM_TEXT_LISTS      names;              // Список имен
        REF_LIST            refs;               // 
        OsmWayInfo          way;
};


CFG_LEX_LIST        g_lex_list_nodes;
CFG_LEX_LIST        g_lex_list_ways;
CFG_LEX_LIST        g_lex_list_rels;
string              g_lex_default_lang;
deque<OsmObject>    g_xml_ctx;

bool                g_nodes_actvie = false;
bool                g_ways_active  = false;
bool                g_rels_active  = false;
ofstream            g_node_idx_file;
ofstream            g_names_file;
ofstream            g_names_idx_file;
ifstream            g_nodes_cache;
ofstream            g_ways_file;
ofstream            g_ways_idx_file;

uint64_t            g_nodes_cache_cnt   = 0;
uint64_t            g_offset_names      = 0;
uint64_t            g_offset_ways       = 0;
string              g_osm_file_name;

bool OpenNodeCache (string osm_file_name) {

    string      file_name;

    file_name = osm_file_name;
    file_name += ".nodes.idx";
    g_node_idx_file.open (file_name, std::ofstream::binary + std::ofstream::trunc);
    if ( !g_node_idx_file ) {
        return false;
    }

    file_name = osm_file_name;
    file_name += ".names.txt";
    g_names_file.open (file_name, std::ofstream::binary + std::ofstream::trunc);
    if ( !g_names_file ) {
        return false;
    }

    file_name = osm_file_name;
    file_name += ".names.idx";
    g_names_idx_file.open (file_name, std::ofstream::binary + std::ofstream::trunc);
    if ( !g_names_idx_file ) {
        return false;
    }

    return true;
}

bool CloseNodeCache () {

    g_node_idx_file.close ();

    return true;
}

bool OpenWayCache (string osm_file_name) {

    string      file_name;

    file_name = osm_file_name;
    file_name += ".nodes.idx";
    g_nodes_cache.open (file_name, ofstream::binary | ios::ate);
    if ( !g_nodes_cache ) {
        return false;
    }

    g_nodes_cache_cnt = g_nodes_cache.tellg ();
    g_nodes_cache_cnt /= sizeof (OSM_NODE_CACHE);

    file_name = osm_file_name;
    file_name += ".ways.txt";
    g_ways_file.open(file_name, ofstream::binary | ios::ate);
    if ( !g_ways_file ) {
        return false;
    }

    file_name = osm_file_name;
    file_name += ".ways.idx";
    g_ways_idx_file.open (file_name, ofstream::binary | ios::ate);
    if ( !g_ways_file ) {
        return false;
    }

    return true;
}

bool MapNodeTypes (XML_PARAMS_LIST& params_list) {

    static const XML_MAPPER mapper[] = {
        { CFG_TEXT_TRANSPORT,                   POINT_ID_TRANSPORT},
        { CFG_TEXT_TRANSPORT_BUS,               POINT_ID_TRANSPORT_BUS},
        { CFG_TEXT_TRANSPORT_TRAM,              POINT_ID_TRANSPORT_TRAM},
        { CFG_TEXT_TRANSPORT_TRAIN,             POINT_ID_TRANSPORT_TRAIN},
        { CFG_TEXT_TRANSPORT_TROLLEY,           POINT_ID_TRANSPORT_TROLLEY},
        { CFG_TEXT_TRANSPORT_TAXI,              POINT_ID_TRANSPORT_TAXI},
        { CFG_TEXT_TRANSPORT_METRO,             POINT_ID_TRANSPORT_METRO},
        { CFG_TEXT_TRANSPORT_METRO_ENTRANCE,    POINT_ID_TRANSPORT_METRO_ENTRANCE},
    };

    uint64_t t = POINT_ID_DEFAULT;
    int i;
    int j;

    for ( i = 1; i < ARRAY_SIZE (params_list.p); i++ ) {

        if ( params_list.p [i].empty () ) {
            break;
        }

        for ( j = 0; j < ARRAY_SIZE (mapper); j++ ) {
            if ( params_list.p [i] == mapper [j].param ) {
                t |= mapper [j].key;
                break;
            }
        }

        if ( j == ARRAY_SIZE (mapper) ) {
            RET_FALSE;
        }
    }

    params_list.info.t = t;
    
    return true;
}

bool MapTypesRoads (XML_PARAMS_LIST& params_list) {

    static const XML_MAPPER mapper[] = {
        { CFG_TEXT_HIGHWAY_MOTORWAY,    ROAD_TYPE_MOTORWAY    },
        { CFG_TEXT_HIGHWAY_PRIMARY,     ROAD_TYPE_PRIMARY     },
        { CFG_TEXT_HIGHWAY_SECONDARY,   ROAD_TYPE_SECONDARY   },
        { CFG_TEXT_HIGHWAY_RESIDENTIAL, ROAD_TYPE_RESIDENTIAL },
        { CFG_TEXT_HIGHWAY_STREET,      ROAD_TYPE_STREET      },
        { CFG_TEXT_HIGHWAY_PATH,        ROAD_TYPE_PATH        },
        { CFG_TEXT_HIGHWAY_METRO,       ROAD_TYPE_METRO       }
    };

    uint64_t t = ROAD_TYPE_DEFAULT;
    int i;
    int j;

    for ( i = 1; i < ARRAY_SIZE (params_list.p); i++ ) {

        if ( params_list.p [i].empty () ) {
            break;
        }

        for ( j = 0; j < ARRAY_SIZE (mapper); j++ ) {
            if ( params_list.p [i] == mapper [j].param ) {
                t |= mapper [j].key;
                break;
            }
        }

        if ( j == ARRAY_SIZE (mapper) ) {
            RET_FALSE;
        }
    }

    params_list.info.t = t;

    return true;
}

bool MapTypesBuildings (XML_PARAMS_LIST& params_list) {

    return true;
}

bool MapTypes (XML_PARAMS_LIST& params_list) {

    if ( params_list.p [0] == CFG_CMD_NAME ) {
        g_lex_default_lang = params_list.p [1];
        params_list.info.t = 0;
        return true;
    }

    if ( params_list.p [0] == CFG_CMD_PARAM ) {
        return MapNodeTypes(params_list);
    }

    if ( params_list.p [0] == CFG_CMD_ROAD ) {
        return MapTypesRoads(params_list);
    }

    if ( params_list.p [0] == CFG_CMD_BUILDING ) {
        return MapTypesBuildings (params_list);
    }

    return false;
}

string& trim (string &s, string delims = " ") {

    size_t l, r, c;
    std::size_t found;

    for ( l = 0; l < s.length (); l++ ) {
        found = delims.find (s [l]);
        if ( found == string::npos ) {
            break;
        }
    }

    if ( l == s.length () ) {
        s = "";
        return s;
    }

    for ( r = s.length () - 1; r > l; r-- ) {
        found = delims.find (s [r]);
        if ( found == string::npos ) {
            break;
        }
    }

    c = (r - l + 1);
    s = s.substr (l, c);

    return s;
}

bool ProcessCfgLine (string str) {

    XML_PARAMS_LIST xml_param;

    if ( str.length () <= 1 ) {
        return true;
    }

    if ( str [0] == '#' ) {
        return true;
    }

    stringstream    ss (str);

    ss >> xml_param.op;
    if ( xml_param.op.empty () ) {
        RET_FALSE;
    }

    ss >> xml_param.info.k;
    ss >> xml_param.info.v;

    if ( xml_param.info.k.empty () || xml_param.info.v.empty () ) {
        RET_FALSE;
    }

    for ( int i = 0; i < ARRAY_SIZE (xml_param.p); i++ ) {
        ss >> xml_param.p [i];
        if ( xml_param.p [i].empty() ) {
            break;
        }
    }

    if ( !MapTypes (xml_param) ) {
        RET_FALSE;
    }

    if ( xml_param.op == "n" ) {
        g_lex_list_nodes.push_back (xml_param.info);
    } else
    if ( xml_param.op == "w" ) {
        g_lex_list_ways.push_back (xml_param.info);
    } else
    if ( xml_param.op == "r" ) {
        g_lex_list_rels.push_back (xml_param.info);
    } else {
        RET_FALSE;
    }

    return true;
}

bool LoadConfigFile (string cfg_file_name) {

    string str;
    ifstream file (cfg_file_name);

    if ( !file ) {
        RET_FALSE;
    }

    int line_id = 0;

    while ( getline (file, str) ) {

        line_id++;

        trim (str, " \t");

        if ( !ProcessCfgLine (str) ) {
            cout << "Bad CFG file at line: " << line_id << " (" << str << ") " << endl;
            RET_FALSE;
        }
    }

    return true;
}

bool LexCmp (string xml_node, string cfg_param) {

    if ( xml_node.length () == 0 ) {
        RET_FALSE;
    }
    if ( cfg_param.length () == 0 ) {
        RET_FALSE;
    }

    if ( cfg_param == "*" ) {
        return true;
    }

    bool bIsTemplate = false;
    if ( cfg_param [cfg_param.length () - 1] == '*' ) {
        bIsTemplate = true;
        cfg_param.erase (cfg_param.length () - 1);
    }

    if ( cfg_param.length () > xml_node.length () ) {
        return false;
    }

    if ( cfg_param.length () == xml_node.length () ) {
        bool ret_val = cfg_param == xml_node;
        return ret_val;
    }

    if ( !bIsTemplate ) {
        return false;
    }

    string substr = xml_node.substr (0, cfg_param.length ());

    if ( substr != cfg_param ) {
        return false;
    }

    return true;
}

void AddTag (string name) {

    OsmObject new_tag;

    if ( name == "node" ) {
        if ( ! g_nodes_actvie ) {

            g_nodes_actvie = true;
            g_ways_active  = false;
            g_rels_active  = false;

            OpenNodeCache (g_osm_file_name);
        }
    } else

    if ( name == "way" ) {
        if ( g_nodes_actvie ) {
            CloseNodeCache ();
        }
        if ( !g_ways_active ) {

            g_ways_active  = true;
            g_nodes_actvie = false;
            g_rels_active  = false;

            OpenWayCache (g_osm_file_name);
        }
    }

    g_xml_ctx.push_front (new_tag);
    g_xml_ctx.front ().node_name = name;
    g_xml_ctx.front ().node.id  = GEO_ID_ERROR;
    g_xml_ctx.front ().node.lat = GEO_COORDINATE_ERROR;
    g_xml_ctx.front ().node.lon = GEO_COORDINATE_ERROR;
}

bool ProcessTag(string name, string val) {

    static const char * skip_list [] = {
        "timestamp",     "version",     "minlat",      "maxlat",
        "minlon",        "maxlon",      "generator",   "version",
        "changeset",     "timestamp",   "user",        "uid",
        "copyright",     "attribution", "license",     "visible"
    };

    for ( int i = 0; i < ARRAY_SIZE (skip_list); i++ ) {
        if ( name == skip_list [i] ) {
            return true;
        }
    }

    if ( name == "k" ) {
        g_xml_ctx.front().tmp.k = val;
        return true;
    }

    if ( name == "v" ) {
        g_xml_ctx.front().tmp.v = val;
        return true;
    }

    if ( name == "id" ) {
        g_xml_ctx.front().node.id = stoll(val);
        return true;
    }

    if ( name == "lat" ) {
        g_xml_ctx.front().node.lat = stod(val);
        return true;
    }

    if ( name == "lon" ) {
        g_xml_ctx.front().node.lon = stod(val);
        return true;
    }

    if ( name == "ref" ) {
        uint64_t id = stoll (val);
        g_xml_ctx.front ().obj_ref = id;
        return true;
    }

    cout << "Unknown tag: " << name << endl;
    RET_FALSE;
}

bool AddName (OsmObject& xml_info, string t, string k, string v) {

    string str_lang;

    auto spliter = k.find (':');

    if ( spliter == string::npos ) {
        str_lang = g_lex_default_lang;
    } else {
        str_lang = k.substr (spliter + 1, k.size ());
    }

    if ( str_lang.empty () ) {
        str_lang = g_lex_default_lang;
    }

    bool is_found = false;
    auto name = xml_info.names.begin ();

    while ( name != xml_info.names.end () ) {
        if ( name->lang == str_lang ) {
            is_found = true;
            break;
        }
        name++;
    }

    if ( !is_found ) {

        OSM_TEXT_INFO name;

        name.id   = xml_info.node.id;
        name.type = t;
        name.lang = str_lang;
        name.text = v;

        xml_info.names.push_back (name);
    }

    return true;
}

bool MapNodeId (uint64_t id, GEO_COORD& geo_coord) {

    uint64_t  l = 0;
    uint64_t  r = g_nodes_cache_cnt;
    uint64_t  m;
    uint64_t  off;

    OSM_NODE_CACHE cache_item;

    while ( l < r ) {

        m  = (r - l) / 2;
        m += l;

        off = m * sizeof(OSM_NODE_CACHE);

        g_nodes_cache.seekg (off, ios_base::beg);

        g_nodes_cache.read ((char*)&cache_item, sizeof (cache_item));

        if ( cache_item.id == id ) {
            geo_coord.lat = cache_item.lat;
            geo_coord.lon = cache_item.lon;
            return true;
        }

        if ( cache_item.id < id ) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    RET_FALSE;
}

bool UpdateNodeInfo (OsmObject& xml_info) {

    xml_info.node.type = POINT_ID_DEFAULT;

    for ( auto it_param = xml_info.params.begin (); it_param != xml_info.params.end (); it_param++ ) {

        if ( LexCmp (it_param->k, "name") ) {
            AddName (xml_info, "n", it_param->k, it_param->v);
            continue;
        }

        if ( LexCmp (it_param->k, "name:*") ) {
            AddName (xml_info, "n", it_param->k, it_param->v);
            continue;
        } 
        
        for ( auto it_lex = g_lex_list_nodes.begin (); it_lex != g_lex_list_nodes.end (); it_lex++ ) {

            if ( LexCmp (it_param->k, it_lex->k) ) {
                if ( LexCmp (it_param->v, it_lex->v) ) {
                    xml_info.node.type |= it_lex->t;
                    break;
                }
            }
        }

    }

    return true;
}

bool UpdateWayInfo (OsmObject& xml_info) {

    GEO_COORD geo_coord;

    xml_info.way.clear ();

    xml_info.way.id = xml_info.node.id;

    for ( auto it_nodes = xml_info.refs.begin (); it_nodes != xml_info.refs.end (); it_nodes++ ) {

        if ( ! MapNodeId (*it_nodes, geo_coord) ) {
            xml_info.way.nodes.clear ();
            break;
        }
        xml_info.way.nodes.emplace_back (geo_coord);
    }

    for ( auto it_param = xml_info.params.begin (); it_param != xml_info.params.end (); it_param++ ) {

        if ( LexCmp (it_param->k, "name") ) {
            AddName (xml_info, "w", it_param->k, it_param->v);
            continue;
        }

        if ( LexCmp (it_param->k, "name:*") ) {
            AddName (xml_info, "w", it_param->k, it_param->v);
            continue;
        }

        if ( LexCmp (it_param->k, "lanes") ) {
            xml_info.way.lanes = atoll(it_param->v.c_str());
            continue;
        }

        for ( auto it_lex = g_lex_list_ways.begin (); it_lex != g_lex_list_ways.end (); it_lex++ ) {

            if ( LexCmp (it_param->k, it_lex->k) ) {
                if ( LexCmp (it_param->v, it_lex->v) ) {
                    xml_info.way.type |= it_lex->t;
                    break;
                }
            }

        }

    }

    return true;
}

bool StoreNodeInfo (OsmObject& xml_info) {

    g_node_idx_file.write ((char*)&xml_info.node, sizeof (xml_info.node));

    if ( xml_info.node.type != 0 ) {
        if ( xml_info.names.size () > 0 ) {

            stringstream   out;
            OSM_DATA_CACHE name_cache_item;

            for ( auto it = xml_info.names.begin (); it != xml_info.names.end (); it++ ) {

                out.str ("");
                out << it->type << " " << it->id << " " << it->lang << " " << it->text << "\r\n";

                name_cache_item.id = it->id;
                name_cache_item.off = g_offset_names;

                g_offset_names += out.str ().length ();

                g_names_idx_file.write ((char*)&name_cache_item, sizeof (name_cache_item));
                g_names_file.write (out.str ().c_str (), out.str ().length ());

            }
        }
    }

    return true;
}

bool StoreWayInfo (OsmObject& xml_info) {

    stringstream s;

    s << "[POLYLINE]\r\nData=";
    s.precision (7);
    for ( auto it = xml_info.way.nodes.begin (); it != xml_info.way.nodes.end (); it++ ) {
        s << "(" << fixed << it->lat << "," << fixed << it->lon << ") ";
    }
    s << "\r\n";
    s << "Type=" << xml_info.way.type << "\r\n";
    s << "Id="   << xml_info.way.id   << "\r\n";
    s << "[END]\r\n\r\n";

    g_ways_file.write (s.str().c_str(), s.str().length() );

    OSM_DATA_CACHE way_cache_entry;

    way_cache_entry.off = g_offset_ways;
    way_cache_entry.id  = xml_info.way.id;
    g_ways_idx_file.write ( (char*)&way_cache_entry, sizeof(way_cache_entry) );

    g_offset_ways += s.str().length();

    g_ways_file.flush ();
    g_ways_idx_file.flush ();

    return true;
}

bool CloseTag (string name) {

    OsmObject xml_info;

    if ( g_xml_ctx.front ().node_name != name ) {
        cout << "Wrong tag closed: " << g_xml_ctx.front ().node_name << " -> " << name << endl;
        RET_FALSE;
    }

    xml_info = g_xml_ctx.front ();
    g_xml_ctx.pop_front ();

    if ( xml_info.node_name == "osm" ) {
        return true;
    }

    if ( xml_info.node_name == "bounds" ) {
        return true;
    }

    if ( xml_info.node_name == "tag" ) {
        g_xml_ctx.front ().params.push_back (xml_info.tmp);
        return true;
    }

    if ( xml_info.node_name == "node" ) {
        UpdateNodeInfo (xml_info);
        StoreNodeInfo (xml_info);
        return true;
    }

    if ( xml_info.node_name == "nd" ) {
        g_xml_ctx.front ().refs.emplace_back (xml_info.obj_ref);
        return true;
    }

    if ( xml_info.node_name == "way" ) {
        UpdateWayInfo (xml_info);
        StoreWayInfo (xml_info);
        return true;
    }

    RET_FALSE;
}


int main (int argc, char* argv[]) {

    string      tag_name;
    int         offset = 0;
    int         stop   = 0;
    uint64_t    cnt = 0;

    if ( argc != 4 ) {
        cout << "Filtering [config_file_name] [in_file_name] [cache_file_name]" << endl;
        return -1;
    }

    if ( !LoadConfigFile(argv [1]) ) {
        cout << "Cannot load config file name: " << argv [1] << endl;
        return -2;
    }

    g_osm_file_name = argv [2];

    Xml::Inspector<Xml::Encoding::Utf8Writer> inspector (argv [2]);
    Xml::Inspected id;

    while ( inspector.Inspect () ) {
    
        cnt++;
        if ( (cnt % 100000) == 0 ) {
            cout << "\r" << cnt;
        }

        id = inspector.GetInspected ();

        switch ( id ) {

            case Xml::Inspected::StartTag:
                tag_name = inspector.GetName ();
                AddTag (tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount (); i++ ) {
                    ProcessTag (inspector.GetAttributeAt (i).Name, inspector.GetAttributeAt (i).Value);
                }
                break;

            case Xml::Inspected::EndTag:
                tag_name = inspector.GetName ();
                CloseTag (tag_name);
                break;

            case Xml::Inspected::EmptyElementTag:
                tag_name = inspector.GetName ();
                AddTag (tag_name);
                for ( int i = 0; i < inspector.GetAttributesCount (); i++ ) {
                    ProcessTag (inspector.GetAttributeAt (i).Name, inspector.GetAttributeAt (i).Value);
                }
                CloseTag (tag_name);
                break;

            case Xml::Inspected::Text:
                break;

            case Xml::Inspected::Comment:
                break;

            default:
                break;
        }
    }

    return 0;
}
