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
