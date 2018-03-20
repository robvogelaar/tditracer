
/*============================================================================================*/

#include <asm/mman.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <ktdim.h>

/******************************************************************************
* LOCAL MACROS                                                                *
*******************************************************************************/

#define TIME_DOCTOR_DESCRIPTION "Time doctor event logger"
#define TIME_DOCTOR_DEVNAME "timedoctor"

//  ----------------------- Log MODULE Debug Level  ------------------------
static int debug_level = 0;
module_param(debug_level, int, 0);
MODULE_PARM_DESC(debug_level, "Debug level (0-1)");

/**** Module Setup ****/
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(TIME_DOCTOR_DESCRIPTION);
MODULE_AUTHOR("REV");

#define CONFIG_TIME_DOCTOR_SAMPLES (128 * 1024)

#define NB_DATA (CONFIG_TIME_DOCTOR_SAMPLES)
#define NB_FIELDS_PER_RECORD (4)
#define DATA_SIZE (NB_DATA * sizeof(timeDoctorRecord_t))

/******************************************************************************
* LOCAL TYPEDEFS                                                              *
*******************************************************************************/
typedef unsigned int timeDoctorRecord_t[NB_FIELDS_PER_RECORD];

/******************************************************************************
* STATIC FUNCTION PROTOTYPES                                                  *
*******************************************************************************/

/* Declare funcions needed by the structures below */
static void *timeDoctor_SeqStart(struct seq_file *, loff_t *);
static void *timeDoctor_SeqNext(struct seq_file *, void *, loff_t *);
static void timedoctor_SeqStop(struct seq_file *, void *);
static int timedoctor_SeqShow(struct seq_file *, void *);

static int timeDoctor_OpenProc(struct inode *, struct file *);
static ssize_t timeDoctor_WriteProc(struct file *, const char __user *, size_t,
                                    loff_t *);

/******************************************************************************
* EXPORTED DATA                                                               *
*******************************************************************************/

/******************************************************************************
* LOCAL DATA                                                                  *
*******************************************************************************/

/* This array stores NB_DATA records, each of which is 4 words */
static timeDoctorRecord_t *gtimeDoctorData;
static struct page *gtimeDoctorSharedPage;

/* Index pointing to the next available word in the array */
static unsigned int gtimeDoctorIndex;

/* Indicates whether we're allowed to wrap around our buffer.
   Default value is on. */
static unsigned int gtimeDoctorDataWrapAllowed = 1;

/* Indicates whether we've wrapped around our buffer. */
static unsigned int gtimeDoctorDataWrapped;

/* Operations for reading /proc/timedoctor one record at a time */
static struct seq_operations timedoctor_seq_ops = {.start = timeDoctor_SeqStart,
                                                   .next = timeDoctor_SeqNext,
                                                   .stop = timedoctor_SeqStop,
                                                   .show = timedoctor_SeqShow};

/* Operations for reading/writing /proc/timedoctor */
static struct file_operations timedoctor_proc_fops = {
    .owner = THIS_MODULE,
    .open = timeDoctor_OpenProc,
    .read = seq_read,
    .write = timeDoctor_WriteProc,
    .llseek = seq_lseek,
    .release = seq_release};

/******************************************************************************
* FUNCTION IMPLEMENTATION                                                     *
*******************************************************************************/

static int timeDoctor_BufferInit(void) {
  gtimeDoctorData =
      (timeDoctorRecord_t *)__get_free_pages(GFP_KERNEL, get_order(DATA_SIZE));
  gtimeDoctorSharedPage = virt_to_page(gtimeDoctorData);

  if (gtimeDoctorData == 0) {
    printk(KERN_ERR "Unable to allocate timedoctor memory\n");
  }

  return 0;
} /* Log init */

/* seq_read takes care of all reading for us */
static int timeDoctor_OpenProc(struct inode *inode, struct file *file) {
  return seq_open(file, &timedoctor_seq_ops);
}

/* Start a seq read, convert initial record# to pointer */
static void *timeDoctor_SeqStart(struct seq_file *s, loff_t *pos) {
  loff_t localPos = *pos;
  unsigned int dataLimit = gtimeDoctorIndex;

  if (gtimeDoctorDataWrapped) {
    localPos = (gtimeDoctorIndex + (*pos)) % NB_DATA;
    dataLimit = NB_DATA;
  }

  if ((*pos) >= dataLimit) return NULL; /* No more to read */
  return gtimeDoctorData + localPos;
}

/* Increment to next record# & convert to pointer */
static void *timeDoctor_SeqNext(struct seq_file *s, void *v, loff_t *pos) {
  loff_t localPos;
  unsigned int dataLimit = gtimeDoctorIndex;

  (*pos)++;
  localPos = *pos;
  if (gtimeDoctorDataWrapped) {
    localPos = (gtimeDoctorIndex + (*pos)) % NB_DATA;
    dataLimit = NB_DATA;
  }
  if ((*pos) >= dataLimit) return NULL;
  return gtimeDoctorData + localPos;
}

