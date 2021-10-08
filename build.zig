const std = @import("std");
const Sdk = @import("sdl/Sdk.zig");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});

    const sdk = Sdk.init(b);

    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("ziggyboy", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    sdk.link(exe, .static);
    exe.addPackage(sdk.getNativePackage("sdl2"));
    // exe.linkSystemLibrary("SDL2");
    // exe.linkSystemLibrary("c");
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

    const test_step = b.step("test", "Run all the tests");
    test_step.dependOn(&tests.step);
}
