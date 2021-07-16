const std = @import("std");
const builtin = std.builtin;

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const zig_part = b.addStaticLibrary("zigpart", "src/main.zig");
    zig_part.setTarget(target);
    zig_part.setBuildMode(mode);

    const exe = b.addExecutable("mmtex", null);
    exe.addCSourceFiles(&.{
        "src/font/dofont.c",
        "src/font/luafont.c",
        "src/font/texfont.c",
        "src/lang/hyphen.c",
        "src/lang/texlang.c",
        "src/lua/lcallbacklib.c",
        "src/lua/lfontlib.c",
        "src/lua/liolibext.c",
        "src/lua/llanglib.c",
        "src/lua/llualib.c",
        "src/lua/lnewtokenlib.c",
        "src/lua/lnodelib.c",
        "src/lua/lstatslib.c",
        "src/lua/lstrlibext.c",
        "src/lua/ltexiolib.c",
        "src/lua/ltexlib.c",
        "src/lua/luainit.c",
        "src/lua/luanode.c",
        "src/lua/luastuff.c",
        "src/lua/luatoken.c",
        "src/tex/align.c",
        "src/tex/arithmetic.c",
        "src/tex/buildpage.c",
        "src/tex/commands.c",
        "src/tex/conditional.c",
        "src/tex/directions.c",
        "src/tex/dumpdata.c",
        "src/tex/equivalents.c",
        "src/tex/errors.c",
        "src/tex/expand.c",
        "src/tex/extensions.c",
        "src/tex/filename.c",
        "src/tex/inputstack.c",
        "src/tex/linebreak.c",
        "src/tex/mainbody.c",
        "src/tex/maincontrol.c",
        "src/tex/mathcodes.c",
        "src/tex/memoryword.c",
        "src/tex/mlist.c",
        "src/tex/nesting.c",
        "src/tex/packaging.c",
        "src/tex/postlinebreak.c",
        "src/tex/primitive.c",
        "src/tex/printing.c",
        "src/tex/scanning.c",
        "src/tex/stringpool.c",
        "src/tex/texdeffont.c",
        "src/tex/texfileio.c",
        "src/tex/texmath.c",
        "src/tex/texnodes.c",
        "src/tex/textcodes.c",
        "src/tex/textoken.c",
        "src/utils/managed-sa.c",
        "src/utils/unistring.c",
        "src/utils/utils.c",
        "src/utils/web2c_utils.c",
        "src/luatex.c",
    }, &.{
        "-DLUA_COMPAT_5_2",
        "-DLUA_USE_POSIX",
        "-DLUA_USE_DLOPEN",
    });
    exe.defineCMacro("MMTEX_VERSION=\"20210515\"");
    exe.addIncludeDir("src");
    exe.linkLibC();
    exe.linkSystemLibrary("lua5.3");
    exe.linkSystemLibrary("zlib");
    exe.linkLibrary(zig_part);
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.disable_sanitize_c = true;
    switch (mode) {
        builtin.Mode.ReleaseFast => {
            exe.strip = true;
            exe.want_lto = true;
        },
        builtin.Mode.ReleaseSmall => {
            exe.strip = true;
        },
        else => {},
    }
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
