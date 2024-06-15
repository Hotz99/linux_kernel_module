mod communication;

use std::fs;
use std::io::{BufRead, BufReader};
use std::os::unix::io::AsRawFd;

use anyhow::anyhow;
use std::path::PathBuf;

const TARGET_PROCESS_NAME: &str = "firefox";

fn main() {
    let dev_file_path: &str = "/dev/my_module";

    let file = fs::OpenOptions::new()
        .read(true)
        .write(true)
        .open(dev_file_path)
        .expect("failed to open device file");

    // https://en.wikipedia.org/wiki/File_descriptor
    let file_descriptor = file.as_raw_fd();

    let target_pid = match get_pid(TARGET_PROCESS_NAME) {
        Ok(pid) => pid,
        Err(e) => {
            eprintln!("{}", e);
            return;
        }
    };

    println!("target pid: {}", target_pid);

    unsafe {
        if let Err(e) = communication::set_target_process(file_descriptor, &target_pid) {
            eprintln!("ioctl set_target_process failed: {}", e);
            return;
        };

        let target_base_address = match get_base_address(&target_pid) {
            Ok(result) => result,
            Err(e) => {
                eprintln!("{}", e);
                return;
            }
        };

        println!("target base address: 0x{:x}", target_base_address);
    }
}

fn get_pid(target_process_name: &str) -> anyhow::Result<u16> {
    let system = sysinfo::System::new_all();
    let process = system
        .processes_by_exact_name(target_process_name)
        .min_by_key(|p| p.pid());

    match process {
        Some(process) => {
            return Ok(process.pid().as_u32() as u16);
        }
        None => Err(anyhow!("failed to find process: {}", target_process_name)),
    }
}

// https://stackoverflow.com/questions/1401359/understanding-linux-proc-pid-maps-or-proc-self-maps
fn get_base_address(pid: &u16) -> anyhow::Result<usize> {
    let file = fs::OpenOptions::new()
        .read(true)
        .open(format!("/proc/{}/maps", pid))?;

    let reader = BufReader::new(file);

    let exe_path: String;

    match get_exe_path(pid) {
        Ok(result) => exe_path = result,
        Err(e) => {
            eprintln!("{}", e);
            return Err(anyhow!("failed to get executable path"));
        }
    };

    println!("exe path: {}", exe_path);

    for line in reader.lines() {
        let line = line?;
        let parts: Vec<&str> = split_maps_line(line.as_str());

        if parts.len() < 6 {
            continue;
        }

        let pathname = parts[5].trim();

        if pathname != exe_path {
            continue;
        }

        let address_range: Vec<&str> = parts[0].split('-').collect();

        if let Ok(base_address) = usize::from_str_radix(address_range[0], 16) {
            return Ok(base_address);
        }
    }

    Err(anyhow!("failed to find base address"))
}

// e.g.: /proc/1234/exe -> /usr/bin/firefox
fn get_exe_path(pid: &u16) -> anyhow::Result<String> {
    let exe_path = PathBuf::from(format!("/proc/{}/exe", pid));
    match fs::read_link(exe_path) {
        Ok(result) => {
            let pathname = result.to_string_lossy().into_owned();

            Ok(pathname)
        }

        Err(e) => Err(anyhow!("failed to get executable path: {}", e)),
    }
}

fn split_maps_line(line: &str) -> Vec<&str> {
    // split into 6 parts: the first five columns plus the rest as the sixth column
    let parts: Vec<&str> = line
        .splitn(6, ' ')
        // filter out empty parts caused by multiple spaces
        .filter(|part| !part.is_empty() && *part != " ")
        .collect();
    parts
}
