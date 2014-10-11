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

#include <iostream>
#include <sstream>
#include <vector>

extern "C" {
#include <sys/mman.h>
}

#include "dbpp.hpp"
#include "syscall.hpp"

namespace fdinject {

#if !defined(__x86_64__)
static_assert(false, "Unsupported architecture. At the moment, fdinject only support Linux on x86_64.");
#endif

long mmap(int pid, dbpp::register_t address, std::size_t length, int protection, int flags, int fd, std::size_t offset) {
	return dbpp::syscall(pid, 9, {{address, length, unsigned(protection), unsigned(flags), unsigned(fd), offset}});
}

int munmap(int pid, dbpp::register_t address, size_t length) {
	return dbpp::syscall(pid, 11, {{address, length, 0, 0, 0, 0}});
}

int write(int pid, int fd, dbpp::register_t address, std::size_t length) {
	return dbpp::syscall(pid, 1, {{unsigned(fd), address, length, 0, 0, 0}});
}

void inject_data(int pid, int fd, void const * data, std::size_t length) {
	std::cout << "Allocating memory in tracee.\n";
	long address = mmap(pid, 0, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (address < 0) throw dbpp::error(pid, {int(-address), std::generic_category()}, "Failed to allocate memory in process");

	std::cout << "Copying memory to tracee.\n";
	dbpp::memcpy_to(pid, address, data, length);

	std::size_t written = 0;
	while (written < length) {
		int result = write(pid, fd, address + written, length - written);
		if (result >= 0) {
			std::cout << "Written " << result << " bytes.\n";
			written += result;
		} else {
			std::cout << "write returned " << result << ".\n";
			std::error_code error(-result, std::generic_category());
			if (error != std::errc::resource_unavailable_try_again && error != std::errc::operation_would_block) {
				throw dbpp::error(pid, error, "Failed to execute write system call in traced process.");
			}
		}
	}

	std::cout << "Deallocating memory in tracee.\n";
	int result = munmap(pid, address, length);
	if (result < 0) throw dbpp::error(pid, {int(-result), std::generic_category()}, "Failed to deallocate memory in process");
}

}

int main(int argc, char * * argv) {
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " pid fd\n";
		return 1;
	}

	int pid = std::stoi(argv[1]);
	int fd  = std::stoi(argv[2]);

	std::cout << "Writing to descriptor " << fd << " of process " << pid << ".\n";

	std::string data;
	{
		std::stringstream buffer;
		copy(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(buffer));
		data = buffer.str();
	}
	try {
		std::cout << "Attaching to process.\n";
		dbpp::attach(pid);
		std::cout << "Interrupting process.\n";
		dbpp::kill(pid, dbpp::sigstop);
		std::cout << "waiting for process to halt.\n";
		dbpp::wait_for_trap(pid);
		std::cout << "Starting remote write.\n";
		fdinject::inject_data(pid, fd, data.data(), data.size());
		std::cout << "Detaching from process.\n";
		dbpp::detach(pid);
	} catch (std::system_error const & e) {
		std::cout << "Error " << e.code().value() << ": " << e.what() << "\n";
	}
}
