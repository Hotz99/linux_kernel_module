#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define DRIVER
#define DEVICE_NAME "my_module"

// _IOW means userland is writing and kernel is reading
// 'k' is the magic number: used to identify the ioctl
// the below are macros: https://gcc.gnu.org/onlinedocs/cpp/Macros.html
#define IOCTL_SET_TARGET_PROCESS _IOW('k', 1, uint16_t)
#define IOCTL_RPM _IOW('k', 2, RPMArgs_t)
#define IOCTL_WPM _IOW('k', 3, WPMArgs_t)

struct RPMArgs {
  // size is dependent on the platform's architecture, not the target process
  // being read/written
  uintptr_t address;
  uint32_t size;
  uintptr_t *buffer;
} typedef RPMArgs_t;

struct WPMArgs {
  uintptr_t address;
  uint32_t size;
  uintptr_t write_value;
} typedef WPMArgs_t;
