#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fcntl.h> /*Helps fix O_ACCMODE*/
#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */
#include <linux/fs.h> /*Helps fix the struct intializer */

#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>


#define MEM_SIZE        1024           //Memory Size

#define MY_MAGIC 'a'
#define WR_VALUE _IOW(MY_MAGIC,'a',int32_t*) //(copy_from_user)
#define RD_VALUE _IOR(MY_MAGIC,'b',int32_t*) //(copy_to_user)
#define MY_IOCTL_MAX_CMD 2

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johny");
MODULE_DESCRIPTION(" major and minor  driver");
MODULE_VERSION("1.0.0");

dev_t dev =0;
static struct class *dev_class;
static struct cdev my_cdev;
uint8_t *kernel_buffer =NULL;
int32_t value = 0;

static int      __init my_driver_init(void);
static void     __exit my_driver_exit(void);
static int      my_open(struct inode *inode, struct file *file);
static int      my_release(struct inode *inode, struct file *file);
static ssize_t  my_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = my_read,
    .write      = my_write,
    .open       = my_open,
	.unlocked_ioctl = my_ioctl,
    .release    = my_release,
};


static int my_open(struct inode *inode, struct file *file)
{
	
	if((kernel_buffer = kmalloc(MEM_SIZE,GFP_KERNEL))==0)
	{
		printk(KERN_INFO "Cannot allocate memory in kernel\n");
		return -1;
	}
	printk(KERN_INFO"Driver open  function called .....\n");
	return 0;	
}
static int my_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	printk(KERN_INFO"Driver release function called ....\n");
	return 0;
}
static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int ret =0;
	//Copy the data from the kernel space to the user-space
    ret = copy_to_user(buf, kernel_buffer, MEM_SIZE);
	if(ret > 0)
	{
		printk(KERN_INFO"writing data to user-space failed\n");
	}
	printk(KERN_INFO"Driver read function called .....\n");
	return 0;
}
//write----> sys_write---->do_write ------------------->fops--->write==> my_write 
static ssize_t my_write(struct file *filp, const char __user *buf,size_t len,loff_t *off)
{
	int ret = 0;
	//Copy the data to kernel space from the user-space
	copy_from_user(kernel_buffer, buf, len);
	if(ret > 0)
	{
		printk(KERN_INFO"Copy the data to kernel space from the user-space\n");
	}
	printk(KERN_INFO"Driver write function called ....%s \n",kernel_buffer);
	return len;
} 
/*
				     big_kernel_lock()	
ioctl----> sys_ioctl---->do_ioctl ------------------->fops--->ioctl==> my_ioctl 
                      unlock()


*/
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	printk(KERN_INFO"Enter ioctl ");
			
	switch(cmd) {
		case WR_VALUE:
			printk(KERN_INFO"Enter write ");
			copy_from_user(&value ,(int32_t*) arg, sizeof(value));
			printk(KERN_INFO "Value = %d\n", value);
			break;
		case RD_VALUE:
			printk(KERN_INFO"Enter read ");
			copy_to_user((int32_t*) arg, &value, sizeof(value));
			 break;
	}
	return 0;
 }

static int __init my_driver_init(void)
{
	if((alloc_chrdev_region(&dev,0,1,"my_Char"))<0)
	{
		printk(KERN_INFO"cannot allocate major and minor number");
		return -1;
	}
	printk(KERN_INFO"Major =%d Minor = %d \n",MAJOR(dev),MINOR(dev));
	
	//creating cdev structure
	cdev_init(&my_cdev,&fops);
	
	//Adding charriver to system
	if((cdev_add(&my_cdev,dev,1)) <0)
	{
		printk(KERN_INFO"cannot add the device to the system \n");
		goto r_class;
	}

	if((dev_class = class_create(THIS_MODULE,"my_Char_class")) == NULL){
		printk(KERN_INFO"Cannot create the struct class for device");
		goto r_class;
	}
	if((device_create(dev_class,NULL,dev,NULL,"my_Ioctl_driver"))==NULL) {
		printk(KERN_INFO" cannot create the device\n");
		goto r_device;
	}

	printk(KERN_INFO"IOCTL : character drivre init sucess\n");

	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit my_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&my_cdev);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "IOCTL :kernel driver removed  ... done \n");

}
module_init(my_driver_init);
module_exit(my_driver_exit);


