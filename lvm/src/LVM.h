#ifndef LVM_H
#define LVM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

// NOTE: Stolen from https://github.com/tsoding/bm
#if defined(_MSC_VER)
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#define LVM_PLATFORM_LINUX 1
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/AbnAlsrag/LVM"
#endif

#define CAST_OP(lvm, src, dst, cast)                                                                      \
    do {                                                                                                  \
        if ((lvm)->stack_size < 1) {                                                                      \
            return LVM_ET_STACK_UNDERFLOW;                                                                \
        }                                                                                                 \
                                                                                                          \
        (lvm)->stack[(lvm)->stack_size - 1].as_##dst = cast (lvm)->stack[(lvm)->stack_size - 1].as_##src; \
    } while (0)                                                                                           \

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define LVM_MAGIC 0x45564F4C
#define LVM_VERSION 0

#define LVM_WORD_SIZE 8
#define LVM_REGISTER_MAX 6LL
#define LVM_STACK_MAX 1024LL
#define LVM_MEMORY_MAX (640 * 1000)
#define LVM_PROGRAM_MAX 1024LL
#define LVM_NATIVE_MAX 1024LL

typedef union LVM_Word {
    int64_t as_i64;
    uint64_t as_u64;
    double as_f64;
    void* as_ptr;
} LVM_Word;

typedef uint64_t LVM_OpAddr;
typedef uint64_t LVM_MemAddr;

typedef enum LVM_PLATFORMS {
    LVM_PLATFORM_NASM,
} LVM_PLATFORMS;

typedef enum LVM_Types {
    LVM_TYPE_ANY,
    LVM_TYPE_INT,
    LVM_TYPE_FLOAT,
} LVM_Types;

typedef enum LVM_ExceptionType {
    LVM_ET_OK,
    LVM_ET_ILLEGAL_OP,
    LVM_ET_ILLEGAL_OP_ACCESS,
    LVM_ET_ILLEGAL_OPERAND,
    LVM_ET_STACK_OVERFLOW,
    LVM_ET_STACK_UNDERFLOW,
    LVM_ET_DIV_BY_ZERO,
    LVM_ILLEGAL_MEMORY_ACCESS,
} LVM_ExceptionType;

typedef enum LVM_OperationType {
    LVM_OP_NOP = 0,
    LVM_OP_PUSH,
    LVM_OP_POP,
    LVM_OP_DUP,
    LVM_OP_SWAP,
    LVM_OP_INCI,
    LVM_OP_INCF,
    LVM_OP_DECI,
    LVM_OP_DECF,
    LVM_OP_ADDI,
    LVM_OP_ADDF,
    LVM_OP_SUBI,
    LVM_OP_SUBF,
    LVM_OP_MULTI,
    LVM_OP_MULTF,
    LVM_OP_DIVI,
    LVM_OP_DIVF,
    LVM_OP_MODI,
    LVM_OP_MODF,
    LVM_OP_EQ,
    LVM_OP_NEQ,
    LVM_OP_GTI,
    LVM_OP_GTF,
    LVM_OP_GEI,
    LVM_OP_GEF,
    LVM_OP_STI,
    LVM_OP_STF,
    LVM_OP_SEI,
    LVM_OP_SEF,
    LVM_OP_AND,
    LVM_OP_NOT,
    LVM_OP_OR,
    LVM_OP_ANDB,
    LVM_OP_NOTB,
    LVM_OP_ORB,
    LVM_OP_XOR,
    LVM_OP_SHL,
    LVM_OP_SHR,
    LVM_OP_ROTL,
    LVM_OP_ROTR,
    LVM_OP_CALL,
    LVM_OP_NATIVE,
    LVM_OP_RETURN,
    LVM_OP_JMP,
    LVM_OP_JMP_IF,
    LVM_OP_I2F,
    LVM_OP_U2F,
    LVM_OP_F2U,
    LVM_OP_F2I,
    LVM_OP_READ8,
    LVM_OP_READ16,
    LVM_OP_READ32,
    LVM_OP_READ64,
    LVM_OP_WRITE8,
    LVM_OP_WRITE16,
    LVM_OP_WRITE32,
    LVM_OP_WRITE64,
    LVM_OP_HLT,
    LVM_OP_PRINT_DEBUG,
    NUMBER_OF_OPERATIONS,
} LVM_OperationType;

