#ifndef INCLUDE_BITS
#define INCLUDE_BITS

#include <limits.h>
#include <stdint.h>

#define HEX(n) 0x ## n ## UL
#define BINARY(x) (((x & 0x0000000FUL)? 1: 0) \
        + ((x & 0x000000F0UL)? 2: 0) \
        + ((x & 0x00000F00UL)? 4: 0) \
        + ((x & 0x0000F000UL)? 8: 0) \
        + ((x & 0x000F0000UL)? 16: 0) \
        + ((x & 0x00F00000UL)? 32: 0) \
        + ((x & 0x0F000000UL)? 64: 0) \
        + ((x & 0xF0000000UL)? 128: 0))

#define b8(bin) ((unsigned char)BINARY(HEX(bin)))
#define b16(binhigh, binlow) (((unsigned short)b8(binhigh) << 8) + b8(binlow))
#define b32(binhigh, binmid1, binmid2, binlow) (((unsigned int)b8(binhigh) << 24) + (b8(binmid1) << 16) + (b8(binmid2) << 8) + b8(binlow))

/* Sets the bit referred to by 'flag'. */
#define F(flag) (1 << (flag))

typedef uint32_t flags_t;

/* Atomic bit-twaddles */
static inline void bit_set(flags_t *value, int bit)
{
    *value |= (1 << bit);
}

static inline void bit_clear(flags_t *value, int bit)
{
    *value &= ~(1 << bit);
}

static inline int bit_test(const flags_t *value, int bit)
{
    return ((1 << (bit & 31)) & (value[bit >> 5])) != 0;
}

static inline int bit_test_and_set(flags_t *value, int bit)
{
    int old = (*value & (1 << bit)) != 0;

    *value |= (1 << bit);

    return old;
}

static inline int bit_find_next_zero(const void *value, size_t bytes, int start_loc)
{
    const uint8_t *b = value;
    size_t i;

    size_t start_byte = start_loc / CHAR_BIT;
    size_t start_bit = start_loc % CHAR_BIT;

    for (i = start_byte; i < bytes; i++) {
        if (b[i] == 0xFF)
            continue;

        uint8_t c = b[i] >> start_bit;
        int k;
        for (k = start_bit; k < CHAR_BIT; k++, c >>= 1)
            if (!(c & 1))
                return i * CHAR_BIT + k;

        start_bit = 0;
    }

    return -1;
}

static inline int bit_find_first_zero(const void *value, size_t bytes)
{
    return bit_find_next_zero(value, bytes, 0);
}

#define flag_set(flags, f) bit_set(flags, f)
#define flag_clear(flags, f) bit_clear(flags, f)
#define flag_test(flags, f) bit_test(flags, f)
#define flag_test_and_set(flags, f) bit_test_and_set(flags, f)

#endif

