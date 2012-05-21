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

#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <utility>

#include "similar.h"

using namespace std;

void similar_words(const string& word, set<string>& result){
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz-_0123456789";

    vector<pair<string,string> > splits;

    for (unsigned int i = 0 ; i <= word.size(); ++i){
        splits.push_back(pair<string,string>(word.substr(0,i),word.substr(i)));
    }

    typedef vector<pair<string,string> >::const_iterator splitIter;

    for (splitIter iter = splits.begin(); iter != splits.end(); ++iter){
        if (iter->second.size() > 0){
            //delete
            result.insert(iter->first+iter->second.substr(1));

            if (iter->second.size() > 1){
                //transpose
                result.insert(iter->first
                               + iter->second[1]
                               + iter->second[0]
                               + iter->second.substr(2));
            }

            for(unsigned int i = 0; i < sizeof(alphabet)/sizeof(char) - 1;++i){
                //replaces
                result.insert(iter->first + alphabet[i] + iter->second.substr(1));

                //inserts
                result.insert(iter->first + alphabet[i] + iter->second);
            }

        }
    }
}

