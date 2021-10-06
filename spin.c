
#include <linux/module.h>
#include<linux/kernel.h> 
#include<linux/kthread.h>
#include<linux/delay.h>
#include<linux/spinlock.h>

MODULE_AUTHOR("Aish");
MODULE_LICENSE("GPL");

static struct task_struct *char_thread1;                                                             
static struct task_struct *char_thread2;  
 
 spinlock_t spinlock_var;

 int val_1=30,val_2=20;
                      
int thread_1(void *p)
{
	while(!kthread_should_stop())
	{
		printk(KERN_INFO"Thread 1 function ");
		if(spin_is_locked(&spinlock_var)==0)		//0=available //you can use
		{
			printk(KERN_INFO"Thread 1 lock not acquired\n ");	
		}
		spin_lock(&spinlock_var);
		if(spin_is_locked(&spinlock_var))
		{
			printk(KERN_INFO"Thread 1 locked.\n");
			printk("Addtion of %d and %d = %d\n",val_1,val_2,val_1+val_2);		
		}
		spin_unlock(&spinlock_var);	
		msleep(500);
	}
	return 0;
}

int thread_2(void *p)
{
	while(!kthread_should_stop())
	{
		printk(KERN_INFO"Thread 2 function ");
		if(spin_is_locked(&spinlock_var)==0)		//0=available //you can use
		{
			printk(KERN_INFO"Thread 2 lock not acquired\n ");	
		}
		spin_lock(&spinlock_var);
		if(spin_is_locked(&spinlock_var))
		{
			printk(KERN_INFO"Thread 2 locked. \n");
			printk("Subtraction of %d and %d = %d\n",val_1,val_2,val_1-val_2);	
		}
		spin_unlock(&spinlock_var);
		msleep(500);
	
	}
	return 0;
}


static int __init basic_spin_init(void)	//return type is int	//function body// hello_world
{
	printk(KERN_INFO"basic spin init func ");		// for printing on console
	
	char_thread1=kthread_run(thread_1,NULL,"my char thread");
	if(char_thread1)
	{
		printk(KERN_INFO"Thread1 is created");
	}
	else
	{
		printk(KERN_INFO"unable to create thread ");
	}
	
	char_thread2=kthread_run(thread_2,NULL,"my char thread 2");
	if(char_thread2)
	{
		printk(KERN_INFO"Thread2 is created");
	}
	else
	{
		printk(KERN_INFO"unable to create thread2 ");
	}
	spin_lock_init(&spinlock_var);
	return 0;
}
		
static void __exit basic_spin_exit(void)		//termination function body// hello_world
{
	printk(KERN_INFO"basic thread exit ");		// for printing on console
	kthread_stop(char_thread1);
	kthread_stop(char_thread2);
}


module_init(basic_spin_init);
module_exit(basic_spin_exit);


/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/spinlock$ make
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD/spinlock modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
  CC [M]  /home/aishwarya/Documents/LDD/spinlock/spin.o
  MODPOST /home/aishwarya/Documents/LDD/spinlock/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/spinlock/spin.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/spinlock/spin.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/spinlock$ sudo insmod spin.ko
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/spinlock$ lsmod | head
Module                  Size  Used by
spin                   16384  0
nls_iso8859_1          16384  1
snd_intel8x0           45056  2
snd_ac97_codec        139264  1 snd_intel8x0
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
aesni_intel           372736  0
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/spinlock$ dmesg | tail
[  852.538404] Thread 1 locked.
[  852.538407] Addtion of 30 and 20 = 50
[  853.050029] Thread 2 function 
[  853.050029] Thread 1 function 
[  853.050035] Thread 2 lock not acquired
                
[  853.050038] Thread 2 locked. 
[  853.050043] Subtraction of 30 and 20 = 10
[  853.050051] Thread 1 locked.
[  853.050054] Addtion of 30 and 20 = 50

*/


