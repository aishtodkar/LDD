#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/kthread.h> // therad 
#include <linux/delay.h>   // sleep 
//#include <linux/sched.h>               //task_struct
 
 MODULE_LICENSE("GPL");
 
atomic_t my_global_variable = ATOMIC_INIT(0);


static struct task_struct *char_thread;
static struct task_struct *char_thread1;

int mydata =0;
int my_thread_fun(void *p);



int my_thread_fun(void *p)
{
	while(!kthread_should_stop())
	{		
		//Atomically add one to v
		atomic_inc(&my_global_variable); // single operation ; no much overhead 
		printk(KERN_INFO "my_thread_fun after inc  %lu\n", atomic_read(&my_global_variable));
		
			
		//atomic_read(&my_global_variable)
		atomic_add(14, &my_global_variable);
		printk(KERN_INFO "my_thread_fun after add   %lu\n", atomic_read(&my_global_variable));

		//Atomically subtract i from v
		atomic_sub(2,&my_global_variable);
		printk(KERN_INFO "my_thread_fun after sub   %lu\n", atomic_read(&my_global_variable));

		//Atomically subtract one from v
		atomic_dec(&my_global_variable);
		printk(KERN_INFO "my_thread_fun after dec   %lu\n", atomic_read(&my_global_variable));
		
		//Atomically set v equal to i
		atomic_set(&my_global_variable, 14);			
		printk(KERN_INFO "my_thread_fun after set    %lu\n", atomic_read(&my_global_variable));
		
		// Function Atomically subtract i from v and return true if the result is zero; otherwise false 
		int res = atomic_sub_and_test(3, &my_global_variable);
		printk(KERN_INFO "my_thread_fun after sub and test    %lu\n", res);

		// Function Atomically dec one from v and return true(1) if the result is zero; otherwise false (0)
		res = atomic_dec_and_test(&my_global_variable);
		printk(KERN_INFO "my_thread_fun after dec and test    %lu\n", res);
		printk(KERN_INFO "my_thread_fun after dec and test    %lu\n", atomic_read(&my_global_variable));
			
		//Atomically add i to v and return true if the result is negative; otherwise false
		res = atomic_add_negative(8,&my_global_variable);
		printk(KERN_INFO "my_thread_fun after add neg    %lu\n", res);
		printk(KERN_INFO "my_thread_fun after add neg     %lu\n", atomic_read(&my_global_variable));

		//Atomically increment v by one and return true if the result is zero; false otherwise
		res = atomic_inc_and_test(&my_global_variable);
		printk(KERN_INFO "my_thread_fun after inc and test     %lu\n", res);
		printk(KERN_INFO "my_thread_fun after inc and test      %lu\n", atomic_read(&my_global_variable));

		printk(KERN_INFO "*****************ATOMIC BIT WSIE OPERATORS *************");
		
		unsigned long word = 0;

		set_bit(0, &word);       /* bit zero is now set (atomically) */
		printk(KERN_INFO "SET BIT      %x\n", word);

		set_bit(1, &word);        /* bit one is now set (atomically) */
		printk(KERN_INFO "SET BIT      %x\n", word);

		set_bit(2, &word);        /* bit two is now set (atomically) */
		printk(KERN_INFO "SET BIT      %x\n", word);

		clear_bit(1, &word);     /* bit one is now unset (atomically) */
		printk(KERN_INFO "CLEAR BIT      %x\n", word);

		change_bit(0, &word);    /* bit zero is flipped; now it is unset (atomically) */
		printk(KERN_INFO "CHANGE BIT      %x\n", word);

		//Atomically set the nr-th bit starting from addr and return the previous value
		res = test_and_set_bit(1, &word);
		printk(KERN_INFO "TEST AND SET    %lu\n", res);
		printk(KERN_INFO "TEST AND SET      %x\n", word);

		//Atomically clear the nr-th bit starting from addr and return the previous value
		res = test_and_clear_bit(1,&word);
		printk(KERN_INFO "TEST AND CLEAR    %lu\n", res);
		printk(KERN_INFO "TEST AND CLEAR      %x\n", word);
		
		//Atomically return the value of the nr-th bit starting from addr
		res = test_bit(2,&word);
		printk(KERN_INFO "TEST BIT    %lu\n", res);  

		msleep(1000);
		
	}
	return 0;	
}
/*
int my_thread_fun1(void *p)
{
	while(!kthread_should_stop())
	{
		
		atomic_inc(&my_global_variable);
		printk(KERN_INFO "my_thread_fun2  %lu\n", atomic_read(&my_global_variable));
		
		msleep(1000);
	}
	return 0;
}
*/