typedef struct LVM_Operation {
    LVM_OperationType type;
    LVM_Word operand;
} LVM_Operation;

PACK(typedef struct LVM_FileMetaData {
    uint32_t magic;
    uint16_t version;
    uint64_t program_size;
    uint64_t memory_size;
} LVM_FileMetaData);

typedef struct LVM LVM;

typedef LVM_ExceptionType(*LVM_Native)(LVM*);

struct LVM {
    //TODO: implement registers
    LVM_Word registers[LVM_REGISTER_MAX];

    LVM_Word stack[LVM_STACK_MAX];
    uint64_t stack_size;

    //TODO: add memory
    //TODO: STOP!!!!! BEEING LAZY
    uint8_t memory[LVM_MEMORY_MAX];

    LVM_Operation program[LVM_PROGRAM_MAX];
    uint64_t program_size;
    LVM_OpAddr ip;

    LVM_Native natives[LVM_NATIVE_MAX];
    size_t natives_size;

    uint8_t halt;
};

#define LVM_CREATE_PUSH_OP(value, cast) { .type = LVM_OP_PUSH, .operand = { .as_##cast = value } }
#define LVM_CREATE_POP_OP() { .type = LVM_OP_POP }
#define LVM_CREATE_DUP_OP(addr) { .type = LVM_OP_DUP, .operand = { .as_u64 = addr } }
#define LVM_CREATE_SWAP_OP(addr) { .type = LVM_OP_SWAP, .operand = { .as_u64 = addr } }
#define LVM_CREATE_INCI_OP() { .type = LVM_OP_INCI }
#define LVM_CREATE_INCF_OP() { .type = LVM_OP_INCF }
#define LVM_CREATE_DECI_OP() { .type = LVM_OP_DECI }
#define LVM_CREATE_DECF_OP() { .type = LVM_OP_DECF }
#define LVM_CREATE_ADDI_OP() { .type = LVM_OP_ADDI }
#define LVM_CREATE_ADDF_OP() { .type = LVM_OP_ADDF }
#define LVM_CREATE_SUBI_OP() { .type = LVM_OP_SUBI }
#define LVM_CREATE_SUBF_OP() { .type = LVM_OP_SUBF }
#define LVM_CREATE_MULTI_OP() { .type = LVM_OP_MULTI }
#define LVM_CREATE_MULTF_OP() { .type = LVM_OP_MULTF }
#define LVM_CREATE_DIVI_OP() { .type = LVM_OP_DIVI }
#define LVM_CREATE_DIVF_OP() { .type = LVM_OP_DIVF }
#define LVM_CREATE_MODI_OP() { .type = LVM_OP_MODI }
#define LVM_CREATE_MODF_OP() { .type = LVM_OP_MODF }
#define LVM_CREATE_EQ_OP() { .type = LVM_OP_EQ }
#define LVM_CREATE_NEQ_OP() { .type = LVM_OP_NEQ }
#define LVM_CREATE_GTI_OP() { .type = LVM_OP_GTI }
#define LVM_CREATE_GTF_OP() { .type = LVM_OP_GTF }
#define LVM_CREATE_GEI_OP() { .type = LVM_OP_GEI }
#define LVM_CREATE_GEF_OP() { .type = LVM_OP_GEF }
#define LVM_CREATE_STI_OP() { .type = LVM_OP_STI }
#define LVM_CREATE_STF_OP() { .type = LVM_OP_STF }
#define LVM_CREATE_SEI_OP() { .type = LVM_OP_SEI }
#define LVM_CREATE_SEF_OP() { .type = LVM_OP_SEF }
#define LVM_CREATE_AND_OP() { .type = LVM_OP_AND }
#define LVM_CREATE_NOT_OP() { .type = LVM_OP_NOT }
#define LVM_CREATE_OR_OP() { .type = LVM_OP_OR }
#define LVM_CREATE_ANDB_OP() { .type = LVM_OP_ANDB }
#define LVM_CREATE_NOTB_OP() { .type = LVM_OP_NOTB }
#define LVM_CREATE_ORB_OP() { .type = LVM_OP_ORB }
#define LVM_CREATE_XOR_OP() { .type = LVM_OP_XOR }
#define LVM_CREATE_SHL_OP() { .type = LVM_OP_SHL }
#define LVM_CREATE_SHR_OP() { .type = LVM_OP_SHR }
#define LVM_CREATE_ROTL_OP() { .type = LVM_OP_ROTL }
#define LVM_CREATE_ROTR_OP() { .type = LVM_OP_ROTR }
#define LVM_CREATE_CALL_OP(addr) { .type = LVM_OP_CALL, .operand = { .as_u64 = addr } }
#define LVM_CREATE_NATIVE_OP(addr) { .type = LVM_OP_NATIVE, .operand = { .as_u64 = addr } }
#define LVM_CREATE_RETURN_OP() { .type = LVM_OP_RETURN }
#define LVM_CREATE_JMP_OP(addr) { .type = LVM_OP_JMP, .operand = { .as_u64 = addr } }
#define LVM_CREATE_JMP_IF_OP(addr) { .type = LVM_OP_JMP_IF, .operand = { .as_u64 = addr } }
#define LVM_CREATE_I2F_OP() { .type = LVM_OP_I2F }
#define LVM_CREATE_U2F_OP() { .type = LVM_OP_U2F }
#define LVM_CREATE_F2U_OP() { .type = LVM_OP_F2U }
#define LVM_CREATE_F2I_OP() { .type = LVM_OP_F2I }
#define LVM_CREATE_READ8_OP() { .type = LVM_OP_READ8 }
#define LVM_CREATE_READ16_OP() { .type = LVM_OP_READ16 }
#define LVM_CREATE_READ32_OP() { .type = LVM_OP_READ32 }
#define LVM_CREATE_READ64_OP() { .type = LVM_OP_READ64 }
#define LVM_CREATE_WRITE8_OP() { .type = LVM_OP_WRITE8 }
#define LVM_CREATE_WRITE16_OP() { .type = LVM_OP_WRITE16 }
#define LVM_CREATE_WRITE32_OP() { .type = LVM_OP_WRITE32 }
#define LVM_CREATE_WRITE64_OP() { .type = LVM_OP_WRITE64 }
#define LVM_CREATE_HLT_OP() { .type = LVM_OP_HLT }
#define LVM_CREATE_PRINT_DEBUG_OP() { .type = LVM_OP_PRINT_DEBUG }