//test_ioctl is used for testing this driver.
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ make clean
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD/Ioctl clean
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ make
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD/Ioctl modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
  CC [M]  /home/aishwarya/Documents/LDD/Ioctl/Ioctl.o
/home/aishwarya/Documents/LDD/Ioctl/Ioctl.c: In function ‘my_ioctl’:
/home/aishwarya/Documents/LDD/Ioctl/Ioctl.c:111:4: warning: ignoring return value of ‘copy_from_user’, declared with attribute warn_unused_result [-Wunused-result]
  111 |    copy_from_user(&value ,(int32_t*) arg, sizeof(value));
      |    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/aishwarya/Documents/LDD/Ioctl/Ioctl.c:116:4: warning: ignoring return value of ‘copy_to_user’, declared with attribute warn_unused_result [-Wunused-result]
  116 |    copy_to_user((int32_t*) arg, &value, sizeof(value));
      |    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/home/aishwarya/Documents/LDD/Ioctl/Ioctl.c: In function ‘my_write’:
/home/aishwarya/Documents/LDD/Ioctl/Ioctl.c:89:2: warning: ignoring return value of ‘copy_from_user’, declared with attribute warn_unused_result [-Wunused-result]
   89 |  copy_from_user(kernel_buffer, buf, len);
      |  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  MODPOST /home/aishwarya/Documents/LDD/Ioctl/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/Ioctl/Ioctl.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/Ioctl/Ioctl.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$  sudo insmod Ioctl.ko

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ lsmod |head
Module                  Size  Used by
Ioctl                  16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
snd_intel8x0           45056  2
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
snd_ac97_codec        139264  1 snd_intel8x0
aesni_intel           372736  0

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ cat /proc/devices
Character devices:
  1 mem
  4 /dev/vc/0
  4 tty
  4 ttyS
  5 /dev/tty
  5 /dev/console
  5 /dev/ptmx
  5 ttyprintk
  6 lp
  7 vcs
 10 misc
 13 input
 21 sg
 29 fb
 89 i2c
 99 ppdev
108 ppp
116 alsa
128 ptm
136 pts
180 usb
189 usb_device
204 ttyMAX
226 drm
237 my_Char
238 aux
239 cec
240 lirc
241 hidraw
242 vfio
243 bsg
244 watchdog
245 remoteproc
246 ptp
247 pps
248 rtc
249 dma_heap
250 dax
251 dimmctl
252 ndctl
253 tpm
254 gpiochip

Block devices:
  7 loop
  8 sd
  9 md
 11 sr
 65 sd
 66 sd
 67 sd
 68 sd
 69 sd
 70 sd
 71 sd
