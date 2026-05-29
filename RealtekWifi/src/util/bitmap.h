#ifndef BITMAP_H
#define BITMAP_H

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITMAP_ENTRIES(n) DIV_ROUND_UP(n, BITS_PER_LONG)

static inline void set_bit(unsigned int nr, unsigned long *addr)
{
    addr[nr / BITS_PER_LONG] |= (1UL << (nr % BITS_PER_LONG));
}

static inline void clear_bit(unsigned int nr, unsigned long *addr)
{
    addr[nr / BITS_PER_LONG] &= ~(1UL << (nr % BITS_PER_LONG));
}

static inline int test_bit(unsigned int nr, const unsigned long *addr)
{
    return (addr[nr / BITS_PER_LONG] >> (nr % BITS_PER_LONG)) & 1;
}

static inline unsigned int find_first_zero_bit(const unsigned long *addr,
                                        unsigned int size)
{
    for (unsigned int i = 0; i < size; i++) {
        if (!test_bit(i, addr)) return i;
    }
    return size;
}

static inline unsigned long find_first_bit(const unsigned long *addr,
                                            unsigned int size)
{
    for (unsigned int i = 0; i < size; i++) {
        if (test_bit(i, addr)) return i;
    }
    return size;
}

#endif
