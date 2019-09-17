#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#define BUFF_SIZE 1024
#define MAJOR_NUMBER 235  // the Major number corresponding to our device driver
#define DEVICE_NAME "simple_character_device"  // this is what I named the device driver

/* declaring some statics for later use */

static int open_count = 0;          // counting the driver opens
static int close_count = 0;
static char *device_buffer;

// would like to know more about inode and file in these parameters
int simple_open(struct inode *inode, struct file *filp){
    printk(KERN_ALERT "inside %s function\n", __FUNCTION__);
    open_count++;
    printk("number of opens:: %d\n", open_count);
    return 0;
}

int simple_close(struct inode *inode, struct file *filp){
    printk(KERN_ALERT "insde %s function\n", __FUNCTION__);
    close_count++;
    printk("number of closes: %d \n", close_count);
    return 0;
}

// ssize_t is used for functions whose return value could be negative number (error) or a 
// valid size
ssize_t simple_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset){
    printk(KERN_ALERT "inside %s function \n", __FUNCTION__);
    // specify max bytes to be read into buffer
    int MAX_BYTES = BUFF_SIZE - *offset;
    /* specify the bytes we'll read and bytes already read in */
    int BYTES_TO_READ, BYTES_READ;
    if(MAX_BYTES > length)  /* in good shape */ {
        BYTES_TO_READ = length;
    }
    else /* we'll read as many bytes as we can */ {
        BYTES_TO_READ == MAX_BYTES;
    }

    if(BYTES_TO_READ = 0) {
        printk(KERN_ALERT "NO BYTES READ IN \n");
    }

    /* now we need to print the number of bytes read
                                            to, from, n */
    BYTES_READ = BYTES_TO_READ - copy_to_user(buffer, device_buffer + *offset, BYTES_TO_READ);
    printk(KERN_ALERT "Device read : %d Bytes \n", BYTES_READ);
    // move the offset however many bytes we read
    *offset += BYTES_READ;
    printk(KERN_ALERT "Device Read Finish\n");
    return BYTES_READ;
}

// Write mirrors read
ssize_t simple_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset){
    // printk(KERN_ALERT "inside %s function \n", __FUNCTION__);

    // int MAX_BYTES = BUFF_SIZE - *offset;
    // int BYTES_TO_WRITE, BYTES_WROTE;
    
    // if(MAX_BYTES > length){
    //     BYTES_TO_WRITE = length;
    // }
    // else {
    //     BYTES_TO_WRITE = MAX_BYTES;
    // }
    
    // if(BYTES_TO_WRITE == 0) {
    //     printk(KERN_ALERT "NO BYTES TO WRITE \n");
    // }

    // BYTES_WROTE = BYTES_TO_WRITE - copy_to_user(device_buffer + *offset, buffer, BYTES_TO_WRITE);
    // printk(KERN_ALERT "Device wrote: %d bytes \n", BYTES_WROTE);
    // *offset += BYTES_WROTE;
    // printk(KERN_ALERT "Device write finished\n");
    // return BYTES_WROTE;
	
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	int MAX_BYTES = BUFF_SIZE - *offset;
	/*number of bytes to WRITE */
	/*number of bytes WRITTEN */
	int BYTES_TO_WRITE, BYTES_WROTE;
	if(MAX_BYTES > length){
		BYTES_TO_WRITE = length;
	} else {
		BYTES_TO_WRITE = MAX_BYTES;
	}
	BYTES_WROTE = BYTES_TO_WRITE - copy_from_user(device_buffer + *offset, buffer, BYTES_TO_WRITE);
	// how many bytes we wrote
	printk(KERN_ALERT "DEVICE WROTE: %d BYTES\n", BYTES_WROTE);
	*offset += BYTES_WROTE;
	printk(KERN_ALERT "DEVICE FINISHED WRITING\n");

	return BYTES_WROTE;
}

loff_t simple_seek (struct file *filp, loff_t offset, int whence){
    printk(KERN_ALERT "inside %s function \n", __FUNCTION__);
    loff_t new_offset = 0;
    switch(whence) {
        case 0: // SEEK_SET
            new_offset = offset;
            break;
        case 1: // SEEK_CUR
            new_offset = filp->f_pos + offset;
            break;
        case 2: // SEEK_END
            new_offset = BUFF_SIZE - offset;
            break;
    }
    if(new_offset < 0){
        printk(KERN_ALERT "ERROR: cannot seek prior to buffer start");
    }
    if(new_offset > BUFF_SIZE){
        printk(KERN_ALERT "ERROR: cannot seek past the buffer end");
    }
    filp->f_pos = new_offset;
    return new_offset;
}

struct file_operations simple_driver_operations = {
	.owner = THIS_MODULE,
    .open = simple_open,
    .read = simple_read,
    .write = simple_write,
    .llseek = simple_seek,
    .release = simple_close,
};

int simple_init(void){
    printk(KERN_ALERT "inside %s function\n", __FUNCTION__);
    device_buffer = kmalloc(BUFF_SIZE, GFP_KERNEL);
    register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &simple_driver_operations);
    return 0;           // success baby
}

void simple_exit(void){
    printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	if(device_buffer){
		kfree(device_buffer);
	}
	/* unregister the character device driver */
	unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
}


module_init(simple_init);
module_exit(simple_exit);

// #include <linux/init.h>
// #include <linux/module.h>
// #include <linux/fs.h>
// #include <linux/uaccess.h>
// #include <asm/uaccess.h>
// #include <linux/slab.h>

