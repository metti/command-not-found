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
#include <filesystem>
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

namespace fs = std::filesystem;
using boost::format;
using boost::locale::translate;

namespace cnf {

std::unique_ptr<Database> getDatabase(const std::string& id,
                                      const bool readonly,
                                      const std::string& base_path) {
    return std::make_unique<TdbDatabase>(id, readonly, base_path);
}

void getCatalogs(const std::string& database_path,
                 std::vector<std::string>& result) {
    TdbDatabase::getCatalogs(database_path, result);
}

void lookup(const std::string& search_string,
            const std::string& database_path,
            ResultMap& result,
            std::vector<std::string>* const inexact_matches) {
    std::vector<std::string> catalogs;
    getCatalogs(database_path, catalogs);

    if (!catalogs.empty()) {
        std::vector<std::string> terms;
        if (inexact_matches) {
            terms = similar_words(search_string);
        }

        for (const auto& catalog : catalogs) {
            std::vector<Package> packs;

            try {
                auto d = getDatabase(catalog, true, database_path);
                if (inexact_matches == nullptr) {
                    d->getPackages(search_string, packs);
                } else {
                    for (const auto& term : terms) {
                        std::vector<Package> tempPack;
                        d->getPackages(term, tempPack);
                        if (!tempPack.empty()) {
                            packs.insert(packs.end(), tempPack.begin(),
                                         tempPack.end());
                            inexact_matches->push_back(term);
                        }
                    }
                }

            } catch (const DatabaseException& e) {
                std::cerr << e.what() << '\n';
            }

            if (!packs.empty()) {
                result[catalog.substr(0, catalog.rfind('-'))].insert(
                    packs.begin(), packs.end());
            }
        }
    } else {
        std::cout << format(translate("WARNING: No database for lookup!"))
                  << '\n';
    }
}

void populate_mirror(const fs::path& mirror_path,
                     const std::string& database_path,
                     const bool truncate,
                     const uint8_t verbosity) {
    using dirIter = fs::directory_iterator;

    static const std::string architectures[] = {"i686", "x86_64"};

    for (const auto& architecture : architectures) {
        std::vector<std::string> catalogs;

        for (dirIter iter = dirIter(mirror_path); iter != dirIter(); ++iter) {
            const std::string& catalog =
                fs::path(*iter).stem().string() + "-" + architecture;

            bool truncated = !truncate;

            bool list_catalog = false;

            fs::path dir = fs::path(*iter) / "os" / architecture;
            if (fs::is_directory(dir)) {
                populate(dir, database_path, catalog, !truncated, verbosity);
                truncated = true;
                list_catalog = true;
            }

            dir = fs::path(*iter) / "os" / "any";
            if (fs::is_directory(dir)) {
                populate(dir, database_path, catalog, !truncated, verbosity);
                list_catalog = true;
            }
            if (list_catalog) {
                catalogs.push_back(catalog);
            }
        }

        const fs::path& catalogs_file_name =
            fs::path(database_path) / ("catalogs-" + architecture + "-tdb");

        std::ofstream catalogs_file;

        catalogs_file.open(catalogs_file_name.c_str(),
                           std::ios::trunc | std::ios::out);

        for (const auto& catalog : catalogs) {
            catalogs_file << catalog << ".tdb" << '\n';
        }
        catalogs_file.close();
    }
}

void populate(const fs::path& path,
              const std::string& database_path,
              const std::string& catalog,
              const bool truncate,
              const uint8_t verbosity) {
    std::unique_ptr<Database> d;
    try {
        d = getDatabase(catalog, false, database_path);
    } catch (const DatabaseException& e) {
        std::cerr << e.what() << '\n';
        return;
    }

    if (truncate) {
        d->truncate();
    }

    using dirIter = fs::directory_iterator;

    uint32_t count = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter) {
        ++count;
    }

    uint32_t current = 0;

    for (dirIter iter = dirIter(path); iter != dirIter(); ++iter) {
        if (verbosity > 0) {
            std::cout << format(translate("[ %d / %d ] %s...")) % ++current %
                             count % *iter;
            std::cout.flush();
        }
        try {
            Package p(*iter, true);
            d->storePackage(p);
            if (verbosity > 0) {
                std::cout << translate("done") << '\n';
            }
        } catch (const InvalidArgumentException& e) {
            if (verbosity > 0) {
                std::cout << format(translate("skipping (%s)")) % e.what()
                          << '\n';
            }
            continue;
        } catch (const DatabaseException& e) {
            std::cerr << e.what() << '\n';
            return;
        }
    }
}

}  // namespace cnf
