#pragma once

#include <cstdint>
#include <utility>

#include "signal.hpp"
#include "exceptions.hpp"

namespace dbpp {
#if !defined(__i386__) && !defined(__x86_64__)
static_assert(false,
	"Unknown or unsupported architecture."
	"Only x86 and x86_64 are supported."
	"If this is error is wrong and you know what you're doing you can add -D__i386__ or -D__x84_64__ to the compile command."
);
#endif

using register_t = std::uintptr_t;

/// Class to hold all general purpose registers.
struct registers_t {
	register_t ax;
	register_t bx;
	register_t cx;
	register_t dx;
	register_t si;
	register_t di;

	register_t sp;
	register_t bp;
	register_t ip;

	register_t orig_ax;
	register_t eflags;

	register_t ds;
	register_t es;
	register_t fs;
	register_t gs;
	register_t cs;
	register_t ss;

#if defined(__i386__)

#elif defined(__x86_64__)
	register_t r8;
	register_t r9;
	register_t r10;
	register_t r11;
	register_t r12;
	register_t r13;
	register_t r14;
	register_t r15;

	register_t fs_base;
	register_t gs_base;
#endif

};

/// A breakpoint.
struct breakpoint {
	/// The process in which the breakpoint is set.
	int pid;

	/// The address of the breakpoint.
	std::uintptr_t address;

	/// The original code.
	unsigned long original_code;

	/// Set and return a breakpoint in a process.
	static breakpoint set(int pid, std::uintptr_t address);

	/// Restore the original code and roll back the instruction pointer.
	void restore();
};

/// Struct to hold the result of calling call_sandboxed.
struct call_result {
	int pid;
	registers_t regs_start;
	registers_t regs_end;
};

/// Process state.
struct process_state {
	/// If true, the process was terminated.
	bool terminated;

	/// If true, the process is still alive but stopped.
	bool stopped;

	/// The exit status of the program. Only valid if terminated is true.
	int status;

	/// The signal that killed or stopped the process.
	int signal;

	/// In boolean context, the status is true if the process is still alive.
	explicit operator bool() { return !terminated; };
};

/// Fork a child process.
/**
 * Throws on failure.
 */
int fork();

/// Request the parent of this process to trace us.
/**
 * Throws on failure.
 */
void trace_me();

/// Attach to a process.
/**
 * Throws on failure.
 */
void attach(int pid);

/// Detach from a process.
/**
 * Throws on failure.
 */
void detach(int pid);

/// Stop a traced process
void interrupt(int pid);

/// Resume a trapped child.
/**
 * Throws on failure.
 */
void resume(int pid);

/// Have a traced process execute one instruction.
/**
 * Throws on failure.
 */
void step(int pid);

/// Have a traced process execute and trap on entry to or exit from a system call.
/**
 * Throws on failure.
 */
void step_syscall(int pid);

/// Get the general purpose registers of a process.
/**
 * Throws on failure.
 */
registers_t get_registers(int pid);

/// Set the general purpose registers of a process.
/**
 * Throws on failure.
 */
void set_registers(int pid, registers_t const &);

/// Read from a memory address of a process.
/**
 * Throws on failure.
 */
unsigned long read_memory(int pid, std::uintptr_t address);

/// Write to a memory address of a process.
/**
 * Throws on failure.
 */
void write_memory(int pid, std::uintptr_t address, unsigned long value);

/// Copy a block of memory to a traced process.
/**
 * Throws on failure.
 */
void memcpy_to(int pid, std::uintptr_t destination, void const * source, std::size_t count);

/// Copy a block of memory from a traced process.
/**
 * Throws on failure.
 */
void memcpy_from(int pid, void * destination, std::uintptr_t source, std::size_t count);

/// Set return address of the current function and return the old address.
/**
 * Must be called before anything has been done to the stack by the function.
 * This function does not examine stack frames.
 *
 * Throws on failure.
 */
std::uintptr_t swap_return_address(int pid, std::uintptr_t address);

/// Wait for a traced child to trap.
/**
 * Throws if the child is already dead or if it terminates before it traps.
 */
void wait_for_trap(int pid);

/// Wait for a traced child to trap at a specific address.
/**
 * Throws if the child is already dead or if it terminates before it traps.
 */
void wait_for_trap(int pid, std::uintptr_t address);

/// Wait for a traced child to trap.
/**
 * \return True if the process trapped on entry to or exit from a system call, false if it trapped for another reason.
 * Throws if the child is already dead or if it terminates before it traps.
 */
bool wait_for_syscall(int pid);

/// Get the address of a trap instruction in this process' memory.
void * get_trap();

/// Call a function in a sandbox by first forking and then tracing the child.
template<typename R, typename... A, typename... B>
bool call_sandboxed(call_result & result, R (*f) (A...), B && ...args) {
	result.pid = fork();

	// We're the forked child.
	if (result.pid == 0) {
		// Ask to be traced and stop.
		trace_me();
		raise(sigtrap);

		// Parent should have set a breakpoint now, so execute the function.
		f(std::forward<B>(args)...);

		// TODO: Copy result to parent, somehow.

		// Parent should kill us before we reach this.
		exit(0);

	// We're the parent.
	} else {
		// Wait for initial stop signal.
		wait_for_trap(result.pid);
		auto breakpoint = dbpp::breakpoint::set(result.pid, reinterpret_cast<std::uintptr_t>(f));
		resume(result.pid);

		// Wait for function start.
		wait_for_trap(result.pid, breakpoint.address);
		breakpoint.restore();
		result.regs_start = get_registers(result.pid);
		write_memory(result.pid, result.regs_start.sp, reinterpret_cast<std::uintptr_t>(get_trap()));
		resume(result.pid);

		// Wait for function return.
		wait_for_trap(result.pid, reinterpret_cast<std::uintptr_t>(get_trap()));
		result.regs_end = get_registers(result.pid);

		return true;
	}
}

}
