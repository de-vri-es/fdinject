#include <iostream>

extern "C" {
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
}

#include "dbpp.hpp"

namespace dbpp {

namespace {
	/// Address of the global (intterupt 3) trap.
	void * trap = nullptr;

	// Perform some static initialization to get the address of a trap instruction.
	struct static_initialization {
		static_initialization() {
			void * trap;
		#if defined(__i386__)
			asm volatile (
				R"(
				call %=f
				int $3
			%=:
				popl %[trap]
				)"
				: [trap] "=r" (trap)
				: :
			);
		#elif defined(__x86_64__)
			asm volatile (
				R"(
				call %=f
				int $3
			%=:
				popq %[trap]
				)"
				: [trap] "=r" (trap)
				: :
			);
		#else
			static_assert(false, "Unsupported architecture.");
		#endif
			dbpp::trap = trap;
		}
	} static_initialization;

	registers_t from_impl(user_regs_struct const & regs) {
		registers_t result;
#if defined(__i386__)
		result.ax = regs.eax;
		result.bx = regs.ebx;
		result.cx = regs.ecx;
		result.dx = regs.edx;
		result.si = regs.esi;
		result.di = regs.edi;

		result.sp = regs.esp;
		result.bp = regs.ebp;
		result.ip = regs.eip;

		result.orig_ax = regs.orig_eax;
		result.eflags  = regs.eflags;

		result.ds = regs.xds;
		result.es = regs.xes;
		result.fs = regs.xfs;
		result.gs = regs.xgs;
		result.cs = regs.xcs;
		result.ss = regs.xss;
#elif defined(__x86_64__)
			result.ax  = regs.rax;
		result.bx  = regs.rbx;
		result.cx  = regs.rcx;
		result.dx  = regs.rdx;
		result.si  = regs.rsi;
		result.di  = regs.rdi;

		result.r8  = regs.r8;
		result.r9  = regs.r9;
		result.r10 = regs.r10;
		result.r11 = regs.r11;
		result.r12 = regs.r12;
		result.r13 = regs.r13;
		result.r14 = regs.r14;
		result.r15 = regs.r15;

		result.sp = regs.rsp;
		result.bp = regs.rbp;
		result.ip = regs.rip;

		result.orig_ax = regs.orig_rax;
		result.eflags  = regs.eflags;

		result.ds = regs.ds;
		result.es = regs.es;
		result.fs = regs.fs;
		result.gs = regs.gs;
		result.cs = regs.cs;
		result.ss = regs.ss;

		result.fs_base = regs.fs_base;
		result.gs_base = regs.gs_base;
#else
		static_assert(false, "Unsupported architecture.");
#endif
		return result;
	}

	user_regs_struct to_impl(registers_t const & regs) {
		user_regs_struct result;
#if defined(__i386__)
		result.eax = regs.ax;
		result.ebx = regs.bx;
		result.ecx = regs.cx;
		result.edx = regs.dx;
		result.esi = regs.si;
		result.edi = regs.di;

		result.esp = regs.sp;
		result.ebp = regs.bp;
		result.eip = regs.ip;

		result.orig_eax = regs.orig_ax;
		result.eflags   = regs.eflags;

		result.xds = regs.ds;
		result.xes = regs.es;
		result.xfs = regs.fs;
		result.xgs = regs.gs;
		result.xcs = regs.cs;
		result.xss = regs.ss;
#elif defined(__x86_64__)
		result.rax  = regs.ax;
		result.rbx  = regs.bx;
		result.rcx  = regs.cx;
		result.rdx  = regs.dx;
		result.rsi  = regs.si;
		result.rdi  = regs.di;

		result.r8  = regs.r8 ;
		result.r9  = regs.r9 ;
		result.r10 = regs.r10;
		result.r11 = regs.r11;
		result.r12 = regs.r12;
		result.r13 = regs.r13;
		result.r14 = regs.r14;
		result.r15 = regs.r15;

		result.rsp = regs.sp;
		result.rbp = regs.bp;
		result.rip = regs.ip;

		result.orig_rax = regs.orig_ax;
		result.eflags   = regs.eflags;

		result.ds = regs.ds;
		result.es = regs.es;
		result.fs = regs.fs;
		result.gs = regs.gs;
		result.cs = regs.cs;
		result.ss = regs.ss;

		result.fs_base = regs.fs_base;
		result.gs_base = regs.gs_base;
#else
		static_assert(false, "Unsupported architecture.");
#endif
		return result;
	}
}

