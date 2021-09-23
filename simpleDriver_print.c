

#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/kthread.h> // therad 
#include <linux/delay.h>   // sleep 
//#include <linux/sched.h>               //task_struct
#include <linux/mutex.h>  

//#define CONFIG_THREAD_INFO_IN_TASK

MODULE_LICENSE("GPL");

static atomic_t	rds_ib_allocation = ATOMIC_INIT(0);
static struct task_struct *char_thread;
static struct task_struct *char_thread1;

int mydata =0;
int my_thread_fun(void *p);
struct mutex my_mutex;


int my_thread_fun(void *p)
{
	while(!kthread_should_stop())
	{		
		mutex_lock(&my_mutex);
		mydata++;
		printk(KERN_INFO "my_thread_fun  %lu\n", mydata);
		mutex_unlock(&my_mutex);
		msleep(500);
		
	}
	return 0;	
}

int my_thread_fun1(void *p)
{
	while(!kthread_should_stop())
	{
		mutex_lock(&my_mutex);
		mydata++;
		printk(KERN_INFO "my_thread_fun2  %lu\n", mydata);
		mutex_unlock(&my_mutex);
		msleep(500);
	}
	return 0;
}


static int __init my_simpledriver_init(void)
{
	printk(KERN_INFO"My sample driver start");

	mutex_init(&my_mutex); // Mutex init 

	char_thread = kthread_run(my_thread_fun,NULL,"my char thread");
	if(char_thread)
	{
		printk(KERN_INFO" create the thread");
		printk("Thread 1 state:%ld",char_thread->state);
		printk(KERN_INFO"thread 1 Flags:%ld",char_thread->thread_info.flags);
	}
	else 
	{
		printk(KERN_INFO"unable to create the thread");
		
	}
	char_thread1 = kthread_run(my_thread_fun1,NULL,"my char thread1");
	if(char_thread1)
	{
		printk(KERN_INFO" create the thread1");
	}
	else 
	{
		printk(KERN_INFO"unable to create the thread");
		
	}
	return 0;

}


static void __exit my_simpledriver_exit(void)
{
	printk(KERN_INFO"My samle driver exit\n");

	kthread_stop(char_thread);
	kthread_stop(char_thread1);

}
module_init(my_simpledriver_init);
module_exit(my_simpledriver_exit);


/*

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Thread/mutex$ dmesg 
[ 6667.215005] simpleDriver: loading out-of-tree module taints kernel.
[ 8937.950485] My sample driver start
[ 8937.950798]  create the thread
[ 8937.950804] Thread 1 state:512
[ 8937.950808] thread 1 Flags:16384
[ 8937.950935] my_thread_fun  1
[ 8937.950948]  create the thread1
[ 8937.954669] my_thread_fun2  2
[ 8938.468194] my_thread_fun2  3
[ 8938.468364] my_thread_fun  4
[ 8939.004632] my_thread_fun2  5
[ 8939.004660] my_thread_fun  6
[ 8939.524273] my_thread_fun  7
[ 8939.524344] my_thread_fun2  8
[ 8940.039099] my_thread_fun  9
[ 8940.039179] my_thread_fun2  10

*/
