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
        printf("%s", LVM_operation_name(lvm.program[i].type));
        if (LVM_operation_has_operand(lvm.program[i].type)) {
            printf(" %lld", lvm.program[i].operand.as_i64);
        }
        printf("\n");
    }

    WINPAUSE;
    return 0;
}