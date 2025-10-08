#include "main.h"
#include "font5x7.h"
#include "gpio.h"
#include "sh1107.h"
#include "spi.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
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

    if (pin == sh1107_user->sh1107_control_pin) {
        HAL_GPIO_WritePin(sh1107_user->sh1107_control_gpio,
                          sh1107_user->sh1107_control_pin,
                          (GPIO_PinState)state);
    } else if (pin == sh1107_user->sh1107_reset_pin) {
        HAL_GPIO_WritePin(sh1107_user->sh1107_reset_gpio,
                          sh1107_user->sh1107_reset_pin,
                          (GPIO_PinState)state);
    }

    return SH1107_ERR_OK;
}

static sh1107_err_t sh1107_initialize_chip(sh1107_t* sh1107)
{
    HAL_Delay(100);

    sh1107_send_set_display_on_off_cmd(sh1107, false);
    sh1107_send_set_display_clock_cmd(sh1107, 0x80, 0x00);
    sh1107_send_set_multiplex_ratio_cmd(sh1107, 0x7F);
    sh1107_send_set_display_offset_cmd(sh1107, 0x00);
    sh1107_send_set_display_start_line_cmd(sh1107, 0x40);
    sh1107_send_set_charge_period_cmd(sh1107, 0x8D, 0x00);
    sh1107_send_set_display_on_off_cmd(sh1107, true);

    HAL_Delay(100);

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

    uint8_t frame_buffer[SH1107_FRAME_BUFFER_SIZE];

    sh1107_user_t sh1107_user = {.sh1107_spi_bus = &hspi3,
                                 .sh1107_slave_select_gpio = SS_GPIO_Port,
                                 .sh1107_slave_select_pin = SS_Pin,
                                 .sh1107_control_gpio = CTRL_GPIO_Port,
                                 .sh1107_control_pin = CTRL_Pin,
                                 .sh1107_reset_gpio = RST_GPIO_Port,
                                 .sh1107_reset_pin = RST_Pin};

    sh1107_t sh1107;
    sh1107_err_t err = sh1107_initialize(
        &sh1107,
        &(sh1107_config_t){.char_width = FONT5X7_CHAR_WIDTH,
                           .font = font5x7,
                           .font_chars = FONT5X7_CHARS,
                           .font_height = FONT5X7_HEIGHT,
                           .font_width = FONT5X7_WIDTH,
                           .line_height = FONT5X7_LINE_HEIGHT,
                           .control_pin = CTRL_Pin,
                           .reset_pin = RST_Pin,
                           .frame_buffer = &frame_buffer},
        &(sh1107_interface_t){.bus_user = &sh1107_user,
                              .bus_initialize = sh1107_bus_initialize,
                              .bus_deinitialize = sh1107_bus_deinitialize,
                              .bus_transmit = sh1107_bus_transmit_data,
                              .gpio_user = &sh1107_user,
                              .gpio_initialize = sh1107_gpio_initialize,
                              .gpio_deinitialize = sh1107_gpio_deinitialize,
                              .gpio_write = sh1107_gpio_write});
    if (err != SH1107_ERR_OK) {
        printf("Error: %d\n\r", err);
    }

    err = sh1107_initialize_chip(&sh1107);
    if (err != SH1107_ERR_OK) {
        printf("Error: %d\n\r", err);
    }

    err = sh1107_draw_string(&sh1107, 0, 0, "DUPA");
    if (err != SH1107_ERR_OK) {
        printf("Error: %d\n\r", err);
    }

    err = sh1107_display_frame_buffer(&sh1107);
    if (err != SH1107_ERR_OK) {
        printf("Error: %d\n\r", err);
    }

    while (1) {
    }
}