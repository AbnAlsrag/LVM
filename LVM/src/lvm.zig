// ██╗     ██╗   ██╗███╗   ███╗
// ██║     ██║   ██║████╗ ████║
// ██║     ██║   ██║██╔████╔██║
// ██║     ╚██╗ ██╔╝██║╚██╔╝██║
// ███████╗ ╚████╔╝ ██║ ╚═╝ ██║
// ╚══════╝  ╚═══╝  ╚═╝     ╚═╝
// This Software Was Made by ABN ALSRAG
// LVM

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

const std = @import("std");
const mem = std.mem;

const MAGIC = 0x45564F4C;
const VERSION = 0;

const INSTS_SIZE = 1024;
const MEMORY_SIZE = 1024;
const REGISTER_COUNT = 5;

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

const Type = enum(u8) {
    any,
    float,
    int,
    signed_int,
    unsigned_int,
    i8,
    i16,
    i32,
    i64,
    u8,
    u16,
    u32,
    u64,
    s8,
    s16,
    s32,
    s64,
    register,
    memory_addr,
    inst_addr,
    native_id,
    bool,
};

pub const InstType = enum(u8) {
    nop,
    push,
    dup,
    pop,
    mov,
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
    notl,
    andb,
    orb,
    xor,
    notb,
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
    load8,
    store16,
    load16,
    store32,
    load32,
    store64,
    load64,
    hlt,
};

//TODO: add inst def
const OutputLoc = enum(u8) {
    any,
    register,
    stack,
    memory,
};

const Output = struct {
    type: Type,
    loc: OutputLoc,

    //TODO: Improve errors
    fn init(comptime typ: Type, comptime loc: OutputLoc) Output {
        var output: Output = Output{ .type = typ, .loc = loc };
        return output;
    }
};

const InstDef = struct {
    type: InstType,
    name: []const u8,
    takes_operands: bool,
    operands_types: [2]Type,
    operands_size: usize,
    has_input: bool,
    input: [2]Type,
    input_size: usize,
    has_output: bool,
    output: Output,

    //TODO: Improve errors
    fn init(comptime typ: InstType, comptime operands: []Type, comptime input: []Type, comptime output: []Output) InstDef {
        var def: InstDef = undefined;
        def.type = typ;
        def.name = @tagName(typ);

        comptime if (operands.len == 0) {
            def.takes_operands = false;
            def.operands_size = 0;
        } else if (operands.len < 3) {
            def.takes_operands = true;
            def.operands_size = operands.len;
            def.operands_types = operands;
        } else {
            @compileError("Can't have more than two operands\n");
        };

        comptime if (input.len == 0) {
            def.has_input = false;
            def.input_size = 0;
        } else if (input.len < 3) {
            def.has_input = true;
            def.input_size = operands.len;
            def.input = operands;
        } else {
            @compileError("Can't have more than two operands\n");
        };

        comptime if (output.len == 0) {
            def.has_output = false;
        } else if (output.len == 1) {
            def.has_output = true;
            def.output = output;
        } else {
            @compileError("Can't have more than one output\n");
        };

        return def;
    }
};

