const std = @import("std");
const lvm = @import("lvm");

pub fn main() !void {
    // var file = try std.fs.cwd().createFile("test.bin", .{});

    // var buf_writer = std.io.bufferedWriter(file.writer());
    // var out_stream = buf_writer.writer();

    // _ = try out_stream.writeInt(u32, 0x45564F4C, .Little);
    // try buf_writer.flush();
    // file.close();

    // file = try std.fs.cwd().openFile("test.bin", .{});
    // defer file.close();

    // var buf_reader = std.io.bufferedReader(file.reader());
    // var in_stream = buf_reader.reader();

    // var magic: u32 = try in_stream.readInt(u32, .Little);

    // std.debug.print("{}\n{}\n", .{ 0x45564F4C, magic });

    var machine: lvm.Machine = lvm.Machine.create(512);

    // machine.pushInst(lvm.Inst{ .type = lvm.InstType.ldi, .operand = ((0 << 62) | 5) });
    // machine.pushInst(lvm.Inst{ .type = lvm.InstType.print_debug, .operand = 0 });
    // machine.pushInst(lvm.Inst{ .type = lvm.InstType.hlt, .operand = 0 });

    // try machine.saveToFile("test.melf");

    try machine.loadFromFile("test.melf");

    machine.run(-1);
}
