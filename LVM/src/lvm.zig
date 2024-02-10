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
const NATIVE_SIZE = 1024;
const REGISTER_COUNT = 5;

const REGISTER_SIZE = 8;

pub const Register = u64;

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
    register,
    memory_addr,
    inst_addr,
    native_id,
    bool,
};

pub const InstType = enum(u8) {
    nop,
    pushi,
    push,
    pop,
    pusha,
    popa,
    enter,
    leave,
    dup,
    mov,
    inc,
    incf,
    dec,
    decf,
    neg,
    addi,
    addu,
    addf,
    subi,
    subu,
    subf,
    muli,
    mulu,
    mulf,
    divi,
    divu,
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
    jz,
    jnz,
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
    inst_max,
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

pub const InstDef = struct {
    type: InstType,
    name: []const u8,
    has_operands: bool,
    operands: [2]Type,

    //TODO: Improve errors
    fn init(comptime typ: InstType, operands: []Type) InstDef {
        var def: InstDef = undefined;
        def.type = typ;
        def.name = @tagName(typ);

        if (operands.len == 0) {
            def.has_operands = false;
            def.operands = operands;
        } else if (operands.len <= 2) {
            def.has_operands = true;
            def.operands = operands;
        } else {
            @panic("[ERROR] instructions can't have more than two operands");
        }

        return def;
    }
};

const inst_defs_lut: [@intFromEnum(InstType.inst_max)]InstDef = [_]InstDef{
    InstDef.init(.nop, .{}),
    InstDef.init(.pushi, .{}),
    InstDef.init(.push, .{}),
    InstDef.init(.pop, .{}),
    InstDef.init(.pusha, .{}),
    InstDef.init(.popa, .{}),
    InstDef.init(.enter, .{}),
    InstDef.init(.leave, .{}),
    InstDef.init(.dup, .{}),
    InstDef.init(.mov, .{}),
    InstDef.init(.inc, .{}),
    InstDef.init(.incf, .{}),
    InstDef.init(.dec, .{}),
    InstDef.init(.decf, .{}),
    InstDef.init(.neg, .{}),
    InstDef.init(.addi, .{}),
    InstDef.init(.addu, .{}),
    InstDef.init(.addf, .{}),
    InstDef.init(.subi, .{}),
    InstDef.init(.subu, .{}),
    InstDef.init(.subf, .{}),
    InstDef.init(.muli, .{}),
    InstDef.init(.mulu, .{}),
    InstDef.init(.mulf, .{}),
    InstDef.init(.divi, .{}),
    InstDef.init(.divu, .{}),
    InstDef.init(.divf, .{}),
    InstDef.init(.mod, .{}),
    InstDef.init(.modf, .{}),
    InstDef.init(.eq, .{}),
    InstDef.init(.neq, .{}),
    InstDef.init(.gti, .{}),
    InstDef.init(.gtf, .{}),
    InstDef.init(.gei, .{}),
    InstDef.init(.gef, .{}),
    InstDef.init(.sti, .{}),
    InstDef.init(.stf, .{}),
    InstDef.init(.sei, .{}),
    InstDef.init(.sef, .{}),
    InstDef.init(.andl, .{}),
    InstDef.init(.orl, .{}),
    InstDef.init(.notl, .{}),
    InstDef.init(.andb, .{}),
    InstDef.init(.orb, .{}),
    InstDef.init(.xor, .{}),
    InstDef.init(.notb, .{}),
    InstDef.init(.shl, .{}),
    InstDef.init(.shr, .{}),
    InstDef.init(.rotl, .{}),
    InstDef.init(.rotr, .{}),
    InstDef.init(.jmp, .{.inst_addr}),
    InstDef.init(.jz, .{}),
    InstDef.init(.jnz, .{}),
    InstDef.init(.call, .{}),
    InstDef.init(.native, .{}),
    InstDef.init(.ret, .{}),
    InstDef.init(.itf, .{}),
    InstDef.init(.utf, .{}),
    InstDef.init(.fti, .{}),
    InstDef.init(.ftu, .{}),
    InstDef.init(.ldi, .{}),
    InstDef.init(.store8, .{}),
    InstDef.init(.load8, .{}),
    InstDef.init(.store16, .{}),
    InstDef.init(.load16, .{}),
    InstDef.init(.store32, .{}),
    InstDef.init(.load32, .{}),
    InstDef.init(.store64, .{}),
    InstDef.init(.load64, .{}),
    InstDef.init(.hlt, .{}),
};

pub fn getInstTypeName(inst_type: InstType) []const u8 {
    _ = inst_type;
    return inst_defs_lut[@intFromEnum(InstType)].name;
}

pub fn isInst(name: []const u8) ?InstType {
    for (inst_defs_lut) |inst_def| {
        if (std.mem.eql(u8, name, inst_def.name)) {
            return inst_def.type;
        }
    }

    return null;
}

pub fn getInstDef(inst_type: InstType) InstDef {
    return inst_defs_lut[@intFromEnum(inst_type)];
}

pub const Inst = struct {
    type: InstType,
    operand0: Word,

    pub fn init(typ: InstType, operand0: Word) Inst {
        return Inst{
            .type = typ,
            .operand0 = operand0,
        };
    }
};

pub const Exception = error{
    illegal_op,
    illegal_op_access,
    illegal_register_access,
    illegal_operand,
    stack_overflow,
    stack_underflow,
    div_by_zero,
    illegal_memory_access,
};

const MetaData = struct {
    magic: u32,
    version: u16,
    program_size: u64,
    memory_size: u64,

    fn init(machine: Machine) MetaData {
        return MetaData{
            .magic = MAGIC,
            .version = VERSION,
            .program_size = machine.program_size,
            .memory_size = machine.memory_size,
        };
    }
};

const Native = *const fn (machine: *Machine) Exception!void;

pub const Machine = struct {
    registers: [REGISTER_COUNT]Register = [_]Register{0} ** REGISTER_COUNT,

    program: [INSTS_SIZE]Inst = [_]Inst{Inst.init(.nop, 0)} ** INSTS_SIZE,
    program_size: usize = 0,
    ip: usize = 0,

    memory: [MEMORY_SIZE]u8 = [_]u8{0} ** MEMORY_SIZE,
    memory_size: usize = 0,
    memory_start: usize,
    stack_begin: usize,
    stack_top: usize,

    natives: [NATIVE_SIZE]Native = [_]Native{undefined} ** NATIVE_SIZE,
    native_size: usize = 0,

    hlt: bool = false,

    pub fn init(stack_size: usize) Machine {
        var machine: Machine = Machine{
            .memory_start = stack_size,
            .stack_begin = stack_size - 1,
            .stack_top = stack_size - 1,
        };

        machine.natives[0] = nativeWrite;
        machine.natives[1] = nativeDebugPrint;
        machine.native_size = 2;

        return machine;
    }

    pub fn pushInst(self: *Machine, inst: Inst) void {
        if (self.program_size >= INSTS_SIZE) {
            @panic("[ERORR] can't push more instrutions. Instrution count exceded the limit");
        }

        self.program[self.program_size] = inst;
        self.program_size += 1;
    }

    pub fn pushNative(self: *Machine, native: Native) void {
        if (self.native_size >= NATIVE_SIZE) {
            @panic("[ERORR] can't push more natives. Native count exceded the limit");
        }

        self.natives[self.native_size] = native;
        self.native_size += 1;
    }

    pub fn executeInst(self: *Machine) Exception!void {
        const inst = self.program[self.ip];

        switch (inst.type) {
            InstType.nop => {
                self.ip += 1;
            },
            InstType.pushi => {
                var value = inst.operand0;
                self.stack_top -= 8;
                std.mem.writeInt(Word, self.memory[self.stack_top..][0..@sizeOf(Word)], value, .Little);
                self.ip += 1;
            },
            InstType.push => {
                var register = inst.operand0;
                self.stack_top -= 8;
                std.mem.writeInt(Word, self.memory[self.stack_top..][0..@sizeOf(Word)], register, .Little);
                self.ip += 1;
            },
            InstType.pop => {
                var register = inst.operand0;
                self.registers[register] =
                    std.mem.readInt(Word, self.memory[self.stack_top..][0..@sizeOf(Word)], .Little);
                self.stack_top += 8;
                self.ip += 1;
            },
            InstType.pusha => {
                @panic("Unimplemented");
            },
            InstType.popa => {
                @panic("Unimplemented");
            },
            InstType.enter => {
                @panic("Unimplemented");
            },
            InstType.leave => {
                @panic("Unimplemented");
            },
            InstType.dup => {
                var value = std.mem.readInt(Word, self.memory[self.stack_top..][0..@sizeOf(Word)], .Little);
                self.stack_top -= 8;
                std.mem.writeInt(Word, self.memory[self.stack_top..][0..@sizeOf(Word)], value, .Little);
                self.ip += 1;
            },
            InstType.mov => {
                @panic("Unimplemented");
            },
            InstType.inc => {
                var register = inst.operand0;
                self.registers[register] += u64FromWord(1);
                self.ip += 1;
            },
            InstType.incf => {
                var register = inst.operand0;
                self.registers[register] = wordFromF64(f64FromWord(self.registers[register]) + 1.0);
                self.ip += 1;
            },
            InstType.dec => {
                var register = inst.operand0;
                self.registers[register] -= u64FromWord(1);
                self.ip += 1;
            },
            InstType.decf => {
                var register = inst.operand0;
                self.registers[register] = wordFromF64(f64FromWord(self.registers[register]) - 1.0);
                self.ip += 1;
            },
            InstType.neg => {
                @panic("Unimplemented");
            },
            InstType.addi => {
                @panic("Unimplemented");
            },
            InstType.addu => {
                @panic("Unimplemented");
            },
            InstType.addf => {
                @panic("Unimplemented");
            },
            InstType.subi => {
                @panic("Unimplemented");
            },
            InstType.subu => {
                @panic("Unimplemented");
            },
            InstType.subf => {
                @panic("Unimplemented");
            },
            InstType.muli => {
                @panic("Unimplemented");
            },
            InstType.mulu => {
                @panic("Unimplemented");
            },
            InstType.mulf => {
                @panic("Unimplemented");
            },
            InstType.divi => {
                @panic("Unimplemented");
            },
            InstType.divu => {
                @panic("Unimplemented");
            },
            InstType.divf => {
                @panic("Unimplemented");
            },
            InstType.mod => {
                @panic("Unimplemented");
            },
            InstType.modf => {
                @panic("Unimplemented");
            },
            InstType.eq => {
                @panic("Unimplemented");
            },
            InstType.neq => {
                @panic("Unimplemented");
            },
            InstType.gti => {
                @panic("Unimplemented");
            },
            InstType.gtf => {
                @panic("Unimplemented");
            },
            InstType.gei => {
                @panic("Unimplemented");
            },
            InstType.gef => {
                @panic("Unimplemented");
            },
            InstType.sti => {
                @panic("Unimplemented");
            },
            InstType.stf => {
                @panic("Unimplemented");
            },
            InstType.sei => {
                @panic("Unimplemented");
            },
            InstType.sef => {
                @panic("Unimplemented");
            },
            InstType.andl => {
                @panic("Unimplemented");
            },
            InstType.orl => {
                @panic("Unimplemented");
            },
            InstType.notl => {
                @panic("Unimplemented");
            },
            InstType.andb => {
                @panic("Unimplemented");
            },
            InstType.orb => {
                @panic("Unimplemented");
            },
            InstType.xor => {
                @panic("Unimplemented");
            },
            InstType.notb => {
                @panic("Unimplemented");
            },
            InstType.shl => {
                @panic("Unimplemented");
            },
            InstType.shr => {
                @panic("Unimplemented");
            },
            InstType.rotl => {
                @panic("Unimplemented");
            },
            InstType.rotr => {
                @panic("Unimplemented");
            },
            InstType.jmp => {
                self.ip = inst.operand0;
            },
            InstType.jz => {
                @panic("Unimplemented");
            },
            InstType.jnz => {
                @panic("Unimplemented");
            },
            InstType.call => {
                @panic("Unimplemented");
            },
            InstType.native => {
                var native: Word = u64FromWord(self.registers[4]);
                try self.natives[native](self);
                self.ip += 1;
            },
            InstType.ret => {
                @panic("Unimplemented");
            },
            InstType.itf => {
                @panic("Unimplemented");
            },
            InstType.utf => {
                @panic("Unimplemented");
            },
            InstType.fti => {
                @panic("Unimplemented");
            },
            InstType.ftu => {
                @panic("Unimplemented");
            },
            InstType.ldi => {
                @panic("Unimplemented");
            },
            InstType.store8 => {
                @panic("Unimplemented");
            },
            InstType.load8 => {
                @panic("Unimplemented");
            },
            InstType.store16 => {
                @panic("Unimplemented");
            },
            InstType.load16 => {
                @panic("Unimplemented");
            },
            InstType.store32 => {
                @panic("Unimplemented");
            },
            InstType.load32 => {
                @panic("Unimplemented");
            },
            InstType.store64 => {
                @panic("Unimplemented");
            },
            InstType.load64 => {
                @panic("Unimplemented");
            },
            InstType.hlt => {
                self.hlt = true;
            },
            InstType.inst_max => {
                @panic("Unkown instruction");
            },
        }
    }

    pub fn run(self: *Machine, limit: isize) void {
        var l = limit;

        while (l != 0 and !self.hlt) {
            self.executeInst() catch |err| {
                @panic(@errorName(err));
            };

            l -= 1;
        }
    }

    //FIXME: rewrite it
    //TODO: add better errors
    pub fn saveToFile(self: Machine, path: []const u8) void {
        var file = std.fs.cwd().createFile(path, .{}) catch {
            @panic("[ERORR] error while saving program to binary file");
        };
        defer file.close();

        var buf_writer = std.io.bufferedWriter(file.writer());
        var out_stream = buf_writer.writer();
        defer buf_writer.flush() catch {
            @panic("[ERORR] error while saving program to binary file");
        };

        const meta_data = MetaData.init(self);

        out_stream.writeIntLittle(u32, meta_data.magic) catch {
            @panic("[ERORR] error while saving program to binary file");
        };

        out_stream.writeIntLittle(u16, meta_data.version) catch {
            @panic("[ERORR] error while saving program to binary file");
        };

        out_stream.writeIntLittle(u64, meta_data.program_size) catch {
            @panic("[ERORR] error while saving program to binary file");
        };

        out_stream.writeIntLittle(u64, meta_data.memory_size) catch {
            @panic("[ERORR] error while saving program to binary file");
        };

        for (self.program[0..self.program_size]) |inst| {
            out_stream.writeIntLittle(u8, @intFromEnum(inst.type)) catch {
                @panic("[ERORR] error while saving program to binary file");
            };

            out_stream.writeIntLittle(Word, inst.operand0) catch {
                @panic("[ERORR] error while saving program to binary file");
            };
        }
    }

    //FIXME: rewrite it
    //TODO: add error checking for file loading
    //TODO: add better errors
    pub fn loadFromFile(self: *Machine, path: []const u8) void {
        var file = std.fs.cwd().openFile(path, .{}) catch {
            @panic("[ERORR] error while loading program from binary file");
        };
        defer file.close();

        var buf_reader = std.io.bufferedReader(file.reader());
        var in_stream = buf_reader.reader();

        var meta_data: MetaData = undefined;

        meta_data.magic = in_stream.readIntLittle(u32) catch {
            @panic("[ERORR] error while loading program from binary file");
        };

        meta_data.version = in_stream.readIntLittle(u16) catch {
            @panic("[ERORR] error while loading program from binary file");
        };

        meta_data.program_size = in_stream.readIntLittle(u64) catch {
            @panic("[ERORR] error while loading program from binary file");
        };

        meta_data.memory_size = in_stream.readIntLittle(u64) catch {
            @panic("[ERORR] error while loading program from binary file");
        };

        self.program_size = meta_data.program_size;

        for (0..self.program_size) |i| {
            self.program[i].type = @enumFromInt(in_stream.readIntLittle(u8) catch {
                @panic("[ERORR] error while loading program from binary file");
            });

            self.program[i].operand0 = in_stream.readIntLittle(Word) catch {
                @panic("[ERORR] error while loading program from binary file");
            };
        }
    }

    fn nativeWrite(machine: *Machine) Exception!void {
        _ = machine;
        // std.debug.print("", .{});
    }

    fn nativeDebugPrint(machine: *Machine) Exception!void {
        std.debug.print("{}\n", .{machine.registers[0]});
    }
};
