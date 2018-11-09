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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "config.h"
#include "custom_exceptions.h"
#include "db_tdb.h"
#include "similar.h"

namespace bf = boost::filesystem;
using namespace std;
using boost::format;
using boost::locale::translate;

namespace cnf {

const shared_ptr<Database> getDatabase(const string& id,
                                       const bool readonly,
                                       const string& base_path) {
    return shared_ptr<Database>(new TdbDatabase(id, readonly, base_path));
}

void getCatalogs(const string& database_path, vector<string>& result) {
    TdbDatabase::getCatalogs(database_path, result);
}

void lookup(const string& search_string,
            const string& database_path,
            ResultMap& result,
            vector<string>* const inexact_matches) {
    vector<string> catalogs;
    getCatalogs(database_path, catalogs);

    if (!catalogs.empty()) {
        set<string> terms;
        if (inexact_matches != nullptr) {
            similar_words(search_string, terms);
        }

        for (const auto& catalog : catalogs) {
            vector<Package> packs;

            try {
                if (inexact_matches == nullptr) {
                    getDatabase(catalog, true, database_path)
                        ->getPackages(search_string, packs);
                } else {
                    const shared_ptr<Database>& d =
                        getDatabase(catalog, true, database_path);

                    for (const auto& term : terms) {
                        vector<Package> tempPack;
                        d->getPackages(term, tempPack);
                        if (!tempPack.empty()) {
                            packs.insert(packs.end(), tempPack.begin(),
                                         tempPack.end());
                            inexact_matches->push_back(term);
                        }
                    }
                }

            } catch (const DatabaseException& e) {
                cerr << e.what() << endl;
            }

            if (!packs.empty()) {
                result[catalog.substr(0, catalog.rfind('-'))].insert(
                    packs.begin(), packs.end());
            }
        }
    } else {
        cout << format(translate("WARNING: No database for lookup!")) << endl;
    }
}

void populate_mirror(const bf::path& mirror_path,
                     const string& database_path,
                     const bool truncate,
                     const uint8_t verbosity) {
    using dirIter = bf::directory_iterator;

    static const string architectures[] = {"i686", "x86_64"};

    for (const auto& architecture : architectures) {
        vector<string> catalogs;

        for (dirIter iter = dirIter(mirror_path); iter != dirIter(); ++iter) {
            const string& catalog =
                bf::path(*iter).stem().string() + "-" + architecture;

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

        const bf::path& catalogs_file_name =
            bf::path(database_path) / ("catalogs-" + architecture + "-tdb");

        ofstream catalogs_file;

        catalogs_file.open(catalogs_file_name.c_str(), ios::trunc | ios::out);

        for (const auto& catalog : catalogs) {
            catalogs_file << catalog << ".tdb" << endl;
        }
        catalogs_file.close();
    }
}

void populate(const bf::path& path,
              const string& database_path,
              const string& catalog,
              const bool truncate,
              const uint8_t verbosity) {
    shared_ptr<Database> d;
    try {
        d = getDatabase(catalog, false, database_path);
    } catch (const DatabaseException& e) {
        cerr << e.what() << endl;
        return;
    }

    if (truncate) {
        d->truncate();
    }

    using dirIter = bf::directory_iterator;

    uint32_t count = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter) {
        ++count;
    }

    uint32_t current = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter) {
        if (verbosity > 0) {
            cout << format(translate("[ %d / %d ] %s...")) % ++current % count %
                        *iter;
            cout.flush();
        }
        try {
            Package p(*iter, true);
            d->storePackage(p);
            if (verbosity > 0) {
                cout << translate("done") << endl;
            }
        } catch (const InvalidArgumentException& e) {
            if (verbosity > 0) {
                cout << format(translate("skipping (%s)")) % e.what() << endl;
            }
            continue;
        } catch (const DatabaseException& e) {
            cerr << e.what() << endl;
            return;
        }
    }
}

}  // namespace cnf
