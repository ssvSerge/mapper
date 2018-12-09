#ifndef COMMAND_HELP_HPP
#define COMMAND_HELP_HPP

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

#include "cmd.hpp" // IWYU pragma: export

#include <string>
#include <vector>

class CommandHelp : public Command {

    std::string m_topic;

public:

    explicit CommandHelp(const CommandFactory& command_factory) :
        Command(command_factory) {
    }

    bool setup(const std::vector<std::string>& arguments) override final;

    bool run() override final;

    const char* name() const noexcept override final {
        return "help";
    }

    const char* synopsis() const noexcept override final {
        return "osmium COMMAND [ARG...]\n"
               "       osmium --version";
    }

}; // class CommandHelp


#endif // COMMAND_HELP_HPP
