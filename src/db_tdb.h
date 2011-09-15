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

#ifndef TDB_H_
#define TDB_H_

#include <fcntl.h>
#include <tdb.h>
#include <string>
#include <vector>

#include "db.h"
#include "custom_exceptions.h"

namespace cnf {
class TdbDatabase: public Database {
public:
    explicit TdbDatabase(const std::string& id,
                          const bool readonly,
                          const std::string& basepath) throw (DatabaseException);
    virtual void storePackage(const Package& p) throw (DatabaseException);
    virtual const std::vector<Package> getPackages(const std::string& search) const
            throw (DatabaseException);
    virtual void truncate() throw (DatabaseException);
    virtual ~TdbDatabase();
    static const std::vector<std::string> getCatalogs(const std::string& database_path)
            throw (DatabaseException);
private:
    TDB_CONTEXT* itsTdbFile;
    std::string itsDatabaseName;
};

class TdbKeyValue {
public:
    explicit TdbKeyValue(const std::string& key, const std::string& value);
    explicit TdbKeyValue();
    void setKey(const std::string& key);
    void setKey(const TDB_DATA& key);
    void setValue(const std::string& key);
    void setValue(const TDB_DATA& key);
    ~TdbKeyValue();
    TDB_DATA key() const {
        return itsKey;
    }
    TDB_DATA value() const {
        return itsValue;
    }
    std::string key_str() const {
        if (itsKey.dptr) {
            return std::string(reinterpret_cast<const char*>(itsKey.dptr));
        } else {
            return std::string();
        }
    }
    std::string value_str() const {
        if (itsValue.dptr) {
            return std::string(reinterpret_cast<const char*>(itsValue.dptr));
        } else {
            return std::string();
        }
    }
private:
    TDB_DATA itsKey;
    TDB_DATA itsValue;
    TdbKeyValue(const TdbKeyValue&);
    TdbKeyValue& operator=(const TdbKeyValue&);
};

inline unsigned char* getWritableUCString(const std::string& aString);

}

#endif /* TDB_H_ */
