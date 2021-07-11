/*

luainit.w

Copyright 2006-2021 Taco Hoekwater <taco@@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

#include "lua/luatex-api.h"

#include <locale.h>

// TODO(mvlasak): added by me, move to ptexlib?
#include <sys/stat.h>

#include <getopt.h>

extern int load_luatex_core_lua (lua_State * L);

/*tex internalized strings: see luatex-api.h */

set_make_keys;

const_string LUATEX_IHELP[] = {
    "Usage: " my_name " --lua=FILE [OPTION]... [TEXNAME[.tex]] [COMMANDS]",
    "   or: " my_name " --lua=FILE [OPTION]... \\FIRST-LINE",
    "   or: " my_name " --lua=FILE [OPTION]... &FMT ARGS",
    "  Run " MyName " on TEXNAME, usually creating TEXNAME.pdf.",
    "  Any remaining COMMANDS are processed as luatex input, after TEXNAME is read.",
    "",
    "  Alternatively, if the first non-option argument begins with a backslash,",
    "  " my_name " interprets all non-option arguments as an input line.",
    "",
    "  Alternatively, if the first non-option argument begins with a &, the",
    "  next word is taken as the FMT to read, overriding all else.  Any",
    "  remaining arguments are processed as above.",
    "",
    "  If no arguments or options are specified, prompt for input.",
    "",
    "  The following regular options are understood: ",
    "",
    "   --credits                     display credits and exit",
    "   --debug-format                enable format debugging",
    "   --[no-]file-line-error        disable/enable file:line:error style messages",
    "   --[no-]file-line-error-style  aliases of --[no-]file-line-error",
    "   --fmt=FORMAT                  load the format file FORMAT",
    "   --halt-on-error               stop processing at the first error",
    "   --help                        display help and exit",
    "   --ini                         be ini" my_name ", for dumping formats",
    "   --interaction=STRING          set interaction mode (STRING=batchmode/nonstopmode/scrollmode/errorstopmode)",
    "   --jobname=STRING              set the job name to STRING",
    "   --lua=FILE                    load and execute a lua initialization script",
    "   --[no-]mktex=FMT              disable/enable mktexFMT generation (FMT=tex/tfm)",
    "   --nosocket                    disable the lua socket library",
    "   --progname=STRING             set the program name to STRING",
    "   --recorder                    enable filename recorder",
    "   --safer                       disable easily exploitable lua commands",
    "   --[no-]shell-escape           disable/enable system commands",
    "   --utc                         init time to UTC",
    "   --version                     display version and exit",
    "",
    "Alternate behaviour models can be obtained by special switches",
    "",
    "  --luaonly                      run a lua file, then exit",
    "  --luaconly                     byte-compile a lua file, then exit",
    "  --luahashchars                 the bits used by current Lua interpreter for strings hashing",
#ifdef LuajitTeX
    "  --jiton                        turns the JIT compiler on (default off)",
    "  --jithash=STRING               choose the hash function for the lua strings (lua51|luajit20: default lua51)",
#endif
    "",
    "See the reference manual for more information about the startup process.",
    NULL
};

/*tex

Later we will put on environment |LC_CTYPE|, |LC_COLLATE| and |LC_NUMERIC| set to
|C|, so we need a place where to store the old values.

*/

const char *lc_ctype;
const char *lc_collate;
const char *lc_numeric;

/*
    "   --8bit                        ignored, input is assumed to be in UTF-8 encoding",
    "   --default-translate-file=FILE ignored, input is assumed to be in UTF-8 encoding",
    "   --etex                        ignored, the etex extensions are always active",
    "   --disable-write18             disable \\write18{SHELL COMMAND}",
    "   --enable-write18              enable \\write18{SHELL COMMAND}",
    "   --[no-]parse-first-line       ignored",
    "   --translate-file=FILE         ignored, input is assumed to be in UTF-8 encoding",
*/

