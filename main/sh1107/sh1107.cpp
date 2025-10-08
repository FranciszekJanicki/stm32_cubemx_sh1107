#include "sh1107.hpp"
#include <utility>

namespace sh1107 {

    void SH1107::initialize() noexcept
    {
        this->device_reset();

        this->bus_transmit_cmd(0xAE); // Display OFF
        this->bus_transmit_cmd(0xD5); // Set Display Clock Divide Ratio
        this->bus_transmit_cmd(0x80);
        this->bus_transmit_cmd(0xA8); // Set Multiplex Ratio
        this->bus_transmit_cmd(0x7F);
        this->bus_transmit_cmd(0xD3); // Display Offset
        this->bus_transmit_cmd(0x00);
        this->bus_transmit_cmd(0x40); // Display Start Line
        this->bus_transmit_cmd(0x8D); // Charge Pump
        this->bus_transmit_cmd(0x14);
        this->bus_transmit_cmd(0xAF); // Display ON

        this->is_initialized = true;
        this->delay(100UL);
    }

    void SH1107::deinitialize() noexcept
    {
        this->device_reset();
        this->display_off();
        this->is_initialized = false;
    }

    void SH1107::device_reset() const noexcept
    {
        this->gpio_write(this->config.reset_pin, 0U);
        this->delay(100UL);
        this->gpio_write(this->config.reset_pin, 1U);
        this->delay(100UL);
    }

    void SH1107::display_on() const noexcept
    {
        this->send_set_display_on_off_cmd(SET_DISPLAY_ON_OFF{.on_off = true});
    }

    void SH1107::display_off() const noexcept
    {
        this->send_set_display_on_off_cmd(SET_DISPLAY_ON_OFF{.on_off = false});
    }

    void SH1107::display_frame_buf() noexcept
    {
        for (std::uint8_t page = 0; page < (SCREEN_HEIGHT / 8); page++) {
            this->bus_transmit_cmd(0xB0 | page);
            this->bus_transmit_cmd(0x00);
            this->bus_transmit_cmd(0x10);

            auto bytes = std::array<std::uint8_t, SCREEN_WIDTH>{};
            std::memset(bytes.data(), 0, bytes.size());
            std::memcpy(bytes.data(),
                        frame_buf.data() + page * SCREEN_WIDTH,
                        SCREEN_WIDTH);
            this->bus_transmit_data(bytes);
        }
    }

    void SH1107::clear_frame_buf() noexcept
    {
        std::memset(this->frame_buf.data(), 0, this->frame_buf.size());
    }

    void SH1107::set_pixel(std::uint8_t x, std::uint8_t y, bool color) noexcept
    {
        if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
            return;
        }

        std::size_t byte_index = (y / 8) * SCREEN_WIDTH + x;
        std::uint8_t bit_mask = 1 << (y % 8);