static void timedoctor_SeqStop(struct seq_file *f, void *v) {
  /* Nothing to do */
}

/* Display a single record, from pointer prepared by start/next */
static int timedoctor_SeqShow(struct seq_file *s, void *v) {
  unsigned int *rec = (unsigned int *)v;
  seq_printf(s, "%08x %08x %08x %08x\n", rec[0], rec[1], rec[2], rec[3]);
  return 0;
}

static ssize_t timeDoctor_WriteProc(struct file *file,
                                    const char __user *buffer, size_t count,
                                    loff_t *data) {
  char command[10];

  if (!count) {
    return 0;
  } else if (count > 10) {
    printk("timeDoctor_WriteProc: Problem ! count = %u\n", count);
    return -EFAULT;
  }

  if (copy_from_user(&command, buffer, count)) {
    return -EFAULT;
  }

  if (!strncmp(command, "off", 3)) {
    timeDoctor_SetLevel(0);
    printk("debug_level:off\n");
  } else if (!strncmp(command, "on", 2)) {
    timeDoctor_SetLevel(1);
    printk("debug_level:on\n");
  } else if (!strncmp(command, "wrapon", 6)) {
    gtimeDoctorDataWrapAllowed = 1;
    printk("wrap:on\n");
  } else if (!strncmp(command, "wrapoff", 7)) {
    gtimeDoctorDataWrapAllowed = 0;
    gtimeDoctorDataWrapped = 0;
    printk("wrap:off\n");
  } else if (!strncmp(command, "status", 6)) {
    if (debug_level) {
      printk("debug_level:on\n");
    } else {
      printk("debug_level:off\n");
    }
  } else if (!strncmp(command, "reset", 5)) {
    timeDoctor_Reset();
  }

  return count;
} /* End of timeDoctor_WriteProc */

/*=============================================================================
| Module functions                                                            |
==============================================================================*/

void timeDoctor_Info(unsigned int data1, unsigned int data2,
                     unsigned int data3) {
  unsigned int time;  //, status;

  /* Read status, disable ints */
  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mfc0 %0, $12, 0\n"
  //         "di\n"
  //         ".set pop"
  //         : "=r" (status) );

  unsigned long flags;

  local_irq_save(flags);

  if (debug_level) {
    /* Read the time counter (config: free running counter) */
    time = *(volatile unsigned int *)0xE0100200;

    gtimeDoctorData[gtimeDoctorIndex][0] = time;
    gtimeDoctorData[gtimeDoctorIndex][1] = data1;
    gtimeDoctorData[gtimeDoctorIndex][2] = data2;
    gtimeDoctorData[gtimeDoctorIndex][3] = data3;
    gtimeDoctorIndex++;

    if (gtimeDoctorIndex >= NB_DATA) {
      if (gtimeDoctorDataWrapAllowed) {
        gtimeDoctorIndex = 0;
        gtimeDoctorDataWrapped = 1;
      } else {
        debug_level = 0;
      }
    }
  }

  local_irq_restore(flags);

  // /* Restore previous interrupt state */
  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mtc0 %0, $12, 0\n"
  //         ".set pop"
  //         : : "r" (status) );

} /* End of log_info */

EXPORT_SYMBOL(timeDoctor_Info);

void timeDoctor_SetLevel(int level) {
  // unsigned int status;
  /* Read status, disable ints */
  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mfc0 %0, $12, 0\n"
  //         "di\n"
  //         ".set pop"
  //         : "=r" (status) );

  unsigned long flags;

  local_irq_save(flags);

  debug_level = level;

  local_irq_restore(flags);

  /* Restore previous interrupt state */

  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mtc0 %0, $12, 0\n"
  //         ".set pop"
  //         : : "r" (status) );

} /* End of timeDoctor_SetLevel */
EXPORT_SYMBOL(timeDoctor_SetLevel);

void timeDoctor_Reset(void) {
  // unsigned int status;
  // /* Read status, disable ints */
  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mfc0 %0, $12, 0\n"
  //         "di\n"
  //         ".set pop"
  //         : "=r" (status) );

  unsigned long flags;

  local_irq_save(flags);

  gtimeDoctorIndex = 0;
  gtimeDoctorDataWrapped = 0;

  local_irq_restore(flags);

  /* Restore previous interrupt state */
  // __asm__ __volatile__ (
  //         ".set push\n"
  //         ".set nomips16\n"
  //         "mtc0 %0, $12, 0\n"
  //         ".set pop"
  //         : : "r" (status) );

} /* End of timeDoctor_Reset */
EXPORT_SYMBOL(timeDoctor_Reset);

