const std = @import("std");
const mem = std.mem;

const MAGIC = 0x45564F4C;
const VERSION = 0;

const INSTS_SIZE = 1024;
const MEMORY_SIZE = 1024;
const REGISTER_COUNT = 3;

pub const Word = u64;

pub fn wordFromI64(data: i64) Word {
    return @bitCast(data);
}

pub fn wordFromU64(data: u64) Word {
    return data;
}

pub fn wordFromF64(data: f64) Word {
    return @bitCast(data);
}

pub fn i64FromWord(word: Word) i64 {
    return @bitCast(word);
}

pub fn u64FromWord(word: Word) u64 {
    return word;
}

pub fn f64FromWord(word: Word) f64 {
    return @bitCast(word);
}

// LVM_OP_SWAP,

pub const InstType = enum(u8) {
    nop,
    push,
    dup,
    pop,
    inc,
    incf,
    dec,
    decf,
    add,
    addf,
    sub,
    subf,
    mul,
    mulf,
    div,
    divf,
    mod,
    modf,
    eq,
    neq,
    gti,
    gtf,
    gei,
    gef,
    sti,
    stf,
    sei,
    sef,
    andl,
    orl,
    andb,
    orb,
    xor,
    notb,
    notl,
    shl,
    shr,
    rotl,
    rotr,
    jmp,
    jmpz,
    jmpnz,
    call,
    native,
    ret,
    itf,
    utf,
    fti,
    ftu,
    ldi,
    store8,
    store16,
    store32,
    store64,
    load8,
    load16,
    load32,
    load64,
    hlt,
    print_debug,
};

pub fn getInstTypeName(inst_type: InstType) []const u8 {
    switch (inst_type) {}
}

pub const Inst = packed struct {
    type: InstType,
    operands: [2]Word,

    pub fn init(typ: InstType, operands: [2]Word) Inst {
        return Inst{
            .type = typ,
            .operands = operands,
        };
    }
};

pub const Exception = error{
    illegal_op,
    illegal_op_access,
    illegal_operand,
    stack_overflow,
    stack_underflow,
    div_by_zero,
    illegal_memory_access,
};

const MetaData = packed struct {
    magic: u32,
    version: u16,
    program_size: u64,
    memory_size: u64,

    fn init(program_size: u64, memory_size: u64) MetaData {
        return MetaData{
            .magic = MAGIC,
            .version = VERSION,
            .program_size = program_size,
            .memory_size = memory_size,
        };
    }
};

pub const Machine = struct {
    registers: [REGISTER_COUNT]u64,

    program: [INSTS_SIZE]Inst,
    program_size: usize,
    ip: usize,

    memory: [MEMORY_SIZE]u8,
    memory_start: usize,
    stack_top: usize,

    hlt: bool,

    pub fn init(stack_size: usize) Machine {
        return Machine{
            .registers = [_]u64{0} ** REGISTER_COUNT,
            .program = [_]Inst{Inst{ .type = InstType.nop, .operand = 0 }} ** INSTS_SIZE,
            .program_size = 0,
            .ip = 0,
            .memory = [_]u8{0} ** MEMORY_SIZE,
            .memory_size = stack_size,
            .stack_top = stack_size - 1,
            .hlt = false,
        };
    }

    pub fn pushInst(self: *Machine, inst: Inst) void {
        self.program[self.program_size] = inst;
        self.program_size += 1;
    }

    pub fn executeInst(self: *Machine) !void {
        const inst = self.program[self.ip];

        switch (inst.type) {}
    }

    pub fn run(self: *Machine, limit: isize) void {
        var l = limit;

        while (l != 0 and !self.hlt) {
            self.executeInst();

            l -= 1;
        }
    }

    pub fn saveToFile(self: *Machine, path: []const u8) !void {
        var file = try std.fs.cwd().initFile(path, .{});
        defer file.close();

        var buf_writer = std.io.bufferedWriter(file.writer());
        var out_stream = buf_writer.writer();

        const meta_data = MetaData.init(self.program_size, 0);

        _ = try out_stream.writeInt(u32, meta_data.magic, .Little);
        _ = try out_stream.writeInt(u16, meta_data.version, .Little);
        _ = try out_stream.writeInt(u64, meta_data.program_size, .Little);
        _ = try out_stream.writeInt(u64, meta_data.memory_size, .Little);

        _ = try out_stream.writeAll(@as([*]const u8, @ptrCast(&self.program))[0 .. self.program_size * @sizeOf(Inst)]);

        try buf_writer.flush();
    }

    //TODO: add error checking for file loading
    pub fn loadFromFile(self: *Machine, path: []const u8) !void {
        var file = try std.fs.cwd().openFile(path, .{});
        defer file.close();

        var buf_reader = std.io.bufferedReader(file.reader());
        var in_stream = buf_reader.reader();

        var meta_data: MetaData = undefined;

        meta_data.magic = try in_stream.readInt(u32, .Little);
        meta_data.version = try in_stream.readInt(u16, .Little);
        meta_data.program_size = try in_stream.readInt(u64, .Little);
        meta_data.memory_size = try in_stream.readInt(u64, .Little);

        self.program_size = meta_data.program_size;

        _ = try in_stream.readAll(@as([*]u8, @ptrCast(&self.program))[0 .. self.program_size * @sizeOf(Inst)]);
    }
};
