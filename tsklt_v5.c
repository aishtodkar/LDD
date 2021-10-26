
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 19
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param (irq, int, S_IRUGO);

static void my_tasklet_handler(struct tasklet_struct *t);

DECLARE_TASKLET(my_tasklet,my_tasklet_handler);

static void my_tasklet_handler(struct tasklet_struct *t)
{
	printk(KERN_INFO"Tasklet handler started...\n");
	irq_counter++;
    	printk (KERN_INFO "Counter = %d\n", irq_counter);
    	printk(KERN_INFO"Tasklet handler ended...\n");
}


static irqreturn_t my_interrupt (int irq, void *dev_id)
{
	printk(KERN_INFO"Scheduling Tasklet...\n");
	tasklet_schedule(&my_tasklet);
	printk(KERN_INFO"Tasklet Scheduled...\n");
    	return IRQ_NONE;            /* we return IRQ_NONE because we are just observing */
}

static int __init my_init (void)
{
    if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
        return -1;
    printk (KERN_INFO "Successfully loading ISR handler\n");
    return 0;
}

static void __exit my_exit (void)
{
    tasklet_kill(&my_tasklet);
    synchronize_irq (irq);
    free_irq (irq, &my_dev_id);
    printk (KERN_INFO "Successfully unloading \n");
}

module_init (my_init);
module_exit (my_exit);

MODULE_AUTHOR ("Aish");
MODULE_LICENSE ("GPL");

/*
ishwarya@aishwarya-VirtualBox:~/Documents/LDD/tasklet$ make
make -C /lib/modules/5.11.0-38-generic/build M=/home/aishwarya/Documents/LDD/tasklet modules
make[1]: Entering directory '/usr/src/linux-headers-5.11.0-38-generic'
  CC [M]  /home/aishwarya/Documents/LDD/tasklet/tsklt_v5.o
  MODPOST /home/aishwarya/Documents/LDD/tasklet/Module.symvers
  CC [M]  /home/aishwarya/Documents/LDD/tasklet/tsklt_v5.mod.o
  LD [M]  /home/aishwarya/Documents/LDD/tasklet/tsklt_v5.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.11.0-38-generic'

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/tasklet$ sudo insmod tsklt_v5.ko
[sudo] password for aishwarya: 

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/tasklet$ lsmod | head
Module                  Size  Used by
tsklt_v5               16384  0
nls_iso8859_1          16384  1
intel_rapl_msr         20480  0
intel_rapl_common      24576  1 intel_rapl_msr
snd_intel8x0           45056  2
snd_ac97_codec        139264  1 snd_intel8x0
ac97_bus               16384  1 snd_ac97_codec
crct10dif_pclmul       16384  1
ghash_clmulni_intel    16384  0

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/tasklet$ cat /proc/interrupts | head
           CPU0       CPU1       
  0:         32          0   IO-APIC   2-edge      timer
  1:        390          0   IO-APIC   1-edge      i8042
  8:          0          0   IO-APIC   8-edge      rtc0
  9:          0          0   IO-APIC   9-fasteoi   acpi
 12:          0        390   IO-APIC  12-edge      i8042
 14:          0          0   IO-APIC  14-edge      ata_piix
 15:          0        551   IO-APIC  15-edge      ata_piix
 18:       1315        604   IO-APIC  18-fasteoi   vmwgfx
 19:        545         49   IO-APIC  19-fasteoi   enp0s3, my_interrupt

aishwarya@aishwarya-VirtualBox:~/Documents/LDD/tasklet$ dmesg | tail
[  473.569528] Scheduling Tasklet...
[  473.569536] Tasklet Scheduled...
[  473.569582] Tasklet handler started...
[  473.569585] Counter = 17
[  473.569589] Tasklet handler ended...
[  475.585193] Scheduling Tasklet...
[  475.585201] Tasklet Scheduled...
[  475.585247] Tasklet handler started...
[  475.585249] Counter = 18
[  475.585254] Tasklet handler ended...

*/

