//This code is not working on my system(version 5.11.0-38 generic)
//getting an error that in interrupt.h file having #define DECLARE_TASKLET(name, ._callback) macro. Not having 3 argument maco
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 19
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param (irq, int, S_IRUGO);

static void my_tasklet_handler(unsigned long flag);

DECLARE_TASKLET(my_tasklet,my_tasklet_handler,0);

static void my_tasklet_handler(unsigned long flag)
{
	//tasklet_disable(&my_tasklet);
	printk(KERN_INFO"Tasklet handler started...\n");
	irq_counter++;
    	printk (KERN_INFO "Counter = %d\n", irq_counter);
    	printk(KERN_INFO"Tassklet handler ended...\n");
	//tasklet_enable(&my_tasklet);
}


static irqreturn_t my_interrupt (int irq, void *dev_id)
{
	printk(KERN_INFO"Scheduling Tasklet...\n");
	tasklet_schedule(&my_tasklet);
	printk(KERN_INFO"Tassklet Scheduled...\n");
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

