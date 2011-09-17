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

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <exception>
#include <string>

namespace cnf {

class ErrorCodeException: public std::exception {
public:
    explicit ErrorCodeException(const int aCode, const std::string& aMessage)
            : itsCode(aCode), itsMessage(aMessage) {
    }
    virtual ~ErrorCodeException() throw(){}
    int code() const {
        return itsCode;
    }
    const char* what() const throw () {
        return itsMessage.c_str();
    }
private:
    const int itsCode;
    const std::string itsMessage;
};

class InvalidArgumentException: public ErrorCodeException {
public:
    InvalidArgumentException(const int aCode, const std::string& aMessage)
            : ErrorCodeException(aCode, aMessage) {
    }
};

class DatabaseException: public ErrorCodeException {
public:
    DatabaseException(const int aCode, const std::string& aMessage)
            : ErrorCodeException(aCode, aMessage) {
    }
};
}

#endif /* EXCEPTIONS_H_ */