static void prepare_cmdline(lua_State * L, char **av, int ac, int zero_offset)
{
    int i;
    char *s;
    luaL_checkstack(L, ac + 3, "too many arguments to script");
    lua_createtable(L, 0, 0);
    for (i = 0; i < ac; i++) {
        lua_pushstring(L, av[i]);
        lua_rawseti(L, -2, (i - zero_offset));
    }
    lua_setglobal(L, "arg");
    lua_getglobal(L, "os");
    // TODO(mvlasak): absolute path to self directory?
        //s = ex_selfdir(argv[0]);
        //lua_pushstring(L, s);
        //xfree(s);
    lua_pushstring(L, argv[0]);
    lua_setfield(L, -2, "selfdir");
    return;
}


string input_name = NULL;

static string user_progname = NULL;

char *startup_filename = NULL;
int lua_only = 0;
int lua_offset = 0;
unsigned char show_luahashchars = 0;

#ifdef LuajitTeX
int luajiton   = 0;
char *jithash_hashname = NULL;
#endif

int safer_option = 0;
int utc_option = 0;

/*tex

Test whether getopt found an option ``A''. Assumes the option index is in the
variable |option_index|, and the option table in a variable |long_options|.

*/

#define ARGUMENT_IS(a) STREQ (long_options[option_index].name, a)

/*tex
    Nota Bene: we still intercept some options that other engines handle
    so that existing scripted usage will not fail.

    SunOS cc can't initialize automatic structs, so make this static.
*/

static struct option long_options[] = {
    {"fmt", 1, 0, 0},
    {"lua", 1, 0, 0},
    {"luaonly", 0, 0, 0},
    {"luahashchars", 0, 0, 0},
#ifdef LuajitTeX
    {"jiton", 0, 0, 0},
    {"jithash", 1, 0, 0},
#endif
    {"safer", 0, &safer_option, 1},
    {"utc", 0, &utc_option, 1},
    {"help", 0, 0, 0},
    {"ini", 0, &ini_version, 1},
    {"interaction", 1, 0, 0},
    {"halt-on-error", 0, &haltonerrorp, 1},
    {"progname", 1, 0, 0},
    {"version", 0, 0, 0},
    {"credits", 0, 0, 0},
    {"recorder", 0, 0, 0},
    {"etex", 0, 0, 0},
    {"shell-escape", 0, &shellenabledp, 1},
    {"no-shell-escape", 0, &shellenabledp, -1},
    {"enable-write18", 0, &shellenabledp, 1},
    {"disable-write18", 0, &shellenabledp, -1},
    {"debug-format", 0, &debug_format_file, 1},
    {"file-line-error-style", 0, &filelineerrorstylep, 1},
    {"no-file-line-error-style", 0, &filelineerrorstylep, -1},
    /*tex Shorter option names for the above. */
    {"file-line-error", 0, &filelineerrorstylep, 1},
    {"no-file-line-error", 0, &filelineerrorstylep, -1},
    {"jobname", 1, 0, 0},
    {"parse-first-line", 0, &parsefirstlinep, 1},
    {"no-parse-first-line", 0, &parsefirstlinep, -1},
    {"translate-file", 1, 0, 0},
    {"default-translate-file", 1, 0, 0},
    {"8bit", 0, 0, 0},
    {0, 0, 0, 0}
};

int lua_numeric_field_by_index(lua_State * L, int name_index, int dflt)
{
    register int i = dflt;
    /*tex fetch the stringptr */
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);
    lua_rawget(L, -2);
    if (lua_type(L, -1) == LUA_TNUMBER) {
        i = lua_roundnumber(L, -1);
    }
    lua_pop(L, 1);
    return i;
}

unsigned int lua_unsigned_numeric_field_by_index(lua_State * L, int name_index, int dflt)
{
    register unsigned int i = dflt;
    /*tex fetch the stringptr */
    lua_rawgeti(L, LUA_REGISTRYINDEX, name_index);
    lua_rawget(L, -2);
    if (lua_type(L, -1) == LUA_TNUMBER) {
        i = lua_uroundnumber(L, -1);
    }
    lua_pop(L, 1);
    return i;
}

