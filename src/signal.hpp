/*
  Copyright 2014 Maarten de Vries <maarten@de-vri.es>
  https://github.com/de-vri-es/fdinject/

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <string>

namespace dbpp {

constexpr int sighup    =  1;      ///< Hangup detected on controlling terminal or death of controlling process.
constexpr int sigint    =  2;      ///< Interrupt from keyboard.
constexpr int sigquit   =  3;      ///< Quit from keyboard.
constexpr int sigill    =  4;      ///< Illegal Instruction.
constexpr int sigtrap   =  5;      ///< Trace/breakpoint trap.
constexpr int sigabrt   =  6;      ///< Abort signal from abort(3).
constexpr int sigbus    =  7;      ///< Bus error (bad memory access).
constexpr int sigfpe    =  8;      ///< Floating point exception.
constexpr int sigkill   =  9;      ///< Kill signal.
constexpr int sigusr1   = 10;      ///< User-defined signal 1.
constexpr int sigsegv   = 11;      ///< Invalid memory reference.
constexpr int sigusr2   = 12;      ///< User-defined signal 2.
constexpr int sigpipe   = 13;      ///< Broken pipe: write to pipe with no readers.
constexpr int sigalrm   = 14;      ///< Timer signal from alarm(2).
constexpr int sigterm   = 15;      ///< Termination signal.
constexpr int sigstkflt = 16;      ///< Stack fault on coprocessor (unused).
constexpr int sigchld   = 17;      ///< Child stopped or terminated.
constexpr int sigcont   = 18;      ///< Continue if stopped.
constexpr int sigstop   = 19;      ///< Stop process.
constexpr int sigtstp   = 20;      ///< Stop typed at terminal.
constexpr int sigttin   = 21;      ///< Terminal input for background process.
constexpr int sigttou   = 22;      ///< Terminal output for background process.
constexpr int sigurg    = 23;      ///< Urgent condition on socket (4.2BSD).
constexpr int sigxcpu   = 24;      ///< CPU time limit exceeded (4.2BSD).
constexpr int sigxfsz   = 25;      ///< File size limit exceeded (4.2BSD).
constexpr int sigvtalrm = 26;      ///< Virtual alarm clock (4.2BSD).
constexpr int sigprof   = 27;      ///< Profiling timer expired.
constexpr int sigwinch  = 28;      ///< Window resize signal (4.3BSD, Sun).
constexpr int sigiot    = sigabrt; ///< IOT trap. A synonym for SIGABRT.
constexpr int sigio     = 29;      ///< I/O now possible (4.2BSD).
constexpr int sigpoll   = sigio;   ///< Pollable event (Sys V).  Synonym for SIGIO.
constexpr int sigpwr    = 30;      ///< Power failure (System V).
constexpr int sigsys    = 31;      ///< Bad argument to routine (SVr4).
constexpr int sigunused = sigsys;  ///< Synonymous with SIGSYS.

/// Raise a signal to the caller.
/**
 * Throws on failure.
 */
void raise(int signal);

/// Send a signal to a process.
/**
 * Throws on failure.
 */
void kill(int pid, int signal);

/// Convert a signal number to a string representation.
std::string strsignal(int signal);
}
