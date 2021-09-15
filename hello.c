

#include<linux/module.h>

MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL v2");
MODULE_LICENSE("Dual BSD/GPL");

MODULE_AUTHOR("Aish");
MODULE_DESCRIPTION("A sample code");
MODULE_VERSION("1:0.0");
static int __init hello_world_init(void)
{
	//printk("hello world started ");			//KERN_INFO is for getting kernel msg
	printk(KERN_INFO"hello world started ");		// for printing on console
	return 0;
}


module_init(hello_world_init);		//start execution from this
//module_exit()		//for exit from function


static void __exit hello_world_exit(void)		
{
	//printk("hello world exit ");
	printk(KERN_INFO"hello world exit ");		// for printing on console
}

module_exit(hello_world_exit);		//exit of module function



/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD$ make clean
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD clean
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'
aishwarya@aishwarya-VirtualBox:~/Documents/LDD$ make
make -C /lib/modules/5.11.0-27-generic/build M=/home/aishwarya/Documents/LDD modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-27-generic'
  CC [M]  /home/aishwarya/Documents/LDD/hello.o
  MODPOST /home/aishwarya/Documents/LDD/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/hello.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/hello.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-27-generic'
aishwarya@aishwarya-VirtualBox:~/Documents/LDD$ sudo insmod hello.ko
[sudo] password for aishwarya: 
aishwarya@aishwarya-VirtualBox:~/Documents/LDD$ lsmod | head
Module                  Size  Used by
hello                  16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0
aesni_intel           372736  0
crypto_simd            16384  1 aesni_intel
cryptd                 24576  2 crypto_simd,ghash_clmulni_intel
*/


