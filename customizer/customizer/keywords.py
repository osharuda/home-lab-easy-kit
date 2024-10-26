KW_DEV_ID       = "dev_id"
KW_REQUIRES     = "requires"
KW_BUFFER_SIZE  = "buffer_size"
KW_MAX_SAMPLES  = "max_samples"
KW_SIGNALS      = "signals"
KW_TYPE         = "type"
KW_DEFAULT      = "default"
KW_PIN          = "pin"
KW_ADDRESS      = "address"
KW_CLOCK_SPEED  = "clock_speed"
KW_DEV_NAME     = "device_name"
KW_BUS          = "bus"

KW_SDA_LINE     = "SDA"
KW_SCL_LINE     = "SCL"
KW_EV_IRQ_HLR   = "ev_irq_handler"
KW_ER_IRQ_HLR   = "er_irq_handler"

KW_TIMER_IRQ_HANDLER = "timer_handler"
KW_MAIN_TIMER   = "main_timer"
KW_INT_TIMER    = "internal_timer"

# Resource types
RT_GPIO         = "gpio"
RT_TIMER        = "timer"
RT_IRQ_HANDLER  = "irq_handler"
RT_EXTI_LINE    = "exti_line"
RT_BACKUP_REG   = "bkp"
RT_UART         = "usart"
RT_I2C          = "i2c"
RT_DMA          = "dma"
RT_DMA_CHANNEL  = "dma_channel"
RT_ADC          = "adc"
RT_ADC_INPUT    = "adc_input"
RT_SPI          = "spi"


# Firmware JSON keywords
FW_FIRMWARE     = "firmware"
FW_I2C          = "i2c_bus"
FW_SYS_TICK     = "sys_tick"
FW_BUFFER_SIZE  = "buffer_size"
FW_DEVICE_NAME  = 'device_name'

# HLEKIO Json keys
KW_BOARD        = "board"
KW_DISTRO       = "distro"
KW_RASPBERRY_PI_OS = "Raspberry Pi OS"
KW_DIET_PI      = "DietPi"
KW_UBUNTU_20_04 = "Ubuntu 20.04"
KW_UBUNTU_22_04 = "Ubuntu 22.04"


KW_INPUTS       = "inputs"
KW_OUTPUTS      = "outputs"
KW_HLEKIO       = "hlekio"

KW_INFO_URL     = "info_url"
KW_CPU          = "cpu"

# Trigger types
KW_TRIGGER      = "trigger"
KW_RISE         = "rise"
KW_FALL         = "fall"
KW_EDGE         = "edge"

# Input pull types
KW_PULL         = "pull"
KW_PULL_UP      = "up"
KW_PULL_DOWN    = "down"
KW_PULL_NONE    = "none"

# Logical levels
KW_LOGICAL_HI  = "high"
KW_LOGICAL_LO  = "low"

# Purpose of the pins
KW_INTERRUPT   = "interrupt"
KW_NEAR_FULL   = "near_full"
KW_NEAR_empty  = "near_empty"

# Pin types
KW_PIN_TYPE = "pin-type"
KW_PUSH_PULL = "push-pull"
KW_OPEN_DRAIN = "open-drain"

# File names from source tree
FILE_SYNCHRONIZATION_HDR = "synchronization.h"
FILE_CIRC_BUF_HDR = "circbuffer.h"
FILE_CIRC_BUF_SRC = "circbuffer.c"
FILE_UTOOLS_BUF_HDR = "utools.h"
FILE_UTOOLS_BUF_SRC = "utools.c"


KW_FEATURE_DEFINES = "feature_defines"

# SPI
KW_SPI_CLOCK_PHASE = 'clock_phase'
KW_SPI_CLOCK_PHASE_FIRST = 'first'
KW_SPI_CLOCK_PHASE_SECOND = 'second'

KW_SPI_CLOCK_POLARITY = 'clock_polarity'
KW_SPI_CLOCK_POLARITY_IDLE_HIGH = 'idle_high'
KW_SPI_CLOCK_POLARITY_IDLE_LOW = 'idle_low'

KW_SPI_CLOCK_SPEED = 'clock_speed'
KW_SPI_FRAME_FORMAT = 'frame_format'
KW_SPI_FRAME_FORMAT_MSB = 'msb'
KW_SPI_FRAME_FORMAT_LSB = 'lsb'



KW_SPI_FRAME_SIZE = 'frame_size'

# SPIDAC
KW_SPIDAC_FRAMES_PER_SAMPLE = 'frames_per_sample'

KW_SPIDAC_LD_MODE = 'ld_mode'
KW_SPIDAC_LD_MODE_RISE = 'rise'
KW_SPIDAC_LD_MODE_FALL = 'fall'

KW_SPIDAC_BITS_PER_SAMPLE = 'bits_per_sample'
KW_SPIDAC_SAMPLE_FORMAT = 'sample_format'
KW_SPIDAC_SAMPLE_FORMAT_DAC7611 = 'SPIDAC_SAMPLE_FORMAT_DAC7611'
KW_SPIDAC_SAMPLE_FORMAT_DAC8564 = 'SPIDAC_SAMPLE_FORMAT_DAC8564'
KW_SPIDAC_SAMPLE_FORMAT_DAC8550 = 'SPIDAC_SAMPLE_FORMAT_DAC8550'
KW_MAX_CHANNELS = 'max_channels'

KW_SPIDAC_PART_NUMBER = 'part_number'
KW_SPIDAC_SAMPLE_NUMBER = 'samples_number'
KW_SPIDAC_CHANNELS = 'channels'

KW_SPIDAC_LOAD = 'ld'