static int __init my_simpledriver_init(void)
{
	printk(KERN_INFO"My sample driver start");

	
	char_thread = kthread_run(my_thread_fun,NULL,"my char thread");
	if(char_thread)
	{
		printk(KERN_INFO" create the thread");
	}
	else 
	{
		printk(KERN_INFO"unable to create the thread");
		
	}
#if 0
	char_thread1 = kthread_run(my_thread_fun1,NULL,"my char thread1");
	if(char_thread1)
	{
		printk(KERN_INFO" create the thread1");
	}
	else 
	{
		printk(KERN_INFO"unable to create the thread");
		
	}
#endif
	return 0;

}


static void __exit my_simpledriver_exit(void)
{
	printk(KERN_INFO"My samle driver exit\n");

	kthread_stop(char_thread);
	//kthread_stop(char_thread1);

}
module_init(my_simpledriver_init);
module_exit(my_simpledriver_exit);


/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Thread/atomic$ dmesg
[   11.225396] systemd[1]: systemd 245.4-4ubuntu3.4 running in system mode. (+PAM +AUDIT +SELINUX +IMA +APPARMOR [  870.550410] Atomic: loading out-of-tree module taints kernel.
[  870.550473] Atomic: module verification failed: signature and/or required key missing - tainting kernel
[  870.550943] My sample driver start
[  870.551239]  create the thread
[  870.551802] my_thread_fun after inc  1
[  870.551814] my_thread_fun after add   15
[  870.551818] my_thread_fun after sub   13
[  870.551822] my_thread_fun after dec   12
[  870.551826] my_thread_fun after set    14
[  870.551830] my_thread_fun after sub and test    0
[  870.551834] my_thread_fun after dec and test    0
[  870.551837] my_thread_fun after dec and test    10
[  870.551841] my_thread_fun after add neg    0
[  870.551844] my_thread_fun after add neg     18
[  870.551846] my_thread_fun after inc and test     0
[  870.551849] my_thread_fun after inc and test      19
[  870.551852] *****************ATOMIC BIT WSIE OPERATORS *************
[  870.551854] SET BIT      1
[  870.551857] SET BIT      3
[  870.551860] SET BIT      7
[  870.551862] CLEAR BIT      5
[  870.551865] CHANGE BIT      4
[  870.551867] TEST AND SET    0
[  870.551869] TEST AND SET      6
[  870.551872] TEST AND CLEAR    1
[  870.551874] TEST AND CLEAR      4
[  870.551877] TEST BIT    1
[  871.565036] my_thread_fun after inc  20
[  871.565045] my_thread_fun after add   34
[  871.565048] my_thread_fun after sub   32
[  871.565051] my_thread_fun after dec   31
[  871.565053] my_thread_fun after set    14
[  871.565055] my_thread_fun after sub and test    0
[  871.565058] my_thread_fun after dec and test    0
[  871.565061] my_thread_fun after dec and test    10
[  871.565063] my_thread_fun after add neg    0
[  871.565066] my_thread_fun after add neg     18
[  871.565068] my_thread_fun after inc and test     0
[  871.565071] my_thread_fun after inc and test      19
[  871.565073] *****************ATOMIC BIT WSIE OPERATORS *************
[  871.565076] SET BIT      1
[  871.565079] SET BIT      3
[  871.565081] SET BIT      7
[  871.565084] CLEAR BIT      5
[  871.565086] CHANGE BIT      4
[  871.565089] TEST AND SET    0
[  871.565091] TEST AND SET      6
[  871.565094] TEST AND CLEAR    1
[  871.565096] TEST AND CLEAR      4
[  871.565098] TEST BIT    1

continue...
*/
