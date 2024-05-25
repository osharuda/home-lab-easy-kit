#include <linux/fs.h>
#include <dt-bindings/pinctrl/bcm2835.h>

#include "hlekio_out_fops.h"
#include "hlekio_common.h"
#include "hlekio_ioctl.h"

ssize_t hlekio_out_read(struct file *file, char __user *buff, size_t count, loff_t *ppos) {
    struct hlekio_file_data* fdata = file->private_data;
    struct hlekio_device* hdev = fdata->hdev;
    u8 level;
    unsigned long flags;
    const size_t buffer_size = 1;

    spin_lock_irqsave(&hdev->info_lock, flags);
    level = hdev->out_info.level;
    spin_unlock_irqrestore(&hdev->info_lock, flags);

    // Prepare buffer
    if (IS_TEXT_MODE(fdata)) {
        level = TO_TEXT_LEVEL(level);
    }

    return hlekio_read(buff, count, ppos, &level, buffer_size);
}

ssize_t hlekio_out_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos) {
    ssize_t res;
    struct hlekio_file_data* fdata = file->private_data;
    struct hlekio_device* hdev = fdata->hdev;
    u8 level;

    res = hlekio_write(buff, count, ppos, &level, sizeof(level));

    if (res<1) {
        goto done;
    }

    if (IS_TEXT_MODE(fdata)) {
        level = TO_BINARY_LEVEL(level);
    } else {
        level = !!level;
    }

    hlekio_set_gpio(hdev, level);
done:
    return res;
}

long hlekio_out_unlocked_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    struct hlekio_file_data* fdata = file->private_data;
    struct hlekio_device* hdev = fdata->hdev;
    long err = -ENOTTY;
    switch (cmd) {
        case HLEKIO_RESET:
            err = hlekio_set_gpio(hdev, hdev->pin.default_level);
        break;
        case HLEKIO_BINARY_MODE:
            SET_BINARY_MODE(fdata, arg);
            err = 0;
        break;
    }

    return err;
}

