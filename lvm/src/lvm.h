// TODO: add memory and instructions in an arraylist

#ifndef _LVM_H_
#define _LVM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef LVM_API
#define LVM_API
#endif

// NOTE: Stolen from https://github.com/tsoding/bm
#if defined(_MSC_VER)
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#define LVM_PLATFORM_LINUX 1
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/AbnAlsrag/LVM"
#endif

#define LVM_MAGIC 0x45564F4C
#define LVM_VERSION 0

#define LVM_WORD_SIZE 8
#define LVM_STACK_MAX 1024LL
#define LVM_MEMORY_MAX (640 * 1000)
#define LVM_NATIVE_MAX 1024LL

typedef union {
    int64_t as_i64;
    uint64_t as_u64;
    double as_f64;
    // TODO: SEE IF AS_PTR IS USEFUL
    // void* as_ptr;
} lvm_Word;

typedef uint64_t lvm_OpAddr;
typedef uint64_t lvm_MemAddr;

typedef enum {
    LVM_TRAP_OK,
    LVM_TRAP_ILLEGAL_INST,
    LVM_TRAP_ILLEGAL_INST_ACCESS,
    LVM_TRAP_ILLEGAL_OPERAND,
    LVM_TRAP_STACK_OVERFLOW,
    LVM_TRAP_STACK_UNDERFLOW,
    LVM_TRAP_DIV_BY_ZERO,
    LVM_TRAP_ILLEGAL_MEMORY_ACCESS,
    LVM_MAX_TRAPS,
} lvm_Trap;

typedef enum {
    LVM_INST_ILLEGAL = 0,
    LVM_INST_NOP,
    LVM_INST_PUSH,
    LVM_INST_POP,
    LVM_INST_DUP,
    LVM_INST_SWAP,
    LVM_INST_INCI,
    LVM_INST_INCF,
    LVM_INST_DECI,
    LVM_INST_DECF,
    LVM_INST_ADDI,
    LVM_INST_ADDF,
    LVM_INST_SUBI,
    LVM_INST_SUBF,
    LVM_INST_MULTI,
    LVM_INST_MULTF,
    LVM_INST_DIVI,
    LVM_INST_DIVU,
    LVM_INST_DIVF,
    LVM_INST_MODI,
    LVM_INST_MODU,
    LVM_INST_MODF,
    LVM_INST_EQ,
    LVM_INST_NEQ,
    LVM_INST_GTI,
    LVM_INST_GTU,
    LVM_INST_GTF,
    LVM_INST_GEI,
    LVM_INST_GEU,
    LVM_INST_GEF,
    LVM_INST_STI,
    LVM_INST_STU,
    LVM_INST_STF,
    LVM_INST_SEI,
    LVM_INST_SEU,
    LVM_INST_SEF,
    LVM_INST_AND,
    LVM_INST_NOT,
    LVM_INST_OR,
    LVM_INST_ANDB,
    LVM_INST_NOTB,
    LVM_INST_ORB,
    LVM_INST_XOR,
    LVM_INST_SHL,
    LVM_INST_SHR,
    // TODO: ADD ROTATE SHIFT INSTS
    // LVM_INST_ROTL,
    // LVM_INST_ROTR,
    LVM_INST_CALL,
    LVM_INST_NATIVE,
    LVM_INST_RETURN,
    LVM_INST_JMP,
    LVM_INST_JZ,
    LVM_INST_JNZ,
    LVM_INST_I2F,
    LVM_INST_U2F,
    LVM_INST_F2I,
    LVM_INST_F2U,
    LVM_INST_READ8,
    LVM_INST_READ16,
    LVM_INST_READ32,
    LVM_INST_READ64,
    LVM_INST_WRITE8,
    LVM_INST_WRITE16,
    LVM_INST_WRITE32,
    LVM_INST_WRITE64,
    LVM_INST_HLT,
    LVM_INST_PRINT_DEBUG,
    LVM_MAX_INSTS,
} lvm_InstType;

// TODO: FIX STATIC ASSERT
// static_assert (LVM_MAX_INSTS == 65, "THE INSTRUCTION SET CHANGED PLEASE UPDATE THE CODE");

typedef struct {
    lvm_InstType type;
    lvm_Word operand;
} lvm_Inst;

typedef struct {
    const lvm_Inst *insts;
    size_t insts_count;

    const uint8_t *memory;
    size_t memory_size;
} lvm_Program;

typedef struct lvm_Machine lvm_Machine;

