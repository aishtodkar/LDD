#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include<sys/ioctl.h>

#define MY_MAGIC 'a'
#define WR_VALUE _IOW(MY_MAGIC,'a',int32_t*)
#define RD_VALUE _IOR(MY_MAGIC,'b',int32_t*)
#define MY_IOCTL_MAX_CMD 2



int main()
{
	int fd;
    int32_t value, number, new_err;	
	char *my_buf;
	printf("[%d] - Opening device my_cdrv\n", getpid() );
	
	fd = open( "/dev/my_Ioctl_driver", O_RDWR);

//	fd = open( NULL, O_RDWR);				// NULL address
//	fd = open( "/dev/data", O_RDWR);			//wrong address
//	fd = open( "", O_RDWR);				// no file
//	fd = open( "/dev/my_Ioctl_driver", O_RDONLY);		// readonly operation
//	fd=-1020;						// invalid fd
//	fd= open("bin_file",O_RDWR);				// binary file
	if( fd < 0 ) 
	{
		printf("\n\nDevice could not be opened\n\n");
		perror("FD FAIL:");
		// return 1;
	
	}
	
	printf("Device opened with ID [%d]\n", fd);
	
	   
	printf("Enter the Value to send\n");
	scanf("%d",&number);
	printf("Writing Value to Driver\n");
	new_err=ioctl(fd, WR_VALUE, (int32_t*) &number); 
//	new_err= write(fd,WR_VALUE, (int32_t*)&number);	//included for writing in binary file
	if(new_err<0)
		perror("WR FAIL:");
		
	printf("Reading Value from Driver\n");
	new_err=ioctl(fd, RD_VALUE, (int32_t*) &value);
	if(new_err!=0)
		perror("RD FAIL:");

	printf("Value is %d\n", value);

	printf("Closing Driver\n");
	new_err=close(fd);
//	new_err=close(fd);
	if(new_err!=0)
		perror("CLOSE FAIL:");			

	exit(0);
}



/************Providing NULL as a address ****************/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ cc test_ioctl.c
test_ioctl.c: In function ‘main’:
test_ioctl.c:23:7: warning: null argument where non-null required (argument 1) [-Wnonnull]
   23 |  fd = open( NULL, O_RDWR);
      |       ^~~~
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[9451] - Opening device my_cdrv


Device could not be opened

FD FAIL:: Bad address
*/

/********************* Providing wrong address ***************/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ cc test_ioctl.c
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[9506] - Opening device my_cdrv


Device could not be opened

FD FAIL:: No such file or directory

*/
/******************** For no file ****************/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ cc test_ioctl.c
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[9587] - Opening device my_cdrv


Device could not be opened

FD FAIL:: No such file or directory

*/
/************* Read only permission ***********/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ cc test_ioctl.c
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[10132] - Opening device my_cdrv
Device opened with ID [3]
Enter the Value to send
10
Writing Value to Driver
Reading Value from Driver
Value is 10
Closing Driver

//dmesg
[ 5383.340989] atkbd serio0: Use 'setkeycodes 6e <keycode>' to make it known.
[ 5444.496891] IOCTL :kernel driver removed  ... done 
[ 5487.037592] Major =237 Minor = 0 
[ 5487.051012] IOCTL : character drivre init sucess
[ 6116.727721] IOCTL :kernel driver removed  ... done 
[ 7095.650086] Major =237 Minor = 0 
[ 7095.656409] IOCTL : character drivre init sucess
[ 8221.912416] hrtimer: interrupt took 9694239 ns
[ 9097.171535] IOCTL :kernel driver removed  ... done 
[ 9706.887007] Major =237 Minor = 0 
[ 9706.887276] IOCTL : character drivre init sucess
[ 9758.780049] Driver open  function called .....
[ 9763.442219] Enter ioctl 
[ 9763.442227] Enter write 
[ 9763.442237] Value = 10
[ 9763.442257] Enter ioctl 
[ 9763.442260] Enter read 
[ 9763.442291] Driver release function called ....

*/
/**************** Closing file two times **********/

/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ cc test_ioctl.c
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[11504] - Opening device my_cdrv
Device opened with ID [3]
Enter the Value to send
12345678
Writing Value to Driver
Reading Value from Driver
Value is 12345678
Closing Driver
CLOSE FAIL:: Bad file descriptor

*/
/************ closing with invalid fd **********/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[11557] - Opening device my_cdrv


Device could not be opened

FD FAIL:: Success
Device opened with ID [-1020]
Enter the Value to send
2
Writing Value to Driver
WR FAIL:: Bad file descriptor
Reading Value from Driver
RD FAIL:: Bad file descriptor
Value is -2067967680
Closing Driver
CLOSE FAIL:: Bad file descriptor
*/

/*************Reading from binary file **********/
/*
aishwarya@aishwarya-VirtualBox:~/Documents/LDD/Ioctl/error$ ./a.out
[11667] - Opening device my_cdrv


Device could not be opened

FD FAIL:: No such file or directory
Device opened with ID [-1]
Enter the Value to send
2
Writing Value to Driver
WR FAIL:: Bad file descriptor
Reading Value from Driver
RD FAIL:: Bad file descriptor
Value is 580800896
Closing Driver
CLOSE FAIL:: Bad file descriptor


*/