const char* const LVM_operations_name[NUMBER_OF_OPERATIONS] = {
    [LVM_OP_NOP]            = "nop",
    [LVM_OP_PUSH]           = "push",
    [LVM_OP_POP]            = "pop",
    [LVM_OP_DUP]            = "dup",
    [LVM_OP_SWAP]           = "swap",
    [LVM_OP_INCI]           = "inci",
    [LVM_OP_INCF]           = "incf",
    [LVM_OP_DECI]           = "deci",
    [LVM_OP_DECF]           = "decf",
    [LVM_OP_ADDI]           = "addi",
    [LVM_OP_ADDF]           = "addf",
    [LVM_OP_SUBI]           = "subi",
    [LVM_OP_SUBF]           = "subf",
    [LVM_OP_MULTI]          = "multi",
    [LVM_OP_MULTF]          = "multf",
    [LVM_OP_DIVI]           = "divi",
    [LVM_OP_DIVF]           = "divf",
    [LVM_OP_MODI]           = "modi",
    [LVM_OP_MODF]           = "modf",
    [LVM_OP_EQ]             = "eq",
    [LVM_OP_NEQ]            = "neq",
    [LVM_OP_GTI]            = "gti",
    [LVM_OP_GTF]            = "gtf",
    [LVM_OP_GEI]            = "gei",
    [LVM_OP_GEF]            = "gef",
    [LVM_OP_STI]            = "sti",
    [LVM_OP_STF]            = "stf",
    [LVM_OP_SEI]            = "sei",
    [LVM_OP_SEF]            = "sef",
    [LVM_OP_AND]            = "and",
    [LVM_OP_NOT]            = "not",
    [LVM_OP_OR]             = "or",
    [LVM_OP_ANDB]           = "andb",
    [LVM_OP_NOTB]           = "notb",
    [LVM_OP_ORB]            = "orb",
    [LVM_OP_XOR]            = "xor",
    [LVM_OP_SHL]            = "shl",
    [LVM_OP_SHR]            = "shr",
    [LVM_OP_ROTL]           = "rotl",
    [LVM_OP_ROTR]           = "rotr",
    [LVM_OP_CALL]           = "call",
    [LVM_OP_NATIVE]         = "native",
    [LVM_OP_RETURN]         = "return",
    [LVM_OP_JMP]            = "jmp",
    [LVM_OP_JMP_IF]         = "jmp_if",
    [LVM_OP_I2F]            = "i2f",
    [LVM_OP_U2F]            = "u2f",
    [LVM_OP_F2U]            = "f2u",
    [LVM_OP_F2I]            = "f2i",
    [LVM_OP_READ8]          = "read8",
    [LVM_OP_READ16]         = "read16",
    [LVM_OP_READ32]         = "read32",
    [LVM_OP_READ64]         = "read64",
    [LVM_OP_WRITE8]         = "write8",
    [LVM_OP_WRITE16]        = "write16",
    [LVM_OP_WRITE32]        = "write32",
    [LVM_OP_WRITE64]        = "write64",
    [LVM_OP_HLT]            = "hlt",
    [LVM_OP_PRINT_DEBUG]    = "print_debug",
};

