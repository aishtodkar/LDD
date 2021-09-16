#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

#define MAX_SIZE 1024
int8_t write_buf[MAX_SIZE];
int8_t read_buf[MAX_SIZE];

#define WR_VALUE _IOW('a','a',int*) 
#define RD_VALUE _IOR('a','b',int*)


int main()
{
	int fd;
	char option;
	int number = 0, value = 0;
	fd = open("/dev/my_Char_driver",O_RDWR);
	if(fd <0)
	{
		printf("can not open device");
		return 0;
	}
	
//	while(1)
	{
		printf("****Please Enter the Option******\n"); 
		printf("        1. Write               \n");
		printf("        2. Read                 \n");		
		printf("		3. IOCTL				\n");
		printf("        4. Exit                 \n");
		printf("*********************************\n");
		scanf(" %c", &option);
		printf("Your Option = %c\n", option);
		
		switch(option)
		{
			case '1':
				printf("Enter the string to write into driver :"); 
				scanf("  %[^\t\n]s", write_buf);
				printf("Data Writing ...");
				write(fd, write_buf, strlen(write_buf)+1);
				printf("Done!\n");
				break;
			case '2':
			   printf("Data Reading ...");
			   read(fd, read_buf, MAX_SIZE);
			   printf("Done!\n\n");
			   printf("Data = %s\n\n", read_buf);
			   break;
			case '3':	
				 printf("Enter the Value to send\n"); 
				 scanf("%d",&number);
				 printf("Writing Value to Driver\n");
				 ioctl(fd, WR_VALUE, (int32_t*) &number); 
				 printf("Reading Value from Driver\n");
				 ioctl(fd, RD_VALUE, (int32_t*) &value);
				 printf("Value is %d\n", value);
				break;
		   case '4':
				   close(fd);
				   exit(1);
				   break;
		   default:
				   printf("Enter Valid option = %c\n",option);
				   break;
		}
	
	}
	close(fd);
}
