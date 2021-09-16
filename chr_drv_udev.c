#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h> 
#include <linux/kdev_t.h>

#define CHAR_DEV_NAME "my_cdrv"
#define MAX_LENGTH 4000
#define SUCCESS 0

static char *char_device_buf;
struct cdev *my_cdev;
dev_t mydev;
int count=1;
static struct class *my_class;

static int char_dev_open(struct inode *inode,
			    struct file  *file)
{
	static int counter = 0;
	counter++;
	printk(KERN_INFO "Number of times open() was called: %d\n", counter);
	printk (KERN_INFO " MAJOR number = %d, MINOR number = %d\n",imajor (inode), iminor (inode));
	printk(KERN_INFO "Process id of the current process: %d\n",current->pid );
	printk (KERN_INFO "ref=%d\n", module_refcount(THIS_MODULE));
	return SUCCESS;
}

static int char_dev_release(struct inode *inode,
		            struct file *file)
{
	return SUCCESS;
}

static ssize_t char_dev_read(struct file *file, 
		            char *buf,
			    size_t lbuf,
			    loff_t *ppos)
{
	int maxbytes; /* number of bytes from ppos to MAX_LENGTH */
	int bytes_to_do; /* number of bytes to read */
	int nbytes; /* number of bytes actually read */

	maxbytes = MAX_LENGTH - *ppos;
	
	if( maxbytes > lbuf ) bytes_to_do = lbuf;
	else bytes_to_do = maxbytes;
	
	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Causes read() to return EOF */
	}
	
	nbytes = bytes_to_do - 
		 raw_copy_to_user( buf, /* to */
			       char_device_buf + *ppos, /* from */
			       bytes_to_do ); /* how many bytes */

	*ppos += nbytes;
	return nbytes;	
}

static ssize_t char_dev_write(struct file *file,
		            const char *buf,
			    size_t lbuf,
			    loff_t *ppos)
{
	int nbytes; /* Number of bytes written */
	int bytes_to_do; /* Number of bytes to write */
	int maxbytes; /* Maximum number of bytes that can be written */

	maxbytes = MAX_LENGTH - *ppos;

	if( maxbytes > lbuf ) bytes_to_do = lbuf;
	else bytes_to_do = maxbytes;

	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Returns EOF at write() */
	}

	nbytes = bytes_to_do -
	         raw_copy_from_user( char_device_buf + *ppos, /* to */
				 buf, /* from */
				 bytes_to_do ); /* how many bytes */
	*ppos += nbytes;
	return nbytes;
}


static loff_t char_dev_lseek (struct file *file, loff_t offset, int orig)
{
    loff_t testpos;
    switch (orig) {

    case 0:                 /* SEEK_SET */
            testpos = offset;
            break;
    case 1:                 /* SEEK_CUR */
            testpos = file->f_pos + offset;
            break;
    case 2:                 /* SEEK_END */
           testpos = MAX_LENGTH + offset;
           break;
    default:
          return -EINVAL;
    }
   
    testpos = testpos < MAX_LENGTH ? testpos : MAX_LENGTH;
    testpos = testpos >= 0 ? testpos : 0;
    file->f_pos = testpos;
    printk (KERN_INFO "Seeking to pos=%ld\n", (long)testpos);
    return testpos;
}


static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.read = char_dev_read,
	.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release,
	.llseek = char_dev_lseek
};

static __init int char_dev_init(void)
{
	int ret;

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

	my_class = class_create (THIS_MODULE, "VIRTUAL11111");
    device_create (my_class, NULL, mydev, NULL, "%s", "my_Char_driver");

	printk(KERN_INFO"\nDevice Registered: %s\n",CHAR_DEV_NAME);
	printk(KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR (mydev),MINOR (mydev));

	char_device_buf =(char *)kmalloc(MAX_LENGTH,GFP_KERNEL);
	return 0;
}

static __exit void  char_dev_exit(void)
{
	 device_destroy (my_class, mydev);
     class_destroy (my_class);
	 cdev_del(my_cdev);
	 unregister_chrdev_region(mydev,1);
	 kfree(char_device_buf);
	 printk(KERN_INFO "\n Driver unregistered \n");
}
module_init(char_dev_init);
module_exit(char_dev_exit);

MODULE_AUTHOR("Aish");
MODULE_DESCRIPTION("Character Device Driver");
MODULE_LICENSE("GPL");


//Testapp.c is device file used for this driver testing.

/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ make
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD/Device/file modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
  CC [M]  /home/aishwarya/Documents/LDD/Device/file/chr_drv_udev.o
/home/aishwarya/Documents/LDD/Device/file/chr_drv_udev.c:7:26: warning: extra tokens at end of #include directive
    7 | #include <linux/device.h>��
      |                          ^
  MODPOST /home/aishwarya/Documents/LDD/Device/file/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/Device/file/chr_drv_udev.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/Device/file/chr_drv_udev.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ sudo insmod chr_drv_udev.ko
