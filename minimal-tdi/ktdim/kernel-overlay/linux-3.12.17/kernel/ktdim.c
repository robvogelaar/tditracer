
#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/types.h>

static DEFINE_SPINLOCK(ktdim_lock);

#define KTDIM_IOCTL_GET_TRACEBUFFERSIZE _IO('T', 0)
#define KTDIM_IOCTL_REWIND _IO('T', 1)
#define KTDIM_IOCTL_ON _IO('T', 2)
#define KTDIM_IOCTL_OFF _IO('T', 3)

/*
 * [TDIT]
 * [RACE]
 * [    ]timeofday_start.tv_usec
 * [    ]timeofday_start.tv_sec
 * [    ]clock_monotonic_start.tv_nsec
 * [    ]clock_monotonic_start.tv_sec
 * ------
 * [    ]marker, lower 2 bytes is total length in dwords,
 * [    ]clock_monotonic_timestamp.tv_nsec
 * [    ]clock_monotonic_timestamp.tv_sec
 * [    ]<optional> numbers
 * [    ]<optional> text, padded with 0's to multiple of 4 bytes
 * ...
 * ------
 */

typedef unsigned long long _u64;

static u32 gtracebuffersize;
static u8 gtditrace_enabled;
static char *gtrace_buffer;
static unsigned int *gtrace_buffer_dword_ptr;

static struct page *gtracebuffer_shared_page;

