#pragma once
#include <linux/fs.h>

int hlekio_open (struct inode *, struct file *);
int hlekio_release (struct inode *, struct file *);
loff_t hlekio_llseek (struct file *, loff_t, int);
