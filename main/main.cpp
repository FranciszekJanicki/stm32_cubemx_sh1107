#include "main.h"
#include "gpio.h"
#include "sh1107/sh1107.hpp"
#include "spi.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    SPI_HandleTypeDef* sh1107_spi_bus;
    GPIO_TypeDef* sh1107_slave_select_gpio;
    uint16_t sh1107_slave_select_pin;
    GPIO_TypeDef* sh1107_reset_gpio;
    uint16_t sh1107_reset_pin;
    GPIO_TypeDef* sh1107_control_gpio;
    uint16_t sh1107_control_pin;
} sh1107_user_t;

static void sh1107_bus_transmit_data(void* user,
                                     uint8_t const* data,
                                     size_t data_size)
{
    sh1107_user_t* sh1107_user = (sh1107_user_t*)user;

    HAL_GPIO_WritePin(sh1107_user->sh1107_slave_select_gpio,
                      sh1107_user->sh1107_slave_select_pin,
                      GPIO_PIN_RESET);
    HAL_SPI_Transmit(sh1107_user->sh1107_spi_bus, data, data_size, 100);
    HAL_GPIO_WritePin(sh1107_user->sh1107_slave_select_gpio,
                      sh1107_user->sh1107_slave_select_pin,
                      GPIO_PIN_SET);
}

static void sh1107_gpio_write(void* user, uint32_t pin, bool state)
{
    sh1107_user_t* sh1107_user = (sh1107_user_t*)user;

    HAL_GPIO_WritePin(sh1107_user->sh1107_control_gpio,
                      pin,
                      (GPIO_PinState)state);
}

extern "C" {
void SystemClock_Config(void);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_SPI3_Init();

    HAL_Delay(500);

    sh1107_user_t sh1107_user = {.sh1107_spi_bus = &hspi3,
                                 .sh1107_slave_select_gpio = SS_GPIO_Port,
                                 .sh1107_slave_select_pin = SS_Pin,
                                 .sh1107_reset_gpio = RST_GPIO_Port,
                                 .sh1107_reset_pin = RST_Pin,
                                 .sh1107_control_gpio = CTRL_GPIO_Port,
                                 .sh1107_control_pin = CTRL_Pin};

    sh1107::SH1107 sh1107 = {
        .config = {.control_pin = CTRL_Pin, .reset_pin = RST_Pin},
        .interface = {.gpio_user = &sh1107_user,
                      .gpio_write = sh1107_gpio_write,
                      .bus_user = &sh1107_user,
                      .bus_transmit = sh1107_bus_transmit_data,
                      .delay = HAL_Delay}};
    sh1107.initialize();
    memset(sh1107.frame_buf.data(), 0xFF, sh1107.frame_buf.size());
    sh1107.display_frame_buf();

    while (1) {
    }
}