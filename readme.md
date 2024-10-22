# Why ?
I wanted to learn about Linux kernel level programs and how they interact with usermode programs.

# What ?

- Linux kernel module written in C, featuring process virtual memory reading and writing, ordered through IOCTL commands. 

- Usermode program written in Rust:
    - finds the PID of the target process
    - finds the base address of the target process
    - makes `ioctl` syscalls to the kernel module device interface, leveraging the `nix` crate

# Getting started
1. read the comments in `kernel_module/install.sh`
2. run `kernel_module/install.sh`
3. run `src/main.rs`

# STUDY NOTES

## IOCTL Communication Workflow
    Device File:
        1. communication endpoint where IOCTL commands are sent and received (not buffered)
        2. interface between the user space and kernel space
        3. buffers data between the user space and kernel space i.e. read and written data to and from the kernel module

    User Mode (UM) program:
        1. gets a file descriptor (fd) of the device file in `/dev/`
        2. uses the ioctl system call on the fd to send commands to the kernel module

    Kernel Module (KM):
        1. implements an unlocked_ioctl function (or compat_ioctl for compatibility commands) that is called when an ioctl request is made from user space
        2. reads the IOCTL command directly and processes it accordingly

## Making Unix system calls
    Leverages the crate `nix` to make syscalls (ioctl in this case)

## Pointers
    1. A pointer variable holds a memory address. This address points to a location in memory where data is stored
    2. When the CPU encounters a pointer variable, it reads the memory address stored in the pointer
    3. The CPU uses this address to access the physical memory location
    4. It then reads/writes the value

## `static` functions and variables
    - `static foo()` limits the scope of the function to the file it is defined in
    - `static foo_t foo` extends the lifetime of `foo` to the duration of the program

## uintptr_t
    A variable of this type may hold a pointer to void or a uint. We use it for memory addresses, since these are simply **hex based numeric values**, having a corresponding uint representation.

## Virtual vs. Physical Addresses
    Virtual:
        Used by applications to access memory, located within the virtual address space provided by the operating system for each process.
        Used in pointer arithmetic, memory allocation and accessing variables and data structures within a program.

    Physical:
        Actual addresses on the physical RAM chips.
        Only the operating system and hardware deal directly with physical addresses.

## Base and limit addresses
    Define the start and end of a memory region allocated to a given process
