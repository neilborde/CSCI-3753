#include <linux/kernel.h>
#include <linux/linkage.h>
asmlinkage long sys_Simple_add(int number1, int number2, int *result){
    // printk(KERN_ALERT "The first number is %d\n", number1);
    // printk(KERN_ALERT "The second number is %d\n", number2);
    *result = number1+number2;

    printk(KERN_ALERT "first number %d + second number %d = %d \n", number1, number2, *result);

    return 0;
}