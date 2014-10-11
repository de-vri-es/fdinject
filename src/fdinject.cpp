#include <iostream>
#include <sstream>
#include <vector>

#include <sys/mman.h>

#include "dbpp.hpp"
#include "syscall.hpp"

namespace fdinject {

long mmap(int pid, dbpp::register_t address, std::size_t length, bool read, bool write, bool exec) {
	return dbpp::syscall(pid, 9, {{
		address,
		length,
		static_cast<unsigned int>(PROT_READ | PROT_WRITE),
		MAP_PRIVATE | MAP_ANONYMOUS,
		0,
		0
	}});
}

int munmap(int pid, dbpp::register_t address, size_t length) {
	return dbpp::syscall(pid, 11, {{
		address,
		length,
		0,
		0,
		0,
		0
	}});
}

int write(int pid, int fd, void const * data, std::size_t length) {
	std::cout << "Calling write(" << fd << ", " << data << ", " << length << ") in process " << pid << ".\n";
	return dbpp::syscall(pid, 1, {{
		static_cast<dbpp::register_t>(fd),
		reinterpret_cast<dbpp::register_t>(data),
		length,
		0, 0, 0
	}});
}

void dwrite(int pid, int fd, void const * data, std::size_t length) {
	std::cout << "Allocating memory in tracee.\n";
	long address = mmap(pid, 0, length, true, true, false);
	if (address < 0) throw dbpp::error(pid, {int(-address), std::generic_category()}, "Failed to allocate memory in process");


	std::cout << "Copying memory to tracee.\n";
	dbpp::memcpy_to(pid, address, data, length);

	std::size_t written = 0;
	while (written < length) {
		int result = write(pid, fd, reinterpret_cast<char const *>(address + written), length - written);
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
	std::cout << "Writing: `" << data << "'.\n";

	try {
		std::cout << "Attaching to process " << pid << "\n";
		dbpp::attach(pid);
		std::cout << "Interrupting process " << pid << "\n";
		dbpp::kill(pid, dbpp::sigstop);
		std::cout << "waiting for process " << pid << "\n";
		dbpp::wait_for_trap(pid);
		fdinject::dwrite(pid, fd, data.data(), data.size());
		std::cout << "Detaching from process " << pid << "\n";
		dbpp::detach(pid);
	} catch (std::system_error const & e) {
		std::cout << "Error " << e.code().value() << ": " << e.what() << "\n";
	}
}
