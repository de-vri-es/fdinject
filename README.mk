fdinject is a tool for writing data to any file descriptor of any process.

It uses ptrace to attach to the target process and invoke system calls inside that process.

The tool requires root rights and currently only works on Linux with x86_64. Support for x86_32 is plannen, and possibly ARM in the future.
