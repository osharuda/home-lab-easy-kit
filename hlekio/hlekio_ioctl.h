#pragma once

#ifndef u64
#define u64 uint64_t
#endif

#ifndef u8
#define u8 uint8_t
#endif

#define HLEKIO_INPUT_DEV        (0)
#define HLEKIO_OUTPUT_DEV       (1)

#define HLEKIO_MAGIC   0xDA
#define RESET          0x01
#define BINARY_MODE    0x02
#define DEBOUNCE       0x03
#define PIN_TYPE       0x04

#define HLEKIO_RESET 	   _IO(HLEKIO_MAGIC,  RESET)
#define HLEKIO_BINARY_MODE _IOW(HLEKIO_MAGIC, BINARY_MODE, unsigned long)
#define HLEKIO_DEBOUNCE    _IOW(HLEKIO_MAGIC, DEBOUNCE, unsigned long)
#define HLEKIO_PIN_TYPE    _IOR(HLEKIO_MAGIC, PIN_TYPE, u8)

#pragma pack(push, 1)
struct hlekio_input_info {
    u64 volatile last_isr_jiffers;
    u64 volatile reset_jiffers;
    unsigned long volatile isr_count;
    unsigned long volatile isr_debounce;
    u8 volatile level;  // This field is meaningful when used with non-blocking IO only. 
                        // When blocking IO is used result may be unreliable due to contact bounce.
};
#pragma pack(pop)

#pragma pack(push, 1)
struct hlekio_out_info {
    u8 volatile level;
};
#pragma pack(pop)

