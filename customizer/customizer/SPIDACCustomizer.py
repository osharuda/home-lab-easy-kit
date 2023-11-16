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
import sys


def get_channels_definition(devname: str, channels: dict) -> (str, int, dict):
    """
    :param devname: Name of the device
    :param channels: Dictionary with the channel names as keys and values as dictionaries with the following key (str)/values:
                     index (int) - 0 based index of the channel
                     min_value (float) - minimum value for the channel
                     max_value (float) - maximum value for the channel
                     default_value (float) - default value for the channel
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

    for k, v in channels.items():
        ch_name = str(k)
        ch_index = int(v["index"])
        ch_min_value = float(v["min_value"])
        ch_max_value = float(v["max_value"])
        ch_def_value = float(v["default_value"])

        if ch_min_value >= ch_max_value:
            raise RuntimeError(f"Device {devname} has minimum value higher or equal then maximum value for the channel {ch_name}")

        if ch_def_value > ch_max_value:
            raise RuntimeError(f"Device {devname} has default value higher then maximum value for the channel {ch_name}")

        if ch_def_value < ch_min_value:
            raise RuntimeError(f"Device {devname} has default value lower then minimum value for the channel {ch_name}")

        if ch_index in channels_by_index:
            raise RuntimeError(f"Device {devname} has duplicate indexes for channels {ch_name} and {channels_by_index[ch_index]['name']}")

        channel = { "name": ch_name,
                    "min_value": ch_min_value,
                    "max_value": ch_max_value,
                    "def_value": ch_def_value}
        channels_by_index[ch_index] = channel

    indexes = list(channels_by_index.keys())
    indexes.sort()

    ch_definition = ""
    for ch_index in indexes:
        channel = channels_by_index[ch_index]
        ch_name = channel["name"]
        ch_min_value = channel["min_value"]
        ch_max_value = channel["max_value"]
        ch_def_value = channel["def_value"]

        ch_definition += f"{{ \"{ch_name}\", {ch_min_value}, {ch_max_value}, {ch_def_value} }}"
    return ch_definition[:-2] + " }", len(indexes), channels_by_index


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
    else:
        raise RuntimeError("Unsupported frame format!")

    return result


class SPIDACCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "SPIDAC")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "spidac_conf.hpp"
        self.sw_lib_source = "spidac_conf.cpp"

        self.little_endian = sys.byteorder == 'little'
        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)


    def read_ld_configuration(self, dev_config: dict, dev_requires: dict, dev_name: str):
        ld_port = "0"
        ld_pin = "0"
        ld_rise = "0"
        if "LD" in dev_requires:
            gpio = dev_requires["LD"]["gpio"]
            ld_port = self.mcu_hw.GPIO_to_port(gpio)
            ld_pin = self.mcu_hw.GPIO_to_pin_number(gpio)

            if "ld_mode" in dev_config:
                ld_mode = dev_config["ld_mode"]

                if ld_mode == "rise":
                    ld_rise = 1
                elif ld_mode == "fall":
                    ld_rise = 0
                else:
                    raise RuntimeError(f"incorrect ld_mode value ({ld_mode}) is specified for device {dev_name}")
            else:
                raise RuntimeError(f"ld_mode is not specified for device {dev_name}")

        return ld_port, ld_pin, ld_rise

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
        timer_irq_handler_list = []  # list of the timer IRQ handlers
        sw_channels_descriptors = []
        sw_config_array_name = "spidac_configs"
        sw_config_declarations = []
        sw_configs = []

        index = 0
        for dev_name, dev_config in self.device_list:

            exclude_resources = {'SPI_MISO', 'SPI_RX_DMA'}  # We are not receiving anything from DAC
            dev_requires = dev_config["requires"]
            dev_id = dev_config["dev_id"]


            timer = self.get_timer(dev_requires["TIMER"])
            time_irq_handler = self.mcu_hw.TIMER_to_IRQHandler(timer)
            timer_irqn = self.mcu_hw.ISRHandler_to_IRQn(time_irq_handler)
            timer_irq_handler_list.append(
                "MAKE_ISR_WITH_INDEX({0}, SPIDAC_COMMON_TIMER_IRQ_HANDLER, {1}) \\".format(time_irq_handler, index))
            dev_requires["TIMER_IRQ"] = {"irq_handlers": time_irq_handler}

            spi_confg_dev = self.get_spi(dev_requires["SPI"])
            spi, remap = self.mcu_hw.is_remaped(spi_confg_dev)
            dev_requires["SPI"] = spi

            baud_rate_control, spi_freq = self.mcu_hw.spi_get_baud_rate_control(spi_confg_dev,
                                                                                dev_config["clock_speed"])
            clock_phase = self.mcu_hw.spi_get_clock_phase(dev_config["clock_phase"])
            clock_polarity = self.mcu_hw.spi_get_clock_polarity(dev_config["clock_polarity"])
            frame_format = dev_config["frame_format"].upper()
            frame_size = int(self.mcu_hw.spi_get_frame_size(dev_config["frame_size"]))
            frames_per_sample = int(dev_config["frames_per_sample"])
            bits_per_sample = int(dev_config["bits_per_sample"])
            samples_number = dev_config["samples_number"]
            channels_descr, channels_count, channels_by_index = get_channels_definition(dev_name, dev_config["channels"])
            channels_descr_varname = f"g_{dev_name.lower()}_channel_descriptors"
            sw_channels_descriptors.append(f"const SPIDACChannelDescriptor {channels_descr_varname}[] = {{ {channels_descr} }};")

            if samples_number <= 0:
                raise RuntimeError(
                    f"Number of samples for device {dev_name} may not be zero")
            buffer_size = int((samples_number + 1)*frames_per_sample*(frame_size+1))*channels_count

            # LD line configuration
            ld_port, ld_pin, ld_rise = self.read_ld_configuration(dev_config, dev_requires, dev_name)

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

            spi_mosi = self.get_required_resource(spi_confg_dev, "SPI_MOSI", "gpio")
            spi_mosi_port = self.mcu_hw.GPIO_to_port(spi_mosi)
            spi_mosi_pin = self.mcu_hw.GPIO_to_pin_number(spi_mosi)

            spi_sck = self.get_required_resource(spi_confg_dev, "SPI_SCK", "gpio")
            spi_sck_port = self.mcu_hw.GPIO_to_port(spi_sck)
            spi_sck_pin = self.mcu_hw.GPIO_to_pin_number(spi_sck)

            spi_nss = self.get_required_resource(spi_confg_dev, "SPI_NSS", "gpio")
            spi_nss_port = self.mcu_hw.GPIO_to_port(spi_nss)
            spi_nss_pin = self.mcu_hw.GPIO_to_pin_number(spi_nss)

            # Make initialization sample
            init_frames_buffer = list()
            for i in range(0, channels_count):
                min_value = float(channels_by_index[i]["min_value"])
                max_value = float(channels_by_index[i]["max_value"])
                def_value = float(channels_by_index[i]["def_value"])
                init_frames_buffer.extend(get_frame_for_channel(
                                            min_value,
                                            max_value,
                                            def_value,
                                            bits_per_sample,
                                            frame_format))
            init_frames_default_value_initialization = ", ".join(init_frames_buffer)
            init_frames_default_value_name = "g_spidac_{0}_default_value".format(str(index))
            fw_device_default_value_buffers.append(
                f"volatile uint8_t {init_frames_default_value_name}[] = {{ {init_frames_default_value_initialization} }};\\")

            self.check_requirements(spi_confg_dev, dev_requires, "dev_{0}".format(dev_name), exclude_resources)

            fw_buffer_name = "g_spidac_{0}_buffer".format(str(index))
            fw_device_buffers.append(
                f"volatile uint8_t {fw_buffer_name}[{buffer_size}+sizeof(SPIDACStatus)] __attribute__ ((aligned));\\")
            fw_device_descriptors.append(f"""{{\
    {{0}},                      /* Device context */\\
    {{ {{0}}, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }},  /* Private data structure */\\
    {fw_buffer_name},           /* Buffer (for status and samples) */\\
    {init_frames_default_value_name}, /* Default values to be put after reset */\\
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
    {buffer_size} + sizeof(SPIDACStatus), /* Buffer size (status and samples) */\\
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
    {dev_id}                    /* Device id */}}""")

            sw_device_desсriptors.append(f"""{{\
    "{dev_name}",               /* Name of the device */\\
    {buffer_size},              /* Device buffer length */\\
    {frames_per_sample},        /* Frames per sample */\\
    {frame_size + 1},           /* Frame size, in bytes */\\
    {dev_id},                   /* Device id */\\
    {frame_format},     /* Frame format */\\
    {channels_count},           /* Number of channels */\\
    {samples_number},           /* Maximum number of samples per channel */\\
    {bits_per_sample},          /* Number of bits per sample */\\
    {self.mcu_hw.get_TIMER_freq(timer)},   /* Timer frequency */ \\
    {channels_descr_varname}    /* Channels description */}}""")

            sw_config_name = "spidac_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const SPIDACConfig* {sw_config_name};")
            sw_configs.append(
                f"const SPIDACConfig* {sw_config_name} = {sw_config_array_name} + {index};")

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
                      "__SPIDAC_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
                      }

        self.patch_templates()
