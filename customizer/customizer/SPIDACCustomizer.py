#   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
#
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

from DeviceCustomizer import *
from tools import *
from keywords import *
from ctypes import *


def get_channels_definition(devname: str, channels: dict, max_channels: int) -> (str, int, dict):
    """
    :param devname: Name of the device
    :param channels: Dictionary with the channel names as keys and values as dictionaries with the following key (str)/values:
                     address (int) - 0 based adderss of the channel
                     min_value (float) - minimum value for the channel
                     max_value (float) - maximum value for the channel
                     default_value (float) - default value for the channel
    :param max_channels: Maximum number of channels for this part number.
    :return: Returns group of three values:
             [0]: C/C++ like definition for array of SPIDACChannelDescriptor structure
             [1]: Number of the channels
             [2]: Modified dictionary where keys are channel index (int) and values are another dictionary with the
                  following key/values possible:
                  name: Name of the channel (str)
                  min_value: minimal value (float)
                  max_value: maximum value (float)
                  def_value: default value (float)
    """
    channels_by_index = dict()
    ch_count = len(channels)

    ch_addresses = set()

    for k, v in channels.items():
        ch_name = str(k)

        # Process address
        if max_channels > 1:
            ch_address = int(v["address"])
        else:
            ch_address = 0

        if ch_address in ch_addresses:
            raise RuntimeError(f'Duplicate channel {ch_address} for device "{devname}"')
        ch_addresses.add(ch_address)
        if ch_address < 0 or ch_address >= ch_count:
            raise RuntimeError(f'Invalid address for device "{devname}"')

        ch_min_value = float(v["min_value"])
        ch_max_value = float(v["max_value"])
        ch_def_value = float(v["default_value"])

        if ch_min_value >= ch_max_value:
            raise RuntimeError(f"Device {devname} has minimum value higher or equal then maximum value for the channel {ch_name}")

        if ch_def_value > ch_max_value:
            raise RuntimeError(f"Device {devname} has default value higher then maximum value for the channel {ch_name}")

        if ch_def_value < ch_min_value:
            raise RuntimeError(f"Device {devname} has default value lower then minimum value for the channel {ch_name}")

        if ch_address in channels_by_index:
            raise RuntimeError(f"Device {devname} has duplicate indexes for channels {ch_name} and {channels_by_index[ch_address]['name']}")

        channel = { "name": ch_name,
                    "min_value": ch_min_value,
                    "max_value": ch_max_value,
                    "def_value": ch_def_value}
        channels_by_index[ch_address] = channel

    indexes = list(channels_by_index.keys())
    indexes.sort()

    ch_def_list = []
    for ch_address in indexes:
        channel = channels_by_index[ch_address]
        ch_name = channel["name"]
        ch_min_value = channel["min_value"]
        ch_max_value = channel["max_value"]
        ch_def_value = channel["def_value"]

        ch_def_list.append(f"{{ {ch_address}, \"{ch_name}\", {ch_min_value}, {ch_max_value}, {ch_def_value} }}")
    return ', '.join(ch_def_list), len(indexes), channels_by_index



def spidac_append_dac8564_sample(value: float,
                                 min_value: float,
                                 max_value: float,
                                 address: int):
    # DAC8564 format (Datasheet information:
    #     "16-Bit, Quad Channel, Ultra-Low Glitch, Voltage Output DIGITAL-TO-ANALOG CONVERTER with 2.5V, 2ppm/°C Internal Reference",
    #     internal shift register, page 29-30)
    #
    # Frame size - 24 bit
    # 23 22 21  20   19  18    17   16    15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0
    # A₁ A₀ LD₁ LD₀  0   SEL₁  SEL₀ PD₀   D₁₅  D₁₄  D₁₃  D₁₂  D₁₁  D₁₀  D₉  D₈  D₇  D₆  D₅  D₄  D₃  D₂  D₁  D₀
    # A₁, A₀ - Chip selection. Currently set to zeroes, corresponding pins must be grounded.
    # LD₁, LD₀ - Load command. LD₁=0 and LD₀=0 is used to do single channel store.
    #            Once data for required channels is set, DACs are updated by LDAC signal.
    # SEL₁  SEL₀ - Channel selection:
    #              SEL₁=0  SEL₀=0 to select buffer A
    #              SEL₁=0  SEL₀=1 to select buffer B
    #              SEL₁=1  SEL₀=0 to select buffer C
    #              SEL₁=1  SEL₀=1 to select buffer D
    # PD₀ - Power down mode selection (by setting to 1)
    # D₁₅ (MSB) - D₀ (LSB) - Data

    x = normalize_value(value, min_value, max_value)
    max_val = 0xFFFF
    val = int(max_val * x)

    result = list()
    result += [hex(((address & 3) << 1) | ((address & 12) << 4))]
    result += [hex((val & 0xFF00) >> 8)]
    result += [hex(val & 0xFF)]

    return result


