#include "syscall.hpp"

namespace dbpp {

/// Make the client perform a syscall with the given number and parameters.
register_t syscall(int pid, register_t syscall, std::array<register_t, 6> const & parameters) {
	registers_t old_registers = get_registers(pid);
	registers_t new_registers = old_registers;
	unsigned long old_code = read_memory(pid, old_registers.ip);
#if defined(__i386__)
	new_registers.ax = syscall;
	new_registers.bx = parameters[0];
	new_registers.cx = parameters[1];
	new_registers.dx = parameters[2];
	new_registers.si = parameters[3];
	new_registers.di = parameters[4];
	new_registers.bp = parameters[5];
	set_registers(pid, new_registers);
	unsigned long new_code = (old_code & ~(0xffff)) | 0x80cd;
	write_memory(pid, old_registers.ip, new_code);
#elif defined(__x86_64__)
	new_registers.ax  = syscall;
	new_registers.di  = parameters[0];
	new_registers.si  = parameters[1];
	new_registers.dx  = parameters[2];
	new_registers.r10 = parameters[3];
	new_registers.r8  = parameters[4];
	new_registers.r9  = parameters[5];
	set_registers(pid, new_registers);
	unsigned long new_code = (old_code & ~(0xffff)) | 0x050f;
	write_memory(pid, old_registers.ip, new_code);
#else
		static_assert(false, "Unsupported architecture.");
#endif

	// Wait for entry and exit.
	step_syscall(pid);
	while (!wait_for_syscall(pid));
	step_syscall(pid);
	while (!wait_for_syscall(pid));

	new_registers = get_registers(pid);

	write_memory(pid, old_registers.ip, old_code);
	set_registers(pid, old_registers);
	return new_registers.ax;
}

}
