# Why ?
I wanted to learn about Linux kernel level programs and how they interact with usermode programs.

# What ?

Linux kernel module written in C, allows for reading and writing memory of a given target process, interfacing with a user-space program via IOCTL commands through a character device file.

# Getting started

1. Run `install.sh` once.
    1.1. The module's `register_chrdev()` dynamically generates a major number for the device file. The major number is printed to the kernel log when the module is loaded.
    1.2. Read the used number from the kernel log and adjust `MAJOR_NUMBER` in `install.sh` to match.
2. Run `install.sh` again with the correct major number.
3. Run `src/main.rs`


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

## Kernel Module Features

- **Target Process Selection**: Allows for setting a target process using a PID, and internally tracks the target using a `task_struct`. 
- **Reading Process Memory**: Uses `access_process_vm` to read memory from the target process, validating the memory address and the target task.
- **Writing Process Memory**: Writes to a specified memory address in the target process using `access_process_vm`, ensuring the process' memory mapping (`mm_struct`) is valid.
- **IOCTL Command Handling**: Supports custom IOCTL commands to set the target process, read process memory, and write process memory.

### Device File Operations

- **open**: Handles opening the device file.
- **release**: Handles releasing the device file.
- **read** and **write**: Basic implementations for reading from and writing to the device file.
- **unlocked_ioctl**: Handles IOCTL commands, including setting the target process, reading memory, and writing memory.

### IOCTL Commands

- **IOCTL_SET_TARGET_PROCESS**: Sets the target process using a PID passed from user space.
- **IOCTL_RPM**: Reads a value from the target process's memory and copies it to user space.
- **IOCTL_WPM**: Writes a value to the target process's memory from user space.

### Initialization and Cleanup

- **Module Initialization**: Registers a character device and initializes the target process handler.
- **Module Cleanup**: Releases any held `task_struct` and unregisters the character device.


# C Study Notes

Some notes I wrote while learning relevant C concepts for this project.

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
