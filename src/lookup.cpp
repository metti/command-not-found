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
    int verbosity;
    string search_string;
} args;

static const char* OPT_STRING = "d:vh?";

static const struct option LONG_OPTS[] = {
    {"database-path", required_argument, NULL, 'd'},
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
         << endl;
    exit(1);
}

int theMain(int argc, char** argv) {

    args.database_path = DATABASE_PATH;
    args.verbosity = 0;
    args.search_string = ""; //actually done implicit

    int opt(0), long_index(0);

    opt = getopt_long(argc, argv, OPT_STRING, LONG_OPTS, &long_index);
    while (opt != -1) {
        switch (opt) {
            case 'd':
                args.database_path = optarg;
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

    const map<string, set<Package> > result = lookup(args.search_string,
                                                        args.database_path);

    typedef map<string, set<Package> >::const_iterator catIter;
    typedef set<Package>::const_iterator packIter;

    stringstream out;
    bool match = false;

    for (catIter oiter = result.begin(); oiter != result.end(); ++oiter) {
        out << "[" << oiter->first << "]" << endl;
        for (packIter piter = oiter->second.begin();
                piter != oiter->second.end(); ++piter) {
            out << piter->hl_str(args.search_string, "\t") << endl;
            match = true;
        }
    }

    if (match) {
        cout << "The command '" << args.search_string
             << "' is been provided by the following packages:" << endl;
        cout << out.str();
        return 0;

    } else {
        boost::shared_ptr<vector<string> > matches(new vector<string>());
        const map<string, set<Package> > inexactResult = lookup(args.search_string,
                                                            args.database_path,
                                                            matches.get());

        for (catIter oiter = inexactResult.begin(); oiter != inexactResult.end(); ++oiter) {
            out << "[" << oiter->first << "]" << endl;

            for (packIter piter = oiter->second.begin();
                          piter != oiter->second.end(); ++piter) {
                out << piter->hl_str(matches.get(), "\t") << endl;
                match = true;
            }
        }
        if (match){
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

