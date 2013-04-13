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

#ifndef PARSEPKG_H_
#define PARSEPKG_H_

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

#include "custom_exceptions.h"

namespace cnf {

class Package {
public:
    explicit Package(const boost::filesystem::path& file,
                     const bool lazy = false);
    explicit Package(const std::string& name,
                     const std::string& version,
                     const std::string& release,
                     const std::string& architecture,
                     const std::string& compression,
                     const std::vector<std::string>& files)
                  : m_name(name)
                  , m_version(version)
                  , m_release(release)
                  , m_architecture(architecture)
                  , m_compression(compression)
                  , m_files(files)
                  , m_filesDetermined(true)
                  , m_path(NULL) {
    }

    ~Package() {
        delete m_path;
    }

    const std::vector<std::string>& files() const {
        if (!m_filesDetermined)
            updateFiles();
        return m_files;
    }
    const std::string& name() const {
        return m_name;
    }
    const std::string& version() const {
        return m_version;
    }
    const std::string& release() const {
        return m_release;
    }
    const std::string& architecture() const {
        return m_architecture;
    }
    const std::string& compression() const {
        return m_compression;
    }
    const std::string hl_str(const std::string& = "", const std::string& files_indent = "", const std::string& color = "") const;
    const std::string hl_str(const std::vector<std::string>* = NULL, const std::string& files_indent = "", const std::string& color = "") const;

private:

    void updateFiles() const;

    std::string m_name;
    std::string m_version;
    std::string m_release;
    std::string m_architecture;
    std::string m_compression;
    mutable std::vector<std::string> m_files;
    mutable bool m_filesDetermined;
    boost::filesystem::path* m_path;
};

enum PackageError {
    MISSING_FILE, INVALID_FILE
};

std::ostream& operator<<(std::ostream& out, const Package& p);

bool operator<(const Package& lhs, const Package& rhs);
bool operator==(const Package& lhs, const Package& rhs);

}

#endif /* PARSEPKG_H */