static int recorderoption = 0;

static void parse_options(int ac, char **av)
{
#ifdef WIN32
    /*tex We save |argc| and |argv|. */
    int sargc = argc;
    char **sargv = argv;
#endif
     /*tex The `getopt' return code. */
    int g;
    int option_index;
    char *firstfile = NULL;
    /*tex Dont whine. */
    opterr = 0;
#ifdef LuajitTeX
    if ((strstr(argv[0], "luajittexlua") != NULL) ||
        (strstr(argv[0], "texluajit") != NULL) ||
        (strstr(argv[0], "texluahbjit") != NULL) ) {
#else
    if ((strstr(argv[0], "luatexlua") != NULL) ||
        (strstr(argv[0], "texlua") != NULL) ||
        (strstr(argv[0], "texluahb") != NULL)) {
#endif
        lua_only = 1;
        luainit = 1;
    }

    for (;;) {
        g = getopt_long_only(ac, av, "+", long_options, &option_index);
        if (g == -1) {
            /*tex End of arguments, exit the loop. */
            break;
        }
        if (g == '?')  {
            /*tex Unknown option. */
            if (!luainit)
                fprintf(stderr,"%s: unrecognized option '%s'\n", argv[0], argv[optind-1]);
            continue;
        }
        /* We have no short option names. */
        assert(g == 0);
        if (ARGUMENT_IS("luaonly")) {
            lua_only = 1;
            lua_offset = optind;
            luainit = 1;
        } else if (ARGUMENT_IS("lua")) {
            startup_filename = optarg;
            lua_offset = (optind - 1);
            luainit = 1;
#ifdef LuajitTeX
        } else if (ARGUMENT_IS("jiton")) {
            luajiton = 1;
        } else if (ARGUMENT_IS("jithash")) {
            size_t len = strlen(optarg);
            if (len<16) {
                jithash_hashname = optarg;
            } else {
                WARNING2("hash name truncated to 15 characters from %d. (%s)", (int) len, optarg);
                jithash_hashname = (string) xmalloc(16);
                strncpy(jithash_hashname, optarg, 15);
                jithash_hashname[15] = 0;
            }
#endif
        } else if (ARGUMENT_IS("luahashchars")) {
            show_luahashchars = 1;
        } else if (ARGUMENT_IS("progname")) {
            user_progname = optarg;
        } else if (ARGUMENT_IS("jobname")) {
            c_job_name = optarg;
        } else if (ARGUMENT_IS("fmt")) {
            dump_name = optarg;
        } else if (ARGUMENT_IS("interaction")) {
            /* These numbers match CPP defines */
            if (STREQ(optarg, "batchmode")) {
                interactionoption = 0;
            } else if (STREQ(optarg, "nonstopmode")) {
                interactionoption = 1;
            } else if (STREQ(optarg, "scrollmode")) {
                interactionoption = 2;
            } else if (STREQ(optarg, "errorstopmode")) {
                interactionoption = 3;
            } else {
                WARNING1("Ignoring unknown argument `%s' to --interaction", optarg);
            }
        } else if (ARGUMENT_IS("recorder")) {
            recorderoption = 1 ;
        } else if (ARGUMENT_IS("help")) {
            usagehelp(LUATEX_IHELP, BUG_ADDRESS);
        } else if (ARGUMENT_IS("version")) {
            print_version_banner();
            /* *INDENT-OFF* */
            puts("\n\nExecute  '" my_name " --credits'  for credits and version details.\n\n"
                 "There is NO warranty. Redistribution of this software is covered by\n"
                 "the terms of the GNU General Public License, version 2 or (at your option)\n"
                 "any later version. For more information about these matters, see the file\n"
                 "named COPYING and the LuaTeX source.\n\n"
                 "LuaTeX is Copyright 2021 Taco Hoekwater and the LuaTeX Team.\n");
            /* *INDENT-ON* */
            uexit(0);
        } else if (ARGUMENT_IS("credits")) {
            char *versions;
            initversionstring(&versions);
            print_version_banner();
            /* *INDENT-OFF* */
            puts("\n\nThe LuaTeX team is Hans Hagen, Hartmut Henkel, Taco Hoekwater, Luigi Scarso.\n\n"
                 MyName " merges and builds upon (parts of) the code from these projects:\n\n"
                 "tex       : Donald Knuth\n"
                 "etex      : Peter Breitenlohner, Phil Taylor and friends\n"
                 "omega     : John Plaice and Yannis Haralambous\n"
                 "aleph     : Giuseppe Bilotta\n"
                 "pdftex    : Han The Thanh and friends\n"
                 "lua       : Roberto Ierusalimschy, Waldemar Celes and Luiz Henrique de Figueiredo\n"
                 "metapost  : John Hobby, Taco Hoekwater, Luigi Scarso, Hans Hagen and friends\n"
                 "pplib     : Paweł Jackowski\n"
                 "fontforge : George Williams (partial)\n"
                 "luajit    : Mike Pall (used in LuajitTeX)\n");
            /* *INDENT-ON* */
            puts(versions);
            uexit(0);
        }
    }
    /*tex attempt to find |input_name| and |dump_name| */
    if (lua_only) {
        if (argv[optind]) {
            startup_filename = xstrdup(argv[optind]);
            lua_offset = optind;
        }
    } else if (argv[optind] && argv[optind][0] == '&') {
        dump_name = xstrdup(argv[optind] + 1);
    } else if (argv[optind] && argv[optind][0] != '\\') {
        if (argv[optind][0] == '*') {
            input_name = xstrdup(argv[optind] + 1);
        } else {
            firstfile = xstrdup(argv[optind]);
            if ((strstr(firstfile, ".lua") ==
                 firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".luc") ==
                    firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".LUA") ==
                    firstfile + strlen(firstfile) - 4)
                || (strstr(firstfile, ".LUC") ==
                    firstfile + strlen(firstfile) - 4)) {
                if (startup_filename == NULL) {
                    startup_filename = firstfile;
                    lua_offset = optind;
                    lua_only = 1;
                    luainit = 1;
                }
            } else {
                input_name = firstfile;
            }
        }
#ifdef WIN32
    } else if (sargc > 1 && sargv[sargc-1] && sargv[sargc-1][0] != '-' &&
               sargv[sargc-1][0] != '\\') {
        if (sargv[sargc-1][0] == '&')
            dump_name = xstrdup(sargv[sargc-1] + 1);
        else  {
            if (sargv[sargc-1][0] == '*')
                input_name = xstrdup(sargv[sargc-1] + 1);
            else
                input_name = xstrdup(sargv[sargc-1]);
            sargv[sargc-1] = normalize_quotes(input_name, "argument");
        }
        return;
#endif
    }
    /*tex Finalize the input filename. */
    if (input_name != NULL) {
        argv[optind] = normalize_quotes(input_name, "argument");
    }
}

