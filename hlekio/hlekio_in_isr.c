#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

#include "hlekio_in_isr.h"

static irqreturn_t hlekio_isr(int irq, void *data)
{
    struct hlekio_device *hdev = data;
    unsigned long flags;
    irqreturn_t res = IRQ_NONE;
    u64 ts = get_jiffies_64();

    spin_lock_irqsave(&hdev->info_lock, flags);
    if (ts - hdev->in_info.last_isr_jiffers >= hdev->in_info.isr_debounce) {
        hdev->in_info.last_isr_jiffers = ts;
        hdev->in_info.isr_count += 1;
        res = IRQ_HANDLED;
    }
    spin_unlock_irqrestore(&hdev->info_lock, flags);

    if (res == IRQ_HANDLED) {
        atomic_inc(&hdev->irq_event_waiters_count);
        complete_all(&hdev->irq_event);
        if (atomic_dec_and_test(&hdev->irq_event_waiters_count)) {
            reinit_completion(&hdev->irq_event);
        }
    }

    dev_info(hdev->dev, "interrupt received: res=%d, irq_event_waiters_count=%d\n", res, atomic_read(&hdev->irq_event_waiters_count));

    return res;
}

int hlekio_configure_isr(struct hlekio_device* hdev) {
    int irq;
    unsigned long irq_flags;
    struct device* dev = hdev->dev;
    u32 trigger;

    int err = of_property_read_u32(dev->of_node, "trigger", &trigger);
    if (err<0) {
        dev_err(dev, "Trigger is not specified in DT, err = %i\n", err);
        goto done;        
    }

    switch (trigger) {
        case HLEKIO_RISE:
            irq_flags = IRQF_TRIGGER_RISING;
        break;

        case HLEKIO_FALL:
            irq_flags = IRQF_TRIGGER_FALLING;
        break;

        case HLEKIO_EDGE:
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
        break;

        default:
            err = -EINVAL;
            dev_err(dev, "Trigger is not specified in DT, err = %i\n", err);
            goto done;
    }

    irq = gpiod_to_irq(hdev->pin_desc);
    if (irq > 0) {
        dev_info(dev, "IRQ: %d\n", irq);
    } else {
        dev_err(dev, "Failed to get IRQ, err = %i\n", irq);
        err = -ENOSYS;
        goto done;
    }

    err = devm_request_irq(dev, irq, hlekio_isr, irq_flags, hdev->pin.pin_name, hdev);
    if (err) {
        dev_err(dev, "Failed to request IRQ, err = %i\n", err);
        err = -EBUSY;
        goto done;
    }

    err = irq;

done:
    hdev->pin.irq = err;
    return err;
}
