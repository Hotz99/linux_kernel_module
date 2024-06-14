use anyhow::anyhow;
use std::{fs, io::Write, os::fd::AsRawFd};

const DUMMY_FILE_PATH: &str = "/tmp/testfile.txt";
const DUMMY_FILE_CONTENT: &str = "foo bar baz";

fn create_dummy_process() -> anyhow::Result<()> {
    let mut file = fs::OpenOptions::new()
        .read(true)
        .write(true)
        .create(true)
        .open(DUMMY_FILE_PATH)?;

    file.write_all(DUMMY_FILE_CONTENT.as_bytes())?;

    let fd = file.as_raw_fd();
    let len = DUMMY_FILE_CONTENT.len();

    // https://www.ibm.com/docs/en/i/7.4?topic=ssw_ibm_i_74/apis/mmap.html
    let addr = unsafe {
        nix::libc::mmap(
            std::ptr::null_mut(),
            len,
            nix::libc::PROT_READ | nix::libc::PROT_WRITE,
            nix::libc::MAP_SHARED,
            fd,
            0,
        )
    };

    if addr == nix::libc::MAP_FAILED {
        return Err(anyhow!("mmap failed: {}", std::io::Error::last_os_error()));
    }

    println!("file mapped at address: {:?}", addr);

    std::thread::sleep(std::time::Duration::from_secs(10));

    unsafe {
        nix::libc::munmap(addr, len);
        println!("unmapped file")
    }

    Ok(())
}
