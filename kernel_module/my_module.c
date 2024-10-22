#include "my_module.h"

// static extends the lifetime of the variable to the duration of the program
// task_struct represents a process in the kernel
// akin to PEPROCESS in Windows
static struct task_struct *target_task;

static int set_target_task_by_pid(uint16_t target_pid) {
  struct pid *pid_struct = find_get_pid(target_pid);

  if (!pid_struct) {
    printk(KERN_INFO DEVICE_NAME ": process with pid %d not found\n",
           target_pid);
    return 1;
  }

  target_task = get_pid_task(pid_struct, PIDTYPE_PID);

  if (!target_task) {
    printk(KERN_INFO DEVICE_NAME ": task_struct with pid %d not found\n",
           target_pid);
    return 1;
  }

  printk(KERN_INFO DEVICE_NAME ": found process with pid %d: %s\n", target_pid,
         target_task->comm);

  put_pid(pid_struct);

  return 0;
}

static int read_process_memory(RPMArgs_t args) {
  printk(KERN_INFO DEVICE_NAME ": RPM --> addr : 0x%lx, size : %ld\n",
         args.address, args.size);

  if (!args.address) {
    printk(KERN_ALERT DEVICE_NAME ": invalid read address\n");
    return 1;
  }
  if (!target_task) {
    printk(KERN_ALERT DEVICE_NAME ": invalid target task\n");
    return 1;
  }

  uintptr_t read_value = 0;

  // https://elixir.bootlin.com/linux/v6.9.3/source/mm/memory.c#L6062
  if (access_process_vm(target_task, args.address, &read_value, args.size, 0) !=
      args.size) {
    printk(KERN_ALERT DEVICE_NAME ": failed to read value at 0x%lx\n",
           args.address);

    return 1;
  }

  printk(KERN_INFO DEVICE_NAME ": read value at 0x%lx: %lu\n", args.address,
         read_value);

  return read_value;
}

static int write_process_memory(WPMArgs_t args) {
  printk(KERN_INFO DEVICE_NAME
         ": WPM --> addr : 0x%lx, size : %ld, value : %ld\n",
         args.address, args.size, args.write_value);

  if (!args.address) {
    printk(KERN_ALERT DEVICE_NAME ": invalid write address\n");
    return 1;
  }
  if (!target_task) {
    printk(KERN_ALERT DEVICE_NAME
           ": target task is not set, make sure set target process first\n");
    return 1;
  }

  struct mm_struct *memory_map = get_task_mm(target_task);

  if (!memory_map) {
    printk(KERN_ALERT DEVICE_NAME
           ": failed to get mm_struct for target_task\n");
    return 1;
  }

  // vm = virtual memory
  if (access_process_vm(target_task, args.address, &args.write_value, args.size,
                        1) != args.size) {
    printk(KERN_ALERT, DEVICE_NAME ": failed to write value at 0x%lx\n",
           args.address);
    return 1;
  }

  printk(KERN_INFO DEVICE_NAME ": wrote value %lu to 0x%lx\n", args.write_value,
         args.address);

  mmput(memory_map);

  return 0;
}

static int open_device(struct inode *inode, struct file *file) {
  printk(KERN_INFO DEVICE_NAME ": opened device file\n");
  return 0;
}

static int release_device(struct inode *inode, struct file *file) {
  printk(KERN_INFO DEVICE_NAME ": released device file\n");
  return 0;
}

static ssize_t read_device(struct file *file, char *buffer, size_t length,
                           loff_t *offset) {
  printk(KERN_INFO DEVICE_NAME ": read device file\n");
  return 0;
}

static ssize_t write_device(struct file *file, const char *buffer,
                            size_t length, loff_t *offset) {
  printk(KERN_INFO DEVICE_NAME ": wrote to device file\n");
  return 0;
}

// https://archive.kernel.org/oldlinux/htmldocs/kernel-api/API---copy-from-user.html
static long device_ioctl(struct file *device_file, unsigned int ioctl_command,
                         unsigned long arg) {
  printk(KERN_INFO DEVICE_NAME ": received IOCTL command\n");

  struct WPMArgs wpm_args;
  struct RPMArgs rpm_args;

  uint16_t pid;

  switch (ioctl_command) {
  case IOCTL_SET_TARGET_PROCESS:
    // bad: set_target_address_by_pid((int)arg);
    // good:
    if (copy_from_user(&pid, (uint16_t *)arg, sizeof(uint16_t))) {
      printk(KERN_ALERT DEVICE_NAME ": failed to copy pid from user\n");
      // kernel error codes are negative by convention
      return -EFAULT;
    }

    return set_target_task_by_pid(pid);

  case IOCTL_RPM:
    if (copy_from_user(&rpm_args, (RPMArgs_t *)arg, sizeof(RPMArgs_t))) {
      printk(KERN_ALERT DEVICE_NAME ": failed to copy rpmargs from user\n");
      return -EFAULT;
    }

    // https://www.cs.bham.ac.uk/~exr/lectures/opsys/13_14/docs/kernelAPI/r3818.html
    // copy the read value to userland
    put_user(read_process_memory(rpm_args), rpm_args.buffer);

    break;

  case IOCTL_WPM:
    if (copy_from_user(&wpm_args, (WPMArgs_t *)arg, sizeof(WPMArgs_t))) {
      printk(KERN_ALERT DEVICE_NAME ": failed to copy wpmargs from user\n");
      return -EFAULT;
    }

    return write_process_memory(wpm_args);

  default:
    printk(KERN_ALERT DEVICE_NAME ": invalid ioctl command\n");
    return -EINVAL;
  }

  return 0;
}

// field names must match the struct file_operations in linux/fs.h
static struct file_operations fileOps = {
    .open = open_device,
    .release = release_device,
    .read = read_device,
    .write = write_device,
    .unlocked_ioctl = device_ioctl,
};

static uint8_t MAJOR_NUMBER;

static int __init my_module_init(void) {
  MAJOR_NUMBER = register_chrdev(0, DEVICE_NAME, &fileOps);
  if (MAJOR_NUMBER < 0) {
    printk(KERN_ALERT DEVICE_NAME ": registering char device failed with %d\n",
           MAJOR_NUMBER);
    return MAJOR_NUMBER;
  }

  printk(KERN_INFO DEVICE_NAME ": registered with major number %d\n",
         MAJOR_NUMBER);

  printk(KERN_INFO DEVICE_NAME ": module loaded\n");

  return 0;
}

static void __exit my_module_exit(void) {
  // release the target task
  if (target_task) {
    put_task_struct(target_task);
  }

  unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
  printk(KERN_INFO DEVICE_NAME ": module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
