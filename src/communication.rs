// https://docs.rs/nix/latest/nix/sys/ioctl/
// ioctl commands are defined by the kernel module in "driver/communication.h"
// all our commands being sent will use a 'write' macro because we are writing to dev file
// not necessarily commanding the driver to perform a write function
// the driver function is entirely dictated by the ioctl sequence number

const IOCTL_MAGIC: u8 = b'k';

nix::ioctl_write_ptr!(set_target_process, IOCTL_MAGIC, 1, u16);
nix::ioctl_write_ptr!(read_process_memory, IOCTL_MAGIC, 2, RPMArgs);
nix::ioctl_write_ptr!(write_process_memory, IOCTL_MAGIC, 3, WPMArgs);

#[repr(C)]
pub struct RPMArgs {
    pub address: usize,
    // no reason for this to be signed, but driver's access_process_vm() expects signed int
    pub size: i32,
    pub buffer: *mut usize,
}

#[repr(C)]
pub struct WPMArgs {
    pub address: usize,
    pub size: i32,
    pub write_value: usize,
}
