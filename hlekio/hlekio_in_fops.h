#pragma once
#include "hlekio_common.h"

ssize_t hlekio_in_read(struct file *file, char __user *buff, size_t count, loff_t *ppos);
long hlekio_in_unlocked_ioctl(struct file *, unsigned int, unsigned long);
