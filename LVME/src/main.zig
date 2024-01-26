const std = @import("std");
const lvm = @import("lvm");

pub fn main() !void {
    var machine: lvm.Machine = lvm.Machine.create(512);

    machine.pushInst(lvm.Inst{ .type = lvm.InstType.ldi, .operand = ((0 << 62) | 5) });
    machine.pushInst(lvm.Inst{ .type = lvm.InstType.print_debug, .operand = 0 });
    machine.pushInst(lvm.Inst{ .type = lvm.InstType.hlt, .operand = 25 });

    try machine.saveToFile("test.melf");

    try machine.loadFromFile("test.melf");

    // var file = try std.fs.cwd().openFile("test.bin", .{});
    // defer file.close();

    // var buffered = std.io.bufferedReader(file.reader());
    // var reader = buffered.reader();

    // machine.program_size = 3;

    // _ = try reader.read(std.mem.sliceAsBytes(machine.program[0..machine.program_size]));

    // std.debug.print("{any}\n", .{machine.program[0..machine.program_size]});
    // std.debug.print("{any}\n", .{std.mem.sliceAsBytes(machine.program[0..machine.program_size])});

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

    // var file = try std.fs.cwd().openFile("test.txt", .{});
    // defer file.close();

    // var buffered = std.io.bufferedReader(file.reader());
    // var reader = buffered.reader();
    // _ = reader;

    // var gp = std.heap.GeneralPurposeAllocator(.{}){};
    // defer _ = gp.deinit();
    // const allocator = gp.allocator();

    // const file_buffer = try file.readToEndAlloc(allocator, 5000);
    // defer allocator.free(file_buffer);

    // std.debug.print("count = {}, {s}\n", .{ file_buffer.len, file_buffer });
}