const uint8_t LVM_operations_has_operand[NUMBER_OF_OPERATIONS] = {
    [LVM_OP_PUSH]   = 1,
    [LVM_OP_DUP]    = 1,
    [LVM_OP_SWAP]   = 1,
    [LVM_OP_CALL]   = 1,
    [LVM_OP_NATIVE] = 1,
    [LVM_OP_JMP]    = 1,
    [LVM_OP_JMP_IF] = 1,
};

#define LVM_GET_OPERATION_TYPE_NAME(operation) "OPERATION "#operation

const char* LVM_exception_value(LVM_ExceptionType type);
inline const char* LVM_operation_name(LVM_OperationType type);
inline uint8_t LVM_operation_has_operand(LVM_OperationType type);
LVM_ExceptionType LVM_execute_operation(LVM* lvm);
LVM_ExceptionType LVM_execute_program(LVM* lvm, int limit);
void LVM_push_native(LVM* lvm, LVM_Native native);
void LVM_dump_stack(const LVM* lvm, FILE* stream);
void LVM_save_program_to_file(const LVM* lvm, uint64_t memory_size, const char* file_path);
void LVM_load_program_from_memory(LVM* lvm, LVM_Operation* program, size_t program_size);
void LVM_load_program_from_file(LVM* lvm, const char* file_path);
//TODO: Add naative compiling
void LVM_compile_lvm_to_native(const LVM* lvm, FILE* stream, LVM_PLATFORMS platform);
void LVM_compile_native_to_lvm(LVM* lvm, FILE* stream, LVM_PLATFORMS platform);

#ifdef LVM_IMPLEMENTATION