static int timeDoctor_GetEntries(void) {
  if (gtimeDoctorDataWrapped) {
    return NB_DATA * NB_FIELDS_PER_RECORD;
  } else {
    return gtimeDoctorIndex * NB_FIELDS_PER_RECORD;
  }
}

static int timedoctor_Ioctl(struct inode *inode, struct file *filp,
                            unsigned int cmd, unsigned long arg);

static int timedoctor_Mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations gtimeDoctorFops = {
    .unlocked_ioctl = timedoctor_Ioctl, .mmap = timedoctor_Mmap};

static struct miscdevice gtimeDoctorMiscDev = {.minor = MISC_DYNAMIC_MINOR,
                                               .name = TIME_DOCTOR_DEVNAME,
                                               .fops = &gtimeDoctorFops};

static int timedoctor_Ioctl(struct inode *inode, struct file *filp,
                            unsigned int cmd, unsigned long arg) {
  switch (cmd) {
    case TIMEDOCTOR_IOCTL_RESET:
      timeDoctor_Reset();
      return 0;

    case TIMEDOCTOR_IOCTL_START:
      timeDoctor_SetLevel(1);
      return 0;

    case TIMEDOCTOR_IOCTL_STOP:
      timeDoctor_SetLevel(0);
      return 0;

    case TIMEDOCTOR_IOCTL_GET_ENTRIES:
      return timeDoctor_GetEntries();

    case TIMEDOCTOR_IOCTL_GET_MAX_ENTRIES:
      return (NB_DATA * NB_FIELDS_PER_RECORD);

    case TIMEDOCTOR_IOCTL_INFO: {
      unsigned int data[TIMEDOCTOR_INFO_DATASIZE];
      copy_from_user(&data, (unsigned int *)arg,
                     sizeof(unsigned int) * TIMEDOCTOR_INFO_DATASIZE);
      timeDoctor_Info(data[0], data[1], data[2]);
    }
      return 0;
  }

  return -ENOSYS;
}

static int timedoctor_Mmap(struct file *file, struct vm_area_struct *vma) {
  unsigned int size;
  if (vma->vm_pgoff != 0) {
    return -EINVAL;
  }

  size = vma->vm_end - vma->vm_start;
  if (size != PAGE_ALIGN(DATA_SIZE)) {
    return -EINVAL;
  }

  /* Prevent the swapper from considering these pages for swap and touching them
   */
  vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP | VM_DONTEXPAND;

  return remap_pfn_range(vma, vma->vm_start, page_to_pfn(gtimeDoctorSharedPage),
                         size, vma->vm_page_prot);
}

/** Module initialisation. */
static int __init timeDoctor_Init(void) {
  int ret;

  static struct proc_dir_entry *entry;

  timeDoctor_BufferInit();
  timeDoctor_Reset();

  /* Register a misc device called "timedoctor". */
  ret = misc_register(&gtimeDoctorMiscDev);
  if (ret < 0) {
    printk("can't register misc device (minor %d)!\n",
           gtimeDoctorMiscDev.minor);
    return ret;
  }

#if 0
extern struct proc_dir_entry *create_proc_entry(const char *name, umode_t mode,
            struct proc_dir_entry *parent);

#define proc_create(name, mode, parent, proc_fops) ({ NULL; })
#endif

  entry =
      proc_create(TIME_DOCTOR_DEVNAME, S_IFREG | S_IRUGO | S_IWUSR, NULL,
                  &timedoctor_proc_fops);
  if (entry == NULL) return -ENOMEM;
  return 0;

#if 0
  entry = create_proc_entry(TIME_DOCTOR_DEVNAME,
                            S_IFREG | S_IRUGO | S_IWUSR,  // protection mode
                            NULL);                        // parent dir: /proc

  if (!entry) {
    printk("%screate_proc_entry : failed\n", TIME_DOCTOR_DESCRIPTION);
    return -ENOMEM;
  } else {
    entry->proc_fops = &timedoctor_proc_fops;
  }

  printk("%s (%s-%s) [%i events]\n", TIME_DOCTOR_DESCRIPTION, __DATE__,
         __TIME__, NB_DATA);
  return 0;
#endif
} /* End of timeDoctor_Init */

/** Module deinitialisation */
static void __exit timeDoctor_Exit(void) {
  printk("Exiting %s\n", TIME_DOCTOR_DESCRIPTION);
  debug_level = 0;

  misc_deregister(&gtimeDoctorMiscDev);

  ClearPageReserved(gtimeDoctorSharedPage);
  free_pages((int)gtimeDoctorData, get_order(DATA_SIZE));

  /* undo proc stuff */
  remove_proc_entry(TIME_DOCTOR_DEVNAME, NULL);

} /* End of timeDoctor_Exit */

module_init(timeDoctor_Init);
module_exit(timeDoctor_Exit);
