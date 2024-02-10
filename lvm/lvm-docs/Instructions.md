# Instruction Layout:

> [!info]
> The size of one instruction is 1 byte or 5 bytes.
> You could know the size of one instruction using the [[#Instruction LUT|Instruction LUT]].

| Type | Size(bits) | Optional |
| ---- | ---- | ---- |
| Opcode | 8 | False |
| Operand | 32 | True |

# Instruction LUT:
| Instruction | Description | Opcode |
| ---- | ---- | ---- |
| [[nop]] | This instruction does nothing | 0x00 |
| [[push]] | adds a word on top of the operand stack | 0x01 |
| [[pop]] | removes a word from the top of the stack | 0x02 |
| [[dup]] | duplicates the top word from the stack | 0x03 |
| [[over]] | duplicate the second top word from the stack | 0x04 |
| [[swap]] | swaps the two top elements from the stack | 0x05 |
| [[rot]] | a b c -- b c a |  |
| [[spin]] | a b c -- c b a |  |
| [[inci]] |  |  |
| [[deci]] |  |  |
| [[neg]] |  |  |
| [[addi]] | a b -- a+b |  |
| [[subi]] | a b -- a-b |  |
| [[muli]] | a b -- a*b |  |
| [[divi]] | a b -- a/b |  |
| [[remi]] | a b -- a%b |  |
| [[jmp]] |  |  |
| [[jmpz]] |  |  |
| [[jmpnz]] |  |  |
| [[call]] |  |  |
| [[native]] |  |  |
| [[ret]] |  |  |
| [[store8]] |  |  |
| [[load8]] |  |  |
| [[store16]] |  |  |
| [[load16]] |  |  |
| [[store32]] |  |  |
| [[load32]] |  |  |
| [[store64]] |  |  |
| [[load64]] |  |  |
| [[hlt]] | halts the machine |  |
| [[addf]] |  |  |
| [[subf]] |  |  |
| [[mulf]] |  |  |
| [[divf]] |  |  |
| [[remf]] |  |  |
| [[addiu]] |  |  |
| [[subiu]] |  |  |
| [[muliu]] |  |  |
| [[diviu]] |  |  |
| [[remiu]] |  |  |
| [[incf]] |  |  |
| [[decf]] |  |  |
| [[itf]] |  |  |
| [[fti]] |  |  |
| [[utf]] |  |  |
| [[ftu]] |  |  |
| [[eq]] |  |  |
| [[neq]] |  |  |
| [[gti]] |  |  |
| [[gtiu]] |  |  |
| [[gtf]] |  |  |
| [[gei]] |  |  |
| [[geiu]] |  |  |
| [[gef]] |  |  |
| [[sti]] |  |  |
| [[stiu]] |  |  |
| [[stf]] |  |  |
| [[sei]] |  |  |
| [[seiu]] |  |  |
| [[sef]] |  |  |
| [[andl]] |  |  |
| [[orl]] |  |  |
| [[notl]] |  |  |
| [[andb]] |  |  |
| [[orb]] |  |  |
| [[xor]] |  |  |
| [[notb]] |  |  |
| [[shl]] |  |  |
| [[shr]] |  |  |
| [[rotl]] |  |  |
| [[rotr]] |  |  |