const char* LVM_exception_value(LVM_ExceptionType type) {
    switch (type)
    {
    case LVM_ET_OK:
        return "OK";
    case LVM_ET_ILLEGAL_OP:
        return "EXCEPTION ILLEGAL LVM_Operation";
    case LVM_ET_ILLEGAL_OP_ACCESS:
        return "EXCEPTION ILLEGAL LVM_Operation ACCESS";
    case LVM_ET_ILLEGAL_OPERAND:
        return "EXCEPTION ILLEGAL OPERAND";
    case LVM_ET_STACK_OVERFLOW:
        return "EXCEPTION STACK OVERFLOW";
    case LVM_ET_STACK_UNDERFLOW:
        return "EXCEPTION STACK UNDERFLOW";
    case LVM_ET_DIV_BY_ZERO:
        return "EXCEPTION DIVISION BY ZERO";
    case LVM_ILLEGAL_MEMORY_ACCESS:
        return "EXCEPTION ILLEGAL MEMORY ACCESS";
    default:
        assert(0 && "UNVALID EXCEPTION TYPE");
        return NULL;
    }
}

inline const char* LVM_operation_name(LVM_OperationType type) {
    assert((type < NUMBER_OF_OPERATIONS && type > -1));
    return LVM_operations_name[type];
}

inline uint8_t LVM_operation_has_operand(LVM_OperationType type) {
    assert((type < NUMBER_OF_OPERATIONS && type > -1));
    return LVM_operations_has_operand[type];
}

