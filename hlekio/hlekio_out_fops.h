#pragma once
#include "hlekio_common.h"

ssize_t hlekio_out_read(struct file *file, char __user *buff, size_t count, loff_t *ppos);
ssize_t hlekio_out_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos);
long hlekio_out_unlocked_ioctl(struct file *, unsigned int, unsigned long);
