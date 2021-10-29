
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h> // Required for workqueues
#include<linux/slab.h>                 //kmalloc()

static struct work_struct *workq;
void workq_fn(struct work_struct *work); 
 


void workq_fn(struct work_struct *work)
{
	long c;
	printk(KERN_INFO"my work queue function");
	
	atomic_long_set(&(workq->data),10);	
	printk(KERN_INFO "In workq function %u",atomic_long_read(&(workq->data)));
}

static int __init my_workqueue_init(void)
{
	workq = kmalloc(sizeof(struct work_struct ),GFP_KERNEL);
	INIT_WORK(workq,workq_fn);
	schedule_work(workq);

	printk(KERN_INFO"My workqueue init function");
return 0;
}

static void __exit my_workqueue_exit(void)
{
	flush_scheduled_work();
	printk(KERN_INFO"My workqueue exit function");
}

module_init(my_workqueue_init);
module_exit(my_workqueue_exit);

MODULE_AUTHOR("Aish");
MODULE_LICENSE("GPL");


/*
ishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo insmod dynamic_workq.ko
[sudo] password for aishwarya: 
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ lsmod | head
Module                  Size  Used by
dynamic_workq          16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
snd_intel8x0           45056  2
crct10dif_pclmul       16384  1
snd_ac97_codec        139264  1 snd_intel8x0
ghash_clmulni_intel    16384  0
aesni_intel           372736  0
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg|tail
[   55.869671] e1000: enp0s3 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: RX
[   55.870611] IPv6: ADDRCONF(NETDEV_CHANGE): enp0s3: link becomes ready
[   61.802350] loop11: detected capacity change from 0 to 8
[  105.828754] rfkill: input handler disabled
[  231.743266] rfkill: input handler enabled
[  247.368799] rfkill: input handler disabled
[  656.986830] dynamic_workq: loading out-of-tree module taints kernel.
[  656.986896] dynamic_workq: module verification failed: signature and/or required key missing - tainting kernel
[  656.987453] My workqueue init function
[  656.987471] my work queue function
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo rmmod dynamic_workq.ko
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg|tail
[   55.870611] IPv6: ADDRCONF(NETDEV_CHANGE): enp0s3: link becomes ready
[   61.802350] loop11: detected capacity change from 0 to 8
[  105.828754] rfkill: input handler disabled
[  231.743266] rfkill: input handler enabled
[  247.368799] rfkill: input handler disabled
[  656.986830] dynamic_workq: loading out-of-tree module taints kernel.
[  656.986896] dynamic_workq: module verification failed: signature and/or required key missing - tainting kernel
[  656.987453] My workqueue init function
[  656.987471] my work queue function
[  656.987474] In workq function 10
*/
