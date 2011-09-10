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

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <iostream>

#include "guard.h"

using namespace std;

namespace cnf {

void install_handler() {
    signal(SIGSEGV, print_trace);
}

void handler(int sig) {
    void *array[100];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    fprintf(stderr, "\nFALLBACK STACKTRACE:\nError: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 2);
    exit(1);
}

void print_trace(int sig) {
    cerr << "Got signal " << sig << ". Printing stacktrace ..." << endl;
    signal(SIGSEGV, handler);
    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
    int child_pid = fork();
    if (!child_pid) {
        dup2(2, 1); // redirect output to stderr
        fprintf(stdout, "stack trace for %s pid=%s\n", name_buf, pid_buf);
        execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt",
               name_buf, pid_buf, NULL);
        abort(); /* If gdb failed to start */
    } else {
        waitpid(child_pid, NULL, 0);
    }
}

}
