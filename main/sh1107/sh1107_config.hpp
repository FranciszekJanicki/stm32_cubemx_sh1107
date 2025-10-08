#ifndef SH1107_CONFIG_H
#define SH1107_CONFIG_H

#include "font5x7.hpp"
#include "sh1107_commands.hpp"
#include <array>
#include <cstdint>
#include <vector>

#define FONT font5x7
#define FONT_WIDTH FONT5X7_WIDTH
#define FONT_HEIGHT FONT5X7_HEIGHT

namespace sh1107 {

    enum struct DevAddress : std::uint8_t {};

    enum struct ControlPad : std::uint8_t {
        DISPLAY_DATA = 0b01,
        COMMAND_DATA = 0b00,
    };

    struct Config {
        std::uint32_t control_pin = {};
        std::uint32_t reset_pin = {};
    };

    struct Font {};

    struct Interface {
        void* gpio_user;
        void (*gpio_init)(void* const);
        void (*gpio_deinit)(void* const);
        void (*gpio_write)(void* const, std::uint32_t const pin, bool const state);

        void* bus_user;
        void (*bus_init)(void* const);
        void (*bus_deinit)(void* const);
        void (*bus_transmit)(void* const, std::uint8_t const* const, std::size_t const);

        void (*delay)(std::uint32_t const ms);
    };

    constexpr auto SCREEN_WIDTH = 128U;
    constexpr auto BYTE_HEIGHT = 5U;
    constexpr auto BYTE_WIDTH = 7U;
    constexpr auto SCREEN_HEIGHT = 128U;
    constexpr auto FRAME_BUF_SIZE = SCREEN_WIDTH * (SCREEN_HEIGHT / 8);

}; // namespace sh1107

#endif // SH1107_CONFIG_H