LVM_ExceptionType LVM_execute_operation(LVM* lvm) {
    if (lvm->ip < 0 || lvm->ip >= lvm->program_size) {
        return LVM_ET_ILLEGAL_OP_ACCESS;
    }

    LVM_Operation operation = lvm->program[lvm->ip];

    switch (operation.type)
    {
    case LVM_OP_NOP:
        break;
    case LVM_OP_PUSH:
        if (lvm->stack_size >= LVM_STACK_MAX) {
            return LVM_ET_STACK_OVERFLOW;
        }

        lvm->stack[lvm->stack_size++] = operation.operand;
        break;
    case LVM_OP_POP:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack_size--;
        break;
    case LVM_OP_DUP:
        if (operation.operand.as_i64 < 0 || operation.operand.as_u64 >= lvm->stack_size) {
            return LVM_ET_ILLEGAL_OPERAND;
        }

        if (lvm->stack_size >= LVM_STACK_MAX) {
            return LVM_ET_STACK_OVERFLOW;
        }

        if (lvm->stack_size - operation.operand.as_u64 <= 0) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size] = lvm->stack[(lvm->stack_size - 1) - operation.operand.as_u64];
        lvm->stack_size++;
        break;
    case LVM_OP_SWAP:
        if (lvm->stack_size < 2 || (lvm->stack_size - 2 - operation.operand.as_u64) < 0) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        LVM_Word tmp = lvm->stack[lvm->stack_size - 1 - operation.operand.as_u64];
        lvm->stack[lvm->stack_size - 1 - operation.operand.as_u64] = lvm->stack[lvm->stack_size - 2 - operation.operand.as_u64];
        lvm->stack[lvm->stack_size - 2 - operation.operand.as_u64] = tmp;

        break;
    case LVM_OP_INCI:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_i64 += 1;
        break;
    case LVM_OP_INCF:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_f64 += 1.0f;
        break;
    case LVM_OP_DECI:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_i64 -= 1;
        break;
    case LVM_OP_DECF:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_f64 -= 1.0f;
        break;
    case LVM_OP_ADDI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 += lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_ADDF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_f64 += lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_SUBI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 -= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_SUBF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_f64 -= lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_MULTI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 *= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_MULTF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_f64 *= lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_DIVI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        if (lvm->stack[lvm->stack_size - 1].as_u64 == 0) {
            return LVM_ET_DIV_BY_ZERO;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 /= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_DIVF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_f64 /= lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_MODI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 %= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_MODF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_f64 = fmod(lvm->stack[lvm->stack_size - 2].as_f64, lvm->stack[lvm->stack_size - 1].as_f64);
        lvm->stack_size--;
        break;
    case LVM_OP_EQ:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 == lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_NEQ:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 != lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_GTI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 > lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_GTF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_f64 > lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_GEI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 >= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_GEF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_f64 >= lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_STI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 < lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_STF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_f64 < lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_SEI:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 <= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_SEF:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_f64 <= lvm->stack[lvm->stack_size - 1].as_f64;
        lvm->stack_size--;
        break;
    case LVM_OP_AND:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 && lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_NOT:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_i64 = !lvm->stack[lvm->stack_size - 1].as_i64;
        break;
    case LVM_OP_OR:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 = lvm->stack[lvm->stack_size - 2].as_i64 || lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_ANDB:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 &= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_NOTB:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 1].as_i64 = ~lvm->stack[lvm->stack_size - 1].as_i64;
        break;
    case LVM_OP_ORB:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 |= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_XOR:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 ^= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_SHL:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 <<= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_SHR:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->stack[lvm->stack_size - 2].as_i64 >>= lvm->stack[lvm->stack_size - 1].as_i64;
        lvm->stack_size--;
        break;
    case LVM_OP_ROTL:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            int64_t x = lvm->stack[lvm->stack_size - 2].as_i64;
            int64_t n = lvm->stack[lvm->stack_size - 1].as_i64;

            if (!(n &= 63) == 0) {
                lvm->stack[lvm->stack_size - 2].as_i64 = x << n | x >> (64 - n);
            }

            lvm->stack_size--;
        }

        break;
    case LVM_OP_ROTR:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            int64_t x = lvm->stack[lvm->stack_size - 2].as_i64;
            int64_t n = lvm->stack[lvm->stack_size - 1].as_i64;

            if (!(n &= 63) == 0) {
                lvm->stack[lvm->stack_size - 2].as_i64 = x >> n | x << (64 - n);
            }

            lvm->stack_size--;
        }

        break;
    case LVM_OP_CALL:
        if (lvm->stack_size >= LVM_STACK_MAX) {
            return LVM_ET_STACK_OVERFLOW;
        }

        lvm->stack[lvm->stack_size++].as_u64 = lvm->ip;
        lvm->ip = operation.operand.as_u64 - 1;
        break;
    case LVM_OP_NATIVE:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        LVM_ExceptionType exception = lvm->natives[operation.operand.as_u64](lvm);
        lvm->ip++;
        return exception;
    case LVM_OP_RETURN:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        lvm->ip = lvm->stack[--lvm->stack_size].as_u64;
        break;
    case LVM_OP_JMP:
        lvm->ip = operation.operand.as_u64 - 1;
        break;
    case LVM_OP_JMP_IF:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        if (lvm->stack[lvm->stack_size - 1].as_u64) {
            lvm->stack_size--;
            lvm->ip = operation.operand.as_u64 - 1;
        }
        break;
    case LVM_OP_I2F:
        CAST_OP(lvm, i64, f64, (double));
        break;
    case LVM_OP_U2F:
        CAST_OP(lvm, u64, f64, (double));
        break;
    case LVM_OP_F2I:
        CAST_OP(lvm, f64, i64, (int64_t));
        break;
    case LVM_OP_F2U:
        CAST_OP(lvm, f64, u64, (uint64_t)(int64_t));
        break;
    case LVM_OP_READ8:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[lvm->stack_size - 1].as_u64;

            if (addr >= LVM_MEMORY_MAX) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            lvm->stack[lvm->stack_size - 1].as_u64 = lvm->memory[addr];
        }

        break;
    case LVM_OP_READ16:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[lvm->stack_size - 1].as_u64;

            if (addr >= LVM_MEMORY_MAX - 1) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            lvm->stack[lvm->stack_size - 1].as_u64 = *(uint16_t*)&lvm->memory[addr];
        }

        break;
    case LVM_OP_READ32:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {

            LVM_MemAddr addr = lvm->stack[lvm->stack_size - 1].as_u64;

            if (addr >= LVM_MEMORY_MAX - 3) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            lvm->stack[lvm->stack_size - 1].as_u64 = *(uint32_t*)&lvm->memory[addr];
        }

        break;
    case LVM_OP_READ64:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[lvm->stack_size - 1].as_u64;

            if (addr >= LVM_MEMORY_MAX - 7) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            lvm->stack[lvm->stack_size - 1].as_u64 = *(uint64_t*)&lvm->memory[addr];
        }

        break;
    case LVM_OP_WRITE8:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[--lvm->stack_size].as_u64;

            if (addr >= LVM_MEMORY_MAX) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            lvm->memory[addr] = (uint8_t)lvm->stack[--lvm->stack_size].as_u64;
        }

        break;
    case LVM_OP_WRITE16:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[--lvm->stack_size].as_u64;

            if (addr >= LVM_MEMORY_MAX - 1) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            *(uint16_t*)&lvm->memory[addr] = (uint16_t)lvm->stack[--lvm->stack_size].as_u64;
        }

        break;
    case LVM_OP_WRITE32:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[--lvm->stack_size].as_u64;

            if (addr >= LVM_MEMORY_MAX - 3) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            *(uint32_t*)&lvm->memory[addr] = (uint32_t)lvm->stack[--lvm->stack_size].as_u64;
        }

        break;
    case LVM_OP_WRITE64:
        if (lvm->stack_size < 2) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        {
            LVM_MemAddr addr = lvm->stack[--lvm->stack_size].as_u64;

            if (addr >= LVM_MEMORY_MAX - 7) {
                return LVM_ET_ILLEGAL_OP_ACCESS;
            }

            *(uint64_t*)&lvm->memory[addr] = lvm->stack[--lvm->stack_size].as_u64;
        }

        break;
    case LVM_OP_HLT:
        lvm->halt = 1;
        break;
    case LVM_OP_PRINT_DEBUG:
        if (lvm->stack_size < 1) {
            return LVM_ET_STACK_UNDERFLOW;
        }

        printf("i64:%lld, u64:%llu, f64:%lf, ptr:%p\n",
            lvm->stack[lvm->stack_size - 1].as_i64, lvm->stack[lvm->stack_size - 1].as_u64,
            lvm->stack[lvm->stack_size - 1].as_f64, lvm->stack[lvm->stack_size - 1].as_ptr);
        lvm->stack_size--;
        break;
    case NUMBER_OF_OPERATIONS:
    default:
        return LVM_ET_ILLEGAL_OP;
        break;
    }

    lvm->ip++;
    return LVM_ET_OK;
}

