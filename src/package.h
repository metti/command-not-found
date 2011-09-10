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

#include "filesystem.h"
#include "custom_exceptions.h"

namespace cnf {

class Package {
public:
    explicit Package(const boost::filesystem::path file,
                     const bool lazy = false)
                             throw (cnf::InvalidArgumentException);
    explicit Package(const std::string name,
                     const std::string version,
                     const std::string release,
                     const std::string architecture,
                     const std::string compression,
                     const std::vector<std::string> files)
                             throw (cnf::InvalidArgumentException)
                  : itsName(name),
                    itsVersion(version),
                    itsRelease(release),
                    itsArchitecture(architecture),
                    itsCompression(compression),
                    itsFiles(files),
                    itsFilesDetermined(true),
                    itsPath(NULL) {
    }

    ~Package() {
        delete itsPath;
    }

    const std::vector<std::string>& files() const {
        if (!itsFilesDetermined)
            updateFiles();
        return itsFiles;
    }
    const std::string name() const {
        return itsName;
    }
    const std::string version() const {
        return itsVersion;
    }
    const std::string release() const {
        return itsRelease;
    }
    const std::string architecture() const {
        return itsArchitecture;
    }
    const std::string compression() const {
        return itsCompression;
    }
    const std::string hl_str(const std::string = "") const;

    typedef std::vector<std::string>::const_iterator const_file_iterator;
    typedef std::vector<std::string>::iterator file_iterator;

private:

    void updateFiles() const;

    std::string itsName;
    std::string itsVersion;
    std::string itsRelease;
    std::string itsArchitecture;
    std::string itsCompression;
    mutable std::vector<std::string> itsFiles;
    mutable bool itsFilesDetermined;
    boost::filesystem::path* itsPath;
};

enum PackageError {
    MISSING_FILE, INVALID_FILE
};

std::ostream& operator<<(std::ostream& out, const Package& p);

}

#endif /* PARSEPKG_H */
