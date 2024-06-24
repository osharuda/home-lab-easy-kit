#include <linux/fs.h>
#include <dt-bindings/pinctrl/bcm2835.h>

#include "hlekio_in_fops.h"
#include "hlekio_common.h"
#include "hlekio_ioctl.h"

static long hlekio_in_reset(struct hlekio_device* hdev) {
    unsigned long flags;
	spin_lock_irqsave(&hdev->info_lock, flags);
    hdev->in_info.last_isr_jiffers = 0;
    hdev->in_info.isr_count = 0;
    hdev->in_info.reset_jiffers = get_jiffies_64();
    spin_unlock_irqrestore(&hdev->info_lock, flags);
    return 0;
}

ssize_t hlekio_in_read(struct file *file, char __user *buff, size_t count, loff_t *ppos) {
    struct hlekio_file_data* fdata = file->private_data;
    struct hlekio_device* hdev = fdata->hdev;
    char text_buffer[100];
    unsigned long flags;
    struct hlekio_input_info in_info;
    char* buffer = (char*)&in_info;
    size_t buffer_size = sizeof(in_info);

    // Wait in blocking mode
    if ((file->f_flags & O_NONBLOCK) == 0) {
        u8 wait_irq = 1;
        int res = 0;
        atomic_inc(&hdev->irq_event_waiters_count);

        // check if pin already at required state
        if (hdev->pin.trigger_by_level) {
            wait_irq = (hlekio_get_gpio(hdev) != hdev->pin.trigger_level);
        }

        if (wait_irq) {
            res = wait_for_completion_interruptible(&hdev->irq_event);
        }

        if (atomic_dec_and_test(&hdev->irq_event_waiters_count)) {
            reinit_completion(&hdev->irq_event);
        }

        if (res) {
            return -ERESTARTSYS;
        }
    }

    spin_lock_irqsave(&hdev->info_lock, flags);
    memcpy(&in_info, &hdev->in_info, sizeof(in_info));
    spin_unlock_irqrestore(&hdev->info_lock, flags);
    in_info.level = hlekio_get_gpio(hdev);

    if (IS_TEXT_MODE(fdata)) {
        if (file->f_flags & O_NONBLOCK) {
            buffer_size = snprintf(text_buffer, sizeof(text_buffer), "%llu,%llu,%lu,%lu,%c",
                in_info.last_isr_jiffers,
                in_info.reset_jiffers,
                in_info.isr_count,
                in_info.isr_debounce,
                TO_TEXT_LEVEL(in_info.level));            
        } else {
            buffer_size = snprintf(text_buffer, sizeof(text_buffer), "%llu,%llu,%lu,%lu",
                in_info.last_isr_jiffers,
                in_info.reset_jiffers,
                in_info.isr_count,
                in_info.isr_debounce);            
        }

        buffer_size++; // for null terminator
        buffer = text_buffer;
    }

    return hlekio_read(buff, count, ppos, buffer, buffer_size);
}

long hlekio_in_unlocked_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    long err = -ENOTTY;
    struct hlekio_file_data* fdata = file->private_data;
    struct hlekio_device* hdev = fdata->hdev;
    unsigned long flags;
    void* __user ubuf;

    switch (cmd) {
        case HLEKIO_RESET:
            err = hlekio_in_reset(hdev);
            break;
        case HLEKIO_BINARY_MODE:
            SET_BINARY_MODE(fdata, arg);
            err = 0;
            break;
        case HLEKIO_DEBOUNCE:
            spin_lock_irqsave(&hdev->info_lock, flags);
            hdev->in_info.isr_debounce = (unsigned long)arg;
            spin_unlock_irqrestore(&hdev->info_lock, flags);
            err = 0;
            break;
        case HLEKIO_PIN_TYPE:
            u8 value = HLEKIO_INPUT_DEV;
            ubuf = (void* __user)arg;

            if(copy_to_user(ubuf, &value, sizeof(value))) {
                err = -EFAULT;
                break;
            }

            err = 0;
        break;
    }

    return err;
}
