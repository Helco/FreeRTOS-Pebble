#include "snowy_qemu.h"
#include "stm32_usart.h"
#include "stm32_power.h"
#include "semphr.h"

static const stm32_usart_config_t _usart2_config = {
    .usart                = USART2,
    .flow_control_enabled = FLOW_CONTROL_DISABLED,
    .usart_periph_bus     = STM32_POWER_APB1,
    .gpio_pin_tx_num      = 2,
    .gpio_pin_rx_num      = 3, // this is even more ignored by QEMU than tx
    .gpio_pin_rts_num     = 0,
    .gpio_pin_cts_num     = 0,
    .gpio_ptr             = GPIOA,
    .gpio_clock           = RCC_AHB1Periph_GPIOA,
    .usart_clock          = RCC_APB1Periph_USART2,
    .af                   = GPIO_AF_USART2,
};

static stm32_usart_t _usart2 = {
    &_usart2_config,
    NULL, /* no dma */
    230400
};

static StaticSemaphore_t _usart2_mutex_mem;
static SemaphoreHandle_t _usart2_mutex;

void hw_qemu_init(void)
{
    stm32_usart_init_device(&_usart2);
    _usart2_mutex = xSemaphoreCreateMutexStatic(&_usart2_mutex_mem);
}

int hw_qemu_has_data(void)
{
    if (xSemaphoreTake(_usart2_mutex, 0) == pdPASS)
    {
        int result = stm32_usart_has_data(&_usart2);
        xSemaphoreGive(_usart2_mutex);
        return result;
    }
    return 0;
}

size_t hw_qemu_read(void *buffer, size_t max_len)
{
    xSemaphoreTake(_usart2_mutex, portMAX_DELAY);
    size_t bytes_read = stm32_usart_read(&_usart2, (uint8_t*)buffer, max_len);
    xSemaphoreGive(_usart2_mutex);
    return bytes_read;
}

size_t hw_qemu_write(const void *buffer, size_t len)
{
    xSemaphoreTake(_usart2_mutex, portMAX_DELAY);
    size_t bytes_written = stm32_usart_write(&_usart2, (const uint8_t*)buffer, len);
    xSemaphoreGive(_usart2_mutex);
    return bytes_written;
}
