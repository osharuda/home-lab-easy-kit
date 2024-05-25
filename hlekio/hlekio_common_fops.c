#include "hlekio_common.h"
#include "hlekio_common_fops.h"

int hlekio_open (struct inode * in, struct file * file) {
	struct hlekio_device* hdev = container_of(file->private_data, struct hlekio_device, miscdev);
	struct hlekio_file_data* fdata = kzalloc(sizeof(struct hlekio_file_data), GFP_KERNEL);
	fdata->hdev = hdev;
	file->private_data = fdata;
	return 0;
}

int hlekio_release (struct inode * in, struct file * file) {
	kfree(file->private_data);
	return 0;
}

loff_t hlekio_llseek (struct file * file, loff_t offset, int whence) {
	if (whence!=SEEK_SET && offset!=0) {
		return -EINVAL;
	}

    file->f_pos = offset;
    return offset;
}
