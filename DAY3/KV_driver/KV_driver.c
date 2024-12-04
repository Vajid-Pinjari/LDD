#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_to_user and copy_from_user
#include<linux/string.h>

#define DEVICE_NAME "KV_device"
#define BUFFER_SIZE 1024
#define MAX 10

static int major_number;
static char device_buffer[BUFFER_SIZE];
static int open_count = 0;
static int cnt;
static int i=1;

// Function prototypes for device operations
static int device_open(struct inode *inode, struct file *file);
static int device_close(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset);
static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset);
void LIST_arr(char *);

// File operations structure
static struct file_operations fops = {
    .open = device_open,
    .release = device_close,
    .read = device_read,
    .write = device_write,
};

static struct KV{
	int num;
	char str[50];
}LIST[MAX];

// Called when the device is opened
static int device_open(struct inode *inode, struct file *file) {
    open_count++;
    printk(KERN_INFO "simple_device: Device opened %d time(s)\n", open_count);
    return 0;
}

// Called when the device is closed
static int device_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "simple_device: Device closed\n");
    return 0;
}

// Called when data is read from the device
static ssize_t device_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
    size_t len = strlen(device_buffer);

    if (*offset >= len) // Check if all data has been read
        return 0;

    if (size > len - *offset) // Adjust size to avoid reading beyond buffer
        size = len - *offset;

    if (copy_to_user(user_buffer, device_buffer + *offset, size)) {
        return -EFAULT;
    }

    *offset += size;
    printk(KERN_INFO "simple_device: Sent %zu bytes to the user\n", size);
    return size;
}

// Called when data is written to the device
static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
    if (size > BUFFER_SIZE - 1) // Limit size to buffer capacity
        size = BUFFER_SIZE - 1;

    if (copy_from_user(device_buffer, user_buffer, size)) {
        return -EFAULT;
    }

    device_buffer[size] = '\0'; // Null-terminate the string
    printk(KERN_INFO "simple_device: Received %zu bytes from the user\n", size);
    LIST_arr(device_buffer);
    return size;
}

int rev(int x)
{
	int y=0;
	while(x)
	{
		y=(y*10)+x%10;
		x/=10;
	}
	return y;
}

void LIST_arr(char *str)
{
	char *c=str;
	if(*str=='I')
	{
		str++;
		int x=0;
		char S[20];
		while(*str !='*')
		{
			x=(x*10)+ (*str)-48;
			str++;
		}
		str++;
		int i=0;
		while(*str)
		{
			S[i]=*str;
			i++;str++;
		}
		S[i]='\0';
		if(cnt<=9)
		{
			LIST[cnt].num = x;
			strcpy(LIST[cnt].str,S);
			printk(KERN_INFO "Data inserted as :%d %s\n",LIST[cnt].num,LIST[cnt].str);
			cnt++;
			strcpy(c,"Data inserted");
		}
		else strcpy(c,"F:Reach to max");

	}
	else if(*str=='R')
	{
		char demo[100];
		str++;
		int x_cnt=0;
		while(*str)
		{
			x_cnt=(x_cnt*10)+*str-48;
			str++;
		}
		if(x_cnt>cnt)
		{
			strcpy(c,"Beyond limit");
			return ;
		}else{
			int x= LIST[x_cnt].num,J=0;
			x=rev(x);
			while(x)
			{
				demo[J]=(x%10)+48;
				x/=10;J++;
			}
			demo[J++]=' ';
			strcpy(demo+J,LIST[x_cnt].str);
			strcpy(c,demo);
		}



	}
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;  // Save the starting address of dest

    // Copy each character from src to dest
    while (*src != '\0') {
        *d = *src;  // Copy the character from src to dest
        d++;         // Move the pointer to the next character in dest
        src++;       // Move the pointer to the next character in src
    }

    *d = '\0';  // Add the null terminator at the end of the destination string
    return dest;  // Return the destination string
}

// Module initialization
static int __init simple_driver_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "simple_device: Failed to register device\n");
        return major_number;
    }
    printk(KERN_INFO "simple_device: Registered with major number %d\n", major_number);
    return 0;
}

// Module cleanup
static void __exit simple_driver_exit(void) {
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "simple_device: Unregistered device\n");
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A Simple Linux Device Driver");