typedef lvm_Trap(*lvm_Native)(lvm_Machine *machine);

struct lvm_Machine {
    lvm_Word stack[LVM_STACK_MAX];
    size_t stack_top;

    uint8_t memory[LVM_MEMORY_MAX];

    lvm_Native natives[LVM_NATIVE_MAX];
    size_t natives_top;

    lvm_Program program;
    
    lvm_OpAddr ip;
    bool hlt;
};

LVM_API const char *lvm_get_trap_name(lvm_Trap trap);
LVM_API const char *lvm_get_inst_name(lvm_InstType inst);
LVM_API lvm_Program lvm_create_program(const lvm_Inst *const insts, size_t insts_count, const uint8_t *const memory, size_t memory_size);
LVM_API lvm_Machine *lvm_create_machine(void);
LVM_API void lvm_machine_load_program(lvm_Machine *machine, lvm_Program program);
void lvm_dump_stack(const lvm_Machine *const lvm, FILE *stream);
LVM_API lvm_Trap lvm_machine_run(lvm_Machine *machine, int64_t limit);

#ifdef LVM_IMPLEMENTATION

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <memory.h>
#include <math.h>
#include <assert.h>
#include <errno.h>

// TODO: FIX STATIC ASSERT
// static_assert (LVM_MAX_TRAPS == 8, "THE TRAPS HAS CHANGED PLEASE UPDATE THE CODE");
const char *const lvm_traps_names[LVM_MAX_TRAPS] = {
    [LVM_TRAP_OK]                    = "ok",
    [LVM_TRAP_ILLEGAL_INST]          = "illegal instruction",
    [LVM_TRAP_ILLEGAL_INST_ACCESS]   = "illegal instruction access",
    [LVM_TRAP_ILLEGAL_OPERAND]       = "illegal operand",
    [LVM_TRAP_STACK_OVERFLOW]        = "stack overflow",
    [LVM_TRAP_STACK_UNDERFLOW]       = "stack underflow",
    [LVM_TRAP_DIV_BY_ZERO]           = "div by zero",
    [LVM_TRAP_ILLEGAL_MEMORY_ACCESS] = "illegal memory access",
};

// TODO: FIX STATIC ASSERT
// static_assert (LVM_MAX_INSTS == 65, "THE INSTRUCTION SET CHANGED PLEASE UPDATE THE CODE");
const char *const lvm_insts_names[LVM_MAX_INSTS] = {
    [LVM_INST_ILLEGAL]     = "illegal",
    [LVM_INST_NOP]         = "nop",
    [LVM_INST_PUSH]        = "push",
    [LVM_INST_POP]         = "pop",
    [LVM_INST_DUP]         = "dup",
    [LVM_INST_SWAP]        = "swap",
    [LVM_INST_INCI]        = "inci",
    [LVM_INST_INCF]        = "incf",
    [LVM_INST_DECI]        = "deci",
    [LVM_INST_DECF]        = "decf",
    [LVM_INST_ADDI]        = "addi",
    [LVM_INST_ADDF]        = "addf",
    [LVM_INST_SUBI]        = "subi",
    [LVM_INST_SUBF]        = "subf",
    [LVM_INST_MULTI]       = "multi",
    [LVM_INST_MULTF]       = "multf",
    [LVM_INST_DIVI]        = "divi",
    [LVM_INST_DIVU]        = "divu",
    [LVM_INST_DIVF]        = "divf",
    [LVM_INST_MODI]        = "modi",
    [LVM_INST_MODU]        = "modu",
    [LVM_INST_MODF]        = "modf",
    [LVM_INST_EQ]          = "eq",
    [LVM_INST_NEQ]         = "neq",
    [LVM_INST_GTI]         = "gti",
    [LVM_INST_GTU]         = "gtu",
    [LVM_INST_GTF]         = "gtf",
    [LVM_INST_GEI]         = "gei",
    [LVM_INST_GEU]         = "geu",
    [LVM_INST_GEF]         = "gef",
    [LVM_INST_STI]         = "sti",
    [LVM_INST_STU]         = "stu",
    [LVM_INST_STF]         = "stf",
    [LVM_INST_SEI]         = "sei",
    [LVM_INST_SEU]         = "seu",
    [LVM_INST_SEF]         = "sef",
    [LVM_INST_AND]         = "and",
    [LVM_INST_NOT]         = "not",
    [LVM_INST_OR]          = "or",
    [LVM_INST_ANDB]        = "andb",
    [LVM_INST_NOTB]        = "notb",
    [LVM_INST_ORB]         = "orb",
    [LVM_INST_XOR]         = "xor",
    [LVM_INST_SHL]         = "shl",
    [LVM_INST_SHR]         = "shr",
    [LVM_INST_CALL]        = "call",
    [LVM_INST_NATIVE]      = "native",
    [LVM_INST_RETURN]      = "return",
    [LVM_INST_JMP]         = "jmp",
    [LVM_INST_JZ]          = "jz",
    [LVM_INST_JNZ]         = "jnz",
    [LVM_INST_I2F]         = "i2f",
    [LVM_INST_U2F]         = "u2f",
    [LVM_INST_F2I]         = "f2i",
    [LVM_INST_F2U]         = "f2u",
    [LVM_INST_READ8]       = "read8",
    [LVM_INST_READ16]      = "read16",
    [LVM_INST_READ32]      = "read32",
    [LVM_INST_READ64]      = "read64",
    [LVM_INST_WRITE8]      = "write8",
    [LVM_INST_WRITE16]     = "write16",
    [LVM_INST_WRITE32]     = "write32",
    [LVM_INST_WRITE64]     = "write64",
    [LVM_INST_HLT]         = "hlt",
    [LVM_INST_PRINT_DEBUG] = "print_debug",
};

