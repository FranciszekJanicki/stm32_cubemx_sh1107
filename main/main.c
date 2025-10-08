#include "main.h"
#include "font5x7.h"
#include "gpio.h"
#include "sh1107.h"
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

static sh1107_err_t sh1107_bus_transmit_data(void* user,
                                             uint8_t const* data,
                                             size_t data_size)
{
    sh1107_user_t* sh1107_user = (sh1107_user_t*)user;

    HAL_GPIO_WritePin(sh1107_user->sh1107_slave_select_gpio,
                      sh1107_user->sh1107_slave_select_pin,
                      GPIO_PIN_RESET);
    HAL_StatusTypeDef err =
        HAL_SPI_Transmit(sh1107_user->sh1107_spi_bus, data, data_size, 100);
    HAL_GPIO_WritePin(sh1107_user->sh1107_slave_select_gpio,
                      sh1107_user->sh1107_slave_select_pin,
                      GPIO_PIN_SET);

    return err == HAL_OK ? SH1107_ERR_OK : SH1107_ERR_FAIL;
}

static sh1107_err_t sh1107_bus_initialize(void* user)
{
    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_bus_deinitialize(void* user)
{
    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_gpio_initialize(void* user)
{
    sh1107_user_t* sh1107_user = (sh1107_user_t*)user;

    HAL_GPIO_WritePin(sh1107_user->sh1107_slave_select_gpio,
                      sh1107_user->sh1107_slave_select_pin,
                      GPIO_PIN_RESET);

    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_gpio_deinitialize(void* user)
{
    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_gpio_write(void* user, uint32_t pin, bool state)
{
    sh1107_user_t* sh1107_user = (sh1107_user_t*)user;

    HAL_GPIO_WritePin(sh1107_user->sh1107_control_gpio,
                      pin,
                      (GPIO_PinState)state);

    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_initialize_chip(sh1107_t* sh1107)
{
    sh1107_gpio_write(sh1107->interface.bus_user, sh1107->config.reset_pin, 0);
    HAL_Delay(100);
    sh1107_gpio_write(sh1107->interface.bus_user, sh1107->config.reset_pin, 1);
    HAL_Delay(100);

    uint8_t cmd = (0xAE); // Display OFF
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0xD5); // Set Display Clock Divide Ratio
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x80);
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0xA8); // Set Multiplex Ratio
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x7F);
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0xD3); // Display Offset
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x00);
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x40); // Display Start Line
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x8D); // Charge Pump
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0x14);
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);
    cmd = (0xAF); // Display // ON
    sh1107_bus_transmit_data(sh1107->interface.bus_user, &cmd, 1);

    return SH1107_ERR_OK;
}

void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_SPI3_Init();

    HAL_Delay(500);

    static uint8_t frame_buffer[SH1107_FRAME_BUFFER_SIZE];

    sh1107_user_t sh1107_user = {.sh1107_spi_bus = &hspi3,
                                 .sh1107_slave_select_gpio = SS_GPIO_Port,
                                 .sh1107_slave_select_pin = SS_Pin,
                                 .sh1107_reset_gpio = RST_GPIO_Port,
                                 .sh1107_reset_pin = RST_Pin,
                                 .sh1107_control_gpio = CTRL_GPIO_Port,
                                 .sh1107_control_pin = CTRL_Pin};

    sh1107_t sh1107;
    sh1107_initialize(
        &sh1107,
        &(sh1107_config_t){.font_buffer = (uint8_t*)font5x7,
                           .font_chars = FONT5X7_CHARS,
                           .font_height = FONT5X7_HEIGHT,
                           .font_width = FONT5X7_WIDTH,
                           .control_pin = CTRL_Pin,
                           .reset_pin = RST_Pin,
                           .frame_buffer = frame_buffer,
                           .frame_width = SH1107_SCREEN_WIDTH,
                           .frame_height = SH1107_SCREEN_HEIGHT},
        &(sh1107_interface_t){.bus_user = &sh1107_user,
                              .bus_initialize = sh1107_bus_initialize,
                              .bus_deinitialize = sh1107_bus_deinitialize,
                              .bus_transmit = sh1107_bus_transmit_data,
                              .gpio_user = &sh1107_user,
                              .gpio_initialize = sh1107_gpio_initialize,
                              .gpio_deinitialize = sh1107_gpio_deinitialize,
                              .gpio_write = sh1107_gpio_write});
    sh1107_initialize_chip(&sh1107);
    sh1107_draw_string(&sh1107, 0, 0, "DUPA ZBITA");
    sh1107_draw_string(&sh1107, 30, 30, "DUPA CIPA");
    sh1107_display_frame_buffer(&sh1107);

    while (1) {
    }
}