/// Fork a child process.
int fork() {
	int pid = ::fork();
	if (pid == -1) throw error(-1, {errno, std::system_category()}, "Failed to fork child process");
	return pid;
}

/// Request the parent of this process to trace us.
void trace_me() {
	if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr)) throw error(-1, {errno, std::system_category()}, "Failed to instruct our parent to trace us");
}

/// Attach to a process.
void attach(int pid) {
	if (ptrace(PTRACE_SEIZE, pid, nullptr, PTRACE_O_TRACESYSGOOD)) throw error(pid, {errno, std::system_category()}, "Failed to attach to process");
}

/// Detach from a process.
void detach(int pid) {
	if (ptrace(PTRACE_DETACH, pid, nullptr, nullptr)) throw error(pid, {errno, std::system_category()}, "Failed to detach from process");
}

/// Stop a traced process
void interrupt(int pid) {
	if (ptrace(PTRACE_INTERRUPT, pid, nullptr, nullptr)) throw error(pid, {errno, std::system_category()}, "Failed to interrupt process");
}

/// Resume a stopped process.
void resume(int pid) {
	if (ptrace(PTRACE_CONT, pid, nullptr, nullptr)) throw error(pid, {errno, std::system_category()}, "Failed to continue process");
}

/// Have a stopped process execute one instruction.
void step(int pid) {
	if (ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr)) throw error(pid, {errno, std::system_category()}, "Failed to step process");
}

/// Have a stopped process execute and trap on entry to or exit from a system call.
void step_syscall(int pid) {
	if (ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr)) throw error(pid, {errno, std::system_category()}, "Failed to systcall step process");
}

/// Get the general purpose registers of a process.
registers_t get_registers(int pid) {
	user_regs_struct regs;
	if (ptrace(PTRACE_GETREGS, pid, &regs, &regs) != 0) throw error(pid, {errno, std::system_category()}, "Failed to read registers");
	return from_impl(regs);
}

/// Set the general purpose registers of a process.
void set_registers(int pid, registers_t const & regs) {
	user_regs_struct impl = to_impl(regs);
	if (ptrace(PTRACE_SETREGS, pid, &impl, &impl) != 0) throw error(pid, {errno, std::system_category()}, "Failed to set registers");
}

/// Read from a memory address of a process.
unsigned long read_memory(int pid, std::uintptr_t address) {
	errno = 0;
	unsigned long result = static_cast<unsigned long>(ptrace(PTRACE_PEEKDATA, pid, address, nullptr));
	if (errno) throw error(pid, {errno, std::system_category()}, "Failed to read memory from process");
	return result;
}

/// Write to a memory address of a process.
void write_memory(int pid, std::uintptr_t address, unsigned long value) {
	if (ptrace(PTRACE_POKEDATA, pid, address, value) != 0) throw error(pid, {errno, std::system_category()}, "Failed to write to process memory");
}

/// Copy a block of memory to a traced process.
void memcpy_to(int pid, std::uintptr_t destination, void const * source, std::size_t count) {
	// Copy unsigned longs as long as it fits.
	std::size_t i;
	for (i = 0; i + sizeof(unsigned long) <= count; i += sizeof(unsigned long)) {
		write_memory(pid, destination + i, reinterpret_cast<unsigned long const *>(source)[i / sizeof(unsigned long)]);
	}

	// Read a long but only write the bytes we really want.
	std::size_t leftovers_start = i;
	if (i < count) {
		unsigned long data = read_memory(pid, destination + i);
		for (; i < count; ++i) {
			reinterpret_cast<std::uint8_t *>(&data)[i - leftovers_start] = reinterpret_cast<std::uint8_t const *>(source)[i];
		}
		write_memory(pid, destination + leftovers_start, data);
	}
}

