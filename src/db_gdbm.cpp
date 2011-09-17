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

#include <gdbm.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "db.h"
#include "db_gdbm.h"
#include "config.h"

namespace bf = boost::filesystem;
using namespace std;
using boost::shared_ptr;

namespace cnf {

GdbmDatabase::GdbmDatabase(const string& id,
                           const bool readonly,
                           const string& base_path) throw (DatabaseException)
        : Database(id, readonly, base_path),
                itsDatabaseName(itsBasePath + "/" + itsId + ".db") {

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
            message += "Error opening gdbm database: ";
            message += "Missing database directory!";
            throw DatabaseException(CONNECT_ERROR, message.c_str());
        }
    }

    boost::shared_ptr<char> dbName(getWritableCString(itsDatabaseName), free);

    if (isReadonly) {
        itsGdbmFile = gdbm_open(dbName.get(), 512, GDBM_READER | GDBM_SYNC, 0644, NULL);
    } else {
        itsGdbmFile = gdbm_open(dbName.get(), 512, GDBM_WRCREAT | GDBM_SYNC, 0644, NULL);
    }

    if (itsGdbmFile == NULL) {
        string message;
        message += "Error opening gdbm database: ";
        message += gdbm_strerror(gdbm_errno);
        message += ": ";
        message += itsDatabaseName;
        throw DatabaseException(CONNECT_ERROR, message.c_str());
    }
}

GdbmDatabase::~GdbmDatabase() {
    if (itsGdbmFile) {
        gdbm_close(itsGdbmFile);
    }
}

void GdbmDatabase::storePackage(const Package& p) throw (DatabaseException) {
    typedef vector<string>::const_iterator fileIter;

    KeyValue kv;

    // check if this package is already indexed
    kv.setKey(p.name() + "-version");
    kv.setValue(gdbm_fetch(itsGdbmFile, kv.key()));

    if (kv.value_str() == p.version()) {
        kv.setKey(p.name() + "-release");
        kv.setValue(gdbm_fetch(itsGdbmFile, kv.key()));
        if (kv.value_str() == p.release()) {
            return;
        }
    }

    // OK, we have something new

    kv.setKey(p.name() + "-version");
    kv.setValue(p.version());
    gdbm_store(itsGdbmFile, kv.key(), kv.value(), GDBM_REPLACE);

    kv.setKey(p.name() + "-release");
    kv.setValue(p.release());
    gdbm_store(itsGdbmFile, kv.key(), kv.value(), GDBM_REPLACE);

    kv.setKey(p.name() + "-architecture");
    kv.setValue(p.architecture());
    gdbm_store(itsGdbmFile, kv.key(), kv.value(), GDBM_REPLACE);

    kv.setKey(p.name() + "-compression");
    kv.setValue(p.compression());
    gdbm_store(itsGdbmFile, kv.key(), kv.value(), GDBM_REPLACE);

    string filesString;
    bool first = true;

    for (fileIter iter = p.files().begin(); iter != p.files().end(); ++iter) {

        KeyValue fkv(*iter, p.name());

        const int ret = gdbm_store(itsGdbmFile, fkv.key(), fkv.value(),
                                   GDBM_INSERT);

        if (ret == 1) {
            fkv.setValue(gdbm_fetch(itsGdbmFile, fkv.key()));
            vector<string> others;
            istringstream iss(fkv.value_str());
            copy(istream_iterator<string>(iss), istream_iterator<string>(),
                 back_inserter<vector<string> >(others));
            others.push_back(p.name());

            typedef vector<string>::iterator strIter;
            strIter uIter = unique(others.begin(), others.end());
            others.resize(uIter - others.begin());

            string newValue;
            bool isthefirst = true;
            for (strIter iter = others.begin(); iter != others.end(); ++iter) {
                if (isthefirst) {
                    isthefirst = false;
                } else {
                    newValue += " ";
                }
                newValue += *iter;
            }

            fkv.setValue(newValue);

            gdbm_store(itsGdbmFile, fkv.key(), fkv.value(), GDBM_REPLACE);
        }

        if (first) {
            filesString.append(*iter);
            first = false;
        } else {
            filesString.append(" " + *iter);
        }
    }

    kv.setKey(p.name() + "-files");
    kv.setValue(filesString);
    gdbm_store(itsGdbmFile, kv.key(), kv.value(), GDBM_REPLACE);
}

