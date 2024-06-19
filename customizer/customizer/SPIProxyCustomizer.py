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

class SPIProxyCustomizer(DeviceCustomizer):
    def __init__(self, mcu_hw, dev_config, common_config):
        super().__init__(mcu_hw, dev_config, common_config, "SPIPROXY")
        self.hlek_lib_common_header, self.shared_header, self.fw_header, self.sw_header, self.shared_token = common_config["generation"]["shared"][self.__class__.__name__]
        self.sw_lib_header = "spiproxy_conf.hpp"
        self.sw_lib_source = "spiproxy_conf.cpp"

        self.add_template(os.path.join(self.fw_inc_templ, self.fw_header),
                          [os.path.join(self.fw_inc_dest, self.fw_header)])

        self.add_template(os.path.join(self.sw_lib_inc_templ_path, self.sw_lib_header),
                          [os.path.join(self.sw_lib_inc_dest, self.sw_lib_header)])

        self.add_template(os.path.join(self.sw_lib_src_templ_path, self.sw_lib_source),
                          [os.path.join(self.sw_lib_src_dest, self.sw_lib_source)])

        self.add_shared_code(os.path.join(self.shared_templ, self.shared_header),
                             self.shared_token)


    def customize(self):
        fw_device_descriptors = []      # these descriptors are used to configure each device on firmware side
        sw_device_desсriptors = []      # these descriptors are used to configure each device on software side
        fw_in_buffers = []  # input (transmit) buffers
        fw_out_buffers = []  # output (transmit) buffers
        fw_tx_dma_irq_list = [] # list of TX DMA irq handlers
        fw_rx_dma_irq_list = []  # list of RX DMA irq handlers
        fw_spi_irq_list = [] # list of the SPI interrupts
        fw_dma_rx_preinit_list = [] # list of dma_rx_preinit structures
        fw_dma_tx_preinit_list = []  # list of dma_tx_preinit structures
        sw_config_array_name = "spiproxy_configs"
        sw_config_declarations = []
        sw_configs = []

        index = 0
        for dev_name, dev_config in self.device_list:
            dev_id       = dev_config[KW_DEV_ID]
            dev_requires = dev_config[KW_REQUIRES]

            use_dma = dev_config["use_dma"] != 0
            bidirectional = dev_config["bidirectional"] != 0
            spi_confg_dev = self.get_spi(dev_requires["SPI"])
            spi, remap = self.mcu_hw.is_remaped(spi_confg_dev)
            dev_requires["SPI"] = spi
            dma_rx = self.mcu_hw.dma_request_map[spi + "_RX"]
            dma_tx = self.mcu_hw.dma_request_map[spi + "_TX"]
            dma_rx_it = "0"
            dma_tx_it = "0"
            buffer_size = dev_config.get(KW_BUFFER_SIZE, 0)  # Due to SPI nature, buffer size is the same for input and output
            dma_tx_preinit = "NULL"
            dma_rx_preinit = "NULL"

            if buffer_size <= 0:
                raise RuntimeError(f"Device {dev_name} output buffer must be greater than zero: out_buffer_size={buffer_size}")

            exclude_resources = set()

            spi_irq_handler = self.get_required_resource(spi_confg_dev, "SPI_IRQ", RT_IRQ_HANDLER)
            spi_irqn = self.mcu_hw.ISRHandler_to_IRQn(spi_irq_handler)

            spi_miso_port = "NULL"
            spi_miso_pin = 0
            rx_dma_channel = "NULL"
            rx_dma_irq_handler = "NULL"
            rx_dma_irqn = 0
            if bidirectional > 0:
                spi_miso = self.get_required_resource(spi_confg_dev, "SPI_MISO", RT_GPIO)
                spi_miso_port = self.mcu_hw.GPIO_to_port(spi_miso)
                spi_miso_pin = self.mcu_hw.GPIO_to_pin_number(spi_miso)
                rx_dma_channel = self.mcu_hw.get_DMA_Channel(spi + "_RX")
                rx_dma_irq_handler = self.mcu_hw.DMA_Channel_to_IRQHandler(rx_dma_channel)
                rx_dma_irqn = self.mcu_hw.ISRHandler_to_IRQn(rx_dma_irq_handler)
                if use_dma:
                    fw_rx_dma_irq_list.append("MAKE_ISR_WITH_INDEX({0}, SPI_COMMON_RX_DMA_IRQ_HANDLER, {1}) \\".format(rx_dma_irq_handler, index))
                    dev_requires["RX_DMA_IRQ"] = {"irq_handlers": rx_dma_irq_handler}

                    dma_rx_preinit = "g_spi_dma_rx_preinit_" + str(index)
                    fw_dma_rx_preinit_list.append(self.tab + f"DMA_InitTypeDef {dma_rx_preinit}; \\")
                    dma_rx_preinit = "&" + dma_rx_preinit
                    dma_rx_it = self.mcu_hw.get_DMA_IT_Flag(rx_dma_channel, "TC")
            else:
                exclude_resources.add('SPI_MISO')  # We are not receiving, MISO is not required
                exclude_resources.add('SPI_RX_DMA') # We are not receiving, RX DMA is not required

            tx_dma_channel = self.mcu_hw.get_DMA_Channel(spi + "_TX")
            tx_dma_irq_handler = self.mcu_hw.DMA_Channel_to_IRQHandler(tx_dma_channel)
            tx_dma_irqn = self.mcu_hw.ISRHandler_to_IRQn(tx_dma_irq_handler)

            if use_dma:
                fw_tx_dma_irq_list.append("MAKE_ISR_WITH_INDEX({0}, SPI_COMMON_TX_DMA_IRQ_HANDLER, {1}) \\".format(tx_dma_irq_handler, index))
                dev_requires["TX_DMA_IRQ"] = {"irq_handlers": tx_dma_irq_handler}
                exclude_resources.add('SPI_IRQ')  # We are not using interrupt mode

                dma_tx_preinit = "g_spi_dma_tx_preinit_" + str(index)
                fw_dma_tx_preinit_list.append(self.tab + f"DMA_InitTypeDef {dma_tx_preinit}; \\")
                dma_tx_preinit = "&" + dma_tx_preinit
                dma_tx_it = self.mcu_hw.get_DMA_IT_Flag(tx_dma_channel, "TC")
            else:
                fw_spi_irq_list.append("MAKE_ISR_WITH_INDEX({0}, SPI_COMMON_IRQ_HANDLER, {1}) \\".format(spi_irq_handler, index))
                exclude_resources.add('SPI_RX_DMA')  # We are not using DMA
                exclude_resources.add('SPI_TX_DMA')  # We are not using DMA

            spi_mosi = self.get_required_resource(spi_confg_dev, "SPI_MOSI", RT_GPIO)
            spi_mosi_port = self.mcu_hw.GPIO_to_port(spi_mosi)
            spi_mosi_pin = self.mcu_hw.GPIO_to_pin_number(spi_mosi)

            spi_sck = self.get_required_resource(spi_confg_dev, "SPI_SCK", RT_GPIO)
            spi_sck_port = self.mcu_hw.GPIO_to_port(spi_sck)
            spi_sck_pin = self.mcu_hw.GPIO_to_pin_number(spi_sck)

            spi_nss = self.get_required_resource(spi_confg_dev, "SPI_NSS", RT_GPIO)
            spi_nss_port = self.mcu_hw.GPIO_to_port(spi_nss)
            spi_nss_pin = self.mcu_hw.GPIO_to_pin_number(spi_nss)

            self.check_requirements(spi_confg_dev, dev_requires, "dev_{0}".format(dev_name), exclude_resources)

            baud_rate_control, spi_freq = self.mcu_hw.spi_get_baud_rate_control(spi_confg_dev, dev_config[KW_CLOCK_SPEED])
            clock_phase  = self.mcu_hw.spi_get_clock_phase(dev_config["clock_phase"])
            clock_polarity = self.mcu_hw.spi_get_clock_polarity(dev_config["clock_polarity"])
            frame_format = self.mcu_hw.spi_get_frame_format(dev_config["frame_format"])
            frame_size = self.mcu_hw.spi_get_frame_size(dev_config["frame_size"])

            # Do not forget to add IRQs, DMA and other related resources

            # Buffers handling
            in_buffer = "g_spi_in_buffer_"+str(index)
            fw_in_buffers.append(self.tab + f"volatile uint8_t {in_buffer}[sizeof(SPIProxyStatus) + {buffer_size}]; \\")

            out_buffer = "g_spi_out_buffer_" + str(index)
            fw_out_buffers.append(self.tab + f"volatile uint8_t {out_buffer}[{buffer_size}]; \\")

            fw_device_descriptors.append(f"""{{\\
    {{0}},\\
    {{ NULL, NULL, {dma_rx_preinit}, {dma_tx_preinit}, 0, 0, 0, 0, 0}},\\
    {out_buffer},\\
    {in_buffer},\\
    {spi},\\
    {tx_dma_channel},\\
    {rx_dma_channel},\\
    {spi_miso_port},\\
    {spi_mosi_port},\\
    {spi_sck_port},\\
    {spi_nss_port},\\
    {dma_rx_it},\\
    {dma_tx_it},\\
    {buffer_size},\\
    {tx_dma_irqn},\\
    {rx_dma_irqn},\\
    {spi_irqn},\\
    {baud_rate_control},\\
    {frame_size},\\
    {int(remap)},\\
    {spi_miso_pin},\\
    {spi_mosi_pin},\\
    {spi_sck_pin},\\
    {spi_nss_pin},\\
    {clock_polarity},\\
    {clock_phase},\\
    {frame_format},\\
    {int(bidirectional)},\\
    {int(use_dma)},\\
    {dev_id} }}""")

            sw_device_desсriptors.append(f'{{ {dev_id}, "{dev_name}", {buffer_size} }}')

            sw_config_name = "spiproxy_{0}_config_ptr".format(dev_name)
            sw_config_declarations.append(f"extern const SPIProxyConfig* {sw_config_name};")
            sw_configs.append(
                f"const SPIProxyConfig* {sw_config_name} = {sw_config_array_name} + {index};")

            index += 1

        self.vocabulary = self.vocabulary | {
                      "__NAMESPACE_NAME__": self.project_name,
                      "__SPIPROXY_DEVICE_COUNT__": len(fw_device_descriptors),
                      "__SPIPROXY_FW_DEV_DESCRIPTOR__": ", ".join(fw_device_descriptors),
                      "__SPIPROXY_SW_DEV_DESCRIPTOR__": ", ".join(sw_device_desсriptors),
                      "__SPIPROXY_FW_IN_BUFFERS__": concat_lines(fw_in_buffers)[:-1],
                      "__SPIPROXY_FW_OUT_BUFFERS__": concat_lines(fw_out_buffers)[:-1],
                      "__SPI_FW_TX_DMA_IRQ_HANDLERS__": concat_lines(fw_tx_dma_irq_list)[:-1],
                      "__SPI_FW_RX_DMA_IRQ_HANDLERS__": concat_lines(fw_rx_dma_irq_list)[:-1],
                      "__SPI_FW_IRQ_HANDLERS__": concat_lines(fw_spi_irq_list)[:-1],
                      "__SPI_FW_DMA_TX_PREINIT__": concat_lines(fw_dma_tx_preinit_list)[:-1],
                      "__SPI_FW_DMA_RX_PREINIT__": concat_lines(fw_dma_rx_preinit_list)[:-1],
                      "__SPIPROXY_CONFIGURATION_DECLARATIONS__": concat_lines(sw_config_declarations),
                      "__SPIPROXY_CONFIGURATIONS__": concat_lines(sw_configs),
                      "__SPIPROXY_CONFIGURATION_ARRAY_NAME__": sw_config_array_name
                      }

        self.patch_templates()
