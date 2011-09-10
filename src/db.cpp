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

#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "db.h"
#include "custom_exceptions.h"
#include "db_gdbm.h"

namespace bf = boost::filesystem;
using namespace std;
using boost::shared_ptr;

namespace cnf {

const shared_ptr<Database> getDatabase(const string id, const bool readonly,
                                       const string base_path)
                                       throw (DatabaseException) {
    return shared_ptr<Database>(new GdbmDatabase(id, readonly, base_path));
}

const vector<string> getCatalogs(const string database_path) throw (DatabaseException) {
    return GdbmDatabase::getCatalogs(database_path);
}

const map<string, vector<Package> > lookup(const string searchString,
                                           const string database_path) {

    const vector<string>& catalogs = getCatalogs(database_path);

    map<string, vector<Package> > result;

    if (catalogs.size() > 0) {

        typedef vector<string>::const_iterator catIter;
        for (catIter iter = catalogs.begin(); iter != catalogs.end(); iter++) {
            vector<Package> packs =
                    getDatabase(*iter, true, database_path)->getPackages(
                            searchString);
            if (packs.size() > 0)
                result[*iter] = packs;
        }
    } else {
        cout << "WARNING: No database for lookup!" << endl;
    }
    return result;
}

void populate_mirror(const bf::path mirror_path,
                     const string database_path,
                     const bool truncate,
                     const int verbosity) {
    typedef bf::directory_iterator dirIter;

    static const string architectures[] = { "i686", "x86_64" };

    for (unsigned int i = 0; i < sizeof(architectures) / sizeof(string); ++i) {

        const string& architecture = architectures[i];

        vector<string> catalogs;

        for (dirIter iter = dirIter(mirror_path); iter != dirIter(); ++iter) {

#if BOOST_FILESYSTEM_VERSION == 2
            string catalog = bf::path(*iter).stem().c_str();
            catalog.append("-"+architecture);
#else
            const string catalog = bf::path(*iter).stem().string() + "-"
                    + architecture;
#endif
            bool truncated = !truncate;

            bool list_catalog = false;

            bf::path dir = bf::path(*iter) / "os" / architecture;
            if (bf::is_directory(dir)) {
                populate(dir, database_path, catalog, !truncated, verbosity);
                truncated = true;
                list_catalog = true;
            }

            dir = bf::path(*iter) / "os" / "any";
            if (bf::is_directory(dir)) {
                populate(dir, database_path, catalog, !truncated, verbosity);
                truncated = true;
                list_catalog = true;
            }
            if (list_catalog) {
                catalogs.push_back(catalog);
            }
        }

        bf::path catalogs_file_name = bf::path(database_path)
                / ("catalogs-" + architecture);
        ofstream catalogs_file;
#if BOOST_FILESYSTEM_VERSION == 2
        catalogs_file.open(catalogs_file_name.string().c_str(), ios::trunc | ios::out);
#else
        catalogs_file.open(catalogs_file_name.c_str(), ios::trunc | ios::out);
#endif
        for (vector<string>::iterator iter = catalogs.begin();
                iter != catalogs.end(); ++iter) {
            catalogs_file << *iter << ".db" << endl;
        }
        catalogs_file.close();
    }
}

void populate(const bf::path path,
              const string database_path,
              const string catalog,
              const bool truncate,
              const int verbosity) {

    shared_ptr<Database> d;
    try {
        d = getDatabase(catalog, false, database_path);
    } catch (DatabaseException& e) {
        cerr << e.what() << endl;
        return;
    }

    if (truncate)
        d->truncate();

    typedef bf::directory_iterator dirIter;

    int count = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter)
        ++count;

    int current = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter) {
        if (verbosity > 0) {
            cout << "[ " << ++current << " / " << count << " ] " << *iter << "...";
            cout.flush();
        }
        try {
            Package p(*iter, true);
            d->storePackage(p);
            if (verbosity > 0) {
                cout << "done" << endl;
            }
        } catch (InvalidArgumentException& e) {
            if (verbosity > 0) {
                cout << "skipping (" << e.what() << ")" << endl;
            }
            continue;
        } catch (DatabaseException &e) {
            cerr << e.what() << endl;
            return;
        }
    }

}

}
