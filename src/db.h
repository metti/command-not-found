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

#ifndef DB_H_
#define DB_H_

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "package.h"

namespace cnf {

enum DatabaseError { CONNECT_ERROR };

class Database {
public:
    explicit Database(std::string id,
                      const bool readonly,
                      std::string base_path)
        : m_id(std::move(id))
        , m_readonly(readonly)
        , m_basePath(std::move(base_path)) {}
    virtual void storePackage(const Package& p) = 0;
    virtual void getPackages(const std::string& search,
                             std::vector<Package>& result) const = 0;
    virtual void truncate() = 0;
    virtual ~Database() = default;
    static void getCatalogs(const std::string& database_path,
                            std::vector<std::string>& result);

private:
    Database& operator=(const Database&);
    Database(const Database&);

protected:
    const std::string m_id;
    const bool m_readonly;
    const std::string m_basePath;
};

using ResultMap = std::map<std::string, std::set<Package>>;

const std::shared_ptr<Database> getDatabase(const std::string& id,
                                            const bool readonly,
                                            const std::string& base_path);

void getCatalogs(const std::string& database_path,
                 std::vector<std::string>& result);

void lookup(const std::string& searchString,
            const std::string& database_path,
            ResultMap& result,
            std::vector<std::string>* const inexact_matches = nullptr);

void populate_mirror(const boost::filesystem::path& path,
                     const std::string& database_path,
                     const bool truncate,
                     const uint8_t verbosity);

void populate(const boost::filesystem::path& path,
              const std::string& database_path,
              const std::string& catalog,
              const bool truncate,
              const uint8_t verbosity);
}  // namespace cnf

#endif /* DB_H_ */
