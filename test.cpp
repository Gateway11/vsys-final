#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Y_TUPLE_SIZE_II(__args) Y_TUPLE_SIZE_I __args
#define Y_TUPLE_SIZE_PREFIX__Y_TUPLE_SIZE_POSTFIX ,,,,,,,,0

#define Y_TUPLE_SIZE_I(A0,A1,A2,A3,A4,A5,A6,A7,N,...) N

#define MPL_ARGS_SIZE(...) Y_TUPLE_SIZE_II((Y_TUPLE_SIZE_PREFIX_ ## __VA_ARGS__ ## _Y_TUPLE_SIZE_POSTFIX,8,7,6,5,4,3,2,1,0))

#define VA_ARG_EXPAND(__args) __args    //Compatible with MSVC
#define __VA_ARG_NUM(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,N,...) N
#define VA_ARG_NUM(...) VA_ARG_EXPAND(__VA_ARG_NUM(0,##__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
//---------------------------------------

#define slot_map8(dst, dst_type, idxary, num_idxary, src, src_type, src_channels, sample_size, clamp) \
({                                                                                                    \
    uint32_t __num_samples = (sample_size) / (src_channels) / sizeof(src_type);                       \
                                                                                                      \
    dst_type *__udst = (dst_type *)(dst);                                                             \
    src_type *__usrc = (src_type *)(src);                                                             \
                                                                                                      \
    for (int32_t __i = __num_samples - 1; __i >= 0; __i--) {                                          \
        for (uint32_t __j = 0; __j < (num_idxary); __j++) {                                           \
            __udst[(idxary)[__j]] = clamp(__usrc[__j & (src_channels - 1)]);                          \
        }                                                                                             \
        __udst += 8;                                                                                  \
        __usrc += (src_channels);                                                                     \
    }                                                                                                 \
})

#define float_from_i16(a) (^(__typeof__(a) x){ return x / (float)(1UL << 15);}(a))

//---------------------------------------

#define __VA_ARG0(A0,...) A0
//#define VA_ARG0(...) VA_ARG_EXPAND(__VA_ARG0(__VA_ARGS__,DFLT)+0)  //想想这里为啥要写个+0？？
#define VA_ARG0(A0,...) A0
//#define __VA_ARG1(A0,A1,...) A1
//#define VA_ARG1(...) VA_ARG_EXPAND(__VA_ARG1(__VA_ARGS__,DFLT,DFLT))
#define VA_ARG1(A0,A1,...) A1

#define __PP_CAT(x,y) x##y //函数可变参数重载
#define PP_CAT(x,y) __PP_CAT(x,y)
#define VA_ARGS_NAME(prefix, ...) VA_ARG_EXPAND(PP_CAT(prefix, VA_ARG_NUM(__VA_ARGS__)))
#define VA_ARGS_FUNC(prefix, ...) VA_ARG_EXPAND(PP_CAT(prefix, VA_ARG_NUM(__VA_ARGS__))(__VA_ARGS__))

#define is_power_of_2(n) ((n) != 0 && ((n) & ((n) - 1)) == 0)

struct array_on_heap{
	int len;
	int data[];
};

static const unsigned char a[] = {
#include "values.txt"
};

#define PP_DIV(...) VA_ARGS_FUNC(div,__VA_ARGS__)

int32_t array[] = {1, 3, 2, 100};
#if 0
#define VA_ARGS_OFPACT(prefix, return_type, ...)                        \
    return_type VA_ARGS_FUNC(prefix, __VA_ARGS__) {                     \
        for (uint32_t __i = 0; __i < VA_ARG_NUM(__VA_ARGS__); __i++) {  \
            printf("%f\n", (float)x / array[__i]);                      \
        }                                                               \
    }
#else
#include <stdarg.h>
void div2(int32_t arg, ...) {
    va_list args;
    va_start(args, arg);
    int sum = arg;
    while (arg != 0) {
        sum += va_arg(args, int);
    }
    va_end(args);
    printf("%d\n", sum);
}

#endif

//VA_ARGS_OFPACT(div, void, int32_t)
//VA_ARGS_OFPACT(div, void, int32_t x, int32_t y)
//VA_ARGS_OFPACT(div, void, int32_t x, int32_t y, int32_t z)

#define __STR(x) #x
#define _STR(x) __STR(x)

#define in ,
#define foreach(...) foreach_in(__VA_ARGS__)
#define foreach_in(e, a) for(int i = 0; i < sizeof(a); i++)

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
/* --- end macros --- */


int x = 0;
void func(const int* values) {
    while(*values) {
        x += *values++;
        /* do whatever with x */
    }
    printf("%s, %lu, %d\n", __func__, (uint8_t)sizeof(values) / sizeof(values[0]), x);
}