LVM_ExceptionType LVM_execute_program(LVM* lvm, int limit) {
    for (; limit != 0 && !lvm->halt; ) {
        //printf(":Operation: %s:\n", LVM_operation_name(lvm->program[lvm->ip].type));
        LVM_ExceptionType exception = LVM_execute_operation(lvm);
        //LVM_dump_stack(lvm, stdout);
        //printf("\n");

        if (exception != LVM_ET_OK) {
            return exception;
        }

        if (limit > 0) {
            limit--;
        }
    }

    return LVM_ET_OK;
}

void LVM_push_native(LVM* lvm, LVM_Native native) {
    assert(lvm->natives_size < LVM_NATIVE_MAX);
    lvm->natives[lvm->natives_size++] = native;
}

void LVM_dump_stack(const LVM* lvm, FILE* stream) {
    fprintf(stream, "Stack:\n");

    if (lvm->stack_size > 0) {
        for (uint64_t i = 0; i < lvm->stack_size; i++) {
            fprintf(stream, "  i64:%lld, u64:%llu, f64:%lf, ptr:%p\n",
                lvm->stack[i].as_i64, lvm->stack[i].as_u64, lvm->stack[i].as_f64, lvm->stack[i].as_ptr);
        }
    }
    else {
        fprintf(stream, "  [THE STACK IS EMPTY]\n");
    }
}

