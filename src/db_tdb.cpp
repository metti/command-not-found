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

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "db.h"
#include "db_tdb.h"
#include "config.h"

namespace bf = boost::filesystem;
using namespace std;

namespace cnf {

TdbDatabase::TdbDatabase(const string& id,
                         const bool readonly,
                         const string& base_path)
        : Database(id, readonly, base_path)
        , m_databaseName(m_basePath + "/" + m_id + ".tdb") {

    if (!bf::is_directory(base_path)) {
        cout << "Directory '" << base_path << "' does not exist. "
             << "Trying to create it ..." << endl;
        try {
            bf::create_directories(base_path);
        } catch (const bf::filesystem_error& e) {
            cerr << "Could not create database directory: "
                    << e.code().message() << ": ";
            if (!e.path1().empty()) {
                cerr << e.path1();
            }
            if (!e.path2().empty()) {
                cerr << ", " << e.path2();
            }
            cerr << endl;

            string message;
            message += "Error opening tdb database: ";
            message += "Missing database directory!";
            throw DatabaseException(CONNECT_ERROR, message.c_str());
        }
    }

    if (m_readonly) {
        m_tdbFile = tdb_open(m_databaseName.c_str(), 512, 0, O_RDONLY, 0);
    } else {
        m_tdbFile = tdb_open(m_databaseName.c_str(), 512, 0, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
    }

    if (m_tdbFile == NULL) {
        string message;
        message += "Error opening tdb database: ";
        message += m_databaseName;
        throw DatabaseException(CONNECT_ERROR, message.c_str());
    }
}

TdbDatabase::~TdbDatabase() {
    if (m_tdbFile) {
        tdb_close(m_tdbFile);
    }
    m_tdbFile = NULL;
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
    if (res != 0 ) tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);

    kv.setKey(p.name() + "-release");
    kv.setValue(p.release());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0 ) tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);

    kv.setKey(p.name() + "-architecture");
    kv.setValue(p.architecture());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0 ) tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);

    kv.setKey(p.name() + "-compression");
    kv.setValue(p.compression());
    res = tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_MODIFY);
    if (res != 0 ) tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);

    string filesString;
    bool first = true;

    for (const auto& elem : p.files()) {

        TdbKeyValue fkv(elem, p.name());

        const int ret = tdb_store(m_tdbFile, fkv.key(), fkv.value(),
                                   TDB_INSERT);

        if (ret != 0) {
            fkv.setValue(tdb_fetch(m_tdbFile, fkv.key()));
            vector<string> others;
            istringstream iss(fkv.value_str());
            copy(istream_iterator<string>(iss), istream_iterator<string>(),
                 back_inserter<vector<string> >(others));
            others.push_back(p.name());

            auto uIter = unique(others.begin(), others.end());
            others.resize(uIter - others.begin());

            string newValue;
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
    if (res != 0 ) tdb_store(m_tdbFile, kv.key(), kv.value(), TDB_INSERT);
}

void TdbDatabase::getPackages(const string& search, vector<Package>& result) const {
    TdbKeyValue name_kv;
    name_kv.setKey(search);
    name_kv.setValue(tdb_fetch(m_tdbFile, name_kv.key()));

    vector<string> package_names;
    istringstream iss(name_kv.value_str());
    copy(istream_iterator<string>(iss), istream_iterator<string>(),
         back_inserter<vector<string> >(package_names));

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

        istringstream iss(files_kv.value_str());
        vector<string> files;
        copy(istream_iterator<string>(iss), istream_iterator<string>(),
             back_inserter<vector<string> >(files));

        Package p(package_name, version_kv.value_str(), release_kv.value_str(),
                  arch_kv.value_str(), compression_kv.value_str(), files);
        result.push_back(p);
    }
}

void TdbDatabase::truncate() {

    if (m_tdbFile) {
        tdb_close(m_tdbFile);
    }

    m_tdbFile = tdb_open(m_databaseName.c_str(), 512, 0, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);


    if (m_tdbFile == NULL) {
        string message;
        message += "Error opening tdb database: ";
        message += m_databaseName;
        throw DatabaseException(CONNECT_ERROR, message.c_str());
    }
}

void TdbDatabase::getCatalogs(const string& database_path, vector<string>& result) {
    const bf::path p(database_path);

    if (bf::is_directory(p)) {

        typedef bf::directory_iterator dirIter;

        for (dirIter iter = dirIter(p); iter != dirIter(); ++iter) {
            bf::path cand(*iter);
            if (cand.extension() == ".tdb" && bf::is_regular_file(cand)) {
                result.push_back(cand.stem().string());
            }
        }
    }
}

TdbKeyValue::TdbKeyValue(const string& key, const string& value)
        : m_key(TDB_DATA()), m_value(TDB_DATA()) {
    setKey(key);
    setValue(value);
}

TdbKeyValue::TdbKeyValue()
        : m_key(TDB_DATA()), m_value(TDB_DATA()) {
}

void TdbKeyValue::setKey(const string& key) {
    if (m_key.dptr != NULL) {
        free(m_key.dptr);
    }
    m_key = TDB_DATA();
    m_key.dptr = getWritableUCString(key);
    m_key.dsize = key.size() + 1;
}

void TdbKeyValue::setKey(const TDB_DATA& key) {
    if (m_key.dptr != NULL) {
        free(m_key.dptr);
    }
    m_key = key;
}

void TdbKeyValue::setValue(const string& value) {
    if (m_value.dptr != NULL) {
        free(m_value.dptr);
    }
    m_value = TDB_DATA();
    m_value.dptr = getWritableUCString(value);
    m_value.dsize = value.size() + 1;
}

void TdbKeyValue::setValue(const TDB_DATA& value) {
    if (m_value.dptr != NULL) {
        free(m_value.dptr);
    }
    m_value = value;
}

TdbKeyValue::~TdbKeyValue() {
    free(m_key.dptr);
    m_key.dptr = NULL;
    free(m_value.dptr);
    m_value.dptr = NULL;
}

unsigned char* getWritableUCString(const string& aString) {
    unsigned char* result = (unsigned char*) malloc((aString.size() + 1) * sizeof(unsigned char));
    copy(aString.begin(), aString.end(), result);
    result[aString.size()] = '\0';
    return result;
}
}