/// Copy a block of memory from a traced process.
void memcpy_from(int pid, void * destination, std::uintptr_t source, std::size_t count) {
	// Copy unsigned longs as long as it fits.
	std::size_t i;
	for (i = 0; i + sizeof(unsigned long) <= count; i += sizeof(unsigned long)) {
		reinterpret_cast<unsigned long *>(destination)[i / sizeof(unsigned long)] = read_memory(pid, source + i);
	}

	// Read a long but only write the bytes we really want.
	std::size_t leftovers_start = i;
	if (i < count) {
		unsigned long data = read_memory(pid, source + i);
		for (; i < count; ++i) {
			reinterpret_cast<std::uint8_t *>(destination)[i] = reinterpret_cast<std::uint8_t *>(&data)[i - leftovers_start];
		}
	}
}

/// Set return address of the current function and return the old address.
std::uintptr_t swap_return_address(int pid, std::uintptr_t address) {
	auto registers = get_registers(pid);

	std::uintptr_t old = read_memory(pid, registers.sp);
	write_memory(pid, registers.sp, address);

	return old;
}

/// Set a breakpoint.
breakpoint breakpoint::set(int pid, std::uintptr_t address) {
	breakpoint result;
	result.pid             = pid;
	result.address         = address;
	result.original_code   = read_memory(pid, address);
	unsigned long new_code = (result.original_code & ~(0xffUL)) | 0xccUL;
	write_memory(pid, address, new_code);

	return result;
}

/// Restore the original code and roll back the instruction pointer.
void breakpoint::restore() {
	write_memory(pid, address, original_code);
	registers_t regs = get_registers(pid);
	--regs.ip;
	set_registers(pid, regs);
}

/// Wait for a traced process to trap.
void wait_for_trap(int pid) {
	while (true) {
		siginfo_t info;
		if (waitid(P_PID, pid, &info, WSTOPPED | WEXITED)) throw error(pid, {errno, std::system_category()}, "Tried to wait for a process that doesn't exist");

		switch (info.si_code) {
		case CLD_EXITED:
		case CLD_KILLED:
		case CLD_DUMPED:
			throw process_terminated(pid, info.si_code == CLD_EXITED, info.si_status, "Process terminated while we were waiting for it to trap");

		case CLD_TRAPPED:
		case CLD_STOPPED:
			if (info.si_status != sigtrap && info.si_status != sigstop) throw unexpected_signal(pid, info.si_status, "Process received unexpected signal");
			return;

		case CLD_CONTINUED:
			continue;
		}
	}
}

/// Wait for a traced process to trap at a specific address.
void wait_for_trap(int pid, std::uintptr_t address) {
	while (true) {
		wait_for_trap(pid);
		registers_t regs = get_registers(pid);
		if (regs.ip - 1 == address) {
			return;
		} else {
			resume(pid);
		}
	}
}
/// Get the address of a trap instruction in this process' memory.
void * get_trap() {
	return trap;
}

/// Wait for a traced process to trap at entry to or exit from a system call.
bool wait_for_syscall(int pid) {
	while (true) {
		siginfo_t info;
		if (waitid(P_PID, pid, &info, WSTOPPED | WEXITED) != 0) throw error(pid, {errno, std::system_category()}, "Tried to wait for a process that doesn't exist");

		switch (info.si_code) {
		case CLD_EXITED:
		case CLD_KILLED:
		case CLD_DUMPED:
			throw process_terminated(pid, info.si_code == CLD_EXITED, info.si_status, "Process terminated while we were waiting for it to trap");

		case CLD_TRAPPED:
		case CLD_STOPPED:
			if (info.si_status == (sigtrap | 0x80)) return true;
			std::cout << "Received unexpected signal " << info.si_status << ": " << strsignal(info.si_status) << ".\n";
			return false;

		case CLD_CONTINUED:
			continue;
		}
	}
}

}
