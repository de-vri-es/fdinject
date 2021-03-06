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

#include <string.h>
#include <sys/signal.h>

#include "signal.hpp"
#include "exceptions.hpp"

namespace dbpp {

/// Raise a signal to the caller.
void raise(int signal) {
	if (::raise(signal) != 0) throw error(-1, {errno, std::system_category()}, "Failed to raise signal.");
}

/// Raise a signal to the caller.
void kill(int pid, int signal) {
	if (::kill(pid, signal) != 0) throw error(pid, {errno, std::system_category()}, "Failed to send signal.");
}

/// Convert a signal number to a string representation.
std::string strsignal(int signal) {
	char const * result = ::strsignal(signal);
	if (!result) return "unknown signal";
	return result;
}

}
