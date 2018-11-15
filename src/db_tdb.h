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

#ifndef DB_TDB_H_
#define DB_TDB_H_

#include <string>
#include <vector>

#include <fcntl.h>
#include <tdb.h>

#include "custom_exceptions.h"
#include "db.h"

namespace cnf {
class TdbDatabase : public Database {
public:
    explicit TdbDatabase(const std::string& id,
                         bool readonly,
                         const std::string& base_path);
    void storePackage(const Package& p) override;
    void getPackages(const std::string& search,
                     std::vector<Package>& result) const override;
    void truncate() override;
    ~TdbDatabase() override;
    static void getCatalogs(const std::string& database_path,
                            std::vector<std::string>& result);

private:
    TDB_CONTEXT* m_tdbFile;
    const std::string m_databaseName;
};

class TdbKeyValue {
public:
    explicit TdbKeyValue(const std::string& key, const std::string& value);
    explicit TdbKeyValue();
    TdbKeyValue(const TdbKeyValue&) = delete;
    TdbKeyValue& operator=(const TdbKeyValue&) = delete;

    void setKey(const std::string& key);
    void setKey(const TDB_DATA& key);
    void setValue(const std::string& value);
    void setValue(const TDB_DATA& value);
    ~TdbKeyValue();
    const TDB_DATA& key() const { return m_key; }
    const TDB_DATA& value() const { return m_value; }
    std::string key_str() const {
        if (m_key.dptr) {
            return std::string(reinterpret_cast<const char*>(m_key.dptr));
        }
            return std::string();
    }
    std::string value_str() const {
        if (m_value.dptr) {
            return std::string(reinterpret_cast<const char*>(m_value.dptr));
        }
            return std::string();
    }

private:
    TDB_DATA m_key;
    TDB_DATA m_value;
};

inline unsigned char* getWritableUCString(const std::string& aString);

}  // namespace cnf

#endif /* DB_TDB_H_ */
