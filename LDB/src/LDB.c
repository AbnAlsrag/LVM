#define LVM_IMPLEMENTATION
#include <LVM.h>

#ifdef _WIN32
#define WINPAUSE system("pause")
#else
#define WINPAUSE
#endif


void command(char** args, int count) {

}

LVM lvm = { 0 };

int main(int argc, char** argv) {
    command(argv, argc);

    LVM_load_program_from_file(&lvm, "examples/example-1.melf");

    for (uint32_t i = 0; i < lvm.program_size; i++) {
        LVM_Operation operation = lvm.program[i];
        switch (operation.type)
        {
        case LVM_OP_PUSH:
            printf("%s %lld\n", LVM_operation_name(operation.type), operation.operand.as_i64);
            break;
        case LVM_OP_POP:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_DUP:
            printf("%s %llu\n", LVM_operation_name(operation.type), operation.operand.as_u64);
            break;
        case LVM_OP_ADD:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_SUB:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_MULT:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_DIV:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_JMP:
            printf("%s %llu\n", LVM_operation_name(operation.type), operation.operand.as_u64);
            break;
        case LVM_OP_JMP_IF:
            printf("%s %llu\n", LVM_operation_name(operation.type), operation.operand.as_u64);
            break;
        case LVM_OP_EQ:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_HLT:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        case LVM_OP_NATIVE:
            printf("%s %llu\n", LVM_operation_name(operation.type), operation.operand.as_u64);
            break;
        case LVM_OP_PRINT_DEBUG:
            printf("%s\n", LVM_operation_name(operation.type));
            break;
        default:
            fprintf(stderr, "INVALID OPERATION\n");
            break;
        }
    }

    WINPAUSE;
    return 0;
}