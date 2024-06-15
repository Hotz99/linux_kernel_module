# Why ?
Wanted to learn about Linux kernel level programs and how they interact with usermode programs.

# What ?

- Linux kernel module written in C, featuring process virtual memory reading and writing. 

- Usermode program written in Rust:
    - finds the PID of the target process
    - finds the base address of the target process
    - makes `ioctl` syscalls to the kernel module, leveraging the `nix` crate

# Getting started
1. read the comments in `kernel_module/install.sh`
2. run `kernel_module/install.sh`
3. run `src/main.rs`

For proper syntax highlighting, include the necessary headers in `.vscode/c_cpp_properties.json`.

# NOTES: Interfacing C kernel module with Rust app

## IOCTL communication workflow
#### User Mode (UM) app
- opens the device file in /dev to get a file descriptor (fd)
- uses the ioctl system call on the fd to send specific commands to the kernel module

#### Device file
- is the communication endpoint through which IOCTL commands are sent and received
- doesn't hold or buffer the IOCTL commands but serves as a conduit for communication
- interfaces with both UM app and driver

#### Kernel Mode (KM) module (or driver)
- implements an unlocked_ioctl function (or compat_ioctl for compatibility commands) that is called when an ioctl request is made from user space
- reads the IOCTL command directly and processes it accordingly

# NOTES: C concepts

## Pointers
A pointer variable holds a memory address defining a location in memory where data is stored.

1. CPU encounters a pointer variable
2. reads the memory address stored in the pointer
3. uses this address to access the physical memory location
4. reads/writes the value

## `static` functions and variables
- `static foo()` limits the scope of the function to the file it is defined in
- `static foo_t foo` extends the lifetime of `foo` to the duration of the program

## uintptr_t
- a variable of this type may hold a pointer to `void` or a `uint`
- we use it to hold memory addresses, since these are simply **base 16 numeric values**, having a corresponding `uint` representation

# NOTES: Memory concepts

## Virtual vs. Physical addresses
#### Virtual
- used by applications to access memory, located within the virtual address space provided by the operating system for each process
- used in pointer arithmetic, memory allocation and accessing variables and data structures within a program

#### Physical
- actual addresses on the physical RAM chips
- only the operating system and hardware deal directly with physical addresses

## Base and limit addresses
Define the start and end of a memory region allocated to a given process.