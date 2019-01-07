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

#ifndef PACKAGE_H_
#define PACKAGE_H_

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "custom_exceptions.h"

namespace cnf {

class Package {
public:
    explicit Package(const std::filesystem::path& path, bool lazy = false);
    explicit Package(std::string name,
                     std::string version,
                     std::string release,
                     std::string architecture,
                     std::string compression,
                     std::vector<std::string> files)
        : m_name(std::move(name))
        , m_version(std::move(version))
        , m_release(std::move(release))
        , m_architecture(std::move(architecture))
        , m_compression(std::move(compression))
        , m_files(std::move(files))
        , m_filesDetermined(true) {}

    const std::vector<std::string>& files() const;

    const std::string& name() const { return m_name; }
    const std::string& version() const { return m_version; }
    const std::string& release() const { return m_release; }
    const std::string& architecture() const { return m_architecture; }
    const std::string& compression() const { return m_compression; }
    const std::string hl_str(const std::string& /*hl*/ = "",
                             const std::string& files_indent = "",
                             const std::string& color = "") const;
    const std::string hl_str(const std::vector<std::string>* /*hl*/ = nullptr,
                             const std::string& files_indent = "",
                             const std::string& color = "") const;

private:
    void updateFiles() const;

    std::string m_name;
    std::string m_version;
    std::string m_release;
    std::string m_architecture;
    std::string m_compression;
    mutable std::vector<std::string> m_files;
    mutable bool m_filesDetermined;
    std::optional<std::filesystem::path> m_path;
};

enum PackageError { MISSING_FILE, INVALID_FILE, UNKNOWN_ERROR };

std::ostream& operator<<(std::ostream& out, const Package& p);

bool operator<(const Package& lhs, const Package& rhs);
bool operator==(const Package& lhs, const Package& rhs);

}  // namespace cnf

#endif /* PACKAGE_H */
