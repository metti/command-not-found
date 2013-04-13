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

#include <getopt.h>
#include <exception>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include <set>

#include "guard.h"
#include "package.h"
#include "db.h"
#include "config.h"

using namespace cnf;
using namespace std;

static struct args_t {
    string database_path;
    bool colors;
    int verbosity;
    string search_string;
} args;

static const char* OPT_STRING = "d:cvh?";

static const struct option LONG_OPTS[] = {
    {"database-path", required_argument, NULL, 'd'},
    {"colors", no_argument, NULL, 'c'},
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}
};

void usage(){
    cout << "       *** " << PROGRAM_NAME << " " << VERSION_LONG << " ***       \n"
            "Usage:                                                             \n"
            "   cnf-lookup [ -d ] <search term>                                 \n"
            "                                                                   \n"
            "Options:                                                           \n"
            " --help            -? -h     Show this help and exit               \n"
            " --verbose         -v        Display verbose output                \n"
            "                                                                   \n"
            " --database-path   -d        Customize the database lookup path    \n"
            "                             default is " << DATABASE_PATH << "    \n"
            " --colors          -c        Pretty colored output                 \n"
         << endl;
    exit(1);
}

int theMain(int argc, char** argv) {

    args.database_path = DATABASE_PATH;
    args.colors = false;
    args.verbosity = 0;
    args.search_string = ""; //actually done implicit

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

    for (auto oiter = result.begin(); oiter != result.end(); ++oiter) {
        for (auto piter = oiter->second.begin();
                piter != oiter->second.end(); ++piter) {
            if (args.colors){
                out << "\33[1m" << piter->name() << "\033[0m";
            } else {
                out << piter->name();
            }
            out << " (" << piter->version() << "-" << piter->release() << ")" 
                << " from " << oiter->first << endl;
            if (args.colors) {
                out << piter->hl_str(args.search_string, "\t", "\033[0;31m") << endl;
            } else {
                out << piter->hl_str(args.search_string, "\t", "") << endl;
            }
        }
    }

    if (!result.empty()) {
        cout << "The command '" << args.search_string
             << "' is provided by the following packages:" << endl;
        cout << out.str();
        return 0;

    } else {
        std::shared_ptr<vector<string> > matches(new vector<string>());
        ResultMap inexactResult;
        lookup(args.search_string, args.database_path, inexactResult, matches.get());

        for (auto oiter = inexactResult.begin(); oiter != inexactResult.end(); ++oiter) {
            for (auto piter = oiter->second.begin();
                          piter != oiter->second.end(); ++piter) {
                if (args.colors) {
                    out << "\033[1m" << piter->name() << "\033[0m";
                } else {
                    out << piter->name();
                }
                out << " (" << piter->version() << "-" << piter->release() << ")" 
                    << " from " << oiter->first << endl;
                if (args.colors) {
                    out << piter->hl_str(matches.get(), "\t", "\033[0;31m") << endl;
                } else {
                    out << piter->hl_str(matches.get(), "\t", "") << endl;
                }
            }
        }
        if (!inexactResult.empty()){
            cout << "A similar command to '" << args.search_string
                 << "' is been provided by the following packages:" << endl;
            cout << out.str();
            return 0;
        }
    }

    return 1;
}

int main(int argc, char** argv) {
    int ret = 0;
#ifndef DEBUG
    install_handler();
    try {
#endif /* DEBUG */
        ret = theMain(argc, argv);
#ifndef DEBUG
    } catch (const exception& e) {
        cerr << "An uncaught exception occured:" << endl;
        cerr << "    " << e.what() << endl;

        ret = 1;
    }
#endif /* DEBUG */
    return ret;
}