// #define BUFF_SIZE 1024
// #define MAJOR_NUMBER 235
// #define DEVICE_NAME "simple_character_device"

// static char *device_buff;
// static int open_Count = 0; //to count number of of times open has been invoked up to now
// static int close_Count = 0; // similar to above, but with close

// /* note that inode represents the actual physical file on the hard disk, pfile will contain all the necessary
// file operations */
// int simple_char_driver_open(struct inode *pinode, struct file *pfile){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	open_Count++;		// keeping count of opens
// 	printk("Number of devices opened: %d\n", open_Count);
// 	return 0;
// }

// int simple_char_driver_close(struct inode *pinode, struct file *pfile){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	close_Count++;		// keeping count of closes
// 	printk("Number of devices closed: %d\n", close_Count);
// 	return 0;
// }

// loff_t simple_char_driver_seek(struct file *pfile, loff_t offset, int whence){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	loff_t new_offset = 0;
// 	switch(whence) {
// 		case 0: /* SEEK_SET */
// 			new_offset = offset;
// 			break;
// 		case 1: /* SEEK_CUR */
// 			new_offset = pfile->f_pos + offset;
// 			break;
// 		case 2: /* SEEK_END */
// 			new_offset = BUFF_SIZE - offset;
// 			break;
// 	}

// 	if(new_offset < 0){
// 		printk(KERN_ALERT "ERROR: cannot seek before buffer start");
// 	}

// 	if(new_offset > BUFF_SIZE){
// 		printk(KERN_ALERT "ERROR: cannot seek past BUFF_SIZE");
// 	}

// 	/* make the file position equal to our new offset*/
// 	pfile->f_pos = new_offset;

// 	/*return how many bytes the new offset is */
// 	return new_offset;
// }

// ssize_t simple_char_driver_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	// max bytes to be read into Buff_size
// 	int MAX_BYTES = BUFF_SIZE - *offset;
// 	/*number of bytes to read */
// 	/*number of bytes that we passed through */
// 	int BYTES_TO_READ, BYTES_READ;
// 	/* logical statements about length of reading buffer */
// 	if(MAX_BYTES > length){
// 		BYTES_TO_READ = length;
// 	} else {
// 		BYTES_TO_READ = MAX_BYTES;
// 	}
// 	/* test for empty file */
// 	if(BYTES_TO_READ == 0){
// 		printk(KERN_ALERT "END OF DEVICE \n");
// 	}
// 	/* how many bytes did we read */
// 	BYTES_READ = BYTES_TO_READ - copy_to_user(buffer, device_buff + *offset, BYTES_TO_READ);	
// 	printk(KERN_ALERT "DEVICE READ: %d BYTES\n", BYTES_READ);
// 	/* move the offset hoewver many bytes we read */
// 	*offset += BYTES_READ;
// 	/* when we get to the end */
// 	printk(KERN_ALERT "DEVICE FINISH READING");
// 	/*return how many bytes read*/
// 	return BYTES_READ; /* 0 for empty file */
// }

// ssize_t simple_char_driver_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	/* max num of bytes that can be WRITE from offset to BUFF_SIZE */
// 	int MAX_BYTES = BUFF_SIZE - *offset;
// 	/*number of bytes to WRITE */
// 	/*number of bytes WRITTEN */
// 	int BYTES_TO_WRITE, BYTES_WROTE;
// 	/* logical statements about length of writing and buffer */	
// 	if(MAX_BYTES > length){
// 		BYTES_TO_WRITE = length;
// 	} else {
// 		BYTES_TO_WRITE = MAX_BYTES;
// 	}
// 	/* the bytes we wrote is equal with the total number of bytes to write, minus the remaining but we have yet read
// 		this remaaing bit comees from user space*/
// 	BYTES_WROTE = BYTES_TO_WRITE - copy_from_user(device_buff + *offset, buffer, BYTES_TO_WRITE);
// 	/* when we say howmany bytes we wrote */
// 	printk(KERN_ALERT "DEVICE WROTE: %d BYTES\n", BYTES_WROTE);
// 	/* move the offset by however many bytes we wrote*/
// 	*offset += BYTES_WROTE;
// 	/* alert kernel that we are finished */
// 	printk(KERN_ALERT "DEVICE FINISHED WRITING\n");

// 	return BYTES_WROTE; /*returning the length of the bytes.*/
// }

// /* file ops on this device */
// struct file_operations simple_char_driver_file_operations ={
// 	.owner		= THIS_MODULE,					/* in the file op struct */
// 	.open 		= simple_char_driver_open, 		/* opens */
// 	.release	= simple_char_driver_close, 	/* closes*/
// 	.llseek 	= simple_char_driver_seek, 		/* seeks */
// 	.read 		= simple_char_driver_read, 		/* reads */
// 	.write 		= simple_char_driver_write, 	/* writes */
// };

// int simple_char_driver_init(void){
// 	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
// 	/* free up space before init */
// 	device_buff = kmalloc(BUFF_SIZE, GFP_KERNEL);
// 	/* register w/ kern and indicate we are doing the char_device_drv*/
// 	register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &simple_char_driver_file_operations);
// 	return 0;
// }

// void simple_char_driver_exit(void){
	// printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	// if(device_buff){
	// 	kfree(device_buff);
	// }
	// /* unregister the character device driver */
	// unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
// }

// module_init(simple_char_driver_init);
// module_exit(simple_char_driver_exit);

