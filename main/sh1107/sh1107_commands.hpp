#ifndef SH1107_COMMANDS_H
#define SH1107_COMMANDS_H

#include <cstdint>

#define PACKED __attribute__((__packed__))

namespace sh1107 {

    struct SET_LOWER_COLUMN_ADDRESS {
        std::uint8_t reserved : 4 = 0b0000;
        std::uint8_t address : 4;
    } PACKED;

    struct SET_HIGHER_COLUMN_ADDRESS {
        std::uint8_t reserved : 5 = 0b00010;
        std::uint8_t address : 3;
    } PACKED;

    struct SET_MEMORY_ADDRESSING_MODE {
        std::uint8_t reserved : 7 = 0b0010000;
        std::uint8_t mode : 1;
    } PACKED;

    struct SET_SEGMENT_REMAP {
        std::uint8_t reserved : 7 = 0b1010000;
        std::uint8_t remap : 1;
    } PACKED;

    struct SET_ENTIRE_DISPLAY_ON_OFF {
        std::uint8_t reserved : 7 = 0b1010010;
        std::uint8_t on_off : 1;
    };

    struct SET_NORMAL_REVERSE_DISPLAY {
        std::uint8_t reserved : 7 = 0b1010011;
        std::uint8_t display : 1;
    } PACKED;

    struct SET_DISPLAY_ON_OFF {
        std::uint8_t reserved : 7 = 0b1010111;
        std::uint8_t on_off : 1;
    } PACKED;

    struct SET_PAGE_ADDRESS {
        std::uint8_t reserved : 4 = 0b1011;
        std::uint8_t address : 4;
    } PACKED;

    struct SET_OUTPUT_SCAN_DIRECTION {
        std::uint8_t reserved : 4 = 0b1100;
        std::uint8_t direction : 1;
        std::uint8_t : 3;
    } PACKED;

    struct READ_MODIFY_WRITE {
        std::uint8_t reserved : 8 = 0b11100000;
    } PACKED;

    struct END {
        std::uint8_t reserved : 8 = 0b11101110;
    } PACKED;

    struct NOP {
        std::uint8_t reserved : 8 = 0b11100011;
    } PACKED;

    struct READ_ID {
        std::uint8_t busy : 1;
        std::uint8_t on_off : 1;
        std::uint8_t id : 6;
    } PACKED;

    struct SET_CONTRAST_CONTROL {
        std::uint8_t reserved : 8 = 0b10000001;
        std::uint8_t contrast : 8;
    } PACKED;

    struct SET_MULTIPLEX_RATIO {
        std::uint8_t reserved : 8 = 0b10101000;
        std::uint8_t : 1;
        std::uint8_t ratio : 7;
    } PACKED;

    struct SET_DISPLAY_OFFSET {
        std::uint8_t reserved : 8 = 0b11010011;
        std::uint8_t : 1;
        std::uint8_t offset : 7;
    } PACKED;

    struct SET_DC_DC_SETTING {
        std::uint16_t reserved : 12 = 0b101011011000;
        std::uint8_t setting : 4;
    } PACKED;

    struct SET_DISPLAY_CLOCK {
        std::uint8_t reserved : 8 = 0b11010101;
        std::uint8_t osc_freq : 4;
        std::uint8_t clock_divide : 4;
    } PACKED;

    struct SET_CHARGE_PERIOD {
        std::uint8_t reserved : 8 = 0b11011001;
        std::uint8_t discharge_period : 4;
        std::uint8_t precharge_period : 4;
    } PACKED;

    struct SET_VCOM_DESELECT_LEVEL {
        std::uint8_t reserved : 8 = 0b11011011;
        std::uint8_t level : 8;
    } PACKED;

    struct SET_DISPLAY_START_LINE {
        std::uint8_t reserved : 8 = 0b11011100;
        std::uint8_t : 1;
        std::uint8_t line : 7;
    } PACKED;

}; // namespace sh1107

#undef PACKED

#endif // SH1107_COMMANDS_H