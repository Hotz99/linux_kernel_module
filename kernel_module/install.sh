#!/bin/bash

MODULE_NAME="my_module"

make clean
make
sudo rmmod $MODULE_NAME
sudo rm /dev/$MODULE_NAME

# the kernel module's register_chrdev() dynamically generates a major number 
# read the used number from the kernel log and adjust below to match
sudo mknod /dev/$MODULE_NAME c 236 0
sudo chmod 666 /dev/$MODULE_NAME
sudo insmod $MODULE_NAME.ko
cat /proc/devices | grep -i "$MODULE_NAME"