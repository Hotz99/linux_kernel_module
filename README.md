# Why ?
I wanted to learn about Linux kernel level programs and how they interact with usermode programs.

# What ?
Linux kernel module written in C, allows for reading and writing memory of a given target process, interfacing with a user-space program via IOCTL commands through a character device file.

# Getting started
1. Run `kernel_module/install.sh` once.
    - The module's `register_chrdev()` dynamically generates a major number for the device file. The major number is printed to the kernel log when the module is loaded.
    - Read the used number from the kernel log and adjust `MAJOR_NUMBER` in `install.sh` to match.
2. Run `kernel_module/install.sh` again with the correct major number.
3. Run `src/main.rs`


## IOCTL Communication Workflow
### Device File
1. communication endpoint where IOCTL commands are sent and received (not buffered)
2. interface between the user space and kernel space
3. buffers data between the user space and kernel space i.e. read and written data to and from the kernel module

### User Space program
1. gets a file descriptor (fd) of the device file in `/dev/`
2. uses the ioctl system call on the fd to send commands to the kernel module

### Kernel Module:
1. implements `unlocked_ioctl` function that is called when an IOCTL request is made from user space
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
