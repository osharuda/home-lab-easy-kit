#pragma once

#define HLEKIO_MAGIC   0xDA
#define RESET          0x01
#define BINARY_MODE    0x02
#define DEBOUNCE       0x03

#define HLEKIO_RESET 	   _IO(HLEKIO_MAGIC,  RESET)
#define HLEKIO_BINARY_MODE _IOW(HLEKIO_MAGIC, BINARY_MODE, unsigned long)
#define HLEKIO_DEBOUNCE    _IOW(HLEKIO_MAGIC, DEBOUNCE, unsigned long)

#ifndef u64
#define u64 uint64_t
#endif

#ifndef u8
#define u8 uint8_t
#endif

struct hlekio_input_info {
    u64 volatile last_isr_jiffers;
    u64 volatile reset_jiffers;
    unsigned long volatile isr_count;
    unsigned long volatile isr_debounce;
    u8 volatile level;  // This field is meaningfull when used with non-blocking IO. When blocking IO is used
                        // result is unreliable due to contact bounce.
};

struct hlekio_out_info {
    u8 volatile level;
};
