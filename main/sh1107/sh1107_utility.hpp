#ifndef SH1107_UTILITY_HPP
#define SH1107_UTILITY_HPP

namespace sh1107 {

    template <std::uint8_t WIDTH, std::uint8_t HEIGHT>
    inline bool get_pixel(std::array<std::uint8_t, (WIDTH * HEIGHT) / 8> const& bitmap,
                          std::uint8_t const x,
                          std::uint8_t const y) noexcept
    {
        auto const byte = (y * WIDTH + x) / 8U;
        auto const bit = 7U - (x % 8U);

        return (bitmap[byte] & (1U << bit)) != 0;
    }

    template <std::uint8_t WIDTH, std::uint8_t HEIGHT>
    inline void set_pixel(std::array<std::uint8_t, (WIDTH * HEIGHT) / 8>& bitmap,
                          std::uint8_t const x,
                          std::uint8_t const y,
                          bool const pixel) noexcept
    {
        auto const byte = (y * WIDTH + x) / 8U;
        auto const bit = 7U - (x % 8U);

        if (pixel) {
            bitmap[byte] |= (1U << bit);
        } else {
            bitmap[byte] &= ~(1U << bit);
        }
    }

    template <std::uint8_t OLD_WIDTH,
              std::uint8_t OLD_HEIGHT,
              std::uint8_t NEW_WIDTH,
              std::uint8_t NEW_HEIGHT>
    inline std::array<std::uint8_t, (NEW_WIDTH * NEW_HEIGHT) / 8> resized_bitmap(
        std::array<std::uint8_t, (OLD_WIDTH * OLD_HEIGHT) / 8> const& bitmap) noexcept
    {
        std::array<std::uint8_t, (NEW_WIDTH * NEW_HEIGHT) / 8> resized_bitmap = {};

        for (std::uint8_t new_y = 0U; new_y < NEW_HEIGHT; ++new_y) {
            for (std::uint8_t new_x = 0U; new_x < NEW_WIDTH; ++new_x) {
                std::uint8_t old_x = new_x * OLD_WIDTH / NEW_WIDTH;
                std::uint8_t old_y = new_y * OLD_HEIGHT / NEW_HEIGHT;

                bool pixel = get_pixel<OLD_WIDTH, OLD_HEIGHT>(bitmap, old_x, old_y);
                set_pixel<NEW_WIDTH, NEW_HEIGHT>(resized_bitmap, new_x, new_y, pixel);
            }
        }

        return resized_bitmap;
    }

}; // namespace sh1107

#endif // SH1107_UTILITY_HPP