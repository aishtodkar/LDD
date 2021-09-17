
#include<linux/module.h>
#include<linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL v2");
MODULE_LICENSE("Dual BSD/GPL");

MODULE_AUTHOR("Aish");
MODULE_DESCRIPTION("A sample code");
MODULE_VERSION("1:0.0");
//static defines function scope within this file

int num, ary[4];
char *name;

module_param(num, int, S_IRUSR|S_IWUSR);
module_param(name, charp, S_IRUSR|S_IWUSR);
module_param_array(ary, int,NULL, S_IRUSR|S_IWUSR);

static int __init hello_world_init(void)	//return type is int	//function body// hello_world
{
	int i;
	//printk("hello world started ");			//KERN_INFO is for getting kernel msg
	printk(KERN_INFO"hello world started\n");		// for printing on console
	printk(KERN_INFO"Number: %d\n",num);
	printk(KERN_INFO"Name: %s\n",name);
	for(i=0; i< (sizeof(ary)/sizeof(int)); i++)
		printk(KERN_INFO"Array[%d]=%d\n",i,ary[i]);
	return 0;
}


module_init(hello_world_init);		//main()//start execution from this
//module_exit()		//for exit from function


static void __exit hello_world_exit(void)		//termination function body// hello_world
{
	//printk("hello world exit ");
	printk(KERN_INFO"hello world exit ");		// for printing on console
}

module_exit(hello_world_exit);		//exit of module function//compulsory//automatically cannot call exit// absence of this will crash the system



/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Basic$ sudo insmod arg.ko num=10 name="Aish" ary=1,2,3,4

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Basic$ lsmod | head
Module                  Size  Used by
arg                    16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
aesni_intel           372736  0
crypto_simd            16384  1 aesni_intel
cryptd                 24576  2 crypto_simd,ghash_clmulni_intel

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Basic$ dmesg | tail
[ 4313.757711] Array[2]=0
[ 4313.757713] Array[3]=0
[ 4364.270954] hello world exit 
[ 4424.103209] hello world started
[ 4424.103218] Number: 10
[ 4424.103223] Name: Aish
[ 4424.103226] Array[0]=1
[ 4424.103230] Array[1]=2
[ 4424.103232] Array[2]=3
[ 4424.103235] Array[3]=4
*/
