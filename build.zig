const std = @import("std");
const pkgs = @import("deps.zig").pkgs;

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});

    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("rustyboy", "src/main.zig");
    pkgs.addAllTo(exe);
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const tests = b.addTest("src/main.zig");
    tests.setBuildMode(mode);
    pkgs.addAllTo(tests);

    const test_step = b.step("test", "Run all the tests");
    test_step.dependOn(&tests.step);
}
