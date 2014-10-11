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

#include <system_error>

namespace dbpp {

struct error : public std::system_error {
	int pid;

	error(int pid, std::error_code const & error_code)                           : std::system_error::system_error(error_code),       pid(pid) {}
	error(int pid, std::error_code const & error_code, char const * what)        : std::system_error::system_error(error_code, what), pid(pid) {}
	error(int pid, std::error_code const & error_code, std::string const & what) : std::system_error::system_error(error_code, what), pid(pid) {}
};

/// Thrown when a process terminated instead of showing the expected behaviour.
class process_terminated : public error {
public:
	/// Indicates if the process exited cleanly by calling exit(), or if it was killed by a signal.
	bool const clean;

	/// The signal or exit status of the process.
	int const status;

	process_terminated(int pid, bool clean, int status) :
		error(pid, std::make_error_code(std::errc::no_such_process)),
		clean(clean),
		status(status) {}

	process_terminated(int pid, bool clean, int status, std::string const & what) :
		error(pid, std::make_error_code(std::errc::no_such_process), what),
		clean(clean),
		status(status) {}

	process_terminated(int pid, bool clean, int status, char const * what) :
		error(pid, std::make_error_code(std::errc::no_such_process), what),
		clean(clean),
		status(status) {}
};

/// Thrown when a process received an unexpected signal.
class unexpected_signal : public error {
public:
	/// The signal or exit status of the process.
	int const signal;

	unexpected_signal(int pid, int signal) :
		error(pid, std::make_error_code(std::errc::no_such_process)),
		signal(signal) {}

	unexpected_signal(int pid, int signal, std::string const & what) :
		error(pid, std::make_error_code(std::errc::no_such_process), what),
		signal(signal) {}

	unexpected_signal(int pid, int signal, char const * what) :
		error(pid, std::make_error_code(std::errc::no_such_process), what),
		signal(signal) {}
};

}
