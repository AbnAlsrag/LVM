const std = @import("std");
const mem = std.mem;

const MAGIC = 0x45564F4C;
const VERSION = 10;

const INSTS_SIZE = 1024;
const MEMORY_SIZE = 1024;
const REGISTER_COUNT = 3;

// LVM_OP_DUP,
// LVM_OP_SWAP,
// LVM_OP_INCF,
// LVM_OP_DECF,
// LVM_OP_ADDF,
// LVM_OP_SUBF,
// LVM_OP_MULTF,
// LVM_OP_DIVF,
// LVM_OP_MODF,
// LVM_OP_GTI,
// LVM_OP_GTF,
// LVM_OP_GEI,
// LVM_OP_GEF,
// LVM_OP_STI,
// LVM_OP_STF,
// LVM_OP_SEI,
// LVM_OP_SEF,
// LVM_OP_ANDB,
// LVM_OP_NOTB,
// LVM_OP_ORB,
// LVM_OP_XOR,
// LVM_OP_NATIVE,
// LVM_OP_I2F,
// LVM_OP_U2F,
// LVM_OP_F2U,
// LVM_OP_F2I,
// LVM_OP_READ8,
// LVM_OP_READ16,
// LVM_OP_READ32,
// LVM_OP_READ64,
// LVM_OP_WRITE8,
// LVM_OP_WRITE16,
// LVM_OP_WRITE32,
// LVM_OP_WRITE64,

pub const InstType = enum(u64) {
    nop,
    push,
    pop,
    inc,
    dec,
    add,
    sub,
    mul,
    div,
    mod,
    eq,
    neq,
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
    ret,
    ldi,
    hlt,
    print_debug,
};

pub fn getInstTypeName(inst_type: InstType) []const u8 {
    switch (inst_type) {
        InstType.nop => {
            return "nop";
        },
        InstType.push => {
            return "push";
        },
        InstType.pop => {
            return "pop";
        },
        InstType.inc => {
            return "inc";
        },
        InstType.dec => {
            return "dec";
        },
        InstType.add => {
            return "add";
        },
        InstType.sub => {
            return "sub";
        },
        InstType.mul => {
            return "mul";
        },
        InstType.div => {
            return "div";
        },
        InstType.mod => {
            return "mod";
        },
        InstType.eq => {
            return "eq";
        },
        InstType.neq => {
            return "neq";
        },
        InstType.andl => {
            return "andl";
        },
        InstType.orl => {
            return "orl";
        },
        InstType.notl => {
            return "notl";
        },
        InstType.andb => {
            return "andb";
        },
        InstType.orb => {
            return "orb";
        },
        InstType.xor => {
            return "xor";
        },
        InstType.notb => {
            return "notb";
        },
        InstType.shl => {
            return "shl";
        },
        InstType.shr => {
            return "shr";
        },
        InstType.rotl => {
            return "rotl";
        },
        InstType.rotr => {
            return "rotr";
        },
        InstType.jmp => {
            return "jmp";
        },
        InstType.jmpz => {
            return "jmpz";
        },
        InstType.jmpnz => {
            return "jmpnz";
        },
        InstType.call => {
            return "call";
        },
        InstType.ret => {
            return "ret";
        },
        InstType.ldi => {
            return "ldi";
        },
        InstType.hlt => {
            return "hlt";
        },
        InstType.print_debug => {
            return "print_debug";
        },
    }
}

pub const Inst = struct {
    type: InstType,
    operand: u64,
};

pub const Exception = error{};

