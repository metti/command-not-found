/*
    This file is part of command-not-found.
    Copyright (C) 2011 Matthias Maennich <matthias@maennich.net>

    command-not-found is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    command-not-found is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with command-not-found.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>

namespace cnf {

extern const std::string PROGRAM_NAME;
extern const std::string PROGRAM_LONG_NAME;
extern const std::string PROGRAM_AUTHOR;

extern const int VERSION_MAJOR;
extern const int VERSION_MINOR;
extern const int VERSION_PATCH;
extern const std::string VERSION_SHORT;
extern const std::string VERSION_HASH;
extern const std::string VERSION_LONG;
extern const std::string VERSION_REFSPEC;

extern const std::string DATABASE_PATH;
extern const std::string LC_MESSAGE_PATH;

}  // namespace cnf

#endif  // CONFIG_H_
