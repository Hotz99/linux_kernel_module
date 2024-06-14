#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/slab.h>

#define DRIVER
#define DEVICE_NAME "my_module"
#define BUFFER_SIZE 1024

// TODO move this to a header file, was getting errors when I tried
// something to do with using types from the system headers vs types from the kernel headers

// _IOW means userland is writing and kernel is reading
// 'k' is the magic number: used to identify the ioctl
// the below are macros: https://gcc.gnu.org/onlinedocs/cpp/Macros.html
#define IOCTL_SET_TARGET_PROCESS _IOW('k', 1, uint16_t)
#define IOCTL_RPM _IOW('k', 2, RPMArgs_t)
#define IOCTL_WPM _IOW('k', 3, WPMArgs_t)

struct RPMArgs
{
    uintptr_t address;
    uint32_t size;
    uintptr_t *buffer;
} typedef RPMArgs_t;

struct WPMArgs
{
    uintptr_t address;
    uint32_t size;
    uintptr_t write_value;
} typedef WPMArgs_t;

static uint8_t MAJOR_NUMBER;
// static extends the lifetime of the variable to the duration of the program
// task_struct represents a process in the kernel
// analogous to PEPROCESS in Windows
static struct task_struct *target_task;

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO DEVICE_NAME ": device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO DEVICE_NAME ": device closed\n");
    return 0;
}

static ssize_t device_read(struct file *file, char *buffer, size_t length,
                           loff_t *offset)
{
    printk(KERN_INFO DEVICE_NAME ": device read\n");
    return 0;
}

static ssize_t device_write(struct file *file, const char *buffer,
                            size_t length, loff_t *offset)
{
    printk(KERN_INFO DEVICE_NAME ": device write\n");
    return 0;
}

static int set_target_task_by_pid(uint16_t target_pid)
{
    struct pid *pid_struct = find_get_pid(target_pid);

    if (!pid_struct)
    {
        printk(KERN_INFO DEVICE_NAME ": process with pid %d not found\n",
               target_pid);
        return 1;
    }

    target_task = get_pid_task(pid_struct, PIDTYPE_PID);

    if (!target_task)
    {
        printk(KERN_INFO DEVICE_NAME ": task_struct with pid %d not found\n",
               target_pid);
        return 1;
    }

    printk(KERN_INFO DEVICE_NAME ": found process with pid %d: %s\n",
           target_pid, target_task->comm);

    put_pid(pid_struct);

    return 0;
}

static int rpm(RPMArgs_t args)
{
    printk(KERN_INFO DEVICE_NAME ": RPM --> addr : 0x%lx, size : %ld\n",
           args.address, args.size);

    if (!args.address)
    {
        printk(KERN_ALERT DEVICE_NAME ": invalid read address\n");
        return 1;
    }
    if (!target_task)
    {
        printk(KERN_ALERT DEVICE_NAME ": invalid target task\n");
        return 1;
    }

    uintptr_t read_value = 0;

    // https://elixir.bootlin.com/linux/v6.9.3/source/mm/memory.c#L6062
    if (access_process_vm(target_task, args.address, &read_value, args.size, 0) != args.size)
    {
        printk(KERN_ALERT DEVICE_NAME ": failed to read value at 0x%lx\n",
               args.address);

        return 1;
    }

    printk(KERN_INFO DEVICE_NAME ": read value at 0x%lx: %lu\n",
           args.address, read_value);

    return read_value;
}

static int wpm(WPMArgs_t args)
{
    printk(KERN_INFO DEVICE_NAME
           ": WPM --> addr : 0x%lx, size : %ld, value : %ld\n",
           args.address, args.size, args.write_value);

    if (!args.address)
    {
        printk(KERN_ALERT DEVICE_NAME ": invalid write address\n");
        return 1;
    }
    if (!target_task)
    {
        printk(KERN_ALERT DEVICE_NAME
               ": target task is not set, make sure set target process first\n");
        return 1;
    }

    struct mm_struct *memory_map = get_task_mm(target_task);

    if (!memory_map)
    {
        printk(KERN_ALERT DEVICE_NAME
               ": failed to get mm_struct for target_task\n");
        return 1;
    }

    // vm = virtual memory
    if (access_process_vm(target_task, args.address, &args.write_value, args.size,
                          1) != args.size)
    {
        printk(KERN_ALERT, DEVICE_NAME ": failed to write value at 0x%lx\n",
               args.address);
        return 1;
    }

    printk(KERN_INFO DEVICE_NAME ": wrote value %lu to 0x%lx\n", args.write_value,
           args.address);

    mmput(memory_map);

    return 0;
}

// https://archive.kernel.org/oldlinux/htmldocs/kernel-api/API---copy-from-user.html
static long device_ioctl(struct file *device_file, unsigned int ioctl_command,
                         unsigned long arg)
{
    printk(KERN_INFO DEVICE_NAME ": IOCTL called\n");

    struct WPMArgs wpm_args;
    struct RPMArgs rpm_args;

    int pid;

    switch (ioctl_command)
    {
    case IOCTL_SET_TARGET_PROCESS:
        // bad: set_target_address_by_pid((int)arg);
        // good:
        if (copy_from_user(&pid, (uint16_t *)arg, sizeof(uint16_t)))
        {
            printk(KERN_ALERT DEVICE_NAME ": failed to copy pid from user\n");
            return EFAULT;
        }

        return set_target_task_by_pid(pid);

    case IOCTL_RPM:
        if (copy_from_user(&rpm_args, (RPMArgs_t *)arg, sizeof(RPMArgs_t)))
        {
            printk(KERN_ALERT DEVICE_NAME ": failed to copy rpmargs from user\n");
            return EFAULT;
        }

        // https://www.cs.bham.ac.uk/~exr/lectures/opsys/13_14/docs/kernelAPI/r3818.html
        // copy the read value to userland
        put_user(rpm(rpm_args), rpm_args.buffer);

        break;

    case IOCTL_WPM:
        if (copy_from_user(&wpm_args, (WPMArgs_t *)arg, sizeof(WPMArgs_t)))
        {
            printk(KERN_ALERT DEVICE_NAME ": failed to copy wpmargs from user\n");
            return EFAULT;
        }

        return wpm(wpm_args);

    default:
        printk(KERN_ALERT DEVICE_NAME ": invalid ioctl command\n");
        return EINVAL;
    }

    return 0;
}

// field names must match the struct file_operations in linux/fs.h
static struct file_operations fileOps = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init baldwin_init(void)
{
    MAJOR_NUMBER = register_chrdev(0, DEVICE_NAME, &fileOps);
    if (MAJOR_NUMBER < 0)
    {
        printk(KERN_ALERT DEVICE_NAME ": registering char device failed with %d\n",
               MAJOR_NUMBER);
        return MAJOR_NUMBER;
    }

    printk(KERN_INFO DEVICE_NAME ": registered with major number %d\n", MAJOR_NUMBER);

    printk(KERN_INFO DEVICE_NAME ": module loaded\n");

    return 0;
}

static void __exit baldwin_exit(void)
{
    // release the target task
    if (target_task)
    {
        put_task_struct(target_task);
    }

    unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
    printk(KERN_INFO DEVICE_NAME ": module unloaded\n");
}

module_init(baldwin_init);
module_exit(baldwin_exit);

MODULE_LICENSE("GPL");