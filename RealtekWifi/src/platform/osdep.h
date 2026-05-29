#ifndef OSDEP_H
#define OSDEP_H

#include <libkern/libkern.h>
#include <sys/types.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef uint64_t dma_addr_t;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

#define BIT(x)              (1UL << (x))
#define BIT_ULL(x)          (1ULL << (x))

#define GENMASK(h, l)       (((~0UL) << (l)) & (~0UL >> (sizeof(long)*8 - 1 - (h))))
#define GENMASK_ULL(h, l)   (((~0ULL) << (l)) & (~0ULL >> (sizeof(long long)*8 - 1 - (h))))

#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))

#define container_of(ptr, type, member) \
    ((type *)((uintptr_t)(ptr) - offsetof(type, member)))

#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#define DIV_ROUND_CLOSEST(n, d) ((((n) * 10) + ((d) / 2)) / (d))

#define min(a, b)           ((a) < (b) ? (a) : (b))
#define max(a, b)           ((a) > (b) ? (a) : (b))
#define min_t(type, a, b)   min((type)(a), (type)(b))
#define max_t(type, a, b)   max((type)(a), (type)(b))
#define clamp(val, lo, hi)  min(max(val, lo), hi)
#define abs(x)              ((x) < 0 ? -(x) : (x))

#define le16_to_cpu(x)      (x)
#define le32_to_cpu(x)      (x)
#define le64_to_cpu(x)      (x)
#define cpu_to_le16(x)      (x)
#define cpu_to_le32(x)      (x)
#define cpu_to_le64(x)      (x)
typedef u32 __le32;
typedef u16 __le16;

#define __always_inline     inline
#define __packed            __attribute__((packed))
#define fallthrough

#define ETH_ALEN            6
#define MAC_FMT             "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(m)          (m)[0],(m)[1],(m)[2],(m)[3],(m)[4],(m)[5]

#define HZ                  100
#define msecs_to_jiffies(m) ((m) * HZ / 1000)
#define round_jiffies_relative(j) (j)

// Locking
typedef IOLock *mutex_t;
#define mutex_init(m)       do { *(m) = IOLockAlloc(); } while(0)
#define mutex_destroy(m)    do { if (*(m)) IOLockFree(*(m)); *(m) = NULL; } while(0)
#define mutex_lock(m)       IOLockLock(*(m))
#define mutex_unlock(m)     IOLockUnlock(*(m))

typedef IOLock *spinlock_t;
#define spin_lock_init(l)   do { *(l) = IOLockAlloc(); } while(0)
#define spin_lock(l)        IOLockLock(*(l))
#define spin_unlock(l)      IOLockUnlock(*(l))
#define spin_lock_bh(l)     spin_lock(l)
#define spin_unlock_bh(l)   spin_unlock(l)
#define spin_lock_irqsave(l, f) do { f = 0; spin_lock(l); } while(0)
#define spin_unlock_irqrestore(l, f) spin_unlock(l)

// Memory
#define kzalloc(s, f)       ({ void *p = IOMalloc(s); if (p) bzero(p, s); p; })
#define kcalloc(n, s, f)    kzalloc((n)*(s), f)
#define kfree(p)            do { if (p) IOFree(p, 0); } while(0)
#define devm_kzalloc(d,s,f) kzalloc(s,f)
#define vmalloc(s)          kzalloc(s, 0)
#define vfree(p)            kfree(p)

// bitmap_zero is provided by util/bitmap.h
#define DECLARE_BITMAP(name, bits) unsigned long name[((bits)+31)/32]
#define bitmap_zero(b, n)          memset((b), 0, ((n)+31)/32 * sizeof(unsigned long))

struct completion { bool done; };
#define init_completion(c)      do { (c)->done = false; } while(0)
#define wait_for_completion(c)  do { int _t = 10000; while(!(c)->done && _t--) IODelay(10); } while(0)
#define complete(c)             do { (c)->done = true; } while(0)
#define reinit_completion(c)    do { (c)->done = false; } while(0)

struct firmware { const u8 *data; size_t size; };

#define IEEE80211_NUM_TIDS      9

// Exponentially Weighted Moving Average
#define DECLARE_EWMA(name, _weight, _factor)                            \
struct ewma_##name {                                                    \
    unsigned long internal;                                             \
};                                                                      \
static inline void ewma_##name##_init(struct ewma_##name *e) {          \
    e->internal = 0;                                                    \
}                                                                       \
static inline void ewma_##name##_add(struct ewma_##name *e,             \
                                      unsigned long val) {               \
    if (!e->internal)                                                   \
        e->internal = (val) << _weight;                                 \
    else                                                                \
        e->internal = ((e->internal << (_weight-1)) +                   \
                       (e->internal >> 1) + val) >> _weight;            \
}                                                                       \
static inline unsigned long ewma_##name##_read(struct ewma_##name *e) { \
    return e->internal >> _weight;                                      \
}

#define IOLog(...)
#undef IOLog
extern void IOLog(const char *format, ...);
#define pr_err(fmt, ...)    IOLog("rtw88 ERR: " fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)   IOLog("rtw88: " fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)   IOLog("rtw88: " fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)  IOLog("rtw88: " fmt, ##__VA_ARGS__)

#define dev_err(d,fmt,...)  IOLog("rtw88: " fmt, ##__VA_ARGS__)
#define dev_warn(d,fmt,...) IOLog("rtw88: " fmt, ##__VA_ARGS__)
#define dev_info(d,fmt,...) IOLog("rtw88: " fmt, ##__VA_ARGS__)

#define WARN_ON(c)          do { if (c) IOLog("WARN %s:%d\n",__FILE__,__LINE__); } while(0)
#define WARN(c,fmt,...)     do { if (c) IOLog("WARN " fmt,##__VA_ARGS__); } while(0)

#endif
