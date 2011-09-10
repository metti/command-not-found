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

#include <map>

#include "package.h"
#include "config.h"

namespace cnf {

enum DatabaseError {
    CONNECT_ERROR
};

class Database {
public:
    explicit Database(const std::string id,
                      bool readonly,
                      const std::string base_path) throw (DatabaseException)
            : itsId(id), isReadonly(readonly), itsBasePath(base_path) {
    }
    virtual void storePackage(const Package& p) throw (DatabaseException) = 0;
    virtual const std::vector<Package> getPackages(const std::string search) const
            throw (DatabaseException) = 0;
    virtual void truncate() throw (DatabaseException) = 0;
    virtual ~Database() {
    }
    static const std::vector<std::string> getCatalogs(const std::string database_path)
            throw (DatabaseException);
private:
    Database& operator=(const Database&);
    Database(const Database&);
protected:
    const std::string itsId;
    const bool isReadonly;
    const std::string itsBasePath;
};

const boost::shared_ptr<Database> getDatabase(const std::string id,
                                              const bool readonly,
                                              const std::string base_path)
                                              throw (DatabaseException);

const std::vector<std::string> getCatalogs(const std::string database_path)
        throw (DatabaseException);

const std::map<std::string, std::vector<Package> > lookup(const std::string searchString,
                                                          const std::string database_path);

void populate_mirror(const boost::filesystem::path path,
                     const std::string database_path,
                     const bool truncate,
                     const int verbosity);

void populate(const boost::filesystem::path path,
              const std::string database_path,
              const std::string catalog,
              const bool truncate,
              const int verbosity);

}

#endif /* DB_H_ */