128 sd
129 sd
130 sd
131 sd
132 sd
133 sd
134 sd
135 sd
253 device-mapper
254 mdp
259 blkext

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ ls -al /dev/
total 4
drwxr-xr-x  19 root      root        4140 Sep 16 10:28 .
drwxr-xr-x  20 root      root        4096 Aug 25 18:12 ..
crw-r--r--   1 root      root     10, 235 Sep 16 10:12 autofs
drwxr-xr-x   2 root      root         360 Sep 16 10:12 block
drwxr-xr-x   2 root      root          80 Sep 16 10:12 bsg
crw-------   1 root      root     10, 234 Sep 16 10:12 btrfs-control
drwxr-xr-x   3 root      root          60 Sep 16 10:12 bus
lrwxrwxrwx   1 root      root           3 Sep 16 10:12 cdrom -> sr0
drwxr-xr-x   2 root      root        3720 Sep 16 10:28 char
crw--w----   1 root      tty       5,   1 Sep 16 10:12 console
lrwxrwxrwx   1 root      root          11 Sep 16 10:12 core -> /proc/kcore
crw-------   1 root      root     10, 123 Sep 16 10:12 cpu_dma_latency
crw-------   1 root      root     10, 203 Sep 16 10:12 cuse
drwxr-xr-x   6 root      root         120 Sep 16 10:12 disk
drwxr-xr-x   2 root      root          60 Sep 16 10:12 dma_heap
drwxr-xr-x   3 root      root         100 Sep 16 10:12 dri
lrwxrwxrwx   1 root      root           3 Sep 16 10:12 dvd -> sr0
crw-------   1 root      root     10, 126 Sep 16 10:12 ecryptfs
crw-rw----   1 root      video    29,   0 Sep 16 10:12 fb0
lrwxrwxrwx   1 root      root          13 Sep 16 10:12 fd -> /proc/self/fd
crw-rw-rw-   1 root      root      1,   7 Sep 16 10:12 full
crw-rw-rw-   1 root      root     10, 229 Sep 16 10:12 fuse
crw-------   1 root      root    241,   0 Sep 16 10:12 hidraw0
crw-------   1 root      root     10, 228 Sep 16 10:12 hpet
drwxr-xr-x   2 root      root           0 Sep 16 10:12 hugepages
crw-------   1 root      root     10, 183 Sep 16 10:12 hwrng
crw-------   1 root      root     89,   0 Sep 16 10:12 i2c-0
lrwxrwxrwx   1 root      root          12 Sep 16 10:12 initctl -> /run/initctl
drwxr-xr-x   4 root      root         340 Sep 16 10:12 input
crw-r--r--   1 root      root      1,  11 Sep 16 10:12 kmsg
drwxr-xr-x   2 root      root          60 Sep 16 10:12 lightnvm
lrwxrwxrwx   1 root      root          28 Sep 16 10:12 log -> /run/systemd/journal/dev-log
brw-rw----   1 root      disk      7,   0 Sep 16 10:12 loop0
brw-rw----   1 root      disk      7,   1 Sep 16 10:12 loop1
brw-rw----   1 root      disk      7,  10 Sep 16 10:12 loop10
brw-rw----   1 root      disk      7,   2 Sep 16 10:12 loop2
brw-rw----   1 root      disk      7,   3 Sep 16 10:12 loop3
brw-rw----   1 root      disk      7,   4 Sep 16 10:12 loop4
brw-rw----   1 root      disk      7,   5 Sep 16 10:12 loop5
brw-rw----   1 root      disk      7,   6 Sep 16 10:12 loop6
brw-rw----   1 root      disk      7,   7 Sep 16 10:12 loop7
brw-rw----   1 root      disk      7,   8 Sep 16 10:12 loop8
brw-rw----   1 root      disk      7,   9 Sep 16 10:12 loop9
crw-rw----   1 root      disk     10, 237 Sep 16 10:12 loop-control
drwxr-xr-x   2 root      root          60 Sep 16 10:12 mapper
crw-------   1 root      root     10, 227 Sep 16 10:12 mcelog
crw-r-----   1 root      kmem      1,   1 Sep 16 10:12 mem
drwxrwxrwt   2 root      root          40 Sep 16 10:12 mqueue
crw-------   1 root      root    237,   0 Sep 16 10:28 my_Ioctl_driver
drwxr-xr-x   2 root      root          60 Sep 16 10:12 net
crw-rw-rw-   1 root      root      1,   3 Sep 16 10:12 null
crw-------   1 root      root     10, 144 Sep 16 10:12 nvram
crw-r-----   1 root      kmem      1,   4 Sep 16 10:12 port
crw-------   1 root      root    108,   0 Sep 16 10:12 ppp
crw-------   1 root      root     10,   1 Sep 16 10:12 psaux
crw-rw-rw-   1 root      tty       5,   2 Sep 16 10:31 ptmx
drwxr-xr-x   2 root      root           0 Sep 16 10:12 pts
crw-rw-rw-   1 root      root      1,   8 Sep 16 10:12 random
crw-rw-r--+  1 root      root     10, 242 Sep 16 10:12 rfkill
lrwxrwxrwx   1 root      root           4 Sep 16 10:12 rtc -> rtc0
crw-------   1 root      root    248,   0 Sep 16 10:12 rtc0
brw-rw----   1 root      disk      8,   0 Sep 16 10:12 sda
brw-rw----   1 root      disk      8,   1 Sep 16 10:12 sda1
brw-rw----   1 root      disk      8,   2 Sep 16 10:12 sda2
brw-rw----   1 root      disk      8,   5 Sep 16 10:12 sda5
crw-rw----+  1 root      cdrom    21,   0 Sep 16 10:12 sg0
crw-rw----   1 root      disk     21,   1 Sep 16 10:12 sg1
drwxrwxrwt   2 root      root          40 Sep 16 10:12 shm
crw-------   1 root      root     10, 231 Sep 16 10:12 snapshot
drwxr-xr-x   3 root      root         180 Sep 16 10:12 snd
brw-rw----+  1 root      cdrom    11,   0 Sep 16 10:12 sr0
lrwxrwxrwx   1 root      root          15 Sep 16 10:12 stderr -> /proc/self/fd/2
lrwxrwxrwx   1 root      root          15 Sep 16 10:12 stdin -> /proc/self/fd/0
lrwxrwxrwx   1 root      root          15 Sep 16 10:12 stdout -> /proc/self/fd/1
crw-rw-rw-   1 root      tty       5,   0 Sep 16 10:25 tty
crw--w----   1 root      tty       4,   0 Sep 16 10:12 tty0
crw--w----   1 root      tty       4,   1 Sep 16 10:12 tty1
crw--w----   1 root      tty       4,  10 Sep 16 10:12 tty10
crw--w----   1 root      tty       4,  11 Sep 16 10:12 tty11
crw--w----   1 root      tty       4,  12 Sep 16 10:12 tty12
crw--w----   1 root      tty       4,  13 Sep 16 10:12 tty13
crw--w----   1 root      tty       4,  14 Sep 16 10:12 tty14
crw--w----   1 root      tty       4,  15 Sep 16 10:12 tty15
crw--w----   1 root      tty       4,  16 Sep 16 10:12 tty16
crw--w----   1 root      tty       4,  17 Sep 16 10:12 tty17
crw--w----   1 root      tty       4,  18 Sep 16 10:12 tty18
crw--w----   1 root      tty       4,  19 Sep 16 10:12 tty19
crw--w----   1 aishwarya tty       4,   2 Sep 16 10:12 tty2
crw--w----   1 root      tty       4,  20 Sep 16 10:12 tty20
crw--w----   1 root      tty       4,  21 Sep 16 10:12 tty21
crw--w----   1 root      tty       4,  22 Sep 16 10:12 tty22
crw--w----   1 root      tty       4,  23 Sep 16 10:12 tty23
crw--w----   1 root      tty       4,  24 Sep 16 10:12 tty24
crw--w----   1 root      tty       4,  25 Sep 16 10:12 tty25
crw--w----   1 root      tty       4,  26 Sep 16 10:12 tty26
crw--w----   1 root      tty       4,  27 Sep 16 10:12 tty27
crw--w----   1 root      tty       4,  28 Sep 16 10:12 tty28
crw--w----   1 root      tty       4,  29 Sep 16 10:12 tty29
crw--w----   1 root      tty       4,   3 Sep 16 10:12 tty3
crw--w----   1 root      tty       4,  30 Sep 16 10:12 tty30
crw--w----   1 root      tty       4,  31 Sep 16 10:12 tty31
crw--w----   1 root      tty       4,  32 Sep 16 10:12 tty32
crw--w----   1 root      tty       4,  33 Sep 16 10:12 tty33
crw--w----   1 root      tty       4,  34 Sep 16 10:12 tty34
crw--w----   1 root      tty       4,  35 Sep 16 10:12 tty35
crw--w----   1 root      tty       4,  36 Sep 16 10:12 tty36
crw--w----   1 root      tty       4,  37 Sep 16 10:12 tty37
crw--w----   1 root      tty       4,  38 Sep 16 10:12 tty38
crw--w----   1 root      tty       4,  39 Sep 16 10:12 tty39
crw--w----   1 root      tty       4,   4 Sep 16 10:12 tty4
crw--w----   1 root      tty       4,  40 Sep 16 10:12 tty40
crw--w----   1 root      tty       4,  41 Sep 16 10:12 tty41
crw--w----   1 root      tty       4,  42 Sep 16 10:12 tty42
crw--w----   1 root      tty       4,  43 Sep 16 10:12 tty43
crw--w----   1 root      tty       4,  44 Sep 16 10:12 tty44
crw--w----   1 root      tty       4,  45 Sep 16 10:12 tty45
crw--w----   1 root      tty       4,  46 Sep 16 10:12 tty46
crw--w----   1 root      tty       4,  47 Sep 16 10:12 tty47
crw--w----   1 root      tty       4,  48 Sep 16 10:12 tty48
crw--w----   1 root      tty       4,  49 Sep 16 10:12 tty49
crw--w----   1 root      tty       4,   5 Sep 16 10:12 tty5
crw--w----   1 root      tty       4,  50 Sep 16 10:12 tty50
crw--w----   1 root      tty       4,  51 Sep 16 10:12 tty51
crw--w----   1 root      tty       4,  52 Sep 16 10:12 tty52
crw--w----   1 root      tty       4,  53 Sep 16 10:12 tty53
crw--w----   1 root      tty       4,  54 Sep 16 10:12 tty54
crw--w----   1 root      tty       4,  55 Sep 16 10:12 tty55
crw--w----   1 root      tty       4,  56 Sep 16 10:12 tty56
crw--w----   1 root      tty       4,  57 Sep 16 10:12 tty57
crw--w----   1 root      tty       4,  58 Sep 16 10:12 tty58
crw--w----   1 root      tty       4,  59 Sep 16 10:12 tty59
crw--w----   1 root      tty       4,   6 Sep 16 10:12 tty6
crw--w----   1 root      tty       4,  60 Sep 16 10:12 tty60
crw--w----   1 root      tty       4,  61 Sep 16 10:12 tty61
crw--w----   1 root      tty       4,  62 Sep 16 10:12 tty62
crw--w----   1 root      tty       4,  63 Sep 16 10:12 tty63
crw--w----   1 root      tty       4,   7 Sep 16 10:12 tty7
crw--w----   1 root      tty       4,   8 Sep 16 10:12 tty8
crw--w----   1 root      tty       4,   9 Sep 16 10:12 tty9
crw-------   1 root      root      5,   3 Sep 16 10:12 ttyprintk
crw-rw----   1 root      dialout   4,  64 Sep 16 10:12 ttyS0
crw-rw----   1 root      dialout   4,  65 Sep 16 10:12 ttyS1
crw-rw----   1 root      dialout   4,  74 Sep 16 10:12 ttyS10
crw-rw----   1 root      dialout   4,  75 Sep 16 10:12 ttyS11
crw-rw----   1 root      dialout   4,  76 Sep 16 10:12 ttyS12
crw-rw----   1 root      dialout   4,  77 Sep 16 10:12 ttyS13
crw-rw----   1 root      dialout   4,  78 Sep 16 10:12 ttyS14
crw-rw----   1 root      dialout   4,  79 Sep 16 10:12 ttyS15
crw-rw----   1 root      dialout   4,  80 Sep 16 10:12 ttyS16
crw-rw----   1 root      dialout   4,  81 Sep 16 10:12 ttyS17
crw-rw----   1 root      dialout   4,  82 Sep 16 10:12 ttyS18
crw-rw----   1 root      dialout   4,  83 Sep 16 10:12 ttyS19
crw-rw----   1 root      dialout   4,  66 Sep 16 10:12 ttyS2
crw-rw----   1 root      dialout   4,  84 Sep 16 10:12 ttyS20
crw-rw----   1 root      dialout   4,  85 Sep 16 10:12 ttyS21
crw-rw----   1 root      dialout   4,  86 Sep 16 10:12 ttyS22
crw-rw----   1 root      dialout   4,  87 Sep 16 10:12 ttyS23
crw-rw----   1 root      dialout   4,  88 Sep 16 10:12 ttyS24
crw-rw----   1 root      dialout   4,  89 Sep 16 10:12 ttyS25
crw-rw----   1 root      dialout   4,  90 Sep 16 10:12 ttyS26
crw-rw----   1 root      dialout   4,  91 Sep 16 10:12 ttyS27
crw-rw----   1 root      dialout   4,  92 Sep 16 10:12 ttyS28
crw-rw----   1 root      dialout   4,  93 Sep 16 10:12 ttyS29
crw-rw----   1 root      dialout   4,  67 Sep 16 10:12 ttyS3
crw-rw----   1 root      dialout   4,  94 Sep 16 10:12 ttyS30
crw-rw----   1 root      dialout   4,  95 Sep 16 10:12 ttyS31
crw-rw----   1 root      dialout   4,  68 Sep 16 10:12 ttyS4
crw-rw----   1 root      dialout   4,  69 Sep 16 10:12 ttyS5
crw-rw----   1 root      dialout   4,  70 Sep 16 10:12 ttyS6
crw-rw----   1 root      dialout   4,  71 Sep 16 10:12 ttyS7
crw-rw----   1 root      dialout   4,  72 Sep 16 10:12 ttyS8
crw-rw----   1 root      dialout   4,  73 Sep 16 10:12 ttyS9
crw-rw----   1 root      kvm      10, 124 Sep 16 10:12 udmabuf
crw-------   1 root      root     10, 239 Sep 16 10:12 uhid
crw-------   1 root      root     10, 223 Sep 16 10:12 uinput
crw-rw-rw-   1 root      root      1,   9 Sep 16 10:12 urandom
crw-------   1 root      root     10, 240 Sep 16 10:12 userio
crw-------   1 root      root     10, 122 Sep 16 10:12 vboxguest
crw-------   1 root      root     10, 121 Sep 16 10:12 vboxuser
crw-rw----   1 root      tty       7,   0 Sep 16 10:12 vcs
crw-rw----   1 root      tty       7,   1 Sep 16 10:12 vcs1
crw-rw----   1 root      tty       7,   2 Sep 16 10:12 vcs2
crw-rw----   1 root      tty       7,   3 Sep 16 10:12 vcs3
crw-rw----   1 root      tty       7,   4 Sep 16 10:12 vcs4
crw-rw----   1 root      tty       7,   5 Sep 16 10:12 vcs5
crw-rw----   1 root      tty       7,   6 Sep 16 10:12 vcs6
crw-rw----   1 root      tty       7, 128 Sep 16 10:12 vcsa
crw-rw----   1 root      tty       7, 129 Sep 16 10:12 vcsa1
crw-rw----   1 root      tty       7, 130 Sep 16 10:12 vcsa2
crw-rw----   1 root      tty       7, 131 Sep 16 10:12 vcsa3
crw-rw----   1 root      tty       7, 132 Sep 16 10:12 vcsa4
crw-rw----   1 root      tty       7, 133 Sep 16 10:12 vcsa5
crw-rw----   1 root      tty       7, 134 Sep 16 10:12 vcsa6
crw-rw----   1 root      tty       7,  64 Sep 16 10:12 vcsu
crw-rw----   1 root      tty       7,  65 Sep 16 10:12 vcsu1
crw-rw----   1 root      tty       7,  66 Sep 16 10:12 vcsu2
crw-rw----   1 root      tty       7,  67 Sep 16 10:12 vcsu3
crw-rw----   1 root      tty       7,  68 Sep 16 10:12 vcsu4
crw-rw----   1 root      tty       7,  69 Sep 16 10:12 vcsu5
crw-rw----   1 root      tty       7,  70 Sep 16 10:12 vcsu6
drwxr-xr-x   2 root      root          60 Sep 16 10:12 vfio
crw-------   1 root      root     10, 127 Sep 16 10:12 vga_arbiter
crw-------   1 root      root     10, 137 Sep 16 10:12 vhci
crw-------   1 root      root     10, 238 Sep 16 10:12 vhost-net
crw-------   1 root      root     10, 241 Sep 16 10:12 vhost-vsock
crw-rw-rw-   1 root      root      1,   5 Sep 16 10:12 zero
crw-------   1 root      root     10, 249 Sep 16 10:12 zfs

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ sudo chmod 777 /dev/my_Ioctl_driver

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ cc test_ioctl.c

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ ./a.out
[5453] - Opening device my_cdrv
Device opened with ID [3]
Enter the Value to send
12
Writing Value to Driver
Reading Value from Driver
Value is 12
Closing Driver
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl$ 

*/