[sudo] password for aishwarya: 
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ lsmod | head
Module                  Size  Used by
chr_drv_udev           16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
snd_intel8x0           45056  2
aesni_intel           372736  0
snd_ac97_codec        139264  1 snd_intel8x0
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ ls -al /dev/
total 4
drwxr-xr-x  19 root      root        4140 Sep 16 18:28 .
drwxr-xr-x  20 root      root        4096 Aug 25 18:12 ..
crw-r--r--   1 root      root     10, 235 Sep 16 18:21 autofs
drwxr-xr-x   2 root      root         360 Sep 16 18:21 block
drwxr-xr-x   2 root      root          80 Sep 16 18:20 bsg
crw-------   1 root      root     10, 234 Sep 16 18:21 btrfs-control
drwxr-xr-x   3 root      root          60 Sep 16 18:21 bus
lrwxrwxrwx   1 root      root           3 Sep 16 18:21 cdrom -> sr0
drwxr-xr-x   2 root      root        3720 Sep 16 18:28 char
crw--w----   1 root      tty       5,   1 Sep 16 18:21 console
lrwxrwxrwx   1 root      root          11 Sep 16 18:20 core -> /proc/kcore
crw-------   1 root      root     10, 123 Sep 16 18:21 cpu_dma_latency
crw-------   1 root      root     10, 203 Sep 16 18:21 cuse
drwxr-xr-x   6 root      root         120 Sep 16 18:20 disk
drwxr-xr-x   2 root      root          60 Sep 16 18:21 dma_heap
drwxr-xr-x   3 root      root         100 Sep 16 18:21 dri
lrwxrwxrwx   1 root      root           3 Sep 16 18:21 dvd -> sr0
crw-------   1 root      root     10, 126 Sep 16 18:21 ecryptfs
crw-rw----   1 root      video    29,   0 Sep 16 18:21 fb0
lrwxrwxrwx   1 root      root          13 Sep 16 18:20 fd -> /proc/self/fd
crw-rw-rw-   1 root      root      1,   7 Sep 16 18:21 full
crw-rw-rw-   1 root      root     10, 229 Sep 16 18:21 fuse
crw-------   1 root      root    241,   0 Sep 16 18:21 hidraw0
crw-------   1 root      root     10, 228 Sep 16 18:21 hpet
drwxr-xr-x   2 root      root           0 Sep 16 18:21 hugepages
crw-------   1 root      root     10, 183 Sep 16 18:21 hwrng
crw-------   1 root      root     89,   0 Sep 16 18:21 i2c-0
lrwxrwxrwx   1 root      root          12 Sep 16 18:21 initctl -> /run/initctl
drwxr-xr-x   4 root      root         340 Sep 16 18:21 input
crw-r--r--   1 root      root      1,  11 Sep 16 18:21 kmsg
drwxr-xr-x   2 root      root          60 Sep 16 18:21 lightnvm
lrwxrwxrwx   1 root      root          28 Sep 16 18:21 log -> /run/systemd/journal/dev-log
brw-rw----   1 root      disk      7,   0 Sep 16 18:21 loop0
brw-rw----   1 root      disk      7,   1 Sep 16 18:21 loop1
brw-rw----   1 root      disk      7,  10 Sep 16 18:21 loop10
brw-rw----   1 root      disk      7,   2 Sep 16 18:21 loop2
brw-rw----   1 root      disk      7,   3 Sep 16 18:21 loop3
brw-rw----   1 root      disk      7,   4 Sep 16 18:21 loop4
brw-rw----   1 root      disk      7,   5 Sep 16 18:21 loop5
brw-rw----   1 root      disk      7,   6 Sep 16 18:21 loop6
brw-rw----   1 root      disk      7,   7 Sep 16 18:21 loop7
brw-rw----   1 root      disk      7,   8 Sep 16 18:21 loop8
brw-rw----   1 root      disk      7,   9 Sep 16 18:21 loop9
crw-rw----   1 root      disk     10, 237 Sep 16 18:21 loop-control
drwxr-xr-x   2 root      root          60 Sep 16 18:20 mapper
crw-------   1 root      root     10, 227 Sep 16 18:21 mcelog
crw-r-----   1 root      kmem      1,   1 Sep 16 18:21 mem
drwxrwxrwt   2 root      root          40 Sep 16 18:21 mqueue
crw-------   1 root      root    237,   0 Sep 16 18:28 my_Char_driver
drwxr-xr-x   2 root      root          60 Sep 16 18:21 net
crw-rw-rw-   1 root      root      1,   3 Sep 16 18:21 null
crw-------   1 root      root     10, 144 Sep 16 18:21 nvram
crw-r-----   1 root      kmem      1,   4 Sep 16 18:21 port
crw-------   1 root      root    108,   0 Sep 16 18:21 ppp
crw-------   1 root      root     10,   1 Sep 16 18:21 psaux
crw-rw-rw-   1 root      tty       5,   2 Sep 16 18:29 ptmx
drwxr-xr-x   2 root      root           0 Sep 16 18:20 pts
crw-rw-rw-   1 root      root      1,   8 Sep 16 18:21 random
crw-rw-r--+  1 root      root     10, 242 Sep 16 18:21 rfkill
lrwxrwxrwx   1 root      root           4 Sep 16 18:21 rtc -> rtc0
crw-------   1 root      root    248,   0 Sep 16 18:21 rtc0
brw-rw----   1 root      disk      8,   0 Sep 16 18:21 sda
brw-rw----   1 root      disk      8,   1 Sep 16 18:21 sda1
brw-rw----   1 root      disk      8,   2 Sep 16 18:21 sda2
brw-rw----   1 root      disk      8,   5 Sep 16 18:21 sda5
crw-rw----+  1 root      cdrom    21,   0 Sep 16 18:21 sg0
crw-rw----   1 root      disk     21,   1 Sep 16 18:21 sg1
drwxrwxrwt   2 root      root          40 Sep 16 18:20 shm
crw-------   1 root      root     10, 231 Sep 16 18:21 snapshot
drwxr-xr-x   3 root      root         180 Sep 16 18:21 snd
brw-rw----+  1 root      cdrom    11,   0 Sep 16 18:21 sr0
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stderr -> /proc/self/fd/2
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stdin -> /proc/self/fd/0
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stdout -> /proc/self/fd/1
crw-rw-rw-   1 root      tty       5,   0 Sep 16 18:28 tty
crw--w----   1 root      tty       4,   0 Sep 16 18:21 tty0
crw--w----   1 root      tty       4,   1 Sep 16 18:21 tty1
crw--w----   1 root      tty       4,  10 Sep 16 18:21 tty10
crw--w----   1 root      tty       4,  11 Sep 16 18:21 tty11
crw--w----   1 root      tty       4,  12 Sep 16 18:21 tty12
crw--w----   1 root      tty       4,  13 Sep 16 18:21 tty13
crw--w----   1 root      tty       4,  14 Sep 16 18:21 tty14
crw--w----   1 root      tty       4,  15 Sep 16 18:21 tty15
crw--w----   1 root      tty       4,  16 Sep 16 18:21 tty16
crw--w----   1 root      tty       4,  17 Sep 16 18:21 tty17
crw--w----   1 root      tty       4,  18 Sep 16 18:21 tty18
crw--w----   1 root      tty       4,  19 Sep 16 18:21 tty19
crw--w----   1 aishwarya tty       4,   2 Sep 16 18:20 tty2
crw--w----   1 root      tty       4,  20 Sep 16 18:21 tty20
crw--w----   1 root      tty       4,  21 Sep 16 18:21 tty21
crw--w----   1 root      tty       4,  22 Sep 16 18:21 tty22
crw--w----   1 root      tty       4,  23 Sep 16 18:21 tty23
crw--w----   1 root      tty       4,  24 Sep 16 18:21 tty24
crw--w----   1 root      tty       4,  25 Sep 16 18:21 tty25
crw--w----   1 root      tty       4,  26 Sep 16 18:21 tty26
crw--w----   1 root      tty       4,  27 Sep 16 18:21 tty27
crw--w----   1 root      tty       4,  28 Sep 16 18:21 tty28
crw--w----   1 root      tty       4,  29 Sep 16 18:21 tty29
crw--w----   1 root      tty       4,   3 Sep 16 18:21 tty3
crw--w----   1 root      tty       4,  30 Sep 16 18:21 tty30
crw--w----   1 root      tty       4,  31 Sep 16 18:21 tty31
crw--w----   1 root      tty       4,  32 Sep 16 18:21 tty32
crw--w----   1 root      tty       4,  33 Sep 16 18:21 tty33
crw--w----   1 root      tty       4,  34 Sep 16 18:21 tty34
crw--w----   1 root      tty       4,  35 Sep 16 18:21 tty35
crw--w----   1 root      tty       4,  36 Sep 16 18:21 tty36
crw--w----   1 root      tty       4,  37 Sep 16 18:21 tty37
crw--w----   1 root      tty       4,  38 Sep 16 18:21 tty38
crw--w----   1 root      tty       4,  39 Sep 16 18:21 tty39
crw--w----   1 root      tty       4,   4 Sep 16 18:21 tty4
crw--w----   1 root      tty       4,  40 Sep 16 18:21 tty40
crw--w----   1 root      tty       4,  41 Sep 16 18:21 tty41
crw--w----   1 root      tty       4,  42 Sep 16 18:21 tty42
crw--w----   1 root      tty       4,  43 Sep 16 18:21 tty43
crw--w----   1 root      tty       4,  44 Sep 16 18:21 tty44
crw--w----   1 root      tty       4,  45 Sep 16 18:21 tty45
crw--w----   1 root      tty       4,  46 Sep 16 18:21 tty46
crw--w----   1 root      tty       4,  47 Sep 16 18:21 tty47
crw--w----   1 root      tty       4,  48 Sep 16 18:21 tty48
crw--w----   1 root      tty       4,  49 Sep 16 18:21 tty49
crw--w----   1 root      tty       4,   5 Sep 16 18:21 tty5
crw--w----   1 root      tty       4,  50 Sep 16 18:21 tty50
crw--w----   1 root      tty       4,  51 Sep 16 18:21 tty51
crw--w----   1 root      tty       4,  52 Sep 16 18:21 tty52
crw--w----   1 root      tty       4,  53 Sep 16 18:21 tty53
crw--w----   1 root      tty       4,  54 Sep 16 18:21 tty54
crw--w----   1 root      tty       4,  55 Sep 16 18:21 tty55
crw--w----   1 root      tty       4,  56 Sep 16 18:21 tty56
crw--w----   1 root      tty       4,  57 Sep 16 18:21 tty57
crw--w----   1 root      tty       4,  58 Sep 16 18:21 tty58
crw--w----   1 root      tty       4,  59 Sep 16 18:21 tty59
crw--w----   1 root      tty       4,   6 Sep 16 18:21 tty6
crw--w----   1 root      tty       4,  60 Sep 16 18:21 tty60
crw--w----   1 root      tty       4,  61 Sep 16 18:21 tty61
crw--w----   1 root      tty       4,  62 Sep 16 18:21 tty62
crw--w----   1 root      tty       4,  63 Sep 16 18:21 tty63
crw--w----   1 root      tty       4,   7 Sep 16 18:21 tty7
crw--w----   1 root      tty       4,   8 Sep 16 18:21 tty8
crw--w----   1 root      tty       4,   9 Sep 16 18:21 tty9
crw-------   1 root      root      5,   3 Sep 16 18:21 ttyprintk
crw-rw----   1 root      dialout   4,  64 Sep 16 18:21 ttyS0
crw-rw----   1 root      dialout   4,  65 Sep 16 18:21 ttyS1
crw-rw----   1 root      dialout   4,  74 Sep 16 18:21 ttyS10
crw-rw----   1 root      dialout   4,  75 Sep 16 18:21 ttyS11
crw-rw----   1 root      dialout   4,  76 Sep 16 18:21 ttyS12
crw-rw----   1 root      dialout   4,  77 Sep 16 18:21 ttyS13
crw-rw----   1 root      dialout   4,  78 Sep 16 18:21 ttyS14
crw-rw----   1 root      dialout   4,  79 Sep 16 18:21 ttyS15
crw-rw----   1 root      dialout   4,  80 Sep 16 18:21 ttyS16
crw-rw----   1 root      dialout   4,  81 Sep 16 18:21 ttyS17
crw-rw----   1 root      dialout   4,  82 Sep 16 18:21 ttyS18
crw-rw----   1 root      dialout   4,  83 Sep 16 18:21 ttyS19
crw-rw----   1 root      dialout   4,  66 Sep 16 18:21 ttyS2
crw-rw----   1 root      dialout   4,  84 Sep 16 18:21 ttyS20
crw-rw----   1 root      dialout   4,  85 Sep 16 18:21 ttyS21
crw-rw----   1 root      dialout   4,  86 Sep 16 18:21 ttyS22
crw-rw----   1 root      dialout   4,  87 Sep 16 18:21 ttyS23
crw-rw----   1 root      dialout   4,  88 Sep 16 18:21 ttyS24
crw-rw----   1 root      dialout   4,  89 Sep 16 18:21 ttyS25
crw-rw----   1 root      dialout   4,  90 Sep 16 18:21 ttyS26
crw-rw----   1 root      dialout   4,  91 Sep 16 18:21 ttyS27
crw-rw----   1 root      dialout   4,  92 Sep 16 18:21 ttyS28
crw-rw----   1 root      dialout   4,  93 Sep 16 18:21 ttyS29
crw-rw----   1 root      dialout   4,  67 Sep 16 18:21 ttyS3
crw-rw----   1 root      dialout   4,  94 Sep 16 18:21 ttyS30
crw-rw----   1 root      dialout   4,  95 Sep 16 18:21 ttyS31
crw-rw----   1 root      dialout   4,  68 Sep 16 18:21 ttyS4
crw-rw----   1 root      dialout   4,  69 Sep 16 18:21 ttyS5
crw-rw----   1 root      dialout   4,  70 Sep 16 18:21 ttyS6
crw-rw----   1 root      dialout   4,  71 Sep 16 18:21 ttyS7
crw-rw----   1 root      dialout   4,  72 Sep 16 18:21 ttyS8
crw-rw----   1 root      dialout   4,  73 Sep 16 18:21 ttyS9
crw-rw----   1 root      kvm      10, 124 Sep 16 18:21 udmabuf
crw-------   1 root      root     10, 239 Sep 16 18:21 uhid
crw-------   1 root      root     10, 223 Sep 16 18:21 uinput
crw-rw-rw-   1 root      root      1,   9 Sep 16 18:21 urandom
crw-------   1 root      root     10, 240 Sep 16 18:21 userio
crw-------   1 root      root     10, 122 Sep 16 18:21 vboxguest
crw-------   1 root      root     10, 121 Sep 16 18:21 vboxuser
crw-rw----   1 root      tty       7,   0 Sep 16 18:21 vcs
crw-rw----   1 root      tty       7,   1 Sep 16 18:21 vcs1
crw-rw----   1 root      tty       7,   2 Sep 16 18:21 vcs2
crw-rw----   1 root      tty       7,   3 Sep 16 18:21 vcs3
crw-rw----   1 root      tty       7,   4 Sep 16 18:21 vcs4
crw-rw----   1 root      tty       7,   5 Sep 16 18:21 vcs5
crw-rw----   1 root      tty       7,   6 Sep 16 18:21 vcs6
crw-rw----   1 root      tty       7, 128 Sep 16 18:21 vcsa
crw-rw----   1 root      tty       7, 129 Sep 16 18:21 vcsa1
crw-rw----   1 root      tty       7, 130 Sep 16 18:21 vcsa2
crw-rw----   1 root      tty       7, 131 Sep 16 18:21 vcsa3
crw-rw----   1 root      tty       7, 132 Sep 16 18:21 vcsa4
crw-rw----   1 root      tty       7, 133 Sep 16 18:21 vcsa5
crw-rw----   1 root      tty       7, 134 Sep 16 18:21 vcsa6
crw-rw----   1 root      tty       7,  64 Sep 16 18:21 vcsu
crw-rw----   1 root      tty       7,  65 Sep 16 18:21 vcsu1
crw-rw----   1 root      tty       7,  66 Sep 16 18:21 vcsu2
crw-rw----   1 root      tty       7,  67 Sep 16 18:21 vcsu3
crw-rw----   1 root      tty       7,  68 Sep 16 18:21 vcsu4
crw-rw----   1 root      tty       7,  69 Sep 16 18:21 vcsu5
crw-rw----   1 root      tty       7,  70 Sep 16 18:21 vcsu6
drwxr-xr-x   2 root      root          60 Sep 16 18:21 vfio
crw-------   1 root      root     10, 127 Sep 16 18:21 vga_arbiter
crw-------   1 root      root     10, 137 Sep 16 18:21 vhci
crw-------   1 root      root     10, 238 Sep 16 18:21 vhost-net
crw-------   1 root      root     10, 241 Sep 16 18:21 vhost-vsock
crw-rw-rw-   1 root      root      1,   5 Sep 16 18:21 zero
crw-------   1 root      root     10, 249 Sep 16 18:21 zfs


aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ dmesg | tail
                exe="/usr/bin/dbus-daemon" sauid=103 hostname=? addr=? terminal=?'
[  143.545425] audit: type=1400 audit(1631796651.803:44): apparmor="DENIED" operation="open" profile="snap.snap-store.ubuntu-software" name="/etc/PackageKit/Vendor.conf" pid=1708 comm="snap-store" requested_mask="r" denied_mask="r" fsuid=1000 ouid=0
[  223.247680] audit: type=1400 audit(1631796731.518:45): apparmor="DENIED" operation="open" profile="snap.snap-store.ubuntu-software" name="/var/lib/snapd/hostfs/usr/share/gdm/greeter/applications/gnome-initial-setup.desktop" pid=1708 comm="snap-store" requested_mask="r" denied_mask="r" fsuid=1000 ouid=0
[  223.822214] audit: type=1400 audit(1631796732.094:46): apparmor="DENIED" operation="open" profile="snap.snap-store.ubuntu-software" name="/var/lib/snapd/hostfs/usr/share/gdm/greeter/applications/gnome-initial-setup.desktop" pid=1708 comm="snap-store" requested_mask="r" denied_mask="r" fsuid=1000 ouid=0
[  227.208864] audit: type=1326 audit(1631796735.478:47): auid=1000 uid=1000 gid=1000 ses=3 subj=snap.snap-store.ubuntu-software pid=1708 comm="snap-store" exe="/snap/snap-store/547/usr/bin/snap-store" sig=0 arch=c000003e syscall=93 compat=0 ip=0x7f490f2714e7 code=0x50000
[  621.923280] chr_drv_udev: loading out-of-tree module taints kernel.
[  621.923351] chr_drv_udev: module verification failed: signature and/or required key missing - tainting kernel
[  621.924639] 
               Device Registered: my_cdrv