/*tex
    Test for readability.
*/

#define is_readable(a) (stat(a,&finfo)==0) \
    && S_ISREG(finfo.st_mode) \
    && (f=fopen(a,"r")) != NULL && !fclose(f)

static char *find_filename(char *name, const char *envkey)
{
    struct stat finfo;
    char *dirname = NULL;
    char *filename = NULL;
    FILE *f;
    if (is_readable(name)) {
        return name;
    } else {
        dirname = getenv(envkey);
        if ((dirname != NULL) && strlen(dirname)) {
            dirname = xstrdup(getenv(envkey));
            if (*(dirname + strlen(dirname) - 1) == '/') {
                *(dirname + strlen(dirname) - 1) = 0;
            }
            filename = xmalloc((unsigned) (strlen(dirname) + strlen(name) + 2));
            filename = concat3(dirname, "/", name);
            xfree(dirname);
            if (is_readable(filename)) {
                return filename;
            }
            xfree(filename);
        }
    }
    return NULL;
}

static void fix_dumpname(void)
{
    int dist;
    if (dump_name) {
        /*tex Adjust array for Pascal and provide extension, if needed. */
        dist = (int) (strlen(dump_name) - strlen(DUMP_EXT));
        if (strstr(dump_name, DUMP_EXT) == dump_name + dist)
            TEX_format_default = dump_name;
        else
            TEX_format_default = concat(dump_name, DUMP_EXT);
    } else {
        /*tex For |dump_name| to be NULL is a bug. */
        if (!ini_version) {
          fprintf(stdout, "no format given, quitting\n");
          exit(1);
        }
    }
}

