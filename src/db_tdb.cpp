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
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "config.h"
#include "db.h"
#include "db_tdb.h"

namespace fs = std::filesystem;
using boost::format;
using boost::locale::translate;

namespace cnf {

TdbDatabase::TdbDatabase(const std::string& id,
                         const bool readonly,
                         const std::string& base_path)
    : Database(id, readonly, base_path)
    , m_databaseName(m_basePath + "/" + m_id + ".tdb") {
    if (!fs::is_directory(base_path)) {
        std::cout
            << format(translate(
                   "Directory '%s' does not exist. Trying to create it ...")) %
                   base_path
            << '\n';
        try {
            fs::create_directories(base_path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << format(translate(
                             "Could not create database directory: %s: ")) %
                             e.code().message();
            if (!e.path1().empty()) {
                std::cerr << e.path1();
            }
            if (!e.path2().empty()) {
                std::cerr << ", " << e.path2();
            }
            std::cerr << '\n';

            std::string message;
            message += translate("Error opening tdb database: ");
            message += translate("Missing database directory!");
            throw DatabaseException(CONNECT_ERROR, message);
        }
    }

    if (m_readonly) {
        m_tdbFile = tdb_open(m_databaseName.c_str(), 512, 0, O_RDONLY, 0);
    } else {
        m_tdbFile = tdb_open(m_databaseName.c_str(), 512, 0, O_RDWR | O_CREAT,
                             S_IRWXU | S_IRGRP | S_IROTH);
    }

    if (m_tdbFile == nullptr) {
        std::string message;
        message += translate("Error opening tdb database: ");
        message += m_databaseName;
        throw DatabaseException(CONNECT_ERROR, message);
    }
}

TdbDatabase::~TdbDatabase() {
    if (m_tdbFile) {
        tdb_close(m_tdbFile);
    }
    m_tdbFile = nullptr;
}

void TdbDatabase::storePackage(const Package& p) {
    TdbKeyValue kv;

    // check if this package is already indexed
    kv.setKey(p.name() + "-version");
    kv.setValue(tdb_fetch(m_tdbFile, kv.key()));

    if (kv.value_str() == p.version()) {
        kv.setKey(p.name() + "-release");
        kv.setValue(tdb_fetch(m_tdbFile, kv.key()));
        if (kv.value_str() == p.release()) {
            return;
        }
    }

    // OK, we have something new

    int res = 0;

    kv.setKey(p.name() + "-version");
    kv.setValue(p.version());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0) {
        tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
    }

    kv.setKey(p.name() + "-release");
    kv.setValue(p.release());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0) {
        tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
    }

    kv.setKey(p.name() + "-architecture");
    kv.setValue(p.architecture());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0) {
        tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
    }

    kv.setKey(p.name() + "-compression");
    kv.setValue(p.compression());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0) {
        tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
    }

    std::string filesString;
    bool first = true;

    for (const auto& elem : p.files()) {
        TdbKeyValue fkv(elem, p.name());

        const int ret =
            tdb_store(m_tdbFile, fkv.key(), fkv.value(), TDB_INSERT);

        if (ret != 0) {
            fkv.setValue(tdb_fetch(m_tdbFile, fkv.key()));
            std::vector<std::string> others;
            std::istringstream iss(fkv.value_str());
            copy(std::istream_iterator<std::string>(iss),
                 std::istream_iterator<std::string>(),
                 std::back_inserter<std::vector<std::string>>(others));
            others.push_back(p.name());

            auto uIter = unique(others.begin(), others.end());
            others.resize(uIter - others.begin());

            std::string newValue;
            bool isthefirst = true;
            for (auto& other : others) {
                if (isthefirst) {
                    isthefirst = false;
                } else {
                    newValue += " ";
                }
                newValue += other;
            }

            fkv.setValue(newValue);

            tdb_store(m_tdbFile, fkv.key(), fkv.value(), TDB_MODIFY);
        }

        if (first) {
            filesString.append(elem);
            first = false;
        } else {
            filesString.append(" " + elem);
        }
    }

    kv.setKey(p.name() + "-files");
    kv.setValue(filesString);
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0) {
        tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
    }
}