[  621.924645] Major number = 237, Minor number = 0
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ cat /proc/devices
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

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ sudo chmod 777 /dev/my_Char_driver
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ ls -al /dev/ 
total 4
drwxr-xr-x  19 root      root        4140 Sep 16 18:28 .
drwxr-xr-x  20 root      root        4096 Aug 25 18:12 ..
crw-r--r--   1 root      root     10, 235 Sep 16 18:21 autofs
drwxr-xr-x   2 root      root         360 Sep 16 18:21 block
drwxr-xr-x   2 root      root          80 Sep 16 18:20 bsg
crw-------   1 root      root     10, 234 Sep 16 18:21 btrfs-control
drwxr-xr-x   3 root      root          60 Sep 16 18:21 bus
lrwxrwxrwx   1 root      root           3 Sep 16 18:21 cdrom -> sr0
drwxr-xr-x   2 root      root        3720 Sep 16 18:28 char
crw--w----   1 root      tty       5,   1 Sep 16 18:21 console
lrwxrwxrwx   1 root      root          11 Sep 16 18:20 core -> /proc/kcore
crw-------   1 root      root     10, 123 Sep 16 18:21 cpu_dma_latency
crw-------   1 root      root     10, 203 Sep 16 18:21 cuse
drwxr-xr-x   6 root      root         120 Sep 16 18:20 disk
drwxr-xr-x   2 root      root          60 Sep 16 18:21 dma_heap
drwxr-xr-x   3 root      root         100 Sep 16 18:21 dri
lrwxrwxrwx   1 root      root           3 Sep 16 18:21 dvd -> sr0
crw-------   1 root      root     10, 126 Sep 16 18:21 ecryptfs
crw-rw----   1 root      video    29,   0 Sep 16 18:21 fb0
lrwxrwxrwx   1 root      root          13 Sep 16 18:20 fd -> /proc/self/fd
crw-rw-rw-   1 root      root      1,   7 Sep 16 18:21 full
crw-rw-rw-   1 root      root     10, 229 Sep 16 18:21 fuse
crw-------   1 root      root    241,   0 Sep 16 18:21 hidraw0
crw-------   1 root      root     10, 228 Sep 16 18:21 hpet
drwxr-xr-x   2 root      root           0 Sep 16 18:21 hugepages
crw-------   1 root      root     10, 183 Sep 16 18:21 hwrng
crw-------   1 root      root     89,   0 Sep 16 18:21 i2c-0
lrwxrwxrwx   1 root      root          12 Sep 16 18:21 initctl -> /run/initctl
drwxr-xr-x   4 root      root         340 Sep 16 18:21 input
crw-r--r--   1 root      root      1,  11 Sep 16 18:21 kmsg
drwxr-xr-x   2 root      root          60 Sep 16 18:21 lightnvm
lrwxrwxrwx   1 root      root          28 Sep 16 18:21 log -> /run/systemd/journal/dev-log
brw-rw----   1 root      disk      7,   0 Sep 16 18:21 loop0
brw-rw----   1 root      disk      7,   1 Sep 16 18:21 loop1
brw-rw----   1 root      disk      7,  10 Sep 16 18:21 loop10
brw-rw----   1 root      disk      7,   2 Sep 16 18:21 loop2
brw-rw----   1 root      disk      7,   3 Sep 16 18:21 loop3
brw-rw----   1 root      disk      7,   4 Sep 16 18:21 loop4
brw-rw----   1 root      disk      7,   5 Sep 16 18:21 loop5
brw-rw----   1 root      disk      7,   6 Sep 16 18:21 loop6
brw-rw----   1 root      disk      7,   7 Sep 16 18:21 loop7
brw-rw----   1 root      disk      7,   8 Sep 16 18:21 loop8
brw-rw----   1 root      disk      7,   9 Sep 16 18:21 loop9
crw-rw----   1 root      disk     10, 237 Sep 16 18:21 loop-control
drwxr-xr-x   2 root      root          60 Sep 16 18:20 mapper
crw-------   1 root      root     10, 227 Sep 16 18:21 mcelog
crw-r-----   1 root      kmem      1,   1 Sep 16 18:21 mem
drwxrwxrwt   2 root      root          40 Sep 16 18:21 mqueue
crwxrwxrwx   1 root      root    237,   0 Sep 16 18:28 my_Char_driver
drwxr-xr-x   2 root      root          60 Sep 16 18:21 net
crw-rw-rw-   1 root      root      1,   3 Sep 16 18:21 null
crw-------   1 root      root     10, 144 Sep 16 18:21 nvram
crw-r-----   1 root      kmem      1,   4 Sep 16 18:21 port
crw-------   1 root      root    108,   0 Sep 16 18:21 ppp
crw-------   1 root      root     10,   1 Sep 16 18:21 psaux
crw-rw-rw-   1 root      tty       5,   2 Sep 16 18:34 ptmx
drwxr-xr-x   2 root      root           0 Sep 16 18:20 pts
crw-rw-rw-   1 root      root      1,   8 Sep 16 18:21 random
crw-rw-r--+  1 root      root     10, 242 Sep 16 18:21 rfkill
lrwxrwxrwx   1 root      root           4 Sep 16 18:21 rtc -> rtc0
crw-------   1 root      root    248,   0 Sep 16 18:21 rtc0
brw-rw----   1 root      disk      8,   0 Sep 16 18:21 sda
brw-rw----   1 root      disk      8,   1 Sep 16 18:21 sda1
brw-rw----   1 root      disk      8,   2 Sep 16 18:21 sda2
brw-rw----   1 root      disk      8,   5 Sep 16 18:21 sda5
crw-rw----+  1 root      cdrom    21,   0 Sep 16 18:21 sg0
crw-rw----   1 root      disk     21,   1 Sep 16 18:21 sg1
drwxrwxrwt   2 root      root          40 Sep 16 18:20 shm
crw-------   1 root      root     10, 231 Sep 16 18:21 snapshot
drwxr-xr-x   3 root      root         180 Sep 16 18:21 snd
brw-rw----+  1 root      cdrom    11,   0 Sep 16 18:21 sr0
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stderr -> /proc/self/fd/2
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stdin -> /proc/self/fd/0
lrwxrwxrwx   1 root      root          15 Sep 16 18:20 stdout -> /proc/self/fd/1
crw-rw-rw-   1 root      tty       5,   0 Sep 16 18:28 tty
crw--w----   1 root      tty       4,   0 Sep 16 18:21 tty0
crw--w----   1 root      tty       4,   1 Sep 16 18:21 tty1
crw--w----   1 root      tty       4,  10 Sep 16 18:21 tty10
crw--w----   1 root      tty       4,  11 Sep 16 18:21 tty11
crw--w----   1 root      tty       4,  12 Sep 16 18:21 tty12
crw--w----   1 root      tty       4,  13 Sep 16 18:21 tty13
crw--w----   1 root      tty       4,  14 Sep 16 18:21 tty14
crw--w----   1 root      tty       4,  15 Sep 16 18:21 tty15
crw--w----   1 root      tty       4,  16 Sep 16 18:21 tty16
crw--w----   1 root      tty       4,  17 Sep 16 18:21 tty17
crw--w----   1 root      tty       4,  18 Sep 16 18:21 tty18
crw--w----   1 root      tty       4,  19 Sep 16 18:21 tty19
crw--w----   1 aishwarya tty       4,   2 Sep 16 18:20 tty2
crw--w----   1 root      tty       4,  20 Sep 16 18:21 tty20
crw--w----   1 root      tty       4,  21 Sep 16 18:21 tty21
crw--w----   1 root      tty       4,  22 Sep 16 18:21 tty22
crw--w----   1 root      tty       4,  23 Sep 16 18:21 tty23
crw--w----   1 root      tty       4,  24 Sep 16 18:21 tty24
crw--w----   1 root      tty       4,  25 Sep 16 18:21 tty25
crw--w----   1 root      tty       4,  26 Sep 16 18:21 tty26
crw--w----   1 root      tty       4,  27 Sep 16 18:21 tty27
crw--w----   1 root      tty       4,  28 Sep 16 18:21 tty28
crw--w----   1 root      tty       4,  29 Sep 16 18:21 tty29
crw--w----   1 root      tty       4,   3 Sep 16 18:21 tty3
crw--w----   1 root      tty       4,  30 Sep 16 18:21 tty30
crw--w----   1 root      tty       4,  31 Sep 16 18:21 tty31
crw--w----   1 root      tty       4,  32 Sep 16 18:21 tty32
crw--w----   1 root      tty       4,  33 Sep 16 18:21 tty33
crw--w----   1 root      tty       4,  34 Sep 16 18:21 tty34
crw--w----   1 root      tty       4,  35 Sep 16 18:21 tty35
crw--w----   1 root      tty       4,  36 Sep 16 18:21 tty36
crw--w----   1 root      tty       4,  37 Sep 16 18:21 tty37
crw--w----   1 root      tty       4,  38 Sep 16 18:21 tty38
crw--w----   1 root      tty       4,  39 Sep 16 18:21 tty39
crw--w----   1 root      tty       4,   4 Sep 16 18:21 tty4
crw--w----   1 root      tty       4,  40 Sep 16 18:21 tty40
crw--w----   1 root      tty       4,  41 Sep 16 18:21 tty41
crw--w----   1 root      tty       4,  42 Sep 16 18:21 tty42
crw--w----   1 root      tty       4,  43 Sep 16 18:21 tty43
crw--w----   1 root      tty       4,  44 Sep 16 18:21 tty44
crw--w----   1 root      tty       4,  45 Sep 16 18:21 tty45
crw--w----   1 root      tty       4,  46 Sep 16 18:21 tty46
crw--w----   1 root      tty       4,  47 Sep 16 18:21 tty47
crw--w----   1 root      tty       4,  48 Sep 16 18:21 tty48
crw--w----   1 root      tty       4,  49 Sep 16 18:21 tty49
crw--w----   1 root      tty       4,   5 Sep 16 18:21 tty5
crw--w----   1 root      tty       4,  50 Sep 16 18:21 tty50
crw--w----   1 root      tty       4,  51 Sep 16 18:21 tty51
crw--w----   1 root      tty       4,  52 Sep 16 18:21 tty52
crw--w----   1 root      tty       4,  53 Sep 16 18:21 tty53
crw--w----   1 root      tty       4,  54 Sep 16 18:21 tty54
crw--w----   1 root      tty       4,  55 Sep 16 18:21 tty55
crw--w----   1 root      tty       4,  56 Sep 16 18:21 tty56
crw--w----   1 root      tty       4,  57 Sep 16 18:21 tty57
crw--w----   1 root      tty       4,  58 Sep 16 18:21 tty58
crw--w----   1 root      tty       4,  59 Sep 16 18:21 tty59
crw--w----   1 root      tty       4,   6 Sep 16 18:21 tty6
crw--w----   1 root      tty       4,  60 Sep 16 18:21 tty60
crw--w----   1 root      tty       4,  61 Sep 16 18:21 tty61
crw--w----   1 root      tty       4,  62 Sep 16 18:21 tty62
crw--w----   1 root      tty       4,  63 Sep 16 18:21 tty63
crw--w----   1 root      tty       4,   7 Sep 16 18:21 tty7
crw--w----   1 root      tty       4,   8 Sep 16 18:21 tty8
crw--w----   1 root      tty       4,   9 Sep 16 18:21 tty9
crw-------   1 root      root      5,   3 Sep 16 18:21 ttyprintk
crw-rw----   1 root      dialout   4,  64 Sep 16 18:21 ttyS0
crw-rw----   1 root      dialout   4,  65 Sep 16 18:21 ttyS1
crw-rw----   1 root      dialout   4,  74 Sep 16 18:21 ttyS10
crw-rw----   1 root      dialout   4,  75 Sep 16 18:21 ttyS11
crw-rw----   1 root      dialout   4,  76 Sep 16 18:21 ttyS12
crw-rw----   1 root      dialout   4,  77 Sep 16 18:21 ttyS13
crw-rw----   1 root      dialout   4,  78 Sep 16 18:21 ttyS14
crw-rw----   1 root      dialout   4,  79 Sep 16 18:21 ttyS15
crw-rw----   1 root      dialout   4,  80 Sep 16 18:21 ttyS16
crw-rw----   1 root      dialout   4,  81 Sep 16 18:21 ttyS17
crw-rw----   1 root      dialout   4,  82 Sep 16 18:21 ttyS18
crw-rw----   1 root      dialout   4,  83 Sep 16 18:21 ttyS19
crw-rw----   1 root      dialout   4,  66 Sep 16 18:21 ttyS2
crw-rw----   1 root      dialout   4,  84 Sep 16 18:21 ttyS20
crw-rw----   1 root      dialout   4,  85 Sep 16 18:21 ttyS21
crw-rw----   1 root      dialout   4,  86 Sep 16 18:21 ttyS22
crw-rw----   1 root      dialout   4,  87 Sep 16 18:21 ttyS23
crw-rw----   1 root      dialout   4,  88 Sep 16 18:21 ttyS24
crw-rw----   1 root      dialout   4,  89 Sep 16 18:21 ttyS25
crw-rw----   1 root      dialout   4,  90 Sep 16 18:21 ttyS26
crw-rw----   1 root      dialout   4,  91 Sep 16 18:21 ttyS27
crw-rw----   1 root      dialout   4,  92 Sep 16 18:21 ttyS28
crw-rw----   1 root      dialout   4,  93 Sep 16 18:21 ttyS29
crw-rw----   1 root      dialout   4,  67 Sep 16 18:21 ttyS3
crw-rw----   1 root      dialout   4,  94 Sep 16 18:21 ttyS30
crw-rw----   1 root      dialout   4,  95 Sep 16 18:21 ttyS31
crw-rw----   1 root      dialout   4,  68 Sep 16 18:21 ttyS4
crw-rw----   1 root      dialout   4,  69 Sep 16 18:21 ttyS5
crw-rw----   1 root      dialout   4,  70 Sep 16 18:21 ttyS6
crw-rw----   1 root      dialout   4,  71 Sep 16 18:21 ttyS7
crw-rw----   1 root      dialout   4,  72 Sep 16 18:21 ttyS8
crw-rw----   1 root      dialout   4,  73 Sep 16 18:21 ttyS9
crw-rw----   1 root      kvm      10, 124 Sep 16 18:21 udmabuf
crw-------   1 root      root     10, 239 Sep 16 18:21 uhid
crw-------   1 root      root     10, 223 Sep 16 18:21 uinput
crw-rw-rw-   1 root      root      1,   9 Sep 16 18:21 urandom
crw-------   1 root      root     10, 240 Sep 16 18:21 userio
crw-------   1 root      root     10, 122 Sep 16 18:21 vboxguest
crw-------   1 root      root     10, 121 Sep 16 18:21 vboxuser
crw-rw----   1 root      tty       7,   0 Sep 16 18:21 vcs
crw-rw----   1 root      tty       7,   1 Sep 16 18:21 vcs1
crw-rw----   1 root      tty       7,   2 Sep 16 18:21 vcs2
crw-rw----   1 root      tty       7,   3 Sep 16 18:21 vcs3
crw-rw----   1 root      tty       7,   4 Sep 16 18:21 vcs4
crw-rw----   1 root      tty       7,   5 Sep 16 18:21 vcs5
crw-rw----   1 root      tty       7,   6 Sep 16 18:21 vcs6
crw-rw----   1 root      tty       7, 128 Sep 16 18:21 vcsa
crw-rw----   1 root      tty       7, 129 Sep 16 18:21 vcsa1
crw-rw----   1 root      tty       7, 130 Sep 16 18:21 vcsa2
crw-rw----   1 root      tty       7, 131 Sep 16 18:21 vcsa3
crw-rw----   1 root      tty       7, 132 Sep 16 18:21 vcsa4
crw-rw----   1 root      tty       7, 133 Sep 16 18:21 vcsa5
crw-rw----   1 root      tty       7, 134 Sep 16 18:21 vcsa6
crw-rw----   1 root      tty       7,  64 Sep 16 18:21 vcsu
crw-rw----   1 root      tty       7,  65 Sep 16 18:21 vcsu1
crw-rw----   1 root      tty       7,  66 Sep 16 18:21 vcsu2
crw-rw----   1 root      tty       7,  67 Sep 16 18:21 vcsu3
crw-rw----   1 root      tty       7,  68 Sep 16 18:21 vcsu4
crw-rw----   1 root      tty       7,  69 Sep 16 18:21 vcsu5
crw-rw----   1 root      tty       7,  70 Sep 16 18:21 vcsu6
drwxr-xr-x   2 root      root          60 Sep 16 18:21 vfio
crw-------   1 root      root     10, 127 Sep 16 18:21 vga_arbiter
crw-------   1 root      root     10, 137 Sep 16 18:21 vhci
crw-------   1 root      root     10, 238 Sep 16 18:21 vhost-net
crw-------   1 root      root     10, 241 Sep 16 18:21 vhost-vsock
crw-rw-rw-   1 root      root      1,   5 Sep 16 18:21 zero
crw-------   1 root      root     10, 249 Sep 16 18:21 zfs

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ cc Testapp.c
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ ./a.out
****Please Enter the Option******
        1. Write               
        2. Read                 
		3. IOCTL				
        4. Exit                 