def spidac_append_dac7611_sample(value: float,
                                 min_value: float,
                                 max_value: float,
                                 address: int):
    max_val = 0x0FFF
    x = normalize_value(value, min_value, max_value)
    val = int(max_val * x)

    result = list()
    result += [hex((val & 0xFF00) >> 8)]
    result += [hex(val & 0xFF)]

    return result


def spidac_append_dac8550_sample(value: float,
                                 min_value: float,
                                 max_value: float,
                                 address: int):
    # DAC8550 format (Datasheet information:
    # "DAC8550 16-bit, Ultra-Low Glitch, Voltage Output Digital-To-Analog Converter",
    # internal shift register, page 19)
    #
    # Frame size - 24 bit (numbered as transferred)
    # 23 22 21 20 19 18    17  16    15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0
    # [  U N U S E D  ]    PD₁ PD₀   D₁₅  D₁₄  D₁₃  D₁₂  D₁₁  D₁₀  D₉  D₈  D₇  D₆  D₅  D₄  D₃  D₂  D₁  D₀
    # PD₁, PD₀ - Mode
    # PD₁=0, PD₀=0 : normal mode
    # PD₁=0, PD₀=1 : Output typically 1kΩ to GND
    # PD₁=1, PD₀=0 : Output typically 100kΩ to GND
    # PD₁=1, PD₀=1 : High-Z state
    # D₁₅ (MSB) - D₀ (LSB) - Data

    max_val = 0xFFFF
    x = normalize_value(value, min_value, max_value) - 0.5
    val = int(max_val * x)

    result = list()
    result += [hex(0)]
    result += [hex(val >> 8)]
    result += [hex(val & 0xFF)]

    return result


def get_frame_for_channel(min_val: float,
                          max_val: float,
                          def_value: float,
                          bits_per_sample: int,
                          frame_format: str) -> list:
    """
    This function will produce a list of bytes for the frames to set default value in SPIDAC device. This function should
    be in sync with SPIDACDev::append_frame_with_sample from the software part.
    :param min_val: Minimal value for the channel
    :param max_val: Maximum value for the channel
    :param def_value: Default value for the channel
    :param bits_per_sample: Number of bits per sample
    :param frame_format: Format of the frame
    :return: List of bytes for the passed frame
    """
    result = list()
    x = (def_value - min_val) / (max_val - min_val)
    bits_mask = (1 << bits_per_sample) - 1
    if 0.0 > x > 1.0:
        raise RuntimeError("Default value is out of range")

    dv = int(float(bits_mask)*x)
    frame_size = 2

    if frame_format == 'MSB':
        for i in range(0, frame_size):
            b = dv & 0xFF
            result += [hex(b)]
            dv = dv >> 8
    elif frame_format == 'LSB':
        for i in range(0, frame_size):
            b = dv & 0xFF
            result += [hex(0)]
            dv = dv >> 8
    else:
        raise RuntimeError("Unsupported frame format!")

    return result

