#include <rtdevice.h>
#include "usart.h"
#include <encoding.h>
#include <platform.h>
/**
 * @brief set uartdbg buard 
 *
 * @param buard
 */
static void usart_init(int buard)
{
	GPIO_REG(GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
	GPIO_REG(GPIO_IOF_EN) |= IOF0_UART0_MASK;
	UART0_REG(UART_REG_DIV) = get_cpu_freq() / buard - 1;
	UART0_REG(UART_REG_TXCTRL) |= UART_TXEN;
}
static void usart_handler(int vector, void *param)
{
	rt_hw_serial_isr((struct rt_serial_device*)param, RT_SERIAL_EVENT_RX_IND);
	return;
}
static rt_err_t usart_configure(struct rt_serial_device *serial,
                                struct serial_configure *cfg)
{
	GPIO_REG(GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
	GPIO_REG(GPIO_IOF_EN) |= IOF0_UART0_MASK;
	UART0_REG(UART_REG_DIV) = get_cpu_freq() / 115200 - 1;
	UART0_REG(UART_REG_TXCTRL) |= UART_TXEN;
	UART0_REG(UART_REG_RXCTRL) |= UART_RXEN;
	UART0_REG(UART_REG_IE) = UART_IP_RXWM;
		return RT_EOK;
}
static rt_err_t usart_control(struct rt_serial_device *serial,
                              int cmd, void *arg)
{
	RT_ASSERT(serial != RT_NULL);
	switch(cmd){
	case RT_DEVICE_CTRL_CLR_INT:
		break;
	case RT_DEVICE_CTRL_SET_INT:
		break;
	}
	return RT_EOK;
}
static int usart_putc(struct rt_serial_device *serial, char c)
{
	while (UART0_REG(UART_REG_TXFIFO) & 0x80000000) ;
	UART0_REG(UART_REG_TXFIFO) = c;
	return 0;
}
static int usart_getc(struct rt_serial_device *serial)
{
	rt_int32_t val = UART0_REG(UART_REG_RXFIFO);
	if (val > 0)
		return (rt_uint8_t)val;
	else
		return -1;
}
static struct rt_uart_ops ops = {
	usart_configure,
	usart_control,
	usart_putc,
	usart_getc,
};
static struct rt_serial_device serial = {
	.ops = &ops,
	.config.baud_rate = BAUD_RATE_115200,
	.config.bit_order = BIT_ORDER_LSB,
	.config.data_bits = DATA_BITS_8,
	.config.parity    = PARITY_NONE,
	.config.stop_bits = STOP_BITS_1,
	.config.invert    = NRZ_NORMAL,
	.config.bufsz     = RT_SERIAL_RB_BUFSZ,
};
void rt_hw_uart_init(void)
{
	rt_hw_serial_register(
			&serial, 
			"dusart", 
			RT_DEVICE_FLAG_STREAM
		       	| RT_DEVICE_FLAG_RDWR
			| RT_DEVICE_FLAG_INT_RX, RT_NULL);
	rt_hw_interrupt_install(
			INT_UART0_BASE,
		       	usart_handler,
		       	(void*)&(serial.parent),
		       	"uart interrupt");
	rt_hw_interrupt_unmask(INT_UART0_BASE);
	return;
}
