#include <linux/of.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <dt-bindings/pinctrl/bcm2835.h>
#include <linux/gpio/consumer.h>

#include "hlekio_hw.h"
#include "hlekio_common.h"
#include "hlekio_probe.h"
#include "hlekio_common_fops.h"
#include "hlekio_out_fops.h"
#include "hlekio_in_fops.h"
#include "hlekio_init.h"
#include "hlekio_in_isr.h"

static const struct file_operations hlekio_in_fops = {
        .owner = THIS_MODULE,
        .open = hlekio_open,
        .release = hlekio_release,
        .read = hlekio_in_read,
        .llseek = hlekio_llseek,
        .unlocked_ioctl = hlekio_in_unlocked_ioctl
};

static const struct file_operations hlekio_out_fops = {
        .owner = THIS_MODULE,
        .open = hlekio_open,
        .release = hlekio_release,
        .read = hlekio_out_read,
        .write = hlekio_out_write,
        .llseek = hlekio_llseek,
        .unlocked_ioctl = hlekio_out_unlocked_ioctl
};

static int hlekio_init_in_pin(struct hlekio_device* hdev) {
    hdev->pin_desc = devm_gpiod_get(hdev->dev, hdev->pin.pin_name, GPIOD_IN);
    if (IS_ERR(hdev->pin_desc)) {
        pr_err("Failed to request pin, %i !!!\n", (int)hdev->pin_desc);
    } else {
        pr_info("gpio_desc = %pa\n", &(hdev->pin_desc));
    }

    dev_info(hdev->dev, LGP"Pull-up for %s (brcm,pull): %s.\n", hdev->pin.pin_name, pull_up_text[hdev->pin.pin_pull]);
    return hlekio_configure_isr(hdev); 
}

static int hlekio_init_out_pin(struct hlekio_device* hdev) {
    enum gpiod_flags flags;

    of_property_read_u32(hdev->dev->of_node, "init_state", &hdev->pin.default_level);
    hdev->pin.default_level = !!hdev->pin.default_level;

    of_property_read_u32(hdev->dev->of_node, "pin_type", &hdev->pin.open_drain);
    hdev->pin.open_drain = !!hdev->pin.open_drain;

    flags = hdev->pin.default_level ? GPIOD_OUT_HIGH : GPIOD_OUT_LOW;
    flags |= hdev->pin.open_drain ? GPIOD_FLAGS_BIT_OPEN_DRAIN : 0;

    hdev->pin_desc = devm_gpiod_get(hdev->dev, hdev->pin.pin_name, flags);
    if (IS_ERR(hdev->pin_desc)) {
        pr_err("Failed to request pin, %i !!!\n", (int)hdev->pin_desc);
    } else {
        pr_info("gpio_desc = %pa\n", &(hdev->pin_desc));
    }

     hdev->pin.can_sleep = gpiod_cansleep(hdev->pin_desc);
     if (hdev->pin.can_sleep) {
        dev_warn(hdev->dev, LGP"Warning: pin operations may sleep!\n");
     }
    
    hdev->out_info.level = hdev->pin.default_level;
    dev_info(hdev->dev, LGP"Default level for %s (brcm,pull): %s.\n", hdev->pin.pin_name, level_text[hdev->out_info.level]);

    return 0;
}

int hlekio_probe(struct platform_device* pdev) {
    int err = 0;
    pr_info(LGP "Probing device: %s\n", pdev->name);
    struct hlekio_device* hdev = devm_kzalloc(&pdev->dev, sizeof(struct hlekio_device), GFP_KERNEL);
    struct device_node* group_node;
    u32 pin_num;

    of_property_read_string(pdev->dev.of_node, "label", &hdev->pin.pin_name);
    hdev->miscdev.minor = MISC_DYNAMIC_MINOR;
    hdev->miscdev.name = hdev->pin.pin_name;

    struct device* dev = &(pdev->dev);
    hdev->dev = dev;
    memset(&hdev->in_info, 0, sizeof(struct hlekio_input_info));
    hdev->in_info.reset_jiffers = get_jiffies_64();


    spin_lock_init(&hdev->info_lock);
    init_completion(&hdev->irq_event);
    atomic_set(&hdev->irq_event_waiters_count, 0);

    of_property_read_string(pdev->dev.of_node, "label", &hdev->pin.pin_name);

    group_node = of_parse_phandle(pdev->dev.of_node, "pinctrl-0", 0);
    of_property_read_u32_index(group_node, "brcm,pins", 0, &(pin_num));
    of_property_read_u32_index(group_node, "brcm,function", 0, &(hdev->pin.pin_func));
    of_property_read_u32_index(group_node, "brcm,pull", 0, &(hdev->pin.pin_pull));
    of_node_put(group_node);

    if ( (hdev->pin.pin_func != BCM2835_FSEL_GPIO_IN) && 
         (hdev->pin.pin_func != BCM2835_FSEL_GPIO_OUT) ){
        pr_err(LGP"Invalid function (%d) number for %s.\n", hdev->pin.pin_func, hdev->pin.pin_name);
        err = -EINVAL;
        goto error;
    }

    if ( (hdev->pin.pin_pull != BCM2835_PUD_OFF) && 
         (hdev->pin.pin_pull != BCM2835_PUD_DOWN) &&
         (hdev->pin.pin_pull != BCM2835_PUD_UP) ){
        pr_err(LGP"Invalid pullup (%d) number for %d.\n", hdev->pin.pin_func, hdev->pin.pin_pull);
        err = -EINVAL;
        goto error;
    }

    pr_info(LGP"Function for %s (brcm,function): %s.\n", hdev->pin.pin_name, function_text[hdev->pin.pin_func]);
    pr_info(LGP"Pin number for %s (brcm,pins): %d.\n", hdev->pin.pin_name, pin_num);
    pr_info(LGP"Pull-up for %s (brcm,pull): %s.\n", hdev->pin.pin_name, pull_up_text[hdev->pin.pin_pull]);

    if (IS_OUTPUT_PIN(hdev->pin)) {
        hdev->miscdev.fops = &hlekio_out_fops;
        hlekio_init_out_pin(hdev);
    } else if (IS_INPUT_PIN(hdev->pin)) {
        hdev->miscdev.fops = &hlekio_in_fops;
        hlekio_init_in_pin(hdev);
    } else {
        dev_err(hdev->dev, LGP"Invalid pin function!!!\n");
        err = -EFAULT;
        goto error;
    }

    err = misc_register(&hdev->miscdev);
    if (err) {
        goto error;
    }

    platform_set_drvdata(pdev, hdev);

error:
    pr_info(LGP"Probing device completed: %s. Errcode=%d\n", pdev->name, err);

    return err;
}

int hlekio_remove(struct platform_device* pdev) {
    pr_info(LGP "Removing device: %s\n", pdev->name);
    struct hlekio_device* hdev = platform_get_drvdata(pdev);
    misc_deregister(&hdev->miscdev);

    return 0;
}

