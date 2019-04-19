#include <stddef.h>
#include "hal.h"
#include "sys_config.h"
#include "mss_gpio.h"
#include "m2sxxx.h"

#include "irqcallback.h"
//#include "version.h"


struct irqcallback_entry {
	irqcallback_t callback;
	void *data;
};


struct irqcallback_entry mss_gpio_irqcallbacks[] = {
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, 
	{NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, {NULL, NULL}, 
};
#define MSS_GPIO_IRQCALLBACK_NUM	(sizeof(mss_gpio_irqcallbacks)/sizeof(struct irqcallback_entry))

#define MSS_GPIO_IrqCallback(n)		if (mss_gpio_irqcallbacks[MSS_GPIO_##n].callback)	\
						mss_gpio_irqcallbacks[MSS_GPIO_##n].callback(NULL, MSS_GPIO_##n, mss_gpio_irqcallbacks[MSS_GPIO_##n].data);


struct irqcallback_entry fabric_irqcallbacks[] = {
	{NULL, NULL}, {NULL, NULL}, 
};
#define FABRIC_IRQCALLBACK_NUM	(sizeof(fabric_irqcallbacks)/sizeof(struct irqcallback_entry))

/* MSS GPIO Interrupt IRQHandlers */
void GPIO8_IRQHandler( void )
{
	MSS_GPIO_IrqCallback(8);
	MSS_GPIO_clear_irq(MSS_GPIO_8);
}


void GPIO9_IRQHandler( void )
{
	MSS_GPIO_IrqCallback(9);
	MSS_GPIO_clear_irq(MSS_GPIO_9);
}


void GPIO10_IRQHandler( void )
{
	MSS_GPIO_IrqCallback(10);
	MSS_GPIO_clear_irq(MSS_GPIO_10);
}


void GPIO11_IRQHandler( void )
{
	MSS_GPIO_IrqCallback(11);
	MSS_GPIO_clear_irq(MSS_GPIO_11);
}


void GPIO12_IRQHandler( void )
{
	MSS_GPIO_IrqCallback(12);
	MSS_GPIO_clear_irq(MSS_GPIO_12);
}

/* Fabric Interrupt IRQHandlers */
void FabricIrq0_IRQHandler( void )
{
	if (fabric_irqcallbacks[0].callback)
		fabric_irqcallbacks[0].callback(NULL, 0, fabric_irqcallbacks[0].data);
	NVIC_ClearPendingIRQ(FabricIrq0_IRQn);
}


void FabricIrq1_IRQHandler( void )
{
	if (fabric_irqcallbacks[1].callback)
		fabric_irqcallbacks[1].callback(NULL, 1, fabric_irqcallbacks[1].data);
	NVIC_ClearPendingIRQ(FabricIrq1_IRQn);
}


int mss_gpio_irqcallback_install(mss_gpio_id_t gpio, irqcallback_t callback, void *data)
{
	if (gpio >= MSS_GPIO_IRQCALLBACK_NUM)
		return -1;
	mss_gpio_irqcallbacks[gpio].callback = callback;
	mss_gpio_irqcallbacks[gpio].data = data;
	return 0;
}


int mss_gpio_irqcallback_remove(mss_gpio_id_t gpio)
{
	if (gpio >= MSS_GPIO_IRQCALLBACK_NUM)
		return -1;
	mss_gpio_irqcallbacks[gpio].callback = NULL;
	mss_gpio_irqcallbacks[gpio].data = NULL;
	return 0;
}


int fabric_irqcallback_install(int id, irqcallback_t callback, void *data)
{
	IRQn_Type irqn;
	if (id >= FABRIC_IRQCALLBACK_NUM || id > 15 || id < 0)
		return -1;

	fabric_irqcallbacks[id].callback = callback;
	fabric_irqcallbacks[id].data = data;

	irqn = (IRQn_Type)(FabricIrq0_IRQn + id);
	NVIC_SetPriority(irqn, 5u);
	NVIC_ClearPendingIRQ(irqn);
	NVIC_EnableIRQ(irqn);

	return 0;
}


int fabric_irqcallback_remove(int id)
{
	IRQn_Type irqn;
	if (id >= FABRIC_IRQCALLBACK_NUM || id > 15 || id < 0)
		return -1;

	fabric_irqcallbacks[id].callback = NULL;
	fabric_irqcallbacks[id].data = NULL;

	irqn = (IRQn_Type)(FabricIrq0_IRQn + id);
	NVIC_ClearPendingIRQ(irqn);
	NVIC_DisableIRQ(irqn);

	return 0;
}
