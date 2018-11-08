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

#include <exception>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <getopt.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "config.h"
#include "db.h"
#include "package.h"

using namespace cnf;
using namespace std;
using boost::format;
using boost::locale::translate;

static struct args_t {
    string database_path;
    bool colors;
    int verbosity;
    string search_string;
} args;

static const char* OPT_STRING = "d:cvh?";

static const struct option LONG_OPTS[] = {
    {"database-path", required_argument, nullptr, 'd'},
    {"colors", no_argument, nullptr, 'c'},
    {"verbose", no_argument, nullptr, 'v'},
    {"help", no_argument, nullptr, 'h'},
    {nullptr, no_argument, nullptr, 0}};

void usage() {
    cout << format(translate("       *** %s %s ***                             "
                             "              \n")) %
                PROGRAM_NAME % VERSION_LONG
         << translate(
                "Usage:                                                        "
                " \n")
         << translate(
                "   cnf-lookup [ -d ] <search term>                            "
                " \n")
         << translate(
                "                                                              "
                " \n")
         << translate(
                "Options:                                                      "
                " \n")
         << translate(
                " --help            -? -h     Show this help and exit          "
                " \n")
         << translate(
                " --verbose         -v        Display verbose output           "
                " \n")
         << translate(
                "                                                              "
                " \n")
         << format(translate(" --database-path   -d        Customize the "
                             "database lookup path\n"
                             "                             default is %s       "
                             "              \n")) %
                DATABASE_PATH
         << translate(
                " --colors          -c        Pretty colored output            "
                " \n")
         << endl;
    exit(1);
}

int main(int argc, char** argv) {
    boost::locale::generator gen;
    gen.add_messages_path(LC_MESSAGE_PATH);
    gen.add_messages_domain(PROGRAM_NAME);
    locale::global(gen(""));
    cout.imbue(locale());

    args.database_path = DATABASE_PATH;
    args.colors = false;
    args.verbosity = 0;
    args.search_string = "";  // actually done implicit

    int opt(0), long_index(0);

    opt = getopt_long(argc, argv, OPT_STRING, LONG_OPTS, &long_index);
    while (opt != -1) {
        switch (opt) {
            case 'd':
                args.database_path = optarg;
                break;
            case 'c':
                args.colors = true;
                break;
            case 'v':
                args.verbosity++;
                break;
            case 'h':
            case '?':
                usage();
                break;
            default:
                break;
        }
        opt = getopt_long(argc, argv, OPT_STRING, LONG_OPTS, &long_index);
    }

    if (argc - optind != 1) {
        usage();
    }

    args.search_string = argv[optind];

    ResultMap result;

    lookup(args.search_string, args.database_path, result);

    stringstream out;

    for (auto& elem : result) {
        for (auto& piter : elem.second) {
            if (args.colors) {
                out << "\33[1m" << piter.name() << "\033[0m";
            } else {
                out << piter.name();
            }
            out << format(translate(" (%s-%s) from %s")) % piter.version() %
                       piter.release() % elem.first
                << endl;
            if (args.colors) {
                out << piter.hl_str(args.search_string, "\t", "\033[0;31m")
                    << endl;
            } else {
                out << piter.hl_str(args.search_string, "\t", "") << endl;
            }
        }
    }

    if (!result.empty()) {
        cout
            << format(translate(
                   "The command '%s' is provided by the following packages:")) %
                   args.search_string
            << endl;
        cout << out.str();
        return 0;

    } else {
        std::shared_ptr<vector<string>> matches(new vector<string>());
        ResultMap inexactResult;
        lookup(args.search_string, args.database_path, inexactResult,
               matches.get());

        for (auto& elem : inexactResult) {
            for (auto& piter : elem.second) {
                if (args.colors) {
                    out << "\033[1m" << piter.name() << "\033[0m";
                } else {
                    out << piter.name();
                }
                out << format(translate(" (%s-%s) from %s")) % piter.version() %
                           piter.release() % elem.first
                    << endl;
                if (args.colors) {
                    out << piter.hl_str(matches.get(), "\t", "\033[0;31m")
                        << endl;
                } else {
                    out << piter.hl_str(matches.get(), "\t", "") << endl;
                }
            }
        }
        if (!inexactResult.empty()) {
            cout << format(translate("A similar command to '%s' is provided by "
                                     "the following packages:")) %
                        args.search_string
                 << endl;
            cout << out.str();
            return 0;
        }
    }

    return 1;
}