/*tex

    Helper variables for the safe keeping of table ids.

*/

int l_pack_type_index       [PACK_TYPE_SIZE] ;
int l_group_code_index      [GROUP_CODE_SIZE];
int l_local_par_index       [LOCAL_PAR_SIZE];
int l_math_style_name_index [MATH_STYLE_NAME_SIZE];
int l_dir_par_index         [DIR_PAR_SIZE];
int l_dir_text_index_normal [DIR_TEXT_SIZE];
int l_dir_text_index_cancel [DIR_TEXT_SIZE];

int img_parms               [img_parms_max];
int img_pageboxes           [img_pageboxes_max];

int lua_show_valid_list(lua_State *L, const char **list, int offset, int max)
{
    int i;
    lua_newtable(L);
    for (i = 0; i < max; i++) {
        lua_pushinteger(L,i+offset);
        lua_pushstring(L, list[i]);
        lua_settable(L, -3);
    }
    return 1;
}

int lua_show_valid_keys(lua_State *L, int *list, int max)
{
    int i;
    lua_newtable(L);
    for (i = 0; i < max; i++) {
        lua_pushinteger(L,i+1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, list[i]);
        lua_settable(L, -3);
    }
    return 1;
}

#if defined(WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
char **suffixlist;

/* Why do we add script stuff to this weird incomplete. Let's go more minimal. */

/*
    #define EXE_SUFFIXES ".com;.exe;.bat;.cmd;.vbs;.vbe;.js;.jse;.wsf;.wsh;.ws;.tcl;.py;.pyw"
*/

#define EXE_SUFFIXES ".com;.exe;.bat;.cmd"

static void mk_suffixlist(void)
{
    char **p;
    char *q, *r, *v;
    int n;
#  if defined(__CYGWIN__)
    v = xstrdup(EXE_SUFFIXES);
#  else
    v = (char *) getenv("PATHEXT");
    /*tex strlwr() exists also in MingW */
    if (v)
        v = (char *) strlwr(xstrdup(v));
    else
        v = xstrdup(EXE_SUFFIXES);
#  endif
    q = v;
    n = 0;
    while ((r = strchr(q, ';')) != NULL) {
        n++;
        r++;
        q = r;
    }
    if (*q)
        n++;
    suffixlist = (char **) xmalloc((n + 2) * sizeof(char *));
    p = suffixlist;
    *p = xstrdup(".dll");
    p++;
    q = v;
    while ((r = strchr(q, ';')) != NULL) {
        *r = '\0';
        *p = xstrdup(q);
        p++;
        r++;
        q = r;
    }
    if (*q) {
        *p = xstrdup(q);
        p++;
    }
    *p = NULL;
    free(v);
}
#endif

