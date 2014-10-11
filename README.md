# Introduction
fdinject is a tool for writing data to any file descriptor of any process.
It uses ptrace to attach to the target process and then makes the that process invoke the required system calls.
The tool requires root rights and currently only works on Linux x86_64.
Support for Linux on x86_32 and ARM are planned for the future.

One use case is to write data to open TCP connection from any process without the difficulty of injecting packets.

# Usage
```
fdinject pid fd
```
This will make fdinject read data from stdin and write it to file descriptor `fd` of the process with PID `pid`.

# Details
fdinject performs the following actions after attaching to the target process:

1. Make the other process call mmap to get a fresh block of memory to hold the injected data.
2. Copy the data to the newly allocated memory.
3. Call write(fd, ...) in the target process untill all data has been written or an error occurs.
4. Unmap the allocated memory in the other process again.

These steps are all implemented by invoking system calls directly to avoid the need to resolve symbol names in the target executable.

Input is buffered until standard input closes.
It is then copied to the target process in one go and written to the file descriptor.
This should probably be changed to write standard input in multiple blocks for large input.