static int tditrace_create_buffer(void) {
  /*
   *
    [TDIT]
   * [RACE]
   * [    ]timeofday_start.tv_usec
   * [    ]timeofday_start.tv_sec
   * [    ]clock_monotonic_start.tv_nsec
   * [    ]clock_monotonic_start.tv_sec
   * ------
   * [    ]marker, lower 2 bytes is total length in dwords
   * [    ]clock_monotonic_timestamp.tv_nsec
   * [    ]clock_monotonic_timestamp.tv_sec
   * [    ]text, padded with 0 to multiple of 4 bytes
   * ...
   * ------
   */

  unsigned int *p;
  _u64 atimeofday_start;
  _u64 amonotonic_start;

  gtracebuffersize = 4 * 1024 * 1024;

  gtrace_buffer =
      (char *)__get_free_pages(GFP_KERNEL, get_order(gtracebuffersize));

  gtracebuffer_shared_page = virt_to_page(gtrace_buffer);

  if (gtrace_buffer == 0)
    printk("ktdim: unable to allocate %dMB tracebuffer\n",
           gtracebuffersize / (1024 * 1024));

  memset(gtrace_buffer, 0, gtracebuffersize);

  printk("ktdim: allocated %dMB @0x%08x tracebuffer\n",
         gtracebuffersize / (1024 * 1024), (u32)gtrace_buffer);

  gtrace_buffer_dword_ptr = (unsigned int *)gtrace_buffer;

  /*
   * write one time start text
   */
  sprintf((char *)gtrace_buffer_dword_ptr, (char *)"TDITRACE");
  gtrace_buffer_dword_ptr += 2;

  p = gtrace_buffer_dword_ptr;

  do_gettimeofday((struct timeval *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  do_posix_clock_monotonic_gettime((struct timespec *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  atimeofday_start = (_u64)*p++ * 1000000000;
  atimeofday_start += (_u64)*p++ * 1000;

  amonotonic_start = (_u64)*p++ * 1000000000;
  amonotonic_start += (_u64)*p++;

  *gtrace_buffer_dword_ptr = 0;

  gtditrace_enabled = 1;

  return 0;
}

static void tditrace_internal(va_list args, const char *format);

void tditrace(const char *format, ...) {
  va_list args;

  va_start(args, format);

  tditrace_internal(args, format);

  va_end(args);
}

EXPORT_SYMBOL(tditrace);

static void tditrace_internal(va_list args, const char *format) {

  unsigned int trace_text[512 / 4];
  unsigned int i;
  char *trace_text_ptr;
  unsigned int *trace_text_dword_ptr;
  char ch;
  struct timespec mytime;
  int nr_textdwords;

  unsigned long flags;

  if (!gtditrace_enabled) {
    return;
  }

  /*
   * take and store timestamp
   */
  do_posix_clock_monotonic_gettime(&mytime);
  /*
   * parse the format string
   */
  trace_text_ptr = (char *)trace_text;
  trace_text_dword_ptr = (unsigned int *)trace_text;

  while ((ch = *(format++))) {
    if (ch == '%') {
      switch (ch = (*format++)) {
      case 's': {
        char *s;
        s = va_arg(args, char *);
        if (s) {
          int i = 0;
          while (*s) {
            *trace_text_ptr++ = *s++;
            i++;
            if (i > 256)
              break;
          }
        } else {
          *trace_text_ptr++ = 'n';
          *trace_text_ptr++ = 'i';
          *trace_text_ptr++ = 'l';
          *trace_text_ptr++ = 'l';
        }
        break;
      }
      case 'd': {
        int n = 0;
        unsigned int d = 1;
        int num = va_arg(args, int);
        if (num < 0) {
          num = -num;
          *trace_text_ptr++ = '-';
        }

        while (num / d >= 10)
          d *= 10;

        while (d != 0) {
          int digit = num / d;
          num %= d;
          d /= 10;
          if (n || digit > 0 || d == 0) {
            *trace_text_ptr++ = digit + '0';
            n++;
          }
        }
        break;
      }
      case 'u': {
        int n = 0;
        unsigned int d = 1;
        unsigned int num = va_arg(args, int);

        while (num / d >= 10)
          d *= 10;

        while (d != 0) {
          int digit = num / d;
          num %= d;
          d /= 10;
          if (n || digit > 0 || d == 0) {
            *trace_text_ptr++ = digit + '0';
            n++;
          }
        }
        break;
      }

      case 'x':
      case 'p': {
        int n = 0;
        unsigned int d = 1;
        unsigned int num = va_arg(args, int);

        while (num / d >= 16)
          d *= 16;

        while (d != 0) {
          int dgt = num / d;
          num %= d;
          d /= 16;
          if (n || dgt > 0 || d == 0) {
            *trace_text_ptr++ = dgt + (dgt < 10 ? '0' : 'a' - 10);
            ++n;
          }
        }
        break;
      }

      default:
        break;
      }

    } else {
      *trace_text_ptr++ = ch;
    }
  }

  while ((unsigned int)trace_text_ptr & 0x3)
    *trace_text_ptr++ = 0;

  nr_textdwords = (trace_text_ptr - (char *)trace_text) >> 2;

  /*
   * store into tracebuffer
   */

  spin_lock_irqsave(&ktdim_lock, flags);

  /*
   * marker, 4 bytes
   *       bytes 1+0 hold total length in dwords : 3 (marker,sec,nsec) +
   *                                               nr_dwordtext
   */

  *gtrace_buffer_dword_ptr++ = (3 + nr_textdwords);
  *gtrace_buffer_dword_ptr++ = mytime.tv_sec;
  *gtrace_buffer_dword_ptr++ = mytime.tv_nsec;

  i = nr_textdwords;
  while (i--) {
    *gtrace_buffer_dword_ptr++ = *trace_text_dword_ptr++;
  }

  /*
   * mark the next marker as invalid
   */
  *gtrace_buffer_dword_ptr = 0;

  if (((unsigned int)gtrace_buffer_dword_ptr - (unsigned int)gtrace_buffer) >
      (gtracebuffersize - 1024)) {
    gtditrace_enabled = 0;
  }

  spin_unlock_irqrestore(&ktdim_lock, flags);

  if (gtditrace_enabled == 0)
    printk("ktdim: full\n");
}

static void tditrace_rewind(void) {

  gtditrace_enabled = 0;

  printk("ktdim: rewind\n");

  gtrace_buffer_dword_ptr = (unsigned int *)gtrace_buffer;

  /*
   * write one time start text
   */
  sprintf((char *)gtrace_buffer_dword_ptr, (char *)"TDITRACE");
  gtrace_buffer_dword_ptr += 2;

  do_gettimeofday((struct timeval *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  do_posix_clock_monotonic_gettime((struct timespec *)gtrace_buffer_dword_ptr);
  gtrace_buffer_dword_ptr += 2;

  *gtrace_buffer_dword_ptr = 0;

  gtditrace_enabled = 1;
}

static long ktdim_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static int ktdim_Mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations gktdimFops = {.unlocked_ioctl = ktdim_Ioctl,
                                            .mmap = ktdim_Mmap};

static struct miscdevice gktdimMiscDev = {
    .minor = MISC_DYNAMIC_MINOR, .name = "ktdim", .fops = &gktdimFops};

static long ktdim_Ioctl(struct file *filp, unsigned int cmd,
                        unsigned long arg) {
  switch (cmd) {
  case KTDIM_IOCTL_GET_TRACEBUFFERSIZE:
    return gtracebuffersize;

  case KTDIM_IOCTL_REWIND:
    tditrace_rewind();
    return 0;

  case KTDIM_IOCTL_ON:
    gtditrace_enabled = 1;
    printk("ktdim: on\n");
    return 0;

  case KTDIM_IOCTL_OFF:
    gtditrace_enabled = 0;
    printk("ktdim: off\n");
    return 0;

  default:
    printk("ktdim: unknown ioctl:0x%08x\n", cmd);
    return -ENOSYS;
  }
}

static int ktdim_Mmap(struct file *file, struct vm_area_struct *vma) {
  unsigned int size;

  if (vma->vm_pgoff != 0) {
    return -EINVAL;
  }

  size = vma->vm_end - vma->vm_start;
  if (size != PAGE_ALIGN(gtracebuffersize)) {
    printk("ktdim: incorrect mmap size\n");
    return -EINVAL;
  }

  /* Prevent the swapper from considering these pages for swap and touching
   * them
   */
  vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP | VM_DONTEXPAND;

  return remap_pfn_range(vma, vma->vm_start,
                         page_to_pfn(gtracebuffer_shared_page), size,
                         vma->vm_page_prot);
}

static void rev_parse_args(char *str, u8 *argc, char **argv, int max_arg) {
  char *ps = str;
  char *token;
  const char delimiters[] = " \t\n";

  while ((token = strsep(&ps, delimiters)) != NULL) {
#if 0
    printk("[%s]\n", token);
#endif
    argv[*argc] = token;
    (*argc)++;
  }
}

int proc_ktdim_status(struct seq_file *sf, void *v) {
  int idx;
  for (idx = 0; idx < 3; idx++) {
    seq_printf(sf, "status(%d):\n", idx);
  }
  return 0;
}

static int do_something(void *kvm, u32 len) {
  void *x;
  if (!kvm || !len)
    return -1;

  for (x = kvm; x < (kvm + len); x += PAGE_SIZE) {
    memset(x, 0xff, 0x10);
  }
  return 0;
}

u32 *vm;
u32 *km;
u32 *fp;

int proc_ktdim_control(struct file *file, const char __user *buffer,
                       size_t count, loff_t *ppos) {
  char *str;
  u8 argc = 0;
  char *argv[5];
#if 1
  u8 i;
#endif
  u32 sz;

  str = kzalloc(count + 1, GFP_KERNEL);
  if (!str)
    return -ENOMEM;

  if (copy_from_user(str, buffer, count))
    return -EFAULT;

  rev_parse_args(str, &argc, argv, 5);

#if 1
  for (i = 0; i < argc; i++) {
    printk("ktdim: argv[%d]=%s, %02x\n", i, argv[i], *argv[i]);
  }
#endif

  if (argc && (*argv[argc - 1] == 0x0))
    argc--;

  if ((argc == 1) && !strcmp(argv[0], "rewind")) {
    tditrace_rewind();
  } else if ((argc == 2) && !strcmp(argv[0], "kmalloc")) {
    sz = (int)simple_strtol(argv[1], NULL, 10);
    km = kmalloc(sz, GFP_KERNEL);
    do_something(km, sz);
    if (!km)
      printk("ktdim: kmalloc of %u bytes FAILED!\n", sz);
    else {
      printk("ktdim: kmalloc'd %u bytes (%u Kb, %u MB) @ 0x%08x, ksize = %u\n",
             sz, sz / 1024, sz / (1024 * 1024), (u32)km, ksize(km));
    }
  } else if ((argc == 2) && !strcmp(argv[0], "vmalloc")) {
    sz = (int)simple_strtol(argv[1], NULL, 10);
    vm = vmalloc(sz);
    do_something(vm, sz);
    if (!vm)
      printk("ktdim: vmalloc of %u bytes FAILED!\n", sz);
    else {
      printk("ktdim: vmalloc'd %u bytes (%u Kb, %u MB) @ 0x%08x\n", sz,
             sz / 1024, sz / (1024 * 1024), (u32)vm);
    }
  } else if ((argc == 1) && !strcmp(argv[0], "kfree")) {
    if (!km)
      printk("ktdim: km = 0!\n");
    else {
      kfree(km);
      printk("ktdim: kfree'd\n");
    }
  } else if ((argc == 1) && !strcmp(argv[0], "vfree")) {
    if (!vm)
      printk("ktdim: vm = 0!\n");
    else {
      vfree(vm);
      printk("ktdim: vfree'd\n");
    }
  } else if ((argc == 1) && !strcmp(argv[0], "__get_free_page")) {
    fp = (u32 *)__get_free_page(GFP_KERNEL);
    do_something(fp, 1);
    if (!fp)
      printk("ktdim: __get_free_page FAILED!\n");
    else {
      printk("ktdim: __get_free_page @ 0x%08x\n", (u32)fp);
    }
  } else if ((argc == 1) && !strcmp(argv[0], "get_zeroed_page")) {
    fp = (u32 *)get_zeroed_page(GFP_KERNEL);
    if (!fp)
      printk("ktdim: get_zeroed_page FAILED!\n");
    else {
      printk("ktdim: get_zeroed_page @ 0x%08x\n", (u32)fp);
    }
  } else if ((argc == 2) && !strcmp(argv[0], "__get_free_pages")) {
    sz = (int)simple_strtol(argv[1], NULL, 10);
    fp = (u32 *)__get_free_pages(GFP_KERNEL, sz);
    if (!fp)
      printk("ktdim: __get_free_pages FAILED!\n");
    else {
      printk("ktdim: __get_free_pages allocated %u pages @ 0x%08x\n", 2 << sz,
             (u32)fp);
    }
  } else {

    printk("ktdim: unknown command\n");
  }

  kfree(str);
  return count;
}

int proc_ktdim_tdi(struct file *file, const char __user *buffer, size_t count,
                   loff_t *ppos) {

  tditrace("%s", buffer);
  return count;
}

int proc_ktdim_help(struct seq_file *sf, void *v) {
  struct timeval now;
  u32 *now_ref = (u32 *)&now;

  do_gettimeofday(&now);
  seq_printf(sf, "tv_sec:tv_usec=0x%08x=%ld:%ld\n", *now_ref, now.tv_sec,
             now.tv_usec);

  seq_printf(sf, "Usage:\n");
  seq_printf(sf, "  echo \"kmalloc <bytes>\" > /proc/ktdim-control\n");
  seq_printf(sf, "  echo \"vmalloc <bytes>\" > /proc/ktdim-control\n");
  seq_printf(sf, "  echo \"kfree\" > /proc/ktdim-control\n");
  seq_printf(sf, "  echo \"vfree\" > /proc/ktdim-control\n");
  return 0;
}

int proc_ktdim_help_open(struct inode *inode, struct file *file) {
  return single_open(file, proc_ktdim_help, PDE_DATA(inode));
}

int proc_ktdim_status_open(struct inode *inode, struct file *file) {
  return single_open(file, proc_ktdim_status, PDE_DATA(inode));
}

struct file_operations proc_ktdim_help_fops = {
    .owner = THIS_MODULE,
    .open = proc_ktdim_help_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

struct file_operations proc_ktdim_control_fops = {
    .owner = THIS_MODULE,
    .read = seq_read,
    .llseek = seq_lseek,
    .write = proc_ktdim_control,
};

struct file_operations proc_ktdim_status_fops = {
    .owner = THIS_MODULE,
    .open = proc_ktdim_status_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};

struct file_operations proc_ktdim_tdi_fops = {
    .owner = THIS_MODULE,
    .read = seq_read,
    .llseek = seq_lseek,
    .write = proc_ktdim_tdi,
};

static int __init ktdim_init(void) {
  int err = -1;
  int ret;

  printk("ktdim: Init.. (%s-%s)\n", __DATE__, __TIME__);

  ret = misc_register(&gktdimMiscDev);
  if (ret < 0) {
    printk("ktdim: can't register misc device (minor %d)!\n",
           gktdimMiscDev.minor);
    return ret;
  }

  tditrace_create_buffer();

  if (proc_create("ktdim-help", S_IFREG | S_IRUGO | S_IWUSR, NULL,
                  &proc_ktdim_help_fops) == NULL) {
    printk("proc create entry error\n");
  } else if (proc_create("ktdim-control", S_IFREG | S_IRUGO | S_IWUSR, NULL,
                         &proc_ktdim_control_fops) == NULL) {
    printk("proc create entry error\n");
  } else if (proc_create("ktdim-status", S_IFREG | S_IRUGO | S_IWUSR, NULL,
                         &proc_ktdim_status_fops) == NULL) {
    printk("proc create entry error\n");
  } else
    err = 0;
  return err;
}

static void __exit ktdim_exit(void) {
  printk("ktdim: Exit..\n");

  misc_deregister(&gktdimMiscDev);

  if (gtracebuffer_shared_page) {
    ClearPageReserved(gtracebuffer_shared_page);
    free_pages((int)gtrace_buffer, get_order(gtracebuffersize));
  }

  remove_proc_entry("ktdim-help", NULL);
  remove_proc_entry("ktdim-control", NULL);
  remove_proc_entry("ktdim-status", NULL);
}

module_init(ktdim_init);
module_exit(ktdim_exit);

MODULE_AUTHOR("REV");
MODULE_DESCRIPTION("ktdim");
MODULE_LICENSE("GPL");