int main() {
#define COMPILE_ASSERT(cond) typedef char __compile_time_assert[ (cond > 0) ? 0 : -1]
    COMPILE_ASSERT(100);

    uint8_t a = 6, b = 2;
    //a ^= b ^= a ^= b;
    printf("       a=%d, b=%d "PRINTF_BINARY_PATTERN_INT8" | "PRINTF_BINARY_PATTERN_INT8"\n",
            a, b, PRINTF_BYTE_TO_BINARY_INT8(a), PRINTF_BYTE_TO_BINARY_INT8(b));
    a ^= b;
    printf("a ^= b a=%d, b=%d "PRINTF_BINARY_PATTERN_INT8" | "PRINTF_BINARY_PATTERN_INT8"\n",
            a, b, PRINTF_BYTE_TO_BINARY_INT8(a), PRINTF_BYTE_TO_BINARY_INT8(b));
    b ^= a;
    printf("b ^= a a=%d, b=%d "PRINTF_BINARY_PATTERN_INT8" | "PRINTF_BINARY_PATTERN_INT8"\n",
            a, b, PRINTF_BYTE_TO_BINARY_INT8(a), PRINTF_BYTE_TO_BINARY_INT8(b));
    a ^= b;
    printf("a ^= b a=%d, b=%d "PRINTF_BINARY_PATTERN_INT8" | "PRINTF_BINARY_PATTERN_INT8"\n",
            a, b, PRINTF_BYTE_TO_BINARY_INT8(a), PRINTF_BYTE_TO_BINARY_INT8(b));
#if 0
           a=6, b=2 00000110 | 00000010
    a ^= b a=4, b=2 00000100 | 00000010
    b ^= a a=4, b=6 00000100 | 00000110
    a ^= b a=2, b=6 00000010 | 00000110

    00000110 ^ 00000010 = 00000100
    00000010 ^ 00000100 = 00000110
    00000100 ^ 00000110 = 00000010
#endif



    http://stackoverflow.com/
    printf("%d, %d\n", __LINE__, printf("%*s%*s", a, "\r", b, "\r"));

    int x = 'ABCD';
    printf("%#x\n", x);

    struct cat {
        unsigned int legs:3;  // 3 bits for legs (0-4 fit in 3 bits)
        unsigned int lives:4; // 4 bits for lives (0-9 fit in 4 bits)
    // ...
    };
    cat kitty;
    kitty.legs = 4;
    kitty.lives = 9;
    
    printf("%lu, %d, %d\n", sizeof(cat), kitty.legs, kitty.lives);

    printf("%d, %d\n", __LINE__, ^(__typeof__(a) x, int y) { return x + y; }(a, b));




    printf("----------------------------------------------\n");

#if 0
    struct llist { int a; struct llist* next;};
    #define cons(x, y) (struct llist[]){{x, y}}
    struct llist *list = cons(1, cons(2, cons(3, cons(4, NULL))));
    struct llist *p = list;

    while(p != 0) {
        printf("%d\n", p->a);
        p = p->next;
    }
#endif

    func((int[]){1, 1, 1, 1, 0});

    printf("%s, %lu\n", VA_ARG0("sdfasdfasdi-------------", 88), sizeof((int[]){1, 2, 3, 0}));

    printf("%d\n", VA_ARG_NUM(a, b, c));
    printf("%d\n", VA_ARG_NUM());
    //VA_ARGS_FUNC(div, 3, 2);
    //PP_DIV(3, 2);

#define LEN   3
    printf("%s, %s\n", __STR(LEN), _STR(LEN));

    int32_t arr[] = {1,2,3,4}, e = 3;
    foreach(e in arr) { // 编译不通过 >_<

    }
#if 0
    for (int s = socket(AF_INET, SOCK_STREAM, 0), _m = 1; _m; _m--, s && close(s)) {

    }

    for (int i = 0; i < sizeof(a); ++i)
        printf("a[%d] = %d\n", i, a[i]);

    int n;
    int array_[n];
    printf("%lu\n", sizeof(array_));

    int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int (*parray)[4] = (int(*)[4])&array, (*parray2)[5][4] = (int (*)[5][4])&array;
    printf("%d, %d, %d\n",array[1 * 4 + 0], (*(parray + 1))[0], (*parray2)[4][3]);

    struct array_on_heap s = {.len = 100};
    printf("%d\n", s.data[-1]);

    printf("%d\n", MPL_ARGS_SIZE(a,b,c,d,e,f,g,a));

    printf("%c\n", "0123456789ABCDEF"[10]);

    printf("%f\n", float_from_i16(2));
    printf("%f\n", (^(__typeof__(2) x){ return x / (float)(1UL << 15);}(2)));

#define DIV(a, b) (^(__typeof__(a) x, int y) { return (float)a / b; }(a, b))
    printf("%f\n", DIV(2, 3));
    printf("%f\n", (^float(int x, int y) { return 2. / 3; }(2, 3)));

    uint32_t sample_rate = 48000;
    uint32_t num_channels = 2;
    uint32_t num_samples = sample_rate * num_channels * sizeof(short) / 100;

    char input[num_samples], output[num_samples << 2];
    uint8_t output_chans[] = {1, 3, 5, 7};

    //slot_map8(output, int16_t, output_chans, sizeof(output_chans), input, int16_t, num_channels, num_samples, );
    //(^(float x){ return x / (1UL << 15);}(usrc[j % (2)]))
#endif
    return 0;
}