const MetaData = packed struct {
    magic: u32,
    version: u16,
    program_size: u64,
    memory_size: u64,

    fn create(program_size: u64, memory_size: u64) MetaData {
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
    stack_top: usize,

    hlt: bool,

    pub fn create(stack_size: usize) Machine {
        return Machine{
            .registers = [_]u64{0} ** REGISTER_COUNT,
            .program = [_]Inst{Inst{ .type = InstType.nop, .operand = 0 }} ** INSTS_SIZE,
            .program_size = 0,
            .ip = 0,
            .memory = [_]u8{0} ** MEMORY_SIZE,
            .stack_top = stack_size - 1,
            .hlt = false,
        };
    }

    pub fn pushInst(self: *Machine, inst: Inst) void {
        self.program[self.program_size] = inst;
        self.program_size += 1;
    }

    pub fn executeInst(self: *Machine) void {
        const inst = self.program[self.ip];

        switch (inst.type) {
            InstType.nop => {
                self.ip += 1;
            },
            InstType.push => {
                if (inst.operand >> 63 == 0) {
                    const p: []u8 = self.memory[0..self.memory.len];
                    @as([*]u64, @ptrCast(@alignCast(p)))[self.stack_top] = (inst.operand << 1) >> 1;
                } else {
                    const p: []u8 = self.memory[0..self.memory.len];
                    @as([*]u64, @ptrCast(@alignCast(p)))[self.stack_top] =
                        self.registers[(inst.operand << 1) >> 1];
                }
                self.stack_top -= 8;
                self.ip += 1;
            },
            InstType.pop => {
                self.stack_top += 8;
                const p: []u8 = self.memory[0..self.memory.len];
                self.registers[inst.operand] = @as([*]u64, @ptrCast(@alignCast(p)))[self.stack_top];
                self.ip += 1;
            },
            InstType.inc => {
                self.registers[0] += 1;
                self.ip += 1;
            },
            InstType.dec => {
                self.registers[0] -= 1;
                self.ip += 1;
            },
            InstType.add => {
                self.registers[0] = self.registers[0] + self.registers[1];
                self.ip += 1;
            },
            InstType.sub => {
                self.registers[0] = self.registers[0] - self.registers[1];
                self.ip += 1;
            },
            InstType.mul => {
                self.registers[0] = self.registers[0] * self.registers[1];
                self.ip += 1;
            },
            InstType.div => {
                self.registers[0] = self.registers[0] / self.registers[1];
                self.ip += 1;
            },
            InstType.mod => {
                self.registers[0] = self.registers[0] % self.registers[1];
                self.ip += 1;
            },
            InstType.eq => {
                self.registers[0] = @intFromBool(self.registers[0] == self.registers[1]);
                self.ip += 1;
            },
            InstType.neq => {
                self.registers[0] = @intFromBool(self.registers[0] != self.registers[1]);
                self.ip += 1;
            },
            InstType.andl => {
                self.registers[0] = @intFromBool(self.registers[0] != 0 and self.registers[1] != 0);
                self.ip += 1;
            },
            InstType.orl => {
                self.registers[0] = @intFromBool(self.registers[0] != 0 or self.registers[1] != 0);
                self.ip += 1;
            },
            InstType.notl => {
                self.registers[0] = @intFromBool(!(self.registers[0] != 0));
                self.ip += 1;
            },
            InstType.andb => {
                self.registers[0] = self.registers[0] & self.registers[1];
                self.ip += 1;
            },
            InstType.orb => {
                self.registers[0] = self.registers[0] | self.registers[1];
                self.ip += 1;
            },
            InstType.xor => {
                self.registers[0] = self.registers[0] ^ self.registers[1];
                self.ip += 1;
            },
            InstType.notb => {
                self.registers[0] = ~self.registers[0];
                self.ip += 1;
            },
            InstType.shl => {
                self.registers[0] = self.registers[0] << @as(u6, @intCast(self.registers[1]));
                self.ip += 1;
            },
            InstType.shr => {
                self.registers[0] = self.registers[0] >> @as(u6, @intCast(self.registers[1]));
                self.ip += 1;
            },
            InstType.rotl => {
                var x = self.registers[0];
                var n = self.registers[1];

                if (!((n & 63) != 0) == false) {
                    n &= 63;
                    self.registers[0] = x << @as(u6, @intCast(n)) | x >> @as(u6, @intCast((64 - n)));
                }

                self.ip += 1;
            },
            InstType.rotr => {
                var x = self.registers[0];
                var n = self.registers[1];

                if (!((n & 63) != 0) == false) {
                    n &= 63;
                    self.registers[0] = x >> @as(u6, @intCast(n)) | x << @as(u6, @intCast((64 - n)));
                }

                self.ip += 1;
            },
            InstType.jmp => {
                self.ip = inst.operand;
            },
            InstType.jmpz => {
                if (self.registers[0] != 0) {
                    self.ip += 1;
                } else {
                    self.ip = inst.operand;
                }
            },
            InstType.jmpnz => {
                if (self.registers[0] == 0) {
                    self.ip += 1;
                } else {
                    self.ip = inst.operand;
                }
            },
            InstType.call => {
                const p: []u8 = self.memory[0..self.memory.len];
                @as([*]u64, @ptrCast(@alignCast(p)))[self.stack_top] = self.ip + 1;
                self.stack_top -= 8;
                self.ip = inst.operand;
            },
            InstType.ret => {
                self.ip = self.registers[0];
            },
            InstType.ldi => {
                const register: usize = inst.operand >> 62;
                const operand: u64 = @intCast((inst.operand << 2) >> 2);
                self.registers[register] = operand;
                self.ip += 1;
            },
            InstType.hlt => {
                self.hlt = true;
                self.ip += 1;
            },
            InstType.print_debug => {
                std.debug.print("{}\n", .{self.registers[inst.operand]});
                self.ip += 1;
            },
        }
    }

    pub fn run(self: *Machine, limit: isize) void {
        var l = limit;

        while (l != 0 and !self.hlt) {
            self.executeInst();

            l -= 1;
        }
    }

    pub fn saveToFile(self: *Machine, path: []const u8) !void {
        var file = try std.fs.cwd().createFile(path, .{});
        defer file.close();

        var buffered = std.io.bufferedWriter(file.writer());

        var meta_data = MetaData.create(self.program_size, 0);
        var meta: [@sizeOf(MetaData)]u8 = @as([*]u8, @ptrCast(&meta_data))[0..@sizeOf(MetaData)].*;

        const prog: []const u8 = std.mem.sliceAsBytes(self.program[0..self.program_size]);

        _ = try buffered.write(meta[0..meta.len]);
        _ = try buffered.write(prog);

        try buffered.flush();
    }

    //TODO: add error checking for file loading
    pub fn loadFromFile(self: *Machine, path: []const u8) !void {
        _ = path;
        _ = self;
        var file = try std.fs.cwd().openFile("test.bin", .{});
        defer file.close();

        var buffered = std.io.bufferedReader(file.reader());
        var reader = buffered.reader();

        var meta_data: MetaData = undefined;

        _ = try reader.read(std.mem.asBytes(&meta_data.magic));

        std.debug.print("{}, ({})\n", .{ meta_data, MAGIC });
        std.debug.print("{any}\n", .{std.mem.asBytes(&meta_data).*});

        std.debug.assert(meta_data.magic == MAGIC);

        // _ = try reader.read(std.mem.sliceAsBytes(self.program[0..meta_data.program_size]));

        // std.debug.print("{any}\n", .{self.program[0..meta_data.program_size]});
        // std.debug.print("{any}\n", .{std.mem.sliceAsBytes(self.program[0..meta_data.program_size])});
    }
};
