#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/slab.h>        /* kmalloc() */


#define CHAR_DEV_NAME "my_cdrv"
#define MAX_LENGTH 4000
#define SUCCESS 0

static char *char_device_buf;
struct cdev *my_cdev;
dev_t mydev;

static int char_dev_open(struct inode *inode,
			    struct file  *file)
{
	printk(KERN_INFO "open operation invoked \n");
	return SUCCESS;
}

static int char_dev_release(struct inode *inode,
		            struct file *file)
{
	return SUCCESS;
}

static ssize_t char_dev_write(struct file *file,
		            const char *buf,
			    size_t lbuf,
			    loff_t *ppos)
{
	int nbytes; /* Number of bytes written */

	nbytes = lbuf - raw_copy_from_user( char_device_buf + *ppos, /* to */
					buf, /* from */ 
					lbuf ); /* how many bytes */
	*ppos += nbytes;
	printk(KERN_INFO "Rec'vd from App data %s of length %d \n",char_device_buf, nbytes);
	return nbytes;
}

static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	//.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release,
};

static __init int char_dev_init(void)
{
	int ret,count=1;

	if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) < 0) {
            printk (KERN_ERR "failed to reserve major/minor range\n");
            return -1;
    	}

	if (!(my_cdev = cdev_alloc ())) {
		printk (KERN_ERR "cdev_alloc() failed\n");
		unregister_chrdev_region (mydev, count);
		return -1;
	}
	cdev_init(my_cdev,&char_dev_fops);

	ret=cdev_add(my_cdev,mydev,count);
	if( ret < 0 ) {
		printk(KERN_INFO "Error registering device driver\n");
	        cdev_del (my_cdev);
                unregister_chrdev_region (mydev, count); 	
		return -1;
	}
	printk(KERN_INFO"\nDevice Registered: %s\n",CHAR_DEV_NAME);
	printk (KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR (mydev),MINOR (mydev));

	//char_device_buf =(char *)kmalloc(MAX_LENGTH,GFP_KERNEL);
	return 0;
}

static __exit void  char_dev_exit(void)
{
	
	 cdev_del(my_cdev);
	 unregister_chrdev_region(mydev,1);
	 kfree(char_device_buf);
	 printk(KERN_INFO "\n Driver unregistered \n");
}
module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_AUTHOR("Johny");
MODULE_DESCRIPTION("Character Device Driver - Test");
MODULE_LICENSE("GPL");


/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/dynamic$ make
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD/Device/dynamic modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
  CC [M]  /home/aishwarya/Documents/LDD/Device/dynamic/chr_drv_dynamic.o
/home/aishwarya/Documents/LDD/Device/dynamic/chr_drv_dynamic.c:33:16: warning: ‘char_dev_write’ defined but not used [-Wunused-function]
   33 | static ssize_t char_dev_write(struct file *file,
      |                ^~~~~~~~~~~~~~
  MODPOST /home/aishwarya/Documents/LDD/Device/dynamic/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/Device/dynamic/chr_drv_dynamic.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/Device/dynamic/chr_drv_dynamic.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/dynamic$ sudo insmod chr_drv_dynamic.ko
[sudo] password for aishwarya: 
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/dynamic$ lsmod | head
Module                  Size  Used by
chr_drv_dynamic        16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
snd_intel8x0           45056  2
aesni_intel           372736  0
snd_ac97_codec        139264  1 snd_intel8x0
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/dynamic$ cat /proc/devices
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
237 my_cdrv
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
*/


