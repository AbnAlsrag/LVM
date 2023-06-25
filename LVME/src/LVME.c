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

LVM_Operation program[] = {
	LVM_CREATE_PUSH_OP(5),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_PUSH_OP(5),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_PUSH_OP(5),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_PUSH_OP(5),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_PUSH_OP(5),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_ADD_OP(),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_SUB_OP(),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_MULT_OP(),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_DIV_OP(),
	LVM_CREATE_NATIVE_OP(0),
	LVM_CREATE_HLT_OP(),
};

LVM_ExceptionType print(LVM* lvm) {
	printf(":Operation: %s:\n", LVM_operation_name(lvm->program[lvm->ip - 1].type));
	LVM_dump_stack(stdout, lvm);
	return LVM_ET_OK;
}

int main(int argc, char** argv) {
	command(argv, argc);

	LVM_push_native(&lvm, print);
	LVM_load_program_from_memory(&lvm, program, ARRAY_SIZE(program));
	//LVM_load_program_from_file("examples/example-2.melf", &lvm);

	LVM_dump_stack(stdout, &lvm);
	
	LVM_ExceptionType exception = LVM_execute_program(&lvm, -1);
	
	if (exception != LVM_ET_OK) {
		fprintf(stderr, "Exception: %s\n", LVM_exception_value(exception));

		WINPAUSE;
		exit(1);
	}

	WINPAUSE;
	return 0;
}