        if (color) {
            frame_buf[byte_index] |= bit_mask;
        } else {
            frame_buf[byte_index] &= ~bit_mask;
        }
    }

    void SH1107::draw_line(std::uint8_t x0,
                           std::uint8_t y0,
                           std::uint8_t x1,
                           std::uint8_t y1,
                           bool color) noexcept
    {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (1) {
            set_pixel(x0, y0, color);
            if (x0 == x1 && y0 == y1)
                break;
            e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void SH1107::draw_rect(std::uint8_t x,
                           std::uint8_t y,
                           std::uint8_t w,
                           std::uint8_t h,
                           bool color) noexcept
    {
        if (h <= 0 || w <= 0) {
            return;
        }

        int x_start = x;
        int x_end = x + w;
        int y_start = y;
        int y_end = y + h;

        for (int x = x_start; x < x_end; x++) {
            for (int y = y_start; y < y_end; y++) {
                if (!color && (x > x_start && x < x_end - 1) &&
                    (y > y_start && y < y_end - 1)) {
                    continue;
                }
                set_pixel(x, y);
            }
        }
    }

    void SH1107::draw_circle(std::uint8_t x0,
                             std::uint8_t y0,
                             std::uint8_t r,
                             bool color) noexcept
    {
        int x = r, y = 0, err = 1 - x;
        while (x >= y) {
            set_pixel(x0 + x, y0 + y, color);
            set_pixel(x0 + y, y0 + x, color);
            set_pixel(x0 - y, y0 + x, color);
            set_pixel(x0 - x, y0 + y, color);
            set_pixel(x0 - x, y0 - y, color);
            set_pixel(x0 - y, y0 - x, color);
            set_pixel(x0 + y, y0 - x, color);
            set_pixel(x0 + x, y0 - y, color);
            y++;
            if (err < 0) {
                err += 2 * y + 1;
            } else {
                x--;
                err += 2 * (y - x) + 1;
            }
        }
    }

    void SH1107::draw_bitmap(std::uint8_t x,
                             std::uint8_t y,
                             std::uint8_t w,
                             std::uint8_t h,
                             std::uint8_t const* bitmap,
                             std::size_t size,
                             bool color) noexcept
    {
        for (std::uint8_t j = 0; j < h; j++) {
            for (std::uint8_t i = 0; i < w; i++) {
                std::size_t index = j * ((w + 7) / 8) + (i / 8);
                if (index > size) {
                    break;
                }

                std::uint8_t byte = bitmap[index];
                if (byte & (1 << (7 - (i % 8)))) {
                    set_pixel(x + i, y + j, color);
                }
            }
        }
    }

    void SH1107::draw_char(std::uint8_t x, std::uint8_t y, char c) noexcept
    {
        if (c < 32 || c > 127)
            return;

        for (std::uint8_t i = 0; i < FONT_WIDTH; i++) {
            std::uint8_t line = FONT[c - 32][i];
            for (std::uint8_t j = 0; j < FONT_HEIGHT; j++) {
                set_pixel(x + i, y + j, line & (1 << j));
            }
        }
    }

    void SH1107::draw_string(std::uint8_t x,
                             std::uint8_t y,
                             char const* s) noexcept
    {
        while (*s != '\0') {
            draw_char(x, y, *s++);
            x += FONT_WIDTH + 1;
            if (x >= SCREEN_WIDTH)
                break;
        }
    }

    void SH1107::draw_string(std::uint8_t x,
                             std::uint8_t y,
                             char const* s,
                             size_t len) noexcept
    {
        char const* c = s;
        while ((c != s + len) && (*c != '\0')) {
            draw_char(x, y, *c++);
            x += FONT_WIDTH + 1;
            if (x >= SCREEN_WIDTH)
                break;
        }
    }

    void SH1107::select_control_pad(ControlPad const control_pad) const noexcept
    {
        this->gpio_write(this->config.control_pin,
                         std::to_underlying(control_pad));
    }

    void SH1107::send_set_lower_column_address_cmd(
        SET_LOWER_COLUMN_ADDRESS const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_higher_column_address_cmd(
        SET_HIGHER_COLUMN_ADDRESS const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_memory_addressing_mode_cmd(
        SET_MEMORY_ADDRESSING_MODE const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_segment_remap_cmd(
        SET_SEGMENT_REMAP const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_entire_display_on_off_cmd(
        SET_ENTIRE_DISPLAY_ON_OFF const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_normal_reverse_display_cmd(
        SET_NORMAL_REVERSE_DISPLAY const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_display_on_off_cmd(
        SET_DISPLAY_ON_OFF const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_page_address_cmd(
        SET_PAGE_ADDRESS const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_output_scan_direction_cmd(
        SET_OUTPUT_SCAN_DIRECTION const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_read_modify_write_cmd() const noexcept
    {
        auto cmd = READ_MODIFY_WRITE{};
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_end_cmd() const noexcept
    {
        auto cmd = END{};
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_nop_cmd() const noexcept
    {
        auto cmd = NOP{};
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_read_id_cmd() const noexcept
    {
        auto cmd = READ_ID{};
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_contrast_control_cmd(
        SET_CONTRAST_CONTROL const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_multiplex_ratio_cmd(
        SET_MULTIPLEX_RATIO const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_display_offset_cmd(
        SET_DISPLAY_OFFSET const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_dc_dc_setting_cmd(
        SET_DC_DC_SETTING const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_display_clock_cmd(
        SET_DISPLAY_CLOCK const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_charge_period_cmd(
        SET_CHARGE_PERIOD const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_vcom_deselect_level_cmd(
        SET_VCOM_DESELECT_LEVEL const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::send_set_display_start_line_cmd(
        SET_DISPLAY_START_LINE const cmd) const noexcept
    {
        this->bus_transmit_cmd(
            std::bit_cast<std::array<std::uint8_t, sizeof(cmd)>>(cmd));
    }

    void SH1107::gpio_init() const noexcept
    {
        if (this->interface.gpio_init) {
            this->interface.gpio_init(this->interface.gpio_user);
        }
    }

    void SH1107::gpio_deinit() const noexcept
    {
        if (this->interface.gpio_deinit) {
            this->interface.gpio_deinit(this->interface.gpio_user);
        }
    }

    void SH1107::gpio_write(std::uint32_t const pin,
                            bool const state) const noexcept
    {
        if (this->interface.gpio_write) {
            this->interface.gpio_write(this->interface.gpio_user, pin, state);
        }
    }

    void SH1107::bus_init() const noexcept
    {
        if (this->interface.bus_init) {
            this->interface.bus_init(this->interface.bus_user);
        }
    }

    void SH1107::bus_deinit() const noexcept
    {
        if (this->interface.bus_deinit) {
            this->interface.bus_deinit(this->interface.bus_user);
        }
    }

    void SH1107::bus_transmit_cmd(std::uint8_t const cmd) const noexcept
    {
        this->bus_transmit_cmd(std::array<std::uint8_t, 1>{cmd});
    }

    void SH1107::delay(std::uint32_t const ms) const noexcept
    {
        if (this->interface.delay) {
            this->interface.delay(ms);
        }
    }

}; // namespace sh1107