LVM_API lvm_Trap lvm_machine_execute_inst(lvm_Machine *machine);
LVM_API lvm_Trap lvm_machine_stack_push(lvm_Machine *machine, lvm_Word word);
LVM_API lvm_Trap lvm_machine_stack_pop(lvm_Machine *machine, lvm_Word *word);
LVM_API void lvm_machine_advance(lvm_Machine *machine);

#define lvm_Machine_Stack_Push(MACHINE_P, WORD) do { lvm_Trap __MACRO__TRAP__ = lvm_machine_stack_push((MACHINE_P), (WORD)); if (LVM_TRAP_OK != __MACRO__TRAP__) { return __MACRO__TRAP__; } } while (0)
#define lvm_Machine_Stack_Pop(MACHINE_P, WORD_P) do { lvm_Trap __MACRO__TRAP__ = lvm_machine_stack_pop((MACHINE_P), (WORD_P)); if (LVM_TRAP_OK != __MACRO__TRAP__) { return __MACRO__TRAP__; } } while (0)
#define lvm_Binary_Inst(MACHINE_P, OUT_TYPE, IN_TYPE, OP)                                                         \
    do {                                                                                                          \
        lvm_Word __MACRO__A__;                                                                                    \
        lvm_Word __MACRO__B__;                                                                                    \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__A__);                                                        \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__B__);                                                        \
        lvm_Word __MACRO__RESULT__ = { .as_##OUT_TYPE = __MACRO__A__.as_##IN_TYPE OP __MACRO__B__.as_##IN_TYPE }; \
        lvm_Machine_Stack_Push((MACHINE_P), __MACRO__RESULT__);                                                   \
    } while (0)

#define lvm_Unary_Inst(MACHINE_P, OUT_TYPE, IN_TYPE, OP)                                \
    do {                                                                                \
        lvm_Word __MACRO__A__;                                                          \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__A__);                              \
        lvm_Word __MACRO__RESULT__ = { .as_##OUT_TYPE = OP __MACRO__A__.as_##IN_TYPE }; \
        lvm_Machine_Stack_Push((MACHINE_P), __MACRO__RESULT__);                         \
    } while (0)

#define lvm_Cast_Inst(MACHINE_P, SRC, DST, CAST)                                      \
    do {                                                                              \
        lvm_Word __MACRO__A__;                                                        \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__A__);                            \
        lvm_Word __MACRO__RESULT__ = { .as_##DST = (CAST((__MACRO__A__).as_##SRC)) }; \
        lvm_Machine_Stack_Push((MACHINE_P), __MACRO__RESULT__);                       \
    } while (0)

#define lvm_Memory_Read_Inst(MACHINE_P, TYPE)                                                                 \
    do {                                                                                                      \
        lvm_Word __MACRO__ADDR__;                                                                             \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__ADDR__);                                                 \
        if (__MACRO__ADDR__.as_u64 >= LVM_MEMORY_MAX - (sizeof(TYPE) - 1)) {                                  \
            return LVM_TRAP_ILLEGAL_MEMORY_ACCESS;                                                            \
        }                                                                                                     \
        lvm_Machine_Stack_Push((MACHINE_P), (lvm_Word){ .as_u64 = machine->memory[__MACRO__ADDR__.as_u64] }); \
    } while (0)

