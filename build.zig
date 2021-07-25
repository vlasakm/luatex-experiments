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

    const mmtex_inc = "include";

    const zlib = b.addStaticLibrary("zlib", null);
    zlib.addCSourceFiles(&.{
        "zlib/inftrees.c",
        "zlib/inflate.c",
        "zlib/adler32.c",
        "zlib/zutil.c",
        "zlib/trees.c",
        "zlib/gzclose.c",
        "zlib/gzwrite.c",
        "zlib/gzread.c",
        "zlib/deflate.c",
        "zlib/compress.c",
        "zlib/crc32.c",
        "zlib/infback.c",
        "zlib/gzlib.c",
        "zlib/uncompr.c",
        "zlib/inffast.c",
    }, &.{});

    zlib.setTarget(target);
    zlib.setBuildMode(mode);
    zlib.addIncludeDir("libpng");
    zlib.linkLibC();

    const libpng = b.addStaticLibrary("libpng", null);
    libpng.addCSourceFiles(&.{
        "libpng/png.c",
        "libpng/pngerror.c",
        "libpng/pngget.c",
        "libpng/pngmem.c",
        "libpng/pngpread.c",
        "libpng/pngread.c",
        "libpng/pngrio.c",
        "libpng/pngrtran.c",
        "libpng/pngrutil.c",
        "libpng/pngset.c",
        "libpng/pngtrans.c",
        "libpng/pngwio.c",
        "libpng/pngwrite.c",
        "libpng/pngwtran.c",
        "libpng/pngwutil.c",

        // "libpng/arm/arm_init.c",
        // "libpng/arm/palette_neon_intrinsics.c",
        // "libpng/arm/filter_neon.S",
        // "libpng/arm/filter_neon_intrinsics.c",
    }, &.{});

    libpng.setTarget(target);
    libpng.setBuildMode(mode);
    libpng.addIncludeDir("libpng");
    libpng.addIncludeDir("zlib");
    libpng.linkLibC();

    const kpathsea = b.addStaticLibrary("kpathsea", null);
    kpathsea.addCSourceFiles(&.{
        "src/kpathsea/kpathsea/absolute.c",
        "src/kpathsea/kpathsea/atou.c",
        "src/kpathsea/kpathsea/cnf.c",
        "src/kpathsea/kpathsea/concat.c",
        "src/kpathsea/kpathsea/concat3.c",
        "src/kpathsea/kpathsea/concatn.c",
        "src/kpathsea/kpathsea/db.c",
        "src/kpathsea/kpathsea/debug.c",
        "src/kpathsea/kpathsea/dir.c",
        "src/kpathsea/kpathsea/elt-dirs.c",
        "src/kpathsea/kpathsea/expand.c",
        "src/kpathsea/kpathsea/extend-fname.c",
        "src/kpathsea/kpathsea/file-p.c",
        "src/kpathsea/kpathsea/find-suffix.c",
        "src/kpathsea/kpathsea/fn.c",
        "src/kpathsea/kpathsea/fontmap.c",
        "src/kpathsea/kpathsea/hash.c",
        "src/kpathsea/kpathsea/kdefault.c",
        "src/kpathsea/kpathsea/kpathsea.c",
        "src/kpathsea/kpathsea/line.c",
        "src/kpathsea/kpathsea/magstep.c",
        "src/kpathsea/kpathsea/make-suffix.c",
        "src/kpathsea/kpathsea/path-elt.c",
        "src/kpathsea/kpathsea/pathsearch.c",
        "src/kpathsea/kpathsea/proginit.c",
        "src/kpathsea/kpathsea/progname.c",
        "src/kpathsea/kpathsea/readable.c",
        "src/kpathsea/kpathsea/rm-suffix.c",
        "src/kpathsea/kpathsea/str-list.c",
        "src/kpathsea/kpathsea/str-llist.c",
        "src/kpathsea/kpathsea/tex-file.c",
        "src/kpathsea/kpathsea/tex-glyph.c",
        "src/kpathsea/kpathsea/tex-hush.c",
        "src/kpathsea/kpathsea/tex-make.c",
        "src/kpathsea/kpathsea/tilde.c",
        "src/kpathsea/kpathsea/uppercasify.c",
        "src/kpathsea/kpathsea/variable.c",
        "src/kpathsea/kpathsea/version.c",
        "src/kpathsea/kpathsea/xbasename.c",
        "src/kpathsea/kpathsea/xcalloc.c",
        "src/kpathsea/kpathsea/xdirname.c",
        "src/kpathsea/kpathsea/xfopen.c",
        "src/kpathsea/kpathsea/xfseek.c",
        "src/kpathsea/kpathsea/xfseeko.c",
        "src/kpathsea/kpathsea/xftell.c",
        "src/kpathsea/kpathsea/xftello.c",
        "src/kpathsea/kpathsea/xgetcwd.c",
        "src/kpathsea/kpathsea/xmalloc.c",
        "src/kpathsea/kpathsea/xopendir.c",
        "src/kpathsea/kpathsea/xputenv.c",
        "src/kpathsea/kpathsea/xrealloc.c",
        "src/kpathsea/kpathsea/xstat.c",
        "src/kpathsea/kpathsea/xstrdup.c",

        //"src/kpathsea/kpathsea/mingw32.c",
        //"src/kpathsea/kpathsea/knj.c",
    }, &.{
        "-DMAKE_KPSE_DLL",
    });

    kpathsea.setTarget(target);
    kpathsea.setBuildMode(mode);
    kpathsea.addIncludeDir(mmtex_inc);
    kpathsea.addIncludeDir("src/kpathsea");
    kpathsea.addIncludeDir("src/kpathsea/kpathsea");
    kpathsea.linkLibC();

    const w2c = b.addStaticLibrary("w2c", null);
    w2c.addCSourceFiles(&.{
        "src/lib/lib/openclose.c",
        "src/lib/lib/uexit.c",
        "src/lib/lib/usage.c",
    }, &.{
        "-DHAVE_CONFIG_H",
    });

    w2c.setTarget(target);
    w2c.setBuildMode(mode);
    w2c.addIncludeDir(mmtex_inc);
    w2c.addIncludeDir(mmtex_inc ++ "/w2c");
    w2c.addIncludeDir("src/lib");
    w2c.addIncludeDir("src/kpathsea");
    w2c.linkLibC();

    const lua = b.addStaticLibrary("lua", null);
    lua.addCSourceFiles(&.{
        // core
        "src/lua/src/lapi.c",
        "src/lua/src/lcode.c",
        "src/lua/src/lctype.c",
        "src/lua/src/ldebug.c",
        "src/lua/src/ldo.c",
        "src/lua/src/ldump.c",
        "src/lua/src/lfunc.c",
        "src/lua/src/lgc.c",
        "src/lua/src/llex.c",
        "src/lua/src/lmem.c",
        "src/lua/src/lobject.c",
        "src/lua/src/lopcodes.c",
        "src/lua/src/lparser.c",
        "src/lua/src/lstate.c",
        "src/lua/src/lstring.c",
        "src/lua/src/ltable.c",
        "src/lua/src/ltm.c",
        "src/lua/src/lundump.c",
        "src/lua/src/lvm.c",
        "src/lua/src/lzio.c",
        // lib
        "src/lua/src/lauxlib.c",
        "src/lua/src/lbaselib.c",
        "src/lua/src/lbitlib.c",
        "src/lua/src/lcorolib.c",
        "src/lua/src/ldblib.c",
        "src/lua/src/liolib.c",
        "src/lua/src/lmathlib.c",
        "src/lua/src/loslib.c",
        "src/lua/src/lstrlib.c",
        "src/lua/src/ltablib.c",
        "src/lua/src/lutf8lib.c",
        "src/lua/src/loadlib.c",
        "src/lua/src/linit.c",
    }, &.{
        "-DLUA_COMPAT_5_2",
        "-DLUA_COMPAT_MODULE",
        //"-DLUA_USE_POSIX",
        //"-DLUA_USE_DLOPEN",
    });

    lua.setTarget(target);
    lua.setBuildMode(mode);
    lua.linkLibC();
    lua.setTarget(target);
    lua.setBuildMode(mode);

    const mplib = b.addStaticLibrary("mplib", null);
    mplib.addCSourceFiles(&.{
        "src/mplib/src/mpmathdecimal.c",
        "src/mplib/src/mpmathdouble.c",
        "src/mplib/src/mpmath.c",
        "src/mplib/src/mpstrings.c",
        "src/mplib/src/mp.c",
        "src/mplib/src/psout.c",
        "src/mplib/src/tfmin.c",
        "src/mplib/src/avl.c",
        "src/mplib/src/decNumber.c",
        "src/mplib/src/decContext.c",
    }, &.{});

    mplib.setTarget(target);
    mplib.setBuildMode(mode);
    mplib.addIncludeDir(mmtex_inc);
    mplib.addIncludeDir("src/mplib/src");
    mplib.addIncludeDir("src/kpathsea");
    mplib.linkLibC();
    //mplib.linkSystemLibrary("libpng");
    //mplib.linkSystemLibrary("zlib");
    mplib.linkLibrary(zlib);
    mplib.addIncludeDir("zlib");
    mplib.linkLibrary(libpng);
    mplib.addIncludeDir("libpng");

    const pplib = b.addStaticLibrary("pplib", null);
    pplib.addCSourceFiles(&.{
        "src/pplib/src/pparray.c",
        "src/pplib/src/ppcrypt.c",
        "src/pplib/src/ppdict.c",
        "src/pplib/src/ppheap.c",
        "src/pplib/src/ppload.c",
        "src/pplib/src/ppstream.c",
        "src/pplib/src/ppxref.c",
        "src/pplib/src/util/utilbasexx.c",
        "src/pplib/src/util/utilcrypt.c",
        "src/pplib/src/util/utilflate.c",
        "src/pplib/src/util/utilfpred.c",
        "src/pplib/src/util/utiliof.c",
        "src/pplib/src/util/utillog.c",
        "src/pplib/src/util/utillzw.c",
        "src/pplib/src/util/utilmd5.c",
        "src/pplib/src/util/utilmem.c",
        "src/pplib/src/util/utilmemheap.c",
        "src/pplib/src/util/utilmemheapiof.c",
        "src/pplib/src/util/utilmeminfo.c",
        "src/pplib/src/util/utilnumber.c",
        "src/pplib/src/util/utilsha.c",
    }, &.{});

    pplib.setTarget(target);
    pplib.setBuildMode(mode);
    pplib.addIncludeDir("src/pplib/src");
    pplib.addIncludeDir("src/pplib/src/util");
    pplib.linkLibC();
    //pplib.linkSystemLibrary("zlib");
    pplib.linkLibrary(zlib);
    pplib.addIncludeDir("zlib");

    const exe = b.addExecutable("mmtex", null);
    exe.addCSourceFiles(&.{
        "src/luatex/src/luafilesystem/src/lfs.c",
        "src/luatex/src/luamd5/md5.c",
        "src/luatex/src/luamd5/md5lib.c",
        "src/luatex/src/luapeg/lpeg.c",
        "src/luatex/src/luazlib/lgzip.c",
        "src/luatex/src/luazlib/lzlib.c",
        "src/luatex/src/slnunicode/slnunico.c",
        "src/luatex/src/unilib/alphabet.c",
        "src/luatex/src/unilib/ArabicForms.c",
        "src/luatex/src/unilib/char.c",
        "src/luatex/src/unilib/cjk.c",
        "src/luatex/src/unilib/gwwiconv.c",
        "src/luatex/src/unilib/ucharmap.c",
        "src/luatex/src/unilib/unialt.c",
        "src/luatex/src/unilib/usprintf.c",
        "src/luatex/src/unilib/ustring.c",
        "src/luatex/src/unilib/utype.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/autohint.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/clipnoui.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/cvundoes.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/dumppfa.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/encoding.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/featurefile.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/fontviewbase.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/fvcomposit.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/fvfonts.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/lookups.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/macbinary.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/macenc.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/mathconstants.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/memory.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/mm.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/namelist.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/noprefs.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/nouiutil.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/parsepfa.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/parsettf.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/parsettfatt.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/psread.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/pua.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/python.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/sfd1.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinechar.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinefill.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinefont.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splineorder2.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splineoverlap.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinerefigure.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinesave.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinesaveafm.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splinestroke.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splineutil.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/splineutil2.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/start.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/stemdb.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/tottf.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/tottfgpos.c",
        "src/luatex/src/luafontloader/fontforge/fontforge/ttfspecial.c",
        "src/luatex/src/luafontloader/fontforge/gutils/fsys.c",
        "src/luatex/src/lua/lstrlibext.c",
        "src/luatex/src/lua/helpers.c",
        "src/luatex/src/lua/texluac.c",
        "src/luatex/src/utils/utils.c",
        "src/luatex/src/lua/luastuff.c",
        "src/luatex/src/lua/luainit.c",
        "src/luatex/src/tex/printing.c",
        "src/luatex/src/luafontloader/src/ffdummies.c",
        "src/luatex/src/luafontloader/src/luafflib.c",
        "src/luatex/src/dvi/dvigen.c",
        "src/luatex/src/font/dofont.c",
        "src/luatex/src/font/luafont.c",
        "src/luatex/src/font/mapfile.c",
        "src/luatex/src/font/pkin.c",
        "src/luatex/src/font/sfnt.c",
        "src/luatex/src/font/texfont.c",
        "src/luatex/src/font/tfmofm.c",
        "src/luatex/src/font/tounicode.c",
        "src/luatex/src/font/tt_glyf.c",
        "src/luatex/src/font/tt_table.c",
        "src/luatex/src/font/vfovf.c",
        "src/luatex/src/font/vfpacket.c",
        "src/luatex/src/font/writecff.c",
        "src/luatex/src/font/writeenc.c",
        "src/luatex/src/font/writefont.c",
        "src/luatex/src/font/writet1.c",
        "src/luatex/src/font/writet3.c",
        "src/luatex/src/font/writettf.c",
        "src/luatex/src/font/writetype0.c",
        "src/luatex/src/font/writetype2.c",
        "src/luatex/src/image/pdftoepdf.c",
        "src/luatex/src/image/writeimg.c",
        "src/luatex/src/image/writejbig2.c",
        "src/luatex/src/image/writejp2.c",
        "src/luatex/src/image/writejpg.c",
        "src/luatex/src/image/writepng.c",
        "src/luatex/src/lang/hnjalloc.c",
        "src/luatex/src/lang/hyphen.c",
        "src/luatex/src/lang/texlang.c",
        "src/luatex/src/lua/lcallbacklib.c",
        "src/luatex/src/lua/lfontlib.c",
        "src/luatex/src/lua/limglib.c",
        "src/luatex/src/lua/lpdfelib.c",
        "src/luatex/src/lua/lpdfscannerlib.c",
        "src/luatex/src/lua/lkpselib.c",
        "src/luatex/src/lua/llanglib.c",
        "src/luatex/src/lua/llualib.c",
        "src/luatex/src/lua/lnodelib.c",
        "src/luatex/src/lua/liolibext.c",
        "src/luatex/src/lua/loslibext.c",
        "src/luatex/src/lua/lpdflib.c",
        "src/luatex/src/lua/lstatslib.c",
        "src/luatex/src/lua/ltexiolib.c",
        "src/luatex/src/lua/ltexlib.c",
        "src/luatex/src/lua/lnewtokenlib.c",
        "src/luatex/src/lua/luatex-core.c",
        "src/luatex/src/lua/luanode.c",
        "src/luatex/src/lua/luatoken.c",
        "src/luatex/src/lua/mplibstuff.c",
        "src/luatex/src/pdf/pdfaction.c",
        "src/luatex/src/pdf/pdfannot.c",
        "src/luatex/src/pdf/pdfcolorstack.c",
        "src/luatex/src/pdf/pdfdest.c",
        "src/luatex/src/pdf/pdffont.c",
        "src/luatex/src/pdf/pdfgen.c",
        "src/luatex/src/pdf/pdfglyph.c",
        "src/luatex/src/pdf/pdfimage.c",
        "src/luatex/src/pdf/pdflink.c",
        "src/luatex/src/pdf/pdflistout.c",
        "src/luatex/src/pdf/pdfliteral.c",
        "src/luatex/src/pdf/pdfobj.c",
        "src/luatex/src/pdf/pdfoutline.c",
        "src/luatex/src/pdf/pdfpage.c",
        "src/luatex/src/pdf/pdfpagetree.c",
        "src/luatex/src/pdf/pdfrule.c",
        "src/luatex/src/pdf/pdfsaverestore.c",
        "src/luatex/src/pdf/pdfsetmatrix.c",
        "src/luatex/src/pdf/pdfshipout.c",
        "src/luatex/src/pdf/pdftables.c",
        "src/luatex/src/pdf/pdfthread.c",
        "src/luatex/src/pdf/pdfxform.c",
        "src/luatex/src/tex/backend.c",
        "src/luatex/src/tex/align.c",
        "src/luatex/src/tex/arithmetic.c",
        "src/luatex/src/tex/buildpage.c",
        "src/luatex/src/tex/commands.c",
        "src/luatex/src/tex/conditional.c",
        "src/luatex/src/tex/directions.c",
        "src/luatex/src/tex/dumpdata.c",
        "src/luatex/src/tex/equivalents.c",
        "src/luatex/src/tex/errors.c",
        "src/luatex/src/tex/expand.c",
        "src/luatex/src/tex/extensions.c",
        "src/luatex/src/tex/filename.c",
        "src/luatex/src/tex/inputstack.c",
        "src/luatex/src/tex/linebreak.c",
        "src/luatex/src/tex/mainbody.c",
        "src/luatex/src/tex/maincontrol.c",
        "src/luatex/src/tex/mathcodes.c",
        "src/luatex/src/tex/memoryword.c",
        "src/luatex/src/tex/mlist.c",
        "src/luatex/src/tex/nesting.c",
        "src/luatex/src/tex/packaging.c",
        "src/luatex/src/tex/postlinebreak.c",
        "src/luatex/src/tex/primitive.c",
        "src/luatex/src/tex/scanning.c",
        "src/luatex/src/tex/stringpool.c",
        "src/luatex/src/tex/texdeffont.c",
        "src/luatex/src/tex/texfileio.c",
        "src/luatex/src/tex/texmath.c",
        "src/luatex/src/tex/texnodes.c",
        "src/luatex/src/tex/textcodes.c",
        "src/luatex/src/tex/textoken.c",
        "src/luatex/src/utils/avl.c",
        "src/luatex/src/utils/avlstuff.c",
        "src/luatex/src/utils/managed-sa.c",
        "src/luatex/src/utils/unistring.c",
        "src/synctex/src/synctex.c",
        "src/luatex/src/luatex.c",
        "src/mplib/src/lmplib.c",
        "src/libmd5/src/md5.c",
    }, &.{
        "-DLUA_FF_LIB",
        "-D_NO_PYTHON",
        "-DX_DISPLAY_MISSING",
        "-DUSE_OUR_MEMORY",
        "-DSYNCTEX_ENGINE_H=<synctex-luatex.h>",
        "-DHAVE_CONFIG_H",
        "-DLUA_COMPAT_5_2",
        "-DLUA_COMPAT_MODULE",
        //"-DLUA_USE_POSIX",
        //"-DLUA_USE_DLOPEN",
    });

    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.addIncludeDir(mmtex_inc);
    exe.addIncludeDir(mmtex_inc ++ "/w2c");
    exe.addIncludeDir("src");
    exe.addIncludeDir("src/lib");
    exe.addIncludeDir("src/mplib/src");
    exe.addIncludeDir("src/lua/src");
    exe.addIncludeDir("src/kpathsea");
    exe.addIncludeDir("src/pplib/src");
    exe.addIncludeDir("src/pplib/src/util");
    exe.addIncludeDir("src/luatex/src/unilib");
    exe.addIncludeDir("src/luatex/src");
    exe.addIncludeDir("src/luatex/src/luafontloader/fontforge/inc");
    exe.addIncludeDir("src/luatex/src/luafontloader/fontforge/fontforge");
    exe.addIncludeDir("src/libmd5/src");
    exe.addIncludeDir("src/synctex/src");
    exe.linkLibC();
    exe.linkLibrary(kpathsea);
    exe.linkLibrary(w2c);
    exe.linkLibrary(lua);
    exe.linkLibrary(mplib);
    exe.linkLibrary(pplib);
    //exe.linkSystemLibrary("zlib");
    //exe.linkSystemLibrary("libpng");
    exe.linkLibrary(zlib);
    exe.addIncludeDir("zlib");
    exe.linkLibrary(libpng);
    exe.addIncludeDir("libpng");
    exe.defineCMacro("MMTEX_VERSION", "\"20210515\"");

    exe.disable_sanitize_c = true;
    switch (mode) {
        builtin.Mode.ReleaseFast => {
            exe.strip = true;
            //exe.want_lto = true;
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
