const std = @import("std");
const lvm = @import("lvm");

pub fn main() !void {
    var machine: lvm.Machine = lvm.Machine.init(512);

    // machine.pushInst(lvm.Inst.init(.pushi, 5));
    // machine.pushInst(lvm.Inst.init(.pushi, 1));
    // machine.pushInst(lvm.Inst.init(.pop, 4));
    // machine.pushInst(lvm.Inst.init(.pop, 0));
    // machine.pushInst(lvm.Inst.init(.native, 0));
    // machine.pushInst(lvm.Inst.init(.hlt, 0));

    // machine.saveToFile("test.melf");

    try machine.loadFromFile("test.melf");

    machine.run(-1);
}