#define lvm_Memory_Write_Inst(MACHINE_P, TYPE)                                                  \
    do {                                                                                        \
        lvm_Word __MACRO__VALUE__;                                                              \
        lvm_Word __MACRO__ADDR__;                                                               \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__VALUE__);                                  \
        lvm_Machine_Stack_Pop((MACHINE_P), &__MACRO__ADDR__);                                   \
        if (__MACRO__ADDR__.as_u64 >= LVM_MEMORY_MAX - sizeof(TYPE)) {                          \
            return LVM_TRAP_ILLEGAL_MEMORY_ACCESS;                                              \
        }                                                                                       \
        *(TYPE*)&((MACHINE_P)->memory[__MACRO__ADDR__.as_u64]) = (TYPE)__MACRO__VALUE__.as_u64; \
    } while (0);

LVM_API const char *lvm_get_trap_name(lvm_Trap trap) {
    assert((uint32_t)trap < LVM_MAX_TRAPS && "Illegal trap value");

    return lvm_traps_names[trap];
}

LVM_API const char *lvm_get_inst_name(lvm_InstType inst) {
    assert((uint32_t)inst < LVM_MAX_INSTS && "Illegal inst value");

    return lvm_insts_names[inst];
}

LVM_API lvm_Program lvm_create_program(const lvm_Inst *const insts, size_t insts_count, const uint8_t *const memory, size_t memory_size) {
    assert(memory_size <= LVM_MEMORY_MAX && "Illegal size of memory for a program");
    assert(insts != NULL && "Illegal pointer(NULL)");
    assert(!(memory == NULL && memory_size != 0) && "Illegal pointer(NULL)");
    
    lvm_Program program = {0};

    program.insts = insts;
    program.insts_count = insts_count;
    program.memory = memory;
    program.memory_size = memory_size;

    return program;
}

LVM_API lvm_Machine *lvm_create_machine(void) {
    lvm_Machine *machine = malloc(sizeof(*machine));

    assert(machine != NULL && "Illegal pointer(NULL)");

    memset(machine, 0, sizeof(*machine));

    return machine;
}

LVM_API void lvm_machine_load_program(lvm_Machine *machine, lvm_Program program) {
    assert(machine != NULL && "Illegal pointer(NULL)");
    
    machine->program = program;
    machine->hlt = false;
    machine->ip = 0;
    machine->stack_top = 0;

    if (program.memory != NULL && program.memory_size != 0) {
        assert(program.memory_size <= LVM_MEMORY_MAX && "Illegal size of memory for a program");
        memcpy(machine->memory, program.memory, program.memory_size);
    }
}

void lvm_dump_stack(const lvm_Machine *const lvm, FILE *stream) {
    fprintf(stream, "-----------------------------------------\n");
    fprintf(stream, "Stack:\n");

    if (lvm->stack_top > 0) {
        for (uint64_t i = 0; i < lvm->stack_top; i++) {
            fprintf(stream, "  i64:%lld, u64:%llu, f64:%lf\n",
                lvm->stack[i].as_i64, lvm->stack[i].as_u64, lvm->stack[i].as_f64);
        }
    }
    else {
        fprintf(stream, "  [THE STACK IS EMPTY]\n");
    }
    fprintf(stream, "-----------------------------------------\n");
}

LVM_API lvm_Trap lvm_machine_run(lvm_Machine *machine, int64_t limit) {
    assert(machine != NULL && "Illegal pointer(NULL)");
    
    for (; limit != 0 && !machine->hlt; ) {
        lvm_Trap trap = lvm_machine_execute_inst(machine);

        if (trap != LVM_TRAP_OK) {
            return trap;
        }

        if (limit > 0) {
            limit--;
        }
    }

    return LVM_TRAP_OK;
}