const vector<Package> GdbmDatabase::getPackages(const string& search) const
        throw (DatabaseException) {

    vector<Package> result;

    KeyValue name_kv;
    name_kv.setKey(search);
    name_kv.setValue(gdbm_fetch(itsGdbmFile, name_kv.key()));

    vector<string> package_names;
    istringstream iss(name_kv.value_str());
    copy(istream_iterator<string>(iss), istream_iterator<string>(),
         back_inserter<vector<string> >(package_names));

    typedef vector<string>::iterator nameIter;

    for (nameIter iter = package_names.begin(); iter != package_names.end();
            ++iter) {
        KeyValue version_kv;
        version_kv.setKey(*iter + "-version");
        version_kv.setValue(gdbm_fetch(itsGdbmFile, version_kv.key()));

        KeyValue release_kv;
        release_kv.setKey(*iter + "-release");
        release_kv.setValue(gdbm_fetch(itsGdbmFile, release_kv.key()));

        KeyValue arch_kv;
        arch_kv.setKey(*iter + "-architecture");
        arch_kv.setValue(gdbm_fetch(itsGdbmFile, arch_kv.key()));

        KeyValue compression_kv;
        compression_kv.setKey(*iter + "-compression");
        compression_kv.setValue(gdbm_fetch(itsGdbmFile, compression_kv.key()));

        KeyValue files_kv;
        files_kv.setKey(*iter + "-files");
        files_kv.setValue(gdbm_fetch(itsGdbmFile, files_kv.key()));

        istringstream iss(files_kv.value_str());
        vector<string> files;
        copy(istream_iterator<string>(iss), istream_iterator<string>(),
             back_inserter<vector<string> >(files));

        Package p(*iter, version_kv.value_str(), release_kv.value_str(),
                  arch_kv.value_str(), compression_kv.value_str(), files);
        result.push_back(p);
    }

    return result;

}

void GdbmDatabase::truncate() throw (DatabaseException) {

    if (itsGdbmFile) {
        gdbm_close(itsGdbmFile);
    }
    boost::shared_ptr<char> dbName(getWritableCString(itsDatabaseName), free);

    itsGdbmFile = gdbm_open(dbName.get(), 512, GDBM_NEWDB | GDBM_SYNC, 0644, NULL);

    if (itsGdbmFile == NULL) {
        string message;
        message += "Error opening gdbm database: ";
        message += gdbm_strerror(gdbm_errno);
        message += ": ";
        message += itsDatabaseName;
        throw DatabaseException(CONNECT_ERROR, message.c_str());
    }
}

const vector<string> GdbmDatabase::getCatalogs(const string& database_path)
        throw (DatabaseException) {

    bf::path p(database_path);

    vector<string> result;

    if (bf::is_directory(p)) {

        typedef bf::directory_iterator dirIter;

        for (dirIter iter = dirIter(p); iter != dirIter(); ++iter) {
            bf::path cand(*iter);
            if (cand.extension() == ".db" && bf::is_regular_file(cand)) {
#if BOOST_FILESYSTEM_VERSION == 2
                result.push_back(cand.stem().c_str());
#else
                result.push_back(cand.stem().string());
#endif
            }
        }
    }

    return result;
}

KeyValue::KeyValue(const string& key, const string& value)
        : itsKey(datum()), itsValue(datum()) {
    setKey(key);
    setValue(value);
}

KeyValue::KeyValue()
        : itsKey(datum()), itsValue(datum()) {
}

void KeyValue::setKey(const string& key) {
    if (itsKey.dptr != NULL) {
        free(itsKey.dptr);
    }
    itsKey = datum();
    itsKey.dptr = getWritableCString(key);
    itsKey.dsize = key.size() + 1;
}

void KeyValue::setKey(const datum& key) {
    if (itsKey.dptr != NULL) {
        free(itsKey.dptr);
    }
    itsKey = key;
}

void KeyValue::setValue(const string& value) {
    if (itsValue.dptr != NULL) {
        free(itsValue.dptr);
    }
    itsValue = datum();
    itsValue.dptr = getWritableCString(value);
    itsValue.dsize = value.size() + 1;
}

void KeyValue::setValue(const datum& value) {
    if (itsValue.dptr != NULL) {
        free(itsValue.dptr);
    }
    itsValue = value;
}

KeyValue::~KeyValue() {
    free(itsKey.dptr);
    itsKey.dptr = NULL;
    free(itsValue.dptr);
    itsValue.dptr = NULL;
}

char* getWritableCString(const string& aString) {
    char* result = (char*) malloc((aString.size() + 1) * sizeof(char));
    copy(aString.begin(), aString.end(), result);
    result[aString.size()] = '\0';
    return result;
}
}
