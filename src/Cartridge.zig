const std = @import("std");
const Allocator = std.mem.Allocator;
const Self = @This();

a: *Allocator,
path: []const u8,
data: []u8,

pub fn init(path: []const u8, alloc: *Allocator) !Self {
    const f = std.os.open(path, std.os.O.RDWR, std.os.O.RDWR) catch |err| {
        std.debug.print("Could not create cartridge: {}\n", .{err});
        std.process.exit(1);
    };
    const file = std.fs.File{ .handle = f };
    const data = try file.readToEndAlloc(alloc, try file.getEndPos());
    // const stats = try file.stat();
    // const data = try std.os.mmap(null, stats.size, std.os.PROT.READ | std.os.PROT.WRITE, std.os.MAP.SHARED, f, 0);
    return Self{ .a = alloc, .path = path, .data = data };
}

pub fn deinit(self: Self) void {
    self.a.free(self.data);
}

pub fn getAddr(self: *Self, addr: u16) *u8 {
    return &self.data[addr];
}
