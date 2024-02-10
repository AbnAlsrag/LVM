const std = @import("std");
const lvm = @import("lvm");
const Linker = lvm.Linker;
const Compiler = lvm.Compiler;
const Module = lvm.Module;
const Program = lvm.Program;

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator: std.mem.Allocator = gpa.allocator();

    var module1: Module = Module.init(allocator);
    defer module1.deinit();

    module1.pushInst(.{ .opcode = .push, .operand = .{ .word = 0xA } });
    module1.pushInst(.{ .opcode = .push, .operand = .{ .word = 0xA } });
    module1.pushInst(.{ .opcode = .add, .operand = .{ .word = 0x0 } });
    module1.pushInst(.{ .opcode = .print_debug, .operand = .{ .word = 0x0 } });
    module1.pushInst(.{ .opcode = .hlt, .operand = .{ .word = 0x0 } });

    module1.linked = true;
    // module1.computeSymbols();

    var module2: Module = Module.init(allocator);
    defer module2.deinit();

    // var prog_module: Module = Linker.link(allocator, &.{ module1, module2 });
    // defer prog_module.deinit();

    var prog: Program = module1.toProgram() catch unreachable;
    defer prog.deinit();

    const cnfg: lvm.Machine.Config = lvm.Machine.Config{ .allocator = allocator, .program = prog };

    var machine: lvm.Machine = lvm.Machine.init(cnfg);
    defer machine.deinit();

    const target: Compiler.Target = Compiler.x86_64_nasm_linux_target.target();
    const output: Compiler.Output = Compiler.compile(machine.program, target);
    defer allocator.free(output.code);

    std.debug.print("{s}\n", .{output.code});

    // try machine.loadFromFile("test.melf");

    machine.run(-1);
}