// TODO: FIX STATIC ASSERT
// static_assert (LVM_MAX_INSTS == 65, "THE INSTRUCTION SET CHANGED PLEASE UPDATE THE CODE");
LVM_API lvm_Trap lvm_machine_execute_inst(lvm_Machine *machine) {
    assert(machine != NULL && "Illegal pointer(NULL)");

    lvm_Program program = machine->program;

    assert((program.insts != NULL || program.insts_count != 0) && "ILLEGAL PROGRAM");

    lvm_Inst inst = program.insts[machine->ip];
    
    switch (inst.type) {
        case LVM_INST_NOP: {
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_PUSH: {
            lvm_Machine_Stack_Push(machine, inst.operand);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_POP: {
            lvm_Machine_Stack_Pop(machine, NULL);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DUP: {
            lvm_Word a;

            lvm_Machine_Stack_Pop(machine, &a);

            lvm_Machine_Stack_Push(machine, a);
            lvm_Machine_Stack_Push(machine, a);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SWAP: {
            if (machine->stack_top < 2 || (uint64_t)(machine->stack_top - 2 - inst.operand.as_u64) > machine->stack_top) {
                return LVM_TRAP_STACK_UNDERFLOW;
            }

            lvm_Word a;
            lvm_Word b;

            lvm_Machine_Stack_Pop(machine, &a);
            b = machine->stack[machine->stack_top - 1 - inst.operand.as_u64];

            machine->stack[machine->stack_top - 1 - inst.operand.as_u64] = a;
            lvm_Machine_Stack_Push(machine, b);
            
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_INCI: {
            lvm_Unary_Inst(machine, i64, i64, ++);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_INCF: {
            lvm_Unary_Inst(machine, f64, f64, ++);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DECI: {
            lvm_Unary_Inst(machine, i64, i64, --);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DECF: {
            lvm_Unary_Inst(machine, f64, f64, --);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_ADDI: {
            lvm_Binary_Inst(machine, i64, i64, +);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_ADDF: {
            lvm_Binary_Inst(machine, f64, f64, +);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SUBI: {
            lvm_Binary_Inst(machine, i64, i64, -);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SUBF: {
            lvm_Binary_Inst(machine, f64, f64, -);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_MULTI: {
            lvm_Binary_Inst(machine, i64, i64, *);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_MULTF: {
            lvm_Binary_Inst(machine, f64, f64, *);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DIVI: {
            lvm_Binary_Inst(machine, i64, i64, /);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DIVU: {
            lvm_Binary_Inst(machine, u64, u64, /);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_DIVF: {
            lvm_Binary_Inst(machine, f64, f64, /);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_MODI: {
            lvm_Binary_Inst(machine, i64, i64, %);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_MODU: {
            lvm_Binary_Inst(machine, u64, u64, %);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_MODF: {
            lvm_Word a;
            lvm_Word b;

            lvm_Machine_Stack_Pop(machine, &a);
            lvm_Machine_Stack_Pop(machine, &b);

            lvm_Word result = { .as_f64 = fmod(a.as_f64, b.as_f64) };

            lvm_Machine_Stack_Push(machine, result);

            lvm_machine_advance(machine);
        } break;
        case LVM_INST_EQ: {
            lvm_Binary_Inst(machine, u64, u64, ==);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_NEQ: {
            lvm_Binary_Inst(machine, u64, u64, !=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GTI: {
            lvm_Binary_Inst(machine, u64, i64, >);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GTU: {
            lvm_Binary_Inst(machine, u64, u64, >);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GTF: {
            lvm_Binary_Inst(machine, u64, f64, >);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GEI: {
            lvm_Binary_Inst(machine, u64, i64, >=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GEU: {
            lvm_Binary_Inst(machine, u64, u64, >=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_GEF: {
            lvm_Binary_Inst(machine, u64, f64, >=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_STI: {
            lvm_Binary_Inst(machine, u64, i64, <);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_STU: {
            lvm_Binary_Inst(machine, u64, u64, <);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_STF: {
            lvm_Binary_Inst(machine, u64, f64, <);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SEI: {
            lvm_Binary_Inst(machine, u64, i64, <=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SEU: {
            lvm_Binary_Inst(machine, u64, u64, <=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SEF: {
            lvm_Binary_Inst(machine, u64, f64, <=);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_AND: {
            lvm_Binary_Inst(machine, u64, u64, &&);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_NOT: {
            lvm_Unary_Inst(machine, u64, u64, !);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_OR: {
            lvm_Binary_Inst(machine, u64, u64, ||);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_ANDB: {
            lvm_Binary_Inst(machine, u64, u64, &);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_NOTB: {
            lvm_Unary_Inst(machine, u64, u64, ~);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_ORB: {
            lvm_Binary_Inst(machine, u64, u64, |);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_XOR: {
            lvm_Binary_Inst(machine, u64, u64, ^);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SHL: {
            lvm_Binary_Inst(machine, u64, u64, <<);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_SHR: {
            lvm_Binary_Inst(machine, u64, u64, >>);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_CALL: {
            lvm_Word addr;

            lvm_Machine_Stack_Pop(machine, &addr);

            lvm_Word old_ip = { .as_u64 = machine->ip + 1 };

            machine->ip = addr.as_u64;

            lvm_Machine_Stack_Push(machine, old_ip);
        } break;
        case LVM_INST_NATIVE: {
            lvm_Word native;

            lvm_Machine_Stack_Pop(machine, &native);

            assert(native.as_u64 < LVM_NATIVE_MAX && "ILLEGAL NATIVE CALL");

            lvm_Trap trap = machine->natives[native.as_u64](machine);
            lvm_machine_advance(machine);

            return trap;
        } break;
        case LVM_INST_RETURN: {
            lvm_Word addr;

            lvm_Machine_Stack_Pop(machine, &addr);

            machine->ip = addr.as_u64;
        } break;
        case LVM_INST_JMP: {
            lvm_Word addr;

            lvm_Machine_Stack_Pop(machine, &addr);

            machine->ip = addr.as_u64;
        } break;
        case LVM_INST_JZ: {
            lvm_Word addr;
            lvm_Word cond;

            lvm_Machine_Stack_Pop(machine, &addr);
            lvm_Machine_Stack_Pop(machine, &cond);

            if (!cond.as_u64) {
                machine->ip = addr.as_u64;
            } else {
                lvm_machine_advance(machine);
            }
        } break;
        case LVM_INST_JNZ: {
            lvm_Word addr;
            lvm_Word cond;

            lvm_Machine_Stack_Pop(machine, &addr);
            lvm_Machine_Stack_Pop(machine, &cond);

            if (cond.as_u64) {
                machine->ip = addr.as_u64;
            } else {
                lvm_machine_advance(machine);
            }
        } break;
        case LVM_INST_I2F: {
            lvm_Cast_Inst(machine, i64, f64, (double));
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_U2F: {
            lvm_Cast_Inst(machine, u64, f64, (double));
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_F2I: {
            lvm_Cast_Inst(machine, f64, i64, (int64_t));
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_F2U: {
            lvm_Cast_Inst(machine, f64, u64, (uint64_t)(int64_t));
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_READ8: {
            lvm_Memory_Read_Inst(machine, uint8_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_READ16: {
            lvm_Memory_Read_Inst(machine, uint16_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_READ32: {
            lvm_Memory_Read_Inst(machine, uint32_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_READ64: {
            lvm_Memory_Read_Inst(machine, uint64_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_WRITE8: {
            lvm_Memory_Write_Inst(machine, uint8_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_WRITE16: {
            lvm_Memory_Write_Inst(machine, uint16_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_WRITE32: {
            lvm_Memory_Write_Inst(machine, uint32_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_WRITE64: {
            lvm_Memory_Write_Inst(machine, uint64_t);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_HLT: {
            machine->hlt = true;
        } break;
        case LVM_INST_PRINT_DEBUG: {
            lvm_Word a;

            lvm_Machine_Stack_Pop(machine, &a);
            printf("[WORD]{ .as_i64 = %"PRId64", .as_u64 = %"PRIu64", .as_f64 = %lf }\n", a.as_i64, a.as_u64, a.as_f64);
            lvm_machine_advance(machine);
        } break;
        case LVM_INST_ILLEGAL:
        case LVM_MAX_INSTS:
        default: {
            return LVM_TRAP_ILLEGAL_INST;
        } break;
    }

    return LVM_TRAP_OK;
}

LVM_API lvm_Trap lvm_machine_stack_push(lvm_Machine *machine, lvm_Word word) {
    assert(machine != NULL && "Illegal pointer(NULL)");
    
    if (machine->stack_top >= LVM_STACK_MAX) {
        return LVM_TRAP_STACK_OVERFLOW;
    }

    machine->stack[machine->stack_top++] = word;

    return LVM_TRAP_OK;
}

LVM_API lvm_Trap lvm_machine_stack_pop(lvm_Machine *machine, lvm_Word *word) {
    assert(machine != NULL && "Illegal pointer(NULL)");
    
    if (machine->stack_top < 1) {
        return LVM_TRAP_STACK_UNDERFLOW;
    }

    if (word != NULL) {
        *word = machine->stack[--machine->stack_top];
    }

    return LVM_TRAP_OK;
}

LVM_API void lvm_machine_advance(lvm_Machine *machine) {
    assert(machine != NULL && "Illegal pointer(NULL)");

    machine->ip++;
}

#endif

#endif