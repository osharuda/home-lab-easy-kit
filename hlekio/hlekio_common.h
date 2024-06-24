#pragma once
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include "hlekio_ioctl.h"
#include <linux/gpio/consumer.h>

struct hlekio_pin {
    u32 default_level;
    u32 pin_func;
    u32 pin_pull;
    const char *pin_name;
    int irq;
    int can_sleep;
    int open_drain;
    int trigger_by_level;
    int trigger_level;
};

struct hlekio_device
{
    struct miscdevice miscdev;
    struct device* dev;
    union {
        struct hlekio_input_info in_info;
        struct hlekio_out_info out_info;
    };

    struct hlekio_pin pin;
    struct gpio_desc* pin_desc;

    spinlock_t info_lock;
    struct completion irq_event;
    atomic_t irq_event_waiters_count;
};

#define HLEKIO_FILE_BIN_MODE    (1 << 0)

struct hlekio_file_data {
    unsigned long file_opts;
    struct hlekio_device* hdev;
};

#define IS_INPUT_PIN(p) ((p).pin_func == BCM2835_FSEL_GPIO_IN)
#define IS_OUTPUT_PIN(p) ((p).pin_func == BCM2835_FSEL_GPIO_OUT)

#define IS_TEXT_MODE(fdata) (((fdata)->file_opts & HLEKIO_FILE_BIN_MODE) == 0)
#define IS_BINARY_MODE(fdata) (((fdata)->file_opts & HLEKIO_FILE_BIN_MODE) != 0)
#define SET_BINARY_MODE(fdata, v)                   \
if ((v)) {                                          \
    (fdata)->file_opts |= HLEKIO_FILE_BIN_MODE;     \
} else {                                            \
    (fdata)->file_opts &= (~HLEKIO_FILE_BIN_MODE);  \
}

#define HLEKIO_RISE (0)
#define HLEKIO_FALL (1)
#define HLEKIO_EDGE (2)
#define HLEKIO_HI   (3)
#define HLEKIO_LO   (4)

#define TO_BINARY_LEVEL(lvl) ((lvl) != '0')
#define TO_TEXT_LEVEL(lvl) ('0' + (!!lvl))

#define DRIVER_DESCRIPTION "HLEK Input/Output Driver"
#define DRIVER_NAME        "hlekio"
#define LGP                DRIVER_NAME": "

int hlekio_set_gpio(struct hlekio_device* hdev, u8 value);
u8 hlekio_get_gpio(struct hlekio_device* dev);
ssize_t hlekio_write(const char __user *buff, size_t count, loff_t *ppos, char __kernel * kbuffer, size_t kbuffer_size);
ssize_t hlekio_read(char __user *buff, size_t count, loff_t *ppos, const char __kernel * kbuffer, size_t kbuffer_size);

extern char* function_text[];
extern char* pull_up_text[];
extern char* level_text[];
