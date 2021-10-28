
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h> // Required for workqueues

void workq_fn(struct work_struct *work); 
DECLARE_WORK(workq,workq_fn); 

void workq_fn(struct work_struct *work)
{
	
	long c;
	printk(KERN_INFO"Work queue static function");
	
	if(in_interrupt())
        printk(KERN_INFO "Running as an interrupt");
	else
		 printk(KERN_INFO "Running as an processt");
		 
	atomic_long_set(&(workq.data),10);	
	printk(KERN_INFO "In workq function %u",atomic_long_read(&(workq.data)));
}

static int __init my_workqueue_init(void)
{
	schedule_work(&workq);
	printk(KERN_INFO"My workqueue init static function");
	return 0;
}

static void __exit my_workqueue_exit(void)
{
	flush_scheduled_work();
	printk(KERN_INFO"My workqueue exit  static function");
}

module_init(my_workqueue_init);
module_exit(my_workqueue_exit);

MODULE_AUTHOR("Aish");
MODULE_LICENSE("GPL");

/*
ishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ lsmod | head
Module                  Size  Used by
static_workq           16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
aesni_intel           372736  0
crypto_simd            16384  1 aesni_intel
cryptd                 24576  2 crypto_simd,ghash_clmulni_intel
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg | tail
[   52.391853] loop11: detected capacity change from 0 to 8
[   88.648321] rfkill: input handler disabled
[   94.598624] loop11: detected capacity change from 0 to 113656
[  519.152010] rfkill: input handler enabled
[  538.140800] rfkill: input handler disabled
[  837.005260] static_workq: loading out-of-tree module taints kernel.
[  837.005327] static_workq: module verification failed: signature and/or required key missing - tainting kernel
[  837.005938] My workqueue init static function
[  837.005957] Work queue static function
[  837.005959] Running as an processt
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo rmmod static_workq.ko
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg | tail
[   88.648321] rfkill: input handler disabled
[   94.598624] loop11: detected capacity change from 0 to 113656
[  519.152010] rfkill: input handler enabled
[  538.140800] rfkill: input handler disabled
[  837.005260] static_workq: loading out-of-tree module taints kernel.
[  837.005327] static_workq: module verification failed: signature and/or required key missing - tainting kernel
[  837.005938] My workqueue init static function
[  837.005957] Work queue static function
[  837.005959] Running as an processt
[  837.005961] In workq function 10

*/

