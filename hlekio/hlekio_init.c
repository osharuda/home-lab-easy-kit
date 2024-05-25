#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#include "hlekio_init.h"
#include "hlekio_common.h"
#include "hlekio_hw.h"
#include "hlekio_probe.h"

static const struct of_device_id hlekio_ids[] = {
        {.compatible="hlek,io"},
        {}
};

static struct platform_driver hlekio_platform_driver = {
    .probe = hlekio_probe,
    .remove = hlekio_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = hlekio_ids,
        .owner = THIS_MODULE
    }
};

static int __init hlekio_init(void)
{
    pr_info(LGP "ENTER\n");
    int err = platform_driver_register(&hlekio_platform_driver);
    if (err) {
        pr_err(LGP "Failed to register platform driver: %d\n", err);
        return err;
    }

    return 0;
}

static void __exit hlekio_cleanup(void) {
    pr_info(LGP "Cleaning up.\n");
    platform_driver_unregister(&hlekio_platform_driver);
    pr_info(LGP "Clean up completed.\n");
}

static void __exit hlekio_exit(void)
{
    hlekio_cleanup();
    pr_info(LGP "EXIT\n");
}

module_init(hlekio_init);
module_exit(hlekio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleh Sharuda");
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);

