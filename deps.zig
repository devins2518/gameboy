const std = @import("std");
const Pkg = std.build.Pkg;
const FileSource = std.build.FileSource;

pub const pkgs = struct {
    pub fn addAllTo(artifact: *std.build.LibExeObjStep) void {
    }
};

pub const exports = struct {
    pub const ziggyboy = Pkg{
        .name = "ziggyboy",
        .path = "src/main.zig",
    };
};
