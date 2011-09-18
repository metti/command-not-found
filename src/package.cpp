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

#include <boost/regex.hpp>
#include <archive.h>
#include <archive_entry.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "package.h"

namespace bf = boost::filesystem;
using namespace std;
using boost::regex;
using boost::regex_match;
using boost::cmatch;

namespace cnf {

Package::Package(const bf::path& path, const bool lazy) throw (InvalidArgumentException)
        : itsFilesDetermined(false), itsPath(new bf::path(path)) {

    // checks
    if (!bf::is_regular_file(path)) {
        string message;
        message += "not a file: ";
        message += path.string();
        throw InvalidArgumentException(MISSING_FILE, message.c_str());
    }

    static const regex valid_name(
            "(.+)-(.+)-(.+)-(any|i686|x86_64).pkg.tar.(xz|gz)");

    cmatch what;

#if BOOST_FILESYSTEM_VERSION == 2
    const string filename = path.filename().c_str();
#else
    const string filename = path.filename().string();
#endif
    if (regex_match(filename.c_str(), what, valid_name)) {

        itsName = what[1];
        itsVersion = what[2];
        itsRelease = what[3];
        itsArchitecture = what[4];
        itsCompression = what[5];
    } else {
        string message;
        message += "this is not a valid package file: ";
        message += path.string();
        throw InvalidArgumentException(INVALID_FILE, message.c_str());

    }

    if (!lazy)
        updateFiles();
}

void Package::updateFiles() const {

    // read package file list

    if (!itsPath) {
        throw InvalidArgumentException(INVALID_FILE, "You should never end up here!");
    }

    struct archive *arc = NULL;
    struct archive_entry *entry = NULL;
    int rc = 0;
    vector<string> candidates;

    arc = archive_read_new();
    archive_read_support_compression_gzip(arc);
    archive_read_support_compression_xz(arc);
    archive_read_support_format_tar(arc);

#if BOOST_FILESYSTEM_VERSION == 2
    rc = archive_read_open_filename(arc, itsPath->string().c_str(), 10240);
#else
    rc = archive_read_open_filename(arc, itsPath->c_str(), 10240);
#endif

    if (rc != ARCHIVE_OK){
        string message;
        message += "could not read file list from: ";
        message += itsPath->string();
        throw InvalidArgumentException(INVALID_FILE, message.c_str());
    }
    while (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
        candidates.push_back(archive_entry_pathname(entry));
    }

    rc = archive_read_finish(arc);

    if (rc != ARCHIVE_OK){
        string message;
        message += "error while closing archive: ";
        message += itsPath->string();
        throw InvalidArgumentException(INVALID_FILE, message.c_str()); 
    }

    const regex significant("((usr/)?(s)?bin/([0-9A-Za-z.-]+))");

    typedef vector<string>::const_iterator canditer;

    cmatch what;
    for (canditer iter(candidates.begin()); iter != candidates.end(); ++iter) {
        if (regex_match(iter->c_str(), what, significant)) {
            itsFiles.push_back(what[4]);
        }
    }

    itsFilesDetermined = true;

}

const string Package::hl_str(const string& hl) const {
    vector<string> hls;
    hls.push_back(hl);

    return hl_str(&hls);
}

const string Package::hl_str(const vector<string>* hl) const {
    stringstream out;
    out << "\33[1m" << name() << "\033[0m" << " (" << version() << "-" << release() << ") [ ";

    for (Package::const_file_iterator iter = files().begin();
            iter != files().end(); ++iter) {

        string color = "\033[0m";

        if (hl != NULL){
            for (vector<string>::const_iterator hlIter = hl->begin();
                                                hlIter != hl->end(); ++hlIter){
                if (*hlIter != "" && *hlIter == *iter){
                    color = "\033[0;31m";
                }
            }
        }

        out << color << *iter << "\033[0m" << " ";
    }
    out << "]";
    return out.str();
}

ostream& operator<<(ostream& out, const Package& p) {
    out << p.hl_str("");
    return out;
}

bool operator<(const Package& lhs, const Package& rhs){
    return lhs.name() < rhs.name();
}

bool operator==(const Package& lhs, const Package& rhs){
   return lhs.name() == rhs.name() &&
          lhs.version() == rhs.version() &&
          lhs.release() == rhs.release() &&
          lhs.architecture() == rhs.architecture();
}

}

