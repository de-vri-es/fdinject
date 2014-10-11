#include <array>

#include "dbpp.hpp"

namespace dbpp {

/// Make the client perform a syscall with the given number and parameters.
register_t syscall(int pid, register_t syscall, std::array<register_t, 6> const & parameters);

}
