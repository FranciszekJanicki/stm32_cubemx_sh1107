#ifndef SH1107_HPP
#define SH1107_HPP

#include "sh1107_commands.hpp"
#include "sh1107_config.hpp"
#include <cstdint>
#include <cstring>
#include <string>

namespace sh1107 {

    struct SH1107 {
    public:
        void initialize() noexcept;
        void deinitialize() noexcept;

        void display_frame_buf() noexcept;
        void clear_frame_buf() noexcept;

        void set_pixel(std::uint8_t x,
                       std::uint8_t y,
                       bool color = true) noexcept;
        void draw_line(std::uint8_t x0,
                       std::uint8_t y0,
                       std::uint8_t x1,
                       std::uint8_t y1,
                       bool color = true) noexcept;
        void draw_rect(std::uint8_t x,
                       std::uint8_t y,
                       std::uint8_t w,
                       std::uint8_t h,
                       bool color = true) noexcept;
        void draw_circle(std::uint8_t x0,
                         std::uint8_t y0,
                         std::uint8_t r,
                         bool color = true) noexcept;
        void draw_bitmap(std::uint8_t x,
                         std::uint8_t y,
                         std::uint8_t w,
                         std::uint8_t h,
                         std::uint8_t const* bitmap,
                         std::size_t size,
                         bool color = true) noexcept;
        void draw_char(std::uint8_t x, std::uint8_t y, char c) noexcept;
        void draw_string(std::uint8_t x,
                         std::uint8_t y,
                         char const* s) noexcept;
        void draw_string(std::uint8_t x,
                         std::uint8_t y,
                         char const* s,
                         size_t len) noexcept;
        void draw_string_formatted(std::uint8_t x,
                                   std::uint8_t y,
                                   char const* fmt,
                                   auto const... args) noexcept;

        void device_reset() const noexcept;

        void display_on() const noexcept;
        void display_off() const noexcept;

        void select_control_pad(ControlPad const control_pad) const noexcept;

        void send_set_lower_column_address_cmd(
            SET_LOWER_COLUMN_ADDRESS const cmd) const noexcept;
        void send_set_higher_column_address_cmd(
            SET_HIGHER_COLUMN_ADDRESS const cmd) const noexcept;
        void send_set_memory_addressing_mode_cmd(
            SET_MEMORY_ADDRESSING_MODE const cmd) const noexcept;
        void send_set_segment_remap_cmd(
            SET_SEGMENT_REMAP const cmd) const noexcept;
        void send_set_entire_display_on_off_cmd(
            SET_ENTIRE_DISPLAY_ON_OFF const cmd) const noexcept;
        void send_set_normal_reverse_display_cmd(
            SET_NORMAL_REVERSE_DISPLAY const cmd) const noexcept;
        void send_set_display_on_off_cmd(
            SET_DISPLAY_ON_OFF const cmd) const noexcept;
        void send_set_page_address_cmd(
            SET_PAGE_ADDRESS const cmd) const noexcept;
        void send_set_output_scan_direction_cmd(
            SET_OUTPUT_SCAN_DIRECTION const cmd) const noexcept;

        void send_read_modify_write_cmd() const noexcept;
        void send_end_cmd() const noexcept;
        void send_nop_cmd() const noexcept;
        void send_read_id_cmd() const noexcept;

        void send_set_contrast_control_cmd(
            SET_CONTRAST_CONTROL const cmd) const noexcept;
        void send_set_multiplex_ratio_cmd(
            SET_MULTIPLEX_RATIO const cmd) const noexcept;
        void send_set_display_offset_cmd(
            SET_DISPLAY_OFFSET const cmd) const noexcept;
        void send_set_dc_dc_setting_cmd(
            SET_DC_DC_SETTING const cmd) const noexcept;
        void send_set_display_clock_cmd(
            SET_DISPLAY_CLOCK const cmd) const noexcept;
        void send_set_charge_period_cmd(
            SET_CHARGE_PERIOD const cmd) const noexcept;
        void send_set_vcom_deselect_level_cmd(
            SET_VCOM_DESELECT_LEVEL const cmd) const noexcept;
        void send_set_display_start_line_cmd(
            SET_DISPLAY_START_LINE const cmd) const noexcept;

        bool is_initialized = false;

        Config config = {};
        Interface interface = {};

        std::array<std::uint8_t, FRAME_BUF_SIZE> frame_buf = {};

    private:
        void gpio_init() const noexcept;
        void gpio_deinit() const noexcept;
        void gpio_write(std::uint32_t const pin,
                        bool const state) const noexcept;

        void bus_init() const noexcept;
        void bus_deinit() const noexcept;

        template <std::size_t SIZE>
        void bus_transmit(
            std::array<std::uint8_t, SIZE> const& data) const noexcept;

        template <std::size_t SIZE>
        void bus_transmit_cmd(
            std::array<std::uint8_t, SIZE> const& cmd) const noexcept;
        void bus_transmit_cmd(std::uint8_t const cmd) const noexcept;

        template <std::size_t SIZE>
        void bus_transmit_data(
            std::array<std::uint8_t, SIZE> const& data) const noexcept;

        void delay(std::uint32_t const ms) const noexcept;
    };

    inline void SH1107::draw_string_formatted(std::uint8_t x,
                                              std::uint8_t y,
                                              char const* fmt,
                                              auto const... args) noexcept
    {
        static char buffer[200];

        char* buf = buffer;
        size_t buf_len = sizeof(buffer);
        bool is_heap_buf = false;

        size_t string_len = std::snprintf(nullptr, 0, fmt, args...) + 1;
        if (string_len > buf_len) {
            buf = static_cast<char*>(std::malloc(string_len));
            buf_len = string_len;
            is_heap_buf = true;
        }

        if (buf) {
            std::snprintf(buf, buf_len, fmt, args...);
            this->draw_string(x, y, buf, buf_len);

            if (is_heap_buf) {
                std::free(buf);
            }
        }
    }

    template <std::size_t SIZE>
    inline void SH1107::bus_transmit(
        std::array<std::uint8_t, SIZE> const& data) const noexcept
    {
        if (this->interface.bus_transmit) {
            this->interface.bus_transmit(this->interface.bus_user,
                                         data.data(),
                                         data.size());
        }
    }

    template <std::size_t SIZE>
    inline void SH1107::bus_transmit_cmd(
        std::array<std::uint8_t, SIZE> const& cmd) const noexcept
    {
        this->select_control_pad(ControlPad::COMMAND_DATA);
        this->bus_transmit(cmd);
    }

    template <std::size_t SIZE>
    inline void SH1107::bus_transmit_data(
        std::array<std::uint8_t, SIZE> const& data) const noexcept
    {
        this->select_control_pad(ControlPad::DISPLAY_DATA);
        this->bus_transmit(data);
    }

}; // namespace sh1107

#endif // SH1107_H