
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h> // Required for workqueues

static struct workqueue_struct *own_workqueue;
 
static void workqueue_fn(struct work_struct *work); 

static DECLARE_DELAYED_WORK(work, workqueue_fn);
 unsigned long onesec;
/*Workqueue Function*/
static void workqueue_fn(struct work_struct *work)
{
    printk(KERN_INFO "Executing my own Workqueue Function\n");
	printk(KERN_INFO "mydelayed  work %u jiffies\n", (unsigned)onesec);
    return;     
}

static int __init my_workqueue_init(void)
{
	 onesec = msecs_to_jiffies(5000);
	 printk(KERN_INFO "mydelayed  loaded %u jiffies\n", (unsigned)onesec);
	 if(own_workqueue==NULL)
	 {
		printk(KERN_INFO "Creating workqueue ");
		own_workqueue = create_workqueue("own_wq");	
	 }
	 if(own_workqueue)
	 {
		 printk(KERN_INFO "delayed workqueue ");
		 queue_delayed_work(own_workqueue,&work,onesec);
	 }
	

	printk(KERN_INFO"My own workqueue init function");
return 0;
}

static void __exit my_workqueue_exit(void)
{
	/* Delete workqueue */
    destroy_workqueue(own_workqueue);
	printk(KERN_INFO"My own workqueue exit  bye bye bye function");
}

module_init(my_workqueue_init);
module_exit(my_workqueue_exit);

MODULE_AUTHOR("Aish");
MODULE_LICENSE("GPL");

/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo insmod workq_delay.ko
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg | tail
[ 1334.342330] My own workqueue init hi hi hi  function
[ 1335.348940] Executing my own Workqueue Function
[ 1335.348948] mydelayed  work 250 jiffies
[ 1379.226016] My own workqueue exit  bye bye bye function
[ 1407.064419] mydelayed  loaded 1250 jiffies
[ 1407.064429] Creating workqueue 
[ 1407.064739] delayed workqueue 
[ 1407.064745] My own workqueue init function
[ 1412.173711] Executing my own Workqueue Function
[ 1412.173719] mydelayed  work 1250 jiffies
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo rmmod workq_delay.k
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo insmod workq_delay.ko
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ dmesg | tail
[ 1407.064419] mydelayed  loaded 1250 jiffies
[ 1407.064429] Creating workqueue 
[ 1407.064739] delayed workqueue 
[ 1407.064745] My own workqueue init function
[ 1412.173711] Executing my own Workqueue Function
[ 1412.173719] mydelayed  work 1250 jiffies
[ 1447.769739] My own workqueue exit  bye bye bye function
[ 1467.619103] mydelayed  loaded 1250 jiffies
[ 1467.619115] Creating workqueue 
[ 1467.619721] delayed workqueue 
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/work_q$ sudo rmmod workq_delay.k

*/


