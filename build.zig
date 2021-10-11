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
    exe.addPackage(sdk.getWrapperPackage("sdl2"));
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
    sdk.link(tests, .static);
    tests.addPackage(sdk.getWrapperPackage("sdl2"));

    const test_step = b.step("test", "Run all the tests");
    test_step.dependOn(&tests.step);
}
