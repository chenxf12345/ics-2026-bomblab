#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t lo  = (uint32_t)challenge;
    uint32_t hi  = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    // p1_step2：先乘加，再异或。
    uint32_t t2 = (3u * hi + lo) ^ 0x92b9u;

    // p1_step0：第二个参数先异或，再加上第一个参数的 8 倍。
    uint32_t t0 = (mid ^ 0x7101u) + 8u * lo;

    // p1_step1：第二个参数加常量，再与第一个参数异或。
    values[0] = (t2 + 0x2554u) ^ t0;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    // 对应循环前对第一个整数的检查。
    values[0] = (uint32_t)((challenge >> 15) & UINT64_C(63))
                + UINT32_C(19);

    // 对应 ecx = 22。
    unsigned shift = 22;

    // 对应 rdx = 1，循环到 rdx == 6。
    for (size_t i = 1; i < count; ++i) {
        uint32_t part =
            (uint32_t)((challenge >> shift) & UINT64_C(63));

        uint32_t offset = part + (uint32_t)i;
        uint32_t previous_times_five = UINT32_C(5) * values[i - 1];

        values[i] = offset ^ previous_times_five;

        shift += 3;
    }

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t x = (uint32_t)(challenge & UINT64_C(7));

    uint32_t y = (uint32_t)(challenge >> 32)
                 ^ (uint32_t)(challenge >> 3);

    uint32_t result;

    switch (x) {
        case 0:  // p3_case_6
            result = UINT32_C(5) * y + UINT32_C(0x20873);
            break;

        case 1:  // p3_case_2
            result = UINT32_C(3) * y + UINT32_C(0x119d);
            break;

        case 2:  // p3_case_0
            result = UINT32_C(5) * y + UINT32_C(0x12ae8);
            break;

        case 3:  // p3_case_7
            result = (y ^ UINT32_C(0x9264)) + (y >> 1);
            break;

        case 4:  // p3_case_3
            result = (y ^ UINT32_C(0x1770)) + (y >> 2);
            break;

        case 5:  // p3_case_4
            result = UINT32_C(5) * y + UINT32_C(0x3154);
            break;

        case 6:  // p3_case_1
            result = (UINT32_C(4) * y) ^ UINT32_C(0xd463);
            break;

        case 7:  // p3_case_5
            result = (UINT32_C(2) * y) ^ UINT32_C(0xf5b4);
            break;

        default:
            return 1;  // x 已经过 & 7，正常情况下不会到这里。
    }

    values[0] = x;
    values[1] = result;

    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t x,
                            uint32_t low,
                            uint32_t high,
                            uint32_t salt) {
    uint32_t mid = low + ((high - low) >> 1);

    if (x == mid) {
        return (mid + UINT32_C(0x3a)) ^ salt;
    }

    uint32_t saved = mid + salt;
    uint32_t result;

    if (x < mid) {
        result = phase4_walk(x, low, mid - UINT32_C(1), salt);

        return (UINT32_C(3) * result)
               ^ saved
               ^ UINT32_C(0x1f);
    }

        result = phase4_walk(x, mid + UINT32_C(1), high, salt);

        return (UINT32_C(3) * result)
            ^ saved
            ^ UINT32_C(0x14);
}


int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (uint32_t)(challenge & UINT64_C(0x7f));

    uint32_t salt = (uint32_t)(challenge >> 7)
                    ^ (uint32_t)(challenge >> 32);

    values[0] = phase4_walk(
        x,
        UINT32_C(0),
        UINT32_C(127),
        salt
    );

    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    /*
     * 按实际 next 链的顺序排列。
     * 每行分别是该节点的 value 和 tag。
     * p5_heads[0..7] 对应这里的 nodes[0..7]。
     */
    static const struct {
        uint32_t value;
        uint32_t tag;
    } nodes[] = {
        {UINT32_C(0x07ed), UINT32_C(0x0736)},
        {UINT32_C(0x63c9), UINT32_C(0x0b61)},
        {UINT32_C(0x8597), UINT32_C(0x014c)},
        {UINT32_C(0x5888), UINT32_C(0x047e)},
        {UINT32_C(0xa30f), UINT32_C(0x0666)},
        {UINT32_C(0xa992), UINT32_C(0x0077)},
        {UINT32_C(0xee97), UINT32_C(0x051a)},
        {UINT32_C(0xb463), UINT32_C(0x0a11)},
        {UINT32_C(0xfde8), UINT32_C(0x0273)},
        {UINT32_C(0x49a3), UINT32_C(0x0367)},
        {UINT32_C(0xe590), UINT32_C(0x0989)},
        {UINT32_C(0x9b20), UINT32_C(0x087b)}
    };

    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t index =
        (uint32_t)(challenge & UINT64_C(7));

    uint32_t steps =
        (uint32_t)((challenge >> 3) & UINT64_C(3));

    uint32_t result =
        (uint32_t)(challenge >> 6)
        ^ (uint32_t)(challenge >> 32)
        ^ UINT32_C(0x3752);

    uint32_t last_tag = 0;

    // 包含起始节点，因此处理 steps + 1 次。
    for (uint32_t i = 0; i <= steps; ++i) {
        uint32_t value = nodes[index + i].value;
        uint32_t tag = nodes[index + i].tag;

        result += value;
        result *= UINT32_C(5);
        result ^= tag;

        last_tag = tag;
    }

    values[0] = result;
    values[1] = last_tag;

    return write_answer(out, cap, values, count);
}
static uint32_t p6_op_0(uint32_t x) {
    return (x + UINT32_C(0xa7ef)) ^ (x >> 7);
}

static uint32_t p6_op_1(uint32_t x) {
    return ((x >> 3) ^ x) + UINT32_C(0xfc29);
}

static uint32_t p6_op_2(uint32_t x) {
    return ((x << 7) + UINT32_C(0xceea))
           ^ (UINT32_C(5) * x);
}

static uint32_t p6_op_3(uint32_t x) {
    uint32_t rotated = (x << 1) | (x >> 31);
    return rotated ^ UINT32_C(0xeb2b);
}

static uint32_t p6_op_4(uint32_t x) {
    return (x >> 7) | (x << 25);
}

static uint32_t p6_op_5(uint32_t x) {
    return (x ^ UINT32_C(0x6b5c)) + (x << 5);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    typedef uint32_t (*Operation)(uint32_t);

    static Operation const ops[8] = {
        p6_op_2,
        p6_op_3,
        p6_op_0,
        p6_op_4,
        p6_op_0,
        p6_op_5,
        p6_op_3,
        p6_op_1
    };

    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t argument =
        (uint32_t)(challenge >> 6)
        ^ (uint32_t)(challenge >> 32);

    uint32_t index1 =
        (uint32_t)(challenge & UINT64_C(7));

    uint32_t index2 =
        (uint32_t)((challenge >> 3) & UINT64_C(7));

    uint32_t result = ops[index1](argument);

    result ^= UINT32_C(0x1215);
    result = ops[index2](result);

    values[0] = result + UINT32_C(0x776d);

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