void LVM_save_program_to_file(const LVM* lvm, uint64_t memory_size, const char* file_path) {
    FILE* file = fopen(file_path, "wb");

    if (file == NULL) {
        fprintf(stderr, "Error: Couldn't open file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    LVM_FileMetaData meta = {
        .magic = LVM_MAGIC,
        .version = LVM_VERSION,
        .program_size = lvm->program_size,
        .memory_size = memory_size,
    };

    fwrite(&meta, sizeof(LVM_FileMetaData), 1, file);

    if (ferror(file)) {
        fprintf(stderr, "Error: Couldn't write to file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fwrite(lvm->program, sizeof(lvm->program[0]), lvm->program_size, file);

    if (ferror(file)) {
        fprintf(stderr, "Error: Couldn't write to file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fwrite(lvm->program, sizeof(lvm->memory[0]), memory_size, file);

    if (ferror(file)) {
        fprintf(stderr, "Error: Couldn't write to file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(file);
}

void LVM_load_program_from_memory(LVM* lvm, LVM_Operation* program, size_t program_size) {
    assert(program_size < LVM_PROGRAM_MAX);
    memcpy(lvm->program, program, sizeof(program[0]) * program_size);
    lvm->program_size = program_size;
}

void LVM_load_program_from_file(LVM* lvm, const char* file_path) {
    FILE* file = fopen(file_path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Error: Couldn't open file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    LVM_FileMetaData meta = { 0 };

    size_t n = fread(&meta, sizeof(meta), 1, file);

    if (n < 1) {
        fprintf(stderr, "ERROR: %s: Couldn't read file\n", file_path);
        exit(1);
    }

    if (meta.magic != LVM_MAGIC) {
        fprintf(stderr, "ERROR: %s: File seem to not be melf file: Excepected magic %04X but got %04X\n", file_path, LVM_MAGIC, meta.magic);
        exit(1);
    }

    if (meta.version > LVM_VERSION) {
        fprintf(stderr, "ERROR: %s: Excepected version %u or bellow but got version %u\n", file_path, LVM_VERSION, meta.version);
        exit(1);
    }

    if (meta.program_size > LVM_PROGRAM_MAX) {
        fprintf(stderr, "ERROR: %s: Excepected program size %lld or bellow but got %lld\n", file_path, LVM_PROGRAM_MAX, meta.program_size);
        exit(1);
    }

    if (meta.memory_size > LVM_MEMORY_MAX) {
        fprintf(stderr, "ERROR: %s: Excepected memory size %i or bellow but got %lld\n", file_path, LVM_MEMORY_MAX, meta.memory_size);
        exit(1);
    }

    lvm->program_size = fread(lvm->program, sizeof(lvm->program[0]), meta.program_size, file);

    if (lvm->program_size != meta.program_size) {
        fprintf(stderr, "ERROR: %s: Excepected %lld instructions but got %lld\n", file_path, meta.program_size, lvm->program_size);
        exit(1);
    }

    n = fread(lvm->program, sizeof(lvm->memory[0]), meta.program_size, file);

    if (n != meta.memory_size) {
        fprintf(stderr, "ERROR: %s: Excepected %lld bytes but got %lld\n", file_path, meta.memory_size, n);
        exit(1);
    }

    fclose(file);
}

//TODO: add native compiling
void LVM_compile_lvm_to_native(const LVM* lvm, FILE* stream, LVM_PLATFORMS platform) {
    switch (platform)
    {
    case LVM_PLATFORM_NASM: 
        //LVM_compile_lvm_to_nasm(lvm, stream);
        break;
    default:
        assert(0 && "Unsupported platform");
        break;
    }
}

void LVM_compile_native_to_lvm(LVM* lvm, FILE* stream, LVM_PLATFORMS platform) {
    switch (platform)
    {
    case LVM_PLATFORM_NASM:
        //LVM_compile_nasm_to_lvm(lvm, stream);
        break;
    default:
        assert(0 && "Unsupported platform");
        break;
    }
}

void LVM_compile_lvm_to_nasm(const LVM* lvm, FILE* stream) {

}

void LVM_compile_nasm_to_lvm(LVM* lvm, FILE* stream) {

}

#endif

#endif