void lua_initialize(int ac, char **av)
{
    char *given_file = NULL;
    char *banner;
    size_t len;
    int starttime;
    int utc;
    static char LC_CTYPE_C[] = "LC_CTYPE=C";
    static char LC_COLLATE_C[] = "LC_COLLATE=C";
    static char LC_NUMERIC_C[] = "LC_NUMERIC=C";
    static char engine_luatex[] = "engine=" my_name;
    char *old_locale = NULL;
    char *env_locale = NULL;
    char *tmp = NULL;
    /*tex Save to pass along to topenin. */
    const char *fmt = "This is " MyName ", Version %s" WEB2CVERSION;
    argc = ac;
    argv = av;
    len = strlen(fmt) + strlen(luatex_version_string) ;
    banner = xmalloc(len);
    sprintf(banner, fmt, luatex_version_string);
    luatex_banner = banner;
#if defined(WIN32) || defined(__MINGW32__) || defined(__CYGWIN__)
    mk_suffixlist();
#endif
    /*tex Must be initialized before options are parsed and might get adapted by config table.  */
    interactionoption = 4;
    filelineerrorstylep = false;
    haltonerrorp = false;
    tracefilenames = 1;
    dump_name = NULL;
    /*tex parse commandline */
    parse_options(ac, av);
    if (lua_only) {
        /*tex Shell has no restrictions. */
        shellenabledp = true;
        safer_option = 0;
    }
    /*tex
        Get the current locale (it should be |C|) and save |LC_CTYPE|, |LC_COLLATE|
        and |LC_NUMERIC|. Later |luainterpreter()| will consciously use them.
    */
    old_locale = xstrdup(setlocale (LC_ALL, NULL));
    lc_ctype = NULL;
    lc_collate = NULL;
    lc_numeric = NULL;
    if (old_locale) {
        /*tex
            If |setlocale| fails here, then the state could be compromised, and
            we exit.
        */
        env_locale = setlocale (LC_ALL, "");
        if (!env_locale && !lua_only) {
            fprintf(stderr,"Unable to read environment locale: exit now.\n");
            exit(1);
        }
        tmp = setlocale (LC_CTYPE, NULL);
        if (tmp) {
            lc_ctype = xstrdup(tmp);
        }
        tmp = setlocale (LC_COLLATE, NULL);
        if (tmp) {
            lc_collate = xstrdup(tmp);
        }
        tmp = setlocale (LC_NUMERIC, NULL);
        if (tmp) {
            lc_numeric = xstrdup(tmp);
        }
        /*tex
            Return to the previous locale if possible, otherwise it's a serious
            error and we exit: we can't ensure a 'sane' locale for lua.
        */
        env_locale = setlocale (LC_ALL, old_locale);
        if (!env_locale) {
          fprintf(stderr,"Unable to restore original locale %s: exit now.\n",old_locale);
          exit(1);
        }
        xfree(old_locale);
    } else {
       fprintf(stderr,"Unable to store environment locale.\n");
    }
    /*tex make sure that the locale is 'sane' (for lua) */
    putenv(LC_CTYPE_C);
    putenv(LC_COLLATE_C);
    putenv(LC_NUMERIC_C);
    /*tex this is sometimes needed */
    putenv(engine_luatex);
    luainterpreter();
    /*tex init internalized strings */
    set_init_keys;
    lua_pushstring(Luas,"lua.functions");
    lua_newtable(Luas);
    lua_settable(Luas,LUA_REGISTRYINDEX);
    /*tex here start the key definitions */
    set_l_pack_type_index;
    set_l_group_code_index;
    set_l_local_par_index;
    set_l_math_style_name_index;
    set_l_dir_par_index;
    set_l_dir_text_index;
    l_set_node_data();
    l_set_whatsit_data();
    l_set_token_data();
    set_l_img_keys_index;
    set_l_img_pageboxes_index;
    /*tex collect arguments */
    prepare_cmdline(Luas, argv, argc, lua_offset);
    if (startup_filename != NULL) {
        given_file = xstrdup(startup_filename);
        if (lua_only) {
            xfree(startup_filename);
        }
        startup_filename = find_filename(given_file, "LUATEXDIR");
    }
    /*tex now run the file */
    if (startup_filename != NULL) {
        char *v1;
        int tex_table_id = hide_lua_table(Luas, "tex");
        int token_table_id = hide_lua_table(Luas, "token");
        int node_table_id = hide_lua_table(Luas, "node");
        int pdf_table_id = hide_lua_table(Luas, "pdf");
        if (lua_only) {
            /*tex hide the 'tex' and 'pdf' table */
            if (load_luatex_core_lua(Luas)) {
                fprintf(stderr, "Error in execution of luatex-core.lua .\n");
            }
            if (luaL_loadfile(Luas, startup_filename)) {
                fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
                exit(1);
            }
            init_tex_table(Luas);
            if (lua_pcall(Luas, 0, 0, 0)) {
                fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
                lua_traceback(Luas);
             /*tex lua_close(Luas); */
                exit(1);
            } else {
                if (given_file)
                    free(given_file);
                /*tex lua_close(Luas); */
                exit(0);
            }
        }
        /*tex a normal tex run */
        init_tex_table(Luas);
        unhide_lua_table(Luas, "tex", tex_table_id);
        unhide_lua_table(Luas, "pdf", pdf_table_id);
        unhide_lua_table(Luas, "token", token_table_id);
        unhide_lua_table(Luas, "node", node_table_id);
        if (luaL_loadfile(Luas, startup_filename)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
            exit(1);
        }
        if (lua_pcall(Luas, 0, 0, 0)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
            lua_traceback(Luas);
            exit(1);
        }
        if (!input_name) {
            get_lua_string("texconfig", "jobname", &input_name);
        }
        if (!dump_name) {
            get_lua_string("texconfig", "formatname", &dump_name);
        }
        /*tex |prohibit_file_trace| (boolean) */
        get_lua_boolean("texconfig", "trace_file_names", &tracefilenames);
        /*tex |file_line_error| */
        get_lua_boolean("texconfig", "file_line_error", &filelineerrorstylep);
        /*tex |halt_on_error| */
        get_lua_boolean("texconfig", "halt_on_error", &haltonerrorp);
        /*tex |interactionoption| */
        get_lua_number("texconfig", "interaction", &interactionoption);
        if ((interactionoption < 0) || (interactionoption > 4)) {
            interactionoption = 4;
        }
        /*tex |shell_escape| */
        v1 = NULL;
        get_lua_string("texconfig", "shell_escape", &v1);
        if (v1) {
            if (*v1 == 't' || *v1 == 'y' || *v1 == '1') {
                shellenabledp = 1;
            }
            free(v1);
        }
        starttime = -1 ;
        get_lua_number("texconfig", "start_time", &starttime);
        if (starttime < 0) {
            /*tex
                We provide this one for compatibility reasons and therefore also in
                uppercase.
            */
            get_lua_number("texconfig", "SOURCE_DATE_EPOCH", &starttime);
        }
        if (starttime >= 0) {
            set_start_time(starttime);
        }
        utc = -1 ;
        get_lua_boolean("texconfig", "use_utc_time", &utc);
        if (utc >= 0 && utc <= 1) {
            utc_option = utc;
        }
        fix_dumpname();
    } else if (luainit) {
        if (given_file) {
            fprintf(stdout, "%s file %s not found\n", (lua_only ? "Script" : "Configuration"), given_file);
            free(given_file);
        } else {
            fprintf(stdout, "No %s file given\n", (lua_only ? "script" : "configuration"));
        }
        exit(1);
    }
    /*tex Here we load luatex-core.lua which takes care of some protection on demand. */
    if (load_luatex_core_lua(Luas)) {
        fprintf(stderr, "Error in execution of luatex-core.lua .\n");
    }
}

void check_texconfig_init(void)
{
    if (Luas != NULL) {
        lua_getglobal(Luas, "texconfig");
        if (lua_istable(Luas, -1)) {
            lua_getfield(Luas, -1, "init");
            if (lua_isfunction(Luas, -1)) {
                int i = lua_pcall(Luas, 0, 0, 0);
                if (i != 0) {
                    /*tex
                        We can't be more precise hereas it's called before \TEX\
                        initialization happens.
                    */
                    fprintf(stderr, "This went wrong: %s\n", lua_tostring(Luas, -1));
                    error();
                }
            }
        }
    }
}
