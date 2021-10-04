const Self = @This();

path: []const u8,

pub fn init(path: []const u8) Self {
    return .{ .path = path };
}