const inst_defs_lut: []InstDef = [_]InstDef{
    InstDef.init(.nop, .{}, .{}, .{}),
    InstDef.init(.push, .{ .bool, .any }, .{.any}, .{Output.init(.any, .stack)}),
    InstDef.init(.dup, .{}, .{.any}, .{Output.init(.any, .stack)}),
    InstDef.init(.pop, .{.register}, .{.any}, .{Output.init(.any, .register)}),
    InstDef.init(.mov, .{ .register, .register }, .{.any}, .{Output.init(.any, .register)}),
    InstDef.init(.inc, .{}, .{.int}, .{Output.init(.int, .register)}),
    InstDef.init(.incf, .{}, .{.float}, .{Output.init(.float, .register)}),
    InstDef.init(.dec, .{}, .{.int}, .{Output.init(.int, .register)}),
    InstDef.init(.decf, .{}, .{.float}, .{Output.init(.float, .register)}),
    InstDef.init(.add, .{}, .{ .int, .int }, .{Output.init(.int, .register)}),
    InstDef.init(.addf, .{}, .{ .float, .float }, .{Output.init(.float, .register)}),
    InstDef.init(.sub, .{}, .{ .int, .int }, .{Output.init(.int, .register)}),
    InstDef.init(.subf, .{}, .{ .float, .float }, .{Output.init(.float, .register)}),
    InstDef.init(.mul, .{}, .{ .int, .int }, .{Output.init(.int, .register)}),
    InstDef.init(.mulf, .{}, .{ .float, .float }, .{Output.init(.float, .register)}),
    InstDef.init(.div, .{}, .{ .int, .int }, .{Output.init(.int, .register)}),
    InstDef.init(.divf, .{}, .{ .float, .float }, .{Output.init(.float, .register)}),
    InstDef.init(.mod, .{}, .{ .int, .int }, .{Output.init(.int, .register)}),
    InstDef.init(.modf, .{}, .{ .float, .float }, .{Output.init(.float, .register)}),
    InstDef.init(.eq, .{}, .{ .any, .any }, .{Output.init(.bool, .register)}),
    InstDef.init(.neq, .{}, .{ .any, .any }, .{Output.init(.bool, .register)}),
    InstDef.init(.gti, .{}, .{ .int, .int }, .{Output.init(.bool, .register)}),
    InstDef.init(.gtf, .{}, .{ .float, .float }, .{Output.init(.bool, .register)}),
    InstDef.init(.gei, .{}, .{ .int, .int }, .{Output.init(.bool, .register)}),
    InstDef.init(.gef, .{}, .{ .float, .float }, .{Output.init(.bool, .register)}),
    InstDef.init(.sti, .{}, .{ .int, .int }, .{Output.init(.bool, .register)}),
    InstDef.init(.stf, .{}, .{ .float, .float }, .{Output.init(.bool, .register)}),
    InstDef.init(.sei, .{}, .{ .int, .int }, .{Output.init(.bool, .register)}),
    InstDef.init(.sef, .{}, .{ .float, .float }, .{Output.init(.bool, .register)}),
    InstDef.init(.andl, .{}, .{ .bool, .bool }, .{Output.init(.bool, .register)}),
    InstDef.init(.orll, .{}, .{ .bool, .bool }, .{Output.init(.bool, .register)}),
    InstDef.init(.notl, .{}, .{.bool}, .{Output.init(.bool, .register)}),
    InstDef.init(.andb, .{}, .{ .any, .any }, .{Output.init(.any, .register)}),
    InstDef.init(.orb, .{}, .{ .any, .any }, .{Output.init(.any, .register)}),
    InstDef.init(.xorb, .{}, .{ .any, .any }, .{Output.init(.any, .register)}),
    InstDef.init(.notb, .{}, .{.any}, .{Output.init(.any, .register)}),
    InstDef.init(.shl, .{}, .{ .any, .unsigned_int }, .{Output.init(.any, .register)}),
    InstDef.init(.shr, .{}, .{ .any, .unsigned_int }, .{Output.init(.any, .register)}),
    InstDef.init(.rotl, .{}, .{ .any, .unsigned_int }, .{Output.init(.any, .register)}),
    InstDef.init(.rotr, .{}, .{ .any, .unsigned_int }, .{Output.init(.any, .register)}),
    InstDef.init(.jmp, .{.inst_addr}, .{}, .{}),
    InstDef.init(.jmpz, .{.inst_addr}, .{.any}, .{}),
    InstDef.init(.jmpnz, .{.inst_addr}, .{.any}, .{}),
    InstDef.init(.call, .{.inst_addr}, .{}, .{Output.init(.inst_addr, .register)}),
    InstDef.init(.native, .{}, .{.native_id}, .{}),
    InstDef.init(.ret, .{}, .{.inst_addr}, .{}),
    InstDef.init(.itf, .{}, .{.signed_int}, .{Output.init(.float, .register)}),
    InstDef.init(.utf, .{}, .{.unsigned_int}, .{Output.init(.float, .register)}),
    InstDef.init(.fti, .{}, .{.float}, .{Output.init(.signed_int, .register)}),
    InstDef.init(.ftu, .{}, .{.float}, .{Output.init(.unsigned_int, .register)}),
    InstDef.init(.ldi, .{ .register, .any }, .{}, .{Output.init(.any, .register)}),
    InstDef.init(.store8, .{.register}, .{.memory_addr}, .{Output.init(.u8, .memory)}),
    InstDef.init(.load8, .{.register}, .{.memory_addr}, .{Output.init(.u8, .register)}),
    InstDef.init(.store16, .{.register}, .{.memory_addr}, .{Output.init(.u16, .memory)}),
    InstDef.init(.load16, .{.register}, .{.memory_addr}, .{Output.init(.u16, .register)}),
    InstDef.init(.store32, .{.register}, .{.memory_addr}, .{Output.init(.u32, .memory)}),
    InstDef.init(.load32, .{.register}, .{.memory_addr}, .{Output.init(.u32, .register)}),
    InstDef.init(.store64, .{.register}, .{.memory_addr}, .{Output.init(.u64, .memory)}),
    InstDef.init(.load64, .{.register}, .{.memory_addr}, .{Output.init(.u64, .register)}),
    InstDef.init(.hlt, .{}, .{}, .{}),
};

pub fn getInstTypeName(inst_type: InstType) []const u8 {
    _ = inst_type;
    return inst_defs_lut[@intFromEnum(InstType)].name;
}

pub const Inst = struct {
    type: InstType,
    operands: [2]Word = [_]Word{wordFromU64(0)} ** 2,

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
    registers: [REGISTER_COUNT]u64 = [_]u64{0} ** REGISTER_COUNT,

    program: [INSTS_SIZE]Inst = [_]Inst{Inst.init(.nop, .{ 0, 0 })} ** INSTS_SIZE,
    program_size: usize = 0,
    ip: usize = 0,

    memory: [MEMORY_SIZE]u8 = [_]u8{0} ** MEMORY_SIZE,
    memory_start: usize,
    stack_top: usize,

    hlt: bool = false,

    pub fn init(stack_size: usize) Machine {
        return Machine{
            .memory_start = stack_size,
            .stack_top = stack_size - 1,
        };
    }

    pub fn pushInst(self: *Machine, inst: Inst) void {
        self.program[self.program_size] = inst;
        self.program_size += 1;
    }

    pub fn executeInst(self: *Machine) Exception!void {
        const inst = self.program[self.ip];

        switch (inst.type) {}
    }

    pub fn run(self: *Machine, limit: isize) void {
        var l = limit;

        while (l != 0 and !self.hlt) {
            self.executeInst() catch |err| {
                @panic(err);
            };

            l -= 1;
        }
    }

    //FIXME: rewrite it
    pub fn saveToFile(self: Machine, path: []const u8) !void {
        var file = try std.fs.cwd().createFile(path, .{});
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

    //FIXME: rewrite it
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