class SPIDacChips:
    def __init__(self):
        self.chip_info_map = {
            'DAC7611': {
                KW_SPI_CLOCK_PHASE: KW_SPI_CLOCK_PHASE_SECOND,
                KW_SPI_CLOCK_POLARITY: KW_SPI_CLOCK_POLARITY_IDLE_HIGH,
                KW_SPI_CLOCK_SPEED: '9MHz',
                KW_SPI_FRAME_FORMAT: KW_SPI_FRAME_FORMAT_MSB,
                KW_SPI_FRAME_SIZE: 16,
                KW_SPIDAC_FRAMES_PER_SAMPLE: 1,
                KW_SPIDAC_LD_MODE: KW_SPIDAC_LD_MODE_FALL,
                KW_SPIDAC_BITS_PER_SAMPLE: 12,
                KW_SPIDAC_SAMPLE_FORMAT: KW_SPIDAC_SAMPLE_FORMAT_DAC7611,
                KW_MAX_CHANNELS: 1
            },
            'DAC8550': {
                KW_SPI_CLOCK_PHASE: KW_SPI_CLOCK_PHASE_FIRST,
                KW_SPI_CLOCK_POLARITY: KW_SPI_CLOCK_POLARITY_IDLE_HIGH,
                KW_SPI_CLOCK_SPEED: '9MHz',
                KW_SPI_FRAME_FORMAT: KW_SPI_FRAME_FORMAT_LSB,
                KW_SPI_FRAME_SIZE: 8,
                KW_SPIDAC_FRAMES_PER_SAMPLE: 3,
                KW_SPIDAC_BITS_PER_SAMPLE: 16,
                KW_SPIDAC_SAMPLE_FORMAT: KW_SPIDAC_SAMPLE_FORMAT_DAC8550,
                KW_MAX_CHANNELS: 1
            },
            'DAC8564': {
                KW_SPI_CLOCK_PHASE: KW_SPI_CLOCK_PHASE_FIRST,
                KW_SPI_CLOCK_POLARITY: KW_SPI_CLOCK_POLARITY_IDLE_HIGH,
                KW_SPI_CLOCK_SPEED: '9MHz',
                KW_SPI_FRAME_FORMAT: KW_SPI_FRAME_FORMAT_LSB,
                KW_SPI_FRAME_SIZE: 8,
                KW_SPIDAC_FRAMES_PER_SAMPLE: 3,
                KW_SPIDAC_LD_MODE: KW_SPIDAC_LD_MODE_RISE,
                KW_SPIDAC_BITS_PER_SAMPLE: 16,
                KW_SPIDAC_SAMPLE_FORMAT: KW_SPIDAC_SAMPLE_FORMAT_DAC8564,
                KW_MAX_CHANNELS: 4
            }
        }
        pass

    # Returns properties for known chip with name being passed as parameter
    def get_chip_info(self, chip_name: str) -> map:
        return self.chip_info_map.get(chip_name, None)



class SPIDACCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "SPIDAC")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "spidac_conf.hpp"
        self.sw_lib_source = "spidac_conf.cpp"

        #self.little_endian = sys.byteorder == 'little'
        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)
        self.dac_chips = SPIDacChips()


    def read_ld_configuration(self, dev_requires: dict, ld_mode: str, dev_name: str):
        ld_port = "0"
        ld_pin = "0"
        ld_rise = "0"
        if ld_mode is not None:
            gpio = get_value_from_dict_list([dev_requires], KW_SPIDAC_LOAD, dev_name)[RT_GPIO]
            ld_port = self.mcu_hw.GPIO_to_port(gpio)
            ld_pin = self.mcu_hw.GPIO_to_pin_number(gpio)

            if ld_mode == KW_SPIDAC_LD_MODE_RISE:
                ld_rise = 1
            elif ld_mode == KW_SPIDAC_LD_MODE_FALL:
                ld_rise = 0
            else:
                raise RuntimeError(f"incorrect ld_mode value ({ld_mode}) is specified for device {dev_name}")

        return ld_port, ld_pin, ld_rise


    def get_spidac_status_length(self, channel_number: int):
        return f"sizeof(struct SPIDACStatus) + {channel_number}*sizeof(struct SPIDACChannelSamplingInfo)"

    def get_private_data_initializer(self, channel_data_buffer_name: str, channel_number: int):
        return f"""{{  /* struct SPIDACPrivData */\\
                    {{0}}, \\
                    NULL,  \\
                    NULL,  \\
                    NULL,  \\
                    {channel_data_buffer_name}, /* Channel data buffer */   \\
                    {channel_data_buffer_name} + {channel_number}, /* The end (the one beyond last) element of the channel_data. */\\
                    NULL, /* Current channel data */ \\
                    0, /* uint32_t                 dma_ccr_enabled */   \\
                    0, /* uint32_t                 dma_ccr_disabled */  \\
                    0, /* uint16_t                 sample_buffer_size */    \\
                    0, /* uint16_t                 spi_cr1_enabled */   \\
                    0, /* uint16_t                 spi_cr1_disabled */  \\
                    0, /* prescaller */ \\
                    0, /* period */ \\
                    0, /* uint8_t                  phase_overflow_status */ }}"""


    # This method is reflection of the SPIDACDev::append_frame_with_sample from software part. Implementation of these
    # methods should match!
    # Returns list of bytes for the buffer for single method

    def customize(self):
        fw_device_descriptors = []  # these descriptors are used to configure each device on firmwire side
        sw_device_desсriptors = []  # these descriptors are used to configure each device on software side
        fw_device_buffers = []  # these buffers are used to be internal buffers for all configured devices on firmware side
        fw_device_default_value_buffers = []  # these buffers are used to be internal buffers for all configured devices on firmware side
        fw_tx_dma_irq_list = []  # list of TX DMA irq handlers
        fw_dma_tx_preinit_list = []  # list of dma_tx_preinit structures
        fw_set_address_func_headers = []
        fw_set_address_func_impls = []
        fw_address_lists = []
        fw_multichannel = 0
        fw_need_ld_pin = 0
        timer_irq_handler_list = []  # list of the timer IRQ handlers
        sw_channels_descriptors = []
        sw_config_array_name = "spidac_configs"
        sw_config_declarations = []
        sw_configs = []

        index = 0
        for dev_name, dev_config in self.device_list:
            exclude_resources = {'SPI_MISO', 'SPI_RX_DMA'}  # We are not receiving anything from DAC
            dev_requires = dev_config[KW_REQUIRES]
            dev_id = dev_config[KW_DEV_ID]

            timer = self.get_timer(dev_requires)
            time_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(timer)
            timer_irqn = self.mcu_hw.ISRHandler_to_IRQn(time_irq_handler)
            timer_irq_handler_list.append(
                "MAKE_ISR_WITH_INDEX({0}, SPIDAC_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(time_irq_handler, index))
            dev_requires["TIMER_IRQ"] = {"irq_handlers": time_irq_handler}

            spi_confg_dev = self.get_spi(dev_requires)
            spi, remap = self.mcu_hw.is_remaped(spi_confg_dev)
            dev_requires[RT_SPI] = spi


            part_number = dev_config.get(KW_SPIDAC_PART_NUMBER, None)
            part_number_config = dict()
            if part_number:
                part_number_config = self.dac_chips.get_chip_info(part_number)
                if part_number_config is None:
                    raise RuntimeError(f'Unknown part numbed is specified: f{part_number}')

            # Clock speed
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPI_CLOCK_SPEED, dev_name)
            baud_rate_control, spi_freq = self.mcu_hw.spi_get_baud_rate_control(spi_confg_dev, v)

            # Clock phase
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPI_CLOCK_PHASE, dev_name)
            clock_phase = self.mcu_hw.spi_get_clock_phase(v.upper())

            # Clock priority
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPI_CLOCK_POLARITY, dev_name)
            clock_polarity = self.mcu_hw.spi_get_clock_polarity(v.upper())

            # Frame format
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPI_FRAME_FORMAT, dev_name)
            frame_format = v.upper()

            # Frame size
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPI_FRAME_SIZE, dev_name)
            frame_size = int(self.mcu_hw.spi_get_frame_size(v))

            # Frames Per Sample ???
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPIDAC_FRAMES_PER_SAMPLE, dev_name)
            frames_per_sample = int(v)

            # Bits Per Sample
            v = get_value_from_dict_list([dev_config, part_number_config], KW_SPIDAC_BITS_PER_SAMPLE, dev_name)
            bits_per_sample = int(v)

            # LD line configuration
            ld_mode = get_value_from_dict_list([dev_config, part_number_config], KW_SPIDAC_LD_MODE, dev_name, throw_exc=False)
            ld_port, ld_pin, ld_rise = self.read_ld_configuration(dev_requires, ld_mode, dev_name)
            if ld_mode is not None:
                fw_need_ld_pin |= 1

            # Sample format
            sample_format = get_value_from_dict_list([dev_config, part_number_config], KW_SPIDAC_SAMPLE_FORMAT, dev_name)
            max_channels = get_value_from_dict_list([part_number_config], KW_MAX_CHANNELS, dev_name)



            channels_descr, channels_count, channels_by_index = get_channels_definition(dev_name, dev_config[KW_SPIDAC_CHANNELS], max_channels)
            channels_addresses = sorted(list(channels_by_index.keys()))
            channels_descr_varname = f"g_{dev_name.lower()}_channel_descriptors"
            sw_channels_descriptors.append(f"const struct SPIDACChannelDescriptor {channels_descr_varname}[] = {{ {channels_descr} }};")

            # Decide if we need multichannel configuration
            fw_multichannel |= int(channels_count > 1)

            # Samples number
            v = get_value_from_dict_list([dev_config], KW_SPIDAC_SAMPLE_NUMBER, dev_name)
            samples_number = int(v)
            if samples_number <= 0:
                raise RuntimeError(
                    f"Number of samples for device {dev_name} may not be zero")
            buffer_size = int((samples_number + channels_count)*frames_per_sample*(frame_size+1))

            dma_tx = self.mcu_hw.dma_request_map[spi + "_TX"]
            dma_tx_it = "0"
            dma_tx_preinit = "NULL"

            spi_miso_port = "NULL"
            spi_miso_pin = 0

            tx_dma_channel = self.mcu_hw.get_DMA_Channel(spi + "_TX")
            tx_dma_irq_handler = self.mcu_hw.DMA_Channel_to_IRQHandler(tx_dma_channel)
            tx_dma_irqn = self.mcu_hw.ISRHandler_to_IRQn(tx_dma_irq_handler)
            tx_dma = self.mcu_hw.get_DMA_from_channel(tx_dma_channel)

            fw_tx_dma_irq_list.append(
                "MAKE_ISR_WITH_INDEX({0}, SPIDAC_COMMON_TX_DMA_IRQ_HANDLER, {1}) \\".format(tx_dma_irq_handler, index))
            dev_requires["TX_DMA_IRQ"] = {"irq_handlers": tx_dma_irq_handler}
            exclude_resources.add('SPI_IRQ')

            dma_tx_preinit = "g_spi_dma_tx_preinit_" + str(index)
            fw_dma_tx_preinit_list.append(self.tab + f"DMA_InitTypeDef {dma_tx_preinit}; \\")
            dma_tx_preinit = "&" + dma_tx_preinit
            dma_tx_it = self.mcu_hw.get_DMA_IT_Flag(tx_dma_channel, "TC")

            spi_mosi = self.get_required_resource(spi_confg_dev, "SPI_MOSI", RT_GPIO)
            spi_mosi_port = self.mcu_hw.GPIO_to_port(spi_mosi)
            spi_mosi_pin = self.mcu_hw.GPIO_to_pin_number(spi_mosi)

            spi_sck = self.get_required_resource(spi_confg_dev, "SPI_SCK", RT_GPIO)
            spi_sck_port = self.mcu_hw.GPIO_to_port(spi_sck)
            spi_sck_pin = self.mcu_hw.GPIO_to_pin_number(spi_sck)

            spi_nss = self.get_required_resource(spi_confg_dev, "SPI_NSS", RT_GPIO)
            spi_nss_port = self.mcu_hw.GPIO_to_port(spi_nss)
            spi_nss_pin = self.mcu_hw.GPIO_to_pin_number(spi_nss)

            # Get sorted list of channels addresses

            # Make initialization sample
            init_frames_buffer = list()
            for address in channels_addresses:
                min_value = float(channels_by_index[address]["min_value"])
                max_value = float(channels_by_index[address]["max_value"])
                def_value = float(channels_by_index[address]["def_value"])

                if sample_format == KW_SPIDAC_SAMPLE_FORMAT_DAC8564:
                    init_frames_buffer.extend(spidac_append_dac8564_sample(def_value, min_value, max_value, address))
                elif sample_format == KW_SPIDAC_SAMPLE_FORMAT_DAC8550:
                    init_frames_buffer.extend(spidac_append_dac8550_sample(def_value, min_value, max_value, address))
                elif sample_format == KW_SPIDAC_SAMPLE_FORMAT_DAC7611:
                    init_frames_buffer.extend(spidac_append_dac7611_sample(def_value, min_value, max_value, address))
                else:
                    raise RuntimeError(f'Unknown sample format, are you sure "{KW_SPIDAC_PART_NUMBER}" was specified in json config file.')

            init_frames_default_value_initialization = ", ".join(init_frames_buffer)
            init_frames_default_value_name = "g_spidac_{0}_default_value".format(str(index))
            fw_device_default_value_buffers.append(
                f"uint8_t {init_frames_default_value_name}[] = {{ {init_frames_default_value_initialization} }};\\")

            self.check_requirements(spi_confg_dev, dev_requires, "dev_{0}".format(dev_name), exclude_resources)

            transaction_size = frames_per_sample * (frame_size + 1)
            sample_size = channels_count * transaction_size
            max_bytes_per_transaction = sample_size * (self.i2c_buffer_size // sample_size)
            spi_dac_status_len = self.get_spidac_status_length(channels_count)

            fw_buffer_name = "g_spidac_{0}_buffer".format(str(index))
            fw_device_buffers.append(
                f"uint8_t {fw_buffer_name}[{buffer_size}+{spi_dac_status_len} + {sample_size}] __attribute__ ((aligned));\\")

            fw_channel_data_name = "g_spidac_{0}_channel_data".format(str(index))
            fw_device_buffers.append(
                f"struct SPIDACChannelData {fw_channel_data_name}[{channels_count}] __attribute__ ((aligned));\\")

            fw_default_start_info_name = "g_spidac_{0}_default_start_info".format(str(index))
            fw_device_buffers.append(
                f"uint8_t {fw_default_start_info_name}[sizeof(struct SPIDACStartInfo) + {spi_dac_status_len}] __attribute__ ((aligned));\\")

            fw_device_descriptors.append(f"""{{\
    {{0}},                      /* Device context */\\
    {self.get_private_data_initializer(fw_channel_data_name, channels_count)}, \\
    (struct SPIDACStartInfo*){fw_default_start_info_name}, /* Default start information */ \\
    {fw_buffer_name},           /* Buffer (for status and samples) */\\
    {init_frames_default_value_name}, /* Default values to be put after reset */\\
    {fw_buffer_name} + {spi_dac_status_len}, /* Default sample (value) buffer base */\\
    {fw_buffer_name} + {spi_dac_status_len} + {sample_size}, /* Sample buffer base */\\
    {spi},                      /* SPI Device */\\
    {timer},                    /* Timer */\\
    {tx_dma_channel},           /* TX DMA Channel */\\
    {spi_mosi_port},            /* MOSI port */\\
    {spi_sck_port},             /* SCK port */\\
    {spi_nss_port},             /* NSS port */\\
    {ld_port},                  /* LD port (0 if not used) */\\
    {tx_dma},                   /* DMA controller to be used */\\
    {dma_tx_it},                /* TX DMA interrupt flag */\\
    (1 << {ld_pin}),            /* LD pin bit mask */\\
    {buffer_size} + {spi_dac_status_len}, /* Buffer size (status and samples) */\\
    {buffer_size - sample_size}, /* Maximum sample buffer size (one sample is dedicated to default values) */\\
    {sample_size}, /* Sample (data for all channels) size in bytes */\\
    {transaction_size}, /* SPI Transaction size (size of the sample data for single channel, in bytes) */\\
    {transaction_size // (frame_size + 1)}, /* SPI Transaction size (size of the sample data for single channel, in frames) */\\ 
    {tx_dma_irqn},              /* TX DMA IRQn value */\\
    {timer_irqn},               /* TIMER IRQn value */\\
    {baud_rate_control},        /* Baud rate control value for SPI */\\
    {frame_size},               /* Frame size */\\
    {int(remap)},               /* Indicates if SPI port is remapped */\\
    {spi_mosi_pin},             /* MOSI pin */\\
    {spi_sck_pin},              /* SCK pin */\\
    {spi_nss_pin},              /* NSS pin */\\
    {ld_rise},                  /* LD line behaviour: 0 - load is triggered by fall, 1 by rise of the signal */\\
    {clock_polarity},           /* Clock polarity */\\
    {clock_phase},              /* Clock phase */\\
    {frames_per_sample},        /* Frames per sample */\\
    {channels_count},           /* Number of channels */\\
    {dev_id}                    /* Device id */}}""")

            sw_device_desсriptors.append(f"""{{\
    "{dev_name}",               /* Name of the device */\\
    {buffer_size - sample_size},/* Maximum sample buffer size (one sample is dedicated to default values) */\\
    {frames_per_sample},        /* Frames per sample */\\
    {frame_size + 1},           /* Frame size, in bytes */\\
    {max_bytes_per_transaction},       /* Max bytes per i2c transaction */\\
    {dev_id},                   /* Device id */\\
    {frame_format},     /* Frame format */\\
    {channels_count},           /* Number of channels */\\
    {samples_number},           /* Maximum number of samples per channel */\\
    {bits_per_sample},          /* Number of bits per sample */\\
    {self.mcu_hw.get_TIMER_freq(timer)},   /* Timer frequency */ \\
    {sample_format},             /* Sample format */ \\
    {channels_descr_varname}     /* Channels description */ }}""")

            sw_config_name = "spidac_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const struct SPIDACConfig* {sw_config_name};")
            sw_configs.append(
                f"const struct SPIDACConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__SPIDAC_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__SPIDAC_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__SPIDAC_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__SPIDAC_FW_BUFFERS__": concat_lines(fw_device_buffers)[:-1],
                      "__SPIDAC_FW_TX_DMA_IRQ_HANDLERS__": concat_lines(fw_tx_dma_irq_list)[:-1],
                      "__SPIDAC_FW_DMA_TX_PREINIT__": concat_lines(fw_dma_tx_preinit_list)[:-1],
                      "__SPIDAC_FW_TIMER_IRQ_HANDLERS__": concat_lines(timer_irq_handler_list)[:-1],
                      "__SPIDAC_SW_CHANNEL_DESCRIPTORS__": ",\\".join(sw_channels_descriptors),
                      "__SPIDAC_FW_DEFAULT_VALUES__": concat_lines(fw_device_default_value_buffers)[:-1],
                      "__SPIDAC_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__SPIDAC_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__SPIDAC_CONFIGURATION_ARRAY_NAME__": sw_config_array_name,
                      "__SPIDAC_SET_ADDRESS_FUNC_DEF__": "\\\n".join(fw_set_address_func_headers),
                      "__SPIDAC_SET_ADDRESS_FUNC_IMPL__": "\\\n".join(fw_set_address_func_impls),
                      "__SPIDAC_SET_ADDRESS_LISTS__": "\\\n".join(fw_address_lists),
                      "__SPIDAC_MULTI_CHANNEL__": fw_multichannel,
                      "__SPIDAC_NEED_LD__": fw_need_ld_pin
                      }

        self.patch_templates()