void TdbDatabase::getPackages(const std::string& search,
                              std::vector<Package>& result) const {
    TdbKeyValue name_kv;
    name_kv.setKey(search);
    name_kv.setValue(tdb_fetch(m_tdbFile, name_kv.key()));

    std::vector<std::string> package_names;
    std::istringstream iss(name_kv.value_str());
    copy(std::istream_iterator<std::string>(iss),
         std::istream_iterator<std::string>(),
         std::back_inserter<std::vector<std::string>>(package_names));

    for (auto& package_name : package_names) {
        TdbKeyValue version_kv;
        version_kv.setKey(package_name + "-version");
        version_kv.setValue(tdb_fetch(m_tdbFile, version_kv.key()));

        TdbKeyValue release_kv;
        release_kv.setKey(package_name + "-release");
        release_kv.setValue(tdb_fetch(m_tdbFile, release_kv.key()));

        TdbKeyValue arch_kv;
        arch_kv.setKey(package_name + "-architecture");
        arch_kv.setValue(tdb_fetch(m_tdbFile, arch_kv.key()));

        TdbKeyValue compression_kv;
        compression_kv.setKey(package_name + "-compression");
        compression_kv.setValue(tdb_fetch(m_tdbFile, compression_kv.key()));

        TdbKeyValue files_kv;
        files_kv.setKey(package_name + "-files");
        files_kv.setValue(tdb_fetch(m_tdbFile, files_kv.key()));

        std::istringstream iss(files_kv.value_str());
        std::vector<std::string> files;
        copy(std::istream_iterator<std::string>(iss),
             std::istream_iterator<std::string>(),
             std::back_inserter<std::vector<std::string>>(files));

        Package p(package_name, version_kv.value_str(), release_kv.value_str(),
                  arch_kv.value_str(), compression_kv.value_str(), files);
        result.push_back(p);
    }
}

void TdbDatabase::truncate() {
    if (m_tdbFile) {
        tdb_close(m_tdbFile);
    }

    m_tdbFile =
        tdb_open(m_databaseName.c_str(), 512, 0, O_RDWR | O_CREAT | O_TRUNC,
                 S_IRWXU | S_IRGRP | S_IROTH);

    if (m_tdbFile == nullptr) {
        std::string message;
        message += translate("Error opening tdb database: ");
        message += m_databaseName;
        throw DatabaseException(CONNECT_ERROR, message);
    }
}

void TdbDatabase::getCatalogs(const std::string& database_path,
                              std::vector<std::string>& result) {
    const fs::path p(database_path);

    if (fs::is_directory(p)) {
        using dirIter = fs::directory_iterator;

        for (dirIter iter = dirIter(p); iter != dirIter(); ++iter) {
            fs::path cand(*iter);
            if (cand.extension() == ".tdb" && fs::is_regular_file(cand)) {
                result.push_back(cand.stem().string());
            }
        }
    }
}

TdbKeyValue::TdbKeyValue(const std::string& key, const std::string& value)
    : m_key(TDB_DATA()), m_value(TDB_DATA()) {
    setKey(key);
    setValue(value);
}

TdbKeyValue::TdbKeyValue() : m_key(TDB_DATA()), m_value(TDB_DATA()) {}

void TdbKeyValue::setKey(const std::string& key) {
    if (m_key.dptr != nullptr) {
        free(m_key.dptr);
    }
    m_key = TDB_DATA();
    m_key.dptr = getWritableUCString(key);
    m_key.dsize = key.size() + 1;
}

void TdbKeyValue::setKey(const TDB_DATA& key) {
    if (m_key.dptr != nullptr) {
        free(m_key.dptr);
    }
    m_key = key;
}

void TdbKeyValue::setValue(const std::string& value) {
    if (m_value.dptr != nullptr) {
        free(m_value.dptr);
    }
    m_value = TDB_DATA();
    m_value.dptr = getWritableUCString(value);
    m_value.dsize = value.size() + 1;
}

void TdbKeyValue::setValue(const TDB_DATA& value) {
    if (m_value.dptr != nullptr) {
        free(m_value.dptr);
    }
    m_value = value;
}

TdbKeyValue::~TdbKeyValue() {
    free(m_key.dptr);
    m_key.dptr = nullptr;
    free(m_value.dptr);
    m_value.dptr = nullptr;
}

unsigned char* getWritableUCString(const std::string& aString) {
    auto* result =
        (unsigned char*)malloc((aString.size() + 1) * sizeof(unsigned char));
    copy(aString.begin(), aString.end(), result);
    result[aString.size()] = '\0';
    return result;
}
}  // namespace cnf
