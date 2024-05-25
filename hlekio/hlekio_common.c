#include <linux/fs.h>
#include <linux/io.h>

#include "hlekio_common.h"
#include "hlekio_hw.h"

char* function_text[] = FUNCTION_TEXTS;
char* pull_up_text[] = PULL_UP_TEXTS;
char* level_text[] = LEVEL_TEXTS;

ssize_t hlekio_read(char __user *buff, size_t count, loff_t *ppos, const char __kernel * kbuffer, size_t kbuffer_size) {
    ssize_t res;
    if (*ppos >= kbuffer_size) {
        res = 0; /* Zero means EOF */
        goto done;
    }

    if (*ppos + count > kbuffer_size) {
        count = kbuffer_size - *ppos;
    }

    if(copy_to_user(buff, kbuffer+*ppos, count)) {
            res = -EFAULT;
            goto done;
    }

    *ppos += count;
    res = count;

done:
    return res;
}

ssize_t hlekio_write(const char __user *buff, size_t count, loff_t *ppos, char __kernel * kbuffer, size_t kbuffer_size) {
    ssize_t res;
    if (*ppos != 0) {
        res = -EINVAL;
        goto done;        
    }

    if (*ppos >= kbuffer_size) {
        res = -EFBIG;
        goto done;
    }

    if (*ppos + count > kbuffer_size) {
        count = kbuffer_size - *ppos;
    }

    if(copy_from_user(kbuffer + *ppos, buff, count)) {
            res = -EFAULT;
            goto done;
    }

    *ppos += count;
    res = count;

done:
    return res;
}

int hlekio_set_gpio(struct hlekio_device* hdev, u8 value) {
    value = !!value;
    unsigned long flags;

    spin_lock_irqsave(&hdev->info_lock, flags);
    hdev->out_info.level = value;
    if (hdev->pin.can_sleep) {
        spin_unlock_irqrestore(&hdev->info_lock, flags);
        gpiod_set_value_cansleep(hdev->pin_desc, value);
    } else {
        gpiod_set_value(hdev->pin_desc, value);
        spin_unlock_irqrestore(&hdev->info_lock, flags);
    }

    return 0;
}

u8 hlekio_get_gpio(struct hlekio_device* hdev) {
    int res;
    if (hdev->pin.can_sleep) {
        res = gpiod_get_value_cansleep(hdev->pin_desc);
    } else {
        res = gpiod_get_value(hdev->pin_desc);
    }
    return !!res;
}
