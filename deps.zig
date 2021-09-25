const std = @import("std");
pub const pkgs = struct {
    pub const SDL = std.build.Pkg{
        .name = "SDL",
        .path = ".gyro/SDL.zig-MasterQ32-5118ef94e93d35cbf7888cb71472fa1c18fadac7/pkg/src/lib.zig",
    };

    pub fn addAllTo(artifact: *std.build.LibExeObjStep) void {
        @setEvalBranchQuota(1_000_000);
        inline for (std.meta.declarations(pkgs)) |decl| {
            if (decl.is_pub and decl.data == .Var) {
                artifact.addPackage(@field(pkgs, decl.name));
            }
        }
    }
};

pub const exports = struct {
    pub const ziggyboy = std.build.Pkg{
        .name = "ziggyboy",
        .path = "src/main.zig",
        .dependencies = &.{
            pkgs.SDL,
        },
    };
};
pub const base_dirs = struct {
    pub const SDL = ".gyro/SDL.zig-MasterQ32-5118ef94e93d35cbf7888cb71472fa1c18fadac7/pkg";
};