*********************************
1
Your Option = 1
Enter the string to write into driver :hello
Data Writing ...Done!
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ dmesg | tail
[  227.208864] audit: type=1326 audit(1631796735.478:47): auid=1000 uid=1000 gid=1000 ses=3 subj=snap.snap-store.ubuntu-software pid=1708 comm="snap-store" exe="/snap/snap-store/547/usr/bin/snap-store" sig=0 arch=c000003e syscall=93 compat=0 ip=0x7f490f2714e7 code=0x50000
[  621.923280] chr_drv_udev: loading out-of-tree module taints kernel.
[  621.923351] chr_drv_udev: module verification failed: signature and/or required key missing - tainting kernel
[  621.924639] 
               Device Registered: my_cdrv
[  621.924645] Major number = 237, Minor number = 0
[  780.552114] Number of times open() was called: 1
[  780.552126]  MAJOR number = 237, MINOR number = 0
[  780.552132] Process id of the current process: 4388
[  780.552137] ref=1
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ ./a.out
****Please Enter the Option******
        1. Write               
        2. Read                 
		3. IOCTL				
        4. Exit                 
*********************************
2
Your Option = 2
Data Reading ...Done!

Data = hello

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ dmesg | tail
               Device Registered: my_cdrv
[  621.924645] Major number = 237, Minor number = 0
[  780.552114] Number of times open() was called: 1
[  780.552126]  MAJOR number = 237, MINOR number = 0
[  780.552132] Process id of the current process: 4388
[  780.552137] ref=1
[  804.935094] Number of times open() was called: 2
[  804.935104]  MAJOR number = 237, MINOR number = 0
[  804.935108] Process id of the current process: 4394
[  804.935111] ref=1
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Device/file$ ./a.out
****Please Enter the Option******
        1. Write               
        2. Read                 
		3. IOCTL				
        4. Exit                 
*********************************
4
Your Option = 4

*/



