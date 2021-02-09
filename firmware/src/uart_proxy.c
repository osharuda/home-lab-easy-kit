/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief USART proxy device C source file.
 *   \author Oleh Sharuda
 */

#include "utools.h"
#include "i2c_bus.h"
#include "fw.h"
#include "uart_proxy.h"

#ifdef UART_PROXY_DEVICE_ENABLED

DEFINE_UART_PROXY_BUFFERS;
UartProxyDevInstance g_uart_proxies[] = UART_PROXY_DESCRIPTOR;

void UART_PROXY_COMMON_IRQ_HANDLER(uint16_t index) {
	assert_param(index < UART_PROXY_DEVICE_NUMBER);

	UartProxyDevInstance* dev_instance = g_uart_proxies + index;

	volatile uint8_t rx_byte=0;
	USART_TypeDef* uart_port = dev_instance->uart_port;

    if ((uart_port->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
    	rx_byte = USART_ReceiveData(uart_port);
    	circbuf_put_byte(dev_instance->dev_ctx.circ_buffer, rx_byte);
	}
}

// !!! This macro defines all ISR routines for all USART proxies.
// !!! Each of ISR will call UART_PROXY_COMMON_IRQ_HANDLER() with correct index value for particular USART port
UART_PROXY_ISR_ROUTINES


void uart_proxy_send_byte(USART_TypeDef* uart_port, uint8_t b)
{
    while(USART_GetFlagStatus(uart_port, USART_FLAG_TC) == RESET)
    {
    }

	USART_SendData(uart_port, b);
}


void uart_proxy_dev_execute(uint8_t cmd_byte, uint8_t* data, uint16_t length) {

	uint16_t index = comm_dev_context(cmd_byte)->dev_index;
	USART_TypeDef* uart_port = g_uart_proxies[index].uart_port;

	for (uint16_t i=0; i<length; i++)
	{
		uart_proxy_send_byte(uart_port, data[i]);
	}

    comm_done(0);
}

void uart_proxy_read_done(uint8_t device_id, uint16_t length) {
	uint16_t index = comm_dev_context(device_id)->dev_index;
	PCircBuffer circ_buffer = g_uart_proxies[index].dev_ctx.circ_buffer;
	circbuf_stop_read(circ_buffer, length);
	circbuf_clear_ovf(circ_buffer);
    comm_done(0);
}

void uart_proxy_init(void)
{
    START_PIN_DECLARATION;

    for (uint16_t i=0; i<UART_PROXY_DEVICE_NUMBER; i++)
    {
		gpio.GPIO_Pin = g_uart_proxies[i].tx_pin_mask;
		gpio.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init( g_uart_proxies[i].tx_port , &gpio);

		gpio.GPIO_Pin = g_uart_proxies[i].rx_pin_mask;
		gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init( g_uart_proxies[i].rx_port , &gpio);

		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate = g_uart_proxies[i].baud_rate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(g_uart_proxies[i].uart_port, &USART_InitStructure);

		circbuf_init(&g_uart_proxies[i].circ_buffer, g_uart_proxies[i].dev_buffer,g_uart_proxies[i].dev_buffer_len);
		PDeviceContext dev_ctx = &(g_uart_proxies[i].dev_ctx);
		dev_ctx->device_id = g_uart_proxies[i].dev_id;
		dev_ctx->buffer = 0;
		dev_ctx->circ_buffer = &(g_uart_proxies[i].circ_buffer);
		dev_ctx->on_command = uart_proxy_dev_execute;
		dev_ctx->on_read_done = uart_proxy_read_done;
		dev_ctx->dev_index = i;

        // Enable USART
        USART_Cmd(g_uart_proxies[i].uart_port, ENABLE);

		// Enable the USART Receive interrupt: this interrupt is generated when the USART receive data register is not empty
        NVIC_SetPriority(g_uart_proxies[i].irq_vector, IRQ_PRIORITY_USART);
        NVIC_EnableIRQ(g_uart_proxies[i].irq_vector);
		USART_ITConfig(g_uart_proxies[i].uart_port, USART_IT_RXNE, ENABLE);

        comm_register_device(dev_ctx);
    }
}

#endif