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

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <archive.h>
#include <archive_entry.h>
#include <boost/format.hpp>
#include <boost/locale.hpp>

#include "package.h"

namespace bf = boost::filesystem;
using namespace std;
using boost::format;
using boost::locale::translate;

namespace cnf {

Package::Package(const bf::path& path, const bool lazy)
    : m_filesDetermined(false), m_path(new bf::path(path)) {
    // checks
    if (!bf::is_regular_file(path)) {
        string message;
        message += translate("not a file: ");
        message += path.string();
        throw InvalidArgumentException(MISSING_FILE, message.c_str());
    }

    static const regex valid_name(
        "(.+)-(.+)-(.+)-(any|i686|x86_64).pkg.tar.(xz|gz)");

    cmatch what;

    const string& filename = path.filename().string();

    try {
        if (regex_match(filename.c_str(), what, valid_name)) {
            m_name = what[1];
            m_version = what[2];
            m_release = what[3];
            m_architecture = what[4];
            m_compression = what[5];
        } else {
            string message;
            message += translate("this is not a valid package file: ");
            message += path.string();
            throw InvalidArgumentException(INVALID_FILE, message.c_str());
        }
    } catch (const std::logic_error& e) {
        throw InvalidArgumentException(UNKNOWN_ERROR, e.what());
    }

    if (!lazy)
        updateFiles();
}

const vector<string>& Package::files() const {
    if (!m_filesDetermined)
        try {
            updateFiles();
        } catch (const InvalidArgumentException& e) {
            cerr << e.what() << endl;
        }
    return m_files;
}

void Package::updateFiles() const {
    // read package file list

    assert(m_path);

    struct archive* arc = NULL;
    struct archive_entry* entry = NULL;
    int rc = 0;
    vector<string> candidates;

    arc = archive_read_new();
    archive_read_support_filter_all(arc);
    archive_read_support_format_tar(arc);

    rc = archive_read_open_filename(arc, m_path->c_str(), 10240);

    if (rc != ARCHIVE_OK) {
        format message;
        message = format(translate("could not read file list from: %s")) %
                  m_path->string();
        throw InvalidArgumentException(INVALID_FILE, message.str());
    }
    while (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
        candidates.push_back(archive_entry_pathname(entry));
    }

    rc = archive_read_close(arc);

    if (rc != ARCHIVE_OK) {
        format message;
        message = format(translate("error while closing archive: %s")) %
                  m_path->string();
        throw InvalidArgumentException(INVALID_FILE, message.str());
    }

    const regex significant("((usr/)?(s)?bin/([0-9A-Za-z.-]+))");

    cmatch what;
    try {
        for (const auto& candidate : candidates) {
            if (regex_match(candidate.c_str(), what, significant)) {
                m_files.push_back(what[4]);
            }
        }
    } catch (const std::logic_error& e) {
        throw InvalidArgumentException(UNKNOWN_ERROR, e.what());
    }

    m_filesDetermined = true;
}

const string Package::hl_str(const string& hl,
                             const string& files_indent,
                             const string& color) const {
    vector<string> hls;
    hls.push_back(hl);

    return hl_str(&hls, files_indent, color);
}

const string Package::hl_str(const vector<string>* hl,
                             const string& files_indent,
                             const string& color) const {
    stringstream out;
    out << files_indent << "[ ";

    int linelength = 0;
    for (const auto& file : files()) {
        bool highlight = false;
        if (hl != NULL) {
            for (const auto& hlIter : *hl) {
                if (hlIter != "" && hlIter == file) {
                    highlight = true;
                    break;
                }
            }
        }

        if (linelength + file.size() > 80) {
            linelength = 0;
            out << endl << files_indent << "  ";
        }

        linelength += file.size() + 1;
        if (highlight) {
            if (color.empty()) {
                out << "*" << file << "* ";
            } else {
                out << color << file << "\033[0m"
                    << " ";
            }
        } else {
            out << file << " ";
        }
    }
    out << "]";
    return out.str();
}

ostream& operator<<(ostream& out, const Package& p) {
    out << p.name() << " (" << p.version() << "-" << p.release() << ")" << endl;
    return out;
}

bool operator<(const Package& lhs, const Package& rhs) {
    return lhs.name() < rhs.name();
}

bool operator==(const Package& lhs, const Package& rhs) {
    return lhs.name() == rhs.name() && lhs.version() == rhs.version() &&
           lhs.release() == rhs.release() &&
           lhs.architecture() == rhs.architecture();
}

}  // namespace cnf
