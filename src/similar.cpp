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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "similar.h"

using namespace std::string_literals;

namespace cnf {

std::vector<std::string> similar_words(const std::string& word) {
    static const auto alphabet = "abcdefghijklmnopqrstuvwxyz-_0123456789"s;

    std::vector<std::string> result;
    if (word.empty()) {
        return result;
    }

    const size_t upper_bound{(alphabet.size() * 2 + 2) * word.size() - 2};
    result.reserve(upper_bound);

    std::vector<std::pair<std::string, std::string>> splits;
    splits.reserve(word.size());

    for (uint32_t i = 0; i < word.size(); ++i) {
        splits.emplace_back(word.substr(0, i), word.substr(i));
    }

    for (const auto& split : splits) {
        if (split.second.size() > 1) {
            // delete
            result.emplace_back(split.first + split.second.substr(1));
            // transpose
            result.emplace_back(split.first + split.second[1] +
                                split.second[0] + split.second.substr(2));
        }

        for (const auto& c : alphabet) {
            // replaces
            result.emplace_back(split.first + c + split.second.substr(1));

            // inserts
            result.emplace_back(split.first + c + split.second);
        }
    }

    assert(upper_bound >= result.size());  // wrong reserve calculation

    sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());

    return result;
}

}  // namespace cnf
