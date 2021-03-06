/* ptexlib.h

   Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
   Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#ifndef PTEXLIB_H
#  define PTEXLIB_H

/* Try to detect if a system header has already been included.  */
#if (defined(__linux__) && defined(_FEATURES_H)) || \
    (defined(_MSC_VER) && (defined(_INC_CRTDEFS) || defined(_OFF_T_DEFINED))) || \
    (defined(__MINGW32__) && defined(__MINGW_H))
ptexlib.h must be included first!!!
#endif

/***************************************************************************/
/* w2c/c-auto.h replacement */
/* TODO(mvlasak): better integrate this */

/* Defined by build script */
/*#define WEB2CVERSION " ()"*/

#define EDITOR "vi +%d '%s'"

#define HAVE_FTIME        0
#define HAVE_GETTIMEOFDAY 1
#define HAVE_SYS_TIMEB_H  1
#define HAVE_SYS_TIME_H   1

#define ZLIB_CONST        1
/***************************************************************************/
/* w2c/c-auto.h replacement */
/* TODO(mvlasak): where is integer used? maybe int?*/

typedef long integer;

/* We need a type that's at least off_t wide */
#include <sys/types.h>
typedef off_t longinteger;
/***************************************************************************/
/* cpascal.h replacement */
/* TODO(mvlasak): better integrate this */

//#define abs(x)   ((((integer)(x)))) >= 0) ? ((integer)(x)) : ((integer)-(x)))
//#define abs(x) ((((integer)(x)))) >= 0) ? ((integer)(x)) : ((integer)-(x)))
//#define abs(x) ((integer)(x))
//#undef abs
//#define abs(x) ((x) >= 0 ? (x) : -(x))
#include <stdlib.h> // abs
#include <math.h> // floor
#define odd(x)	 ((x) & 1)
#undef floor /* MacOSX */
#define floor(x)	((integer)floor((double)(x)))

#define halfp(i) ((i) >> 1)

#define decr(x) --(x)
#define incr(x) ++(x)

typedef double real;
#define addressof(x) (&(x))
/* Allocate an array of a given type. Add 1 to size to account for the
   fact that Pascal arrays are used from [1..size], unlike C arrays which
   use [0..size). */
#define xmallocarray(type,size) ((type*)xmalloc((size+1)*sizeof(type)))
#define xreallocarray(ptr,type,size) ((type*)xrealloc(ptr,(size+1)*sizeof(type)))
#define xcallocarray(type,nmemb,size) ((type*)xcalloc(nmemb+1,(size+1)*sizeof(type)))

#ifdef WIN32
#define promptfilenamehelpmsg "(Press Enter to retry, or Control-Z to exit"
#else
#define promptfilenamehelpmsg "(Press Enter to retry, or Control-D to exit"
#endif
/***************************************************************************/
/* kpathsea/simpletypes.h replacement */
/* TODO(mvlasak): better integrate this */

typedef int boolean;
#define true 1
#define false 0
typedef char *string;
typedef const char *const_string;
/***************************************************************************/
/* kpathsea/lib.h replacement */
/* TODO(mvlasak): better integrate this */

#define STREQ(s1, s2) (((s1) != NULL) && ((s2) != NULL) && (strcmp (s1, s2) == 0))

#define START_WARNING() do { fputs ("warning: ", stderr)
#define END_WARNING() fputs (".\n", stderr); fflush (stderr); } while (0)

#define WARNING(str)                                                    \
  START_WARNING (); fputs (str, stderr); END_WARNING ()
#define WARNING1(str, e1)                                               \
  START_WARNING (); fprintf (stderr, str, e1); END_WARNING ()
#define WARNING2(str, e1, e2)                                           \
  START_WARNING (); fprintf (stderr, str, e1, e2); END_WARNING ()
#define WARNING3(str, e1, e2, e3)                                       \
  START_WARNING (); fprintf (stderr, str, e1, e2, e3); END_WARNING ()
#define WARNING4(str, e1, e2, e3, e4)                                   \
  START_WARNING (); fprintf (stderr, str, e1, e2, e3, e4); END_WARNING ()
/***************************************************************************/
/* w2c/config.h replacement */

__attribute__ ((noreturn)) void uexit(int code);

/***************************************************************************/



/* WEB2C macros and prototypes */
#  include "luatex.h"

#  ifdef _MSC_VER
extern double rint(double x);
#  endif

/* Replicate these here. They are hardcoded anyway */

#  define eTeX_version_string "2.2"     /* current \eTeX\ version */
#  define eTeX_version 2        /* \.{\\eTeXversion}  */
#  define eTeX_minor_version 2  /* \.{\\eTeXminorversion}  */
#  define eTeX_revision ".2"    /* \.{\\eTeXrevision} */


#define LUA_COMPAT_MODULE 1
#  include "lua.h"


/* pdftexlib macros from ptexmac.h */

#  ifdef WIN32
#    define inline __inline
#  endif

/**********************************************************************/
/* Pascal WEB macros */

#  define max_integer      0x7FFFFFFF
#  define max_dimen        0x3FFFFFFF

/**********************************************************************/

#  define PRINTF_BUF_SIZE     1024
#  define MAX_CSTRING_LEN     1024 * 1024
#  define MAX_PSTRING_LEN     1024
#  define SMALL_BUF_SIZE      256
#  define SMALL_ARRAY_SIZE    256

#  define check_buf(size, buf_size)                                 \
  if ((unsigned)(size) > (unsigned)(buf_size))                      \
    formatted_error("internal","buffer overflow: %d > %d at file %s, line %d",     \
                (int)(size), (int)(buf_size), __FILE__,  __LINE__ )

#  define append_char_to_buf(c, p, buf, buf_size) do { \
    if (c == 9)                                        \
        c = 32;                                        \
    if (c == 13 || c == EOF)                           \
        c = 10;                                        \
    if (c != ' ' || (p > buf && p[-1] != 32)) {        \
        check_buf(p - buf + 1, (buf_size));            \
        *p++ = c;				       \
    }                                                  \
} while (0)

#  define append_eol(p, buf, buf_size) do {            \
    check_buf(p - buf + 2, (buf_size));                \
    if (p - buf > 1 && p[-1] != 10)                    \
        *p++ = 10;                                     \
    if (p - buf > 2 && p[-2] == 32) {                  \
        p[-2] = 10;                                    \
        p--;                                           \
    }                                                  \
    *p = 0;                                            \
} while (0)

#  define remove_eol(p, buf) do {                      \
    p = strend(buf) - 1;                               \
    if (*p == 10)                                      \
        *p = 0;                                        \
} while (0)

#  define skip_char(p, c)   if (*p == c)  p++

#  define alloc_array(T, n, s) do {					\
	if (T##_array == NULL) {					\
	    T##_limit = (size_t)(s);					\
	    if ((unsigned)(n) > (unsigned)T##_limit)			\
		T##_limit = (size_t)(n);				\
	    T##_array = xtalloc((unsigned)T##_limit, T##_entry);	\
	    T##_ptr = T##_array;					\
	}								\
	else if ((unsigned)(T##_ptr - T##_array + (unsigned)(n)) > (unsigned)(T##_limit)) { \
	    size_t last_ptr_index = (size_t)(T##_ptr - T##_array);	\
	    T##_limit *= 2;						\
	    if ((unsigned)(T##_ptr - T##_array + (unsigned)(n)) > (unsigned)(T##_limit)) \
		T##_limit = (size_t)(T##_ptr - T##_array + (unsigned)(n));	\
	    xretalloc(T##_array, (unsigned)T##_limit, T##_entry);	\
	    T##_ptr = T##_array + last_ptr_index;			\
	}								\
    } while (0)

#  define define_array(T)                   \
T##_entry      *T##_ptr, *T##_array = NULL; \
size_t          T##_limit

#  define xfree(a)            do { free(a); a = NULL; } while (0)
#  define dxfree(a,b)         do { free(a); a = b; } while (0)
#  define strend(s)           strchr(s, 0)
#  define xtalloc             XTALLOC
#  define xretalloc           XRETALLOC

#  define cmp_return(a, b) \
    if ((a) > (b))         \
        return 1;          \
    if ((a) < (b))         \
        return -1

#  define str_prefix(s1, s2)  (strncmp((s1), (s2), strlen(s2)) == 0)

#  include "tex/mainbody.h"
/**********************************************************************/
/* TODO(mvlasak): from deleted: */
#include "lua/luatex-api.h"

scaled round_xn_over_d(scaled x, int n, unsigned int d);

extern scaled one_true_inch;
extern scaled one_inch;

extern boolean doing_leaders;

typedef struct scaled_whd_ {
    scaled wd; /* TeX width */
    scaled ht; /* TeX height */
    scaled dp; /* TeX depth */
} scaled_whd;

#include <stdint.h>

typedef struct scaledpos_ {
    int64_t h;
    int64_t v;
} scaledpos;

typedef int internal_font_number;                  /* |font| in a |char_node| */

#  include "utils/managed-sa.h"
#  include "font/texfont.h"

extern void recorder_change_filename(char *filename);
extern void recorder_record_input(const char *input_filename);
extern void recorder_record_output(const char *output_filename);
extern char *fullnameoffile;
extern int recorder_enabled;
extern char *output_directory;

extern void usage (const char *str);
extern void usagehelp (const char **message, const char *bug_email);

#define IS_DIR_SEP(ch) ((ch) == '/')


void * xmalloc(size_t size);
void * xcalloc(size_t n, size_t size);
char * xstrdup(const char *s);
void * xrealloc(void *old, size_t size);
char * concat3(const char *s1, const char *s2, const char *s3);
char * concat(const char *s1, const char *s2);


/**********************************************************************/
#  include "tex/expand.h"
#  include "tex/conditional.h"

#  include "utils/managed-sa.h"

#  include "utils/utils.h"
#  include "utils/unistring.h"

#  include "lang/texlang.h"

#  include "tex/textcodes.h"
#  include "tex/mathcodes.h"

#  include "tex/align.h"
#  include "tex/directions.h"
#  include "tex/errors.h"
#  include "tex/inputstack.h"
#  include "tex/stringpool.h"
#  include "tex/textoken.h"
#  include "tex/printing.h"
#  include "tex/texfileio.h"
#  include "tex/arithmetic.h"
#  include "tex/nesting.h"
#  include "tex/packaging.h"
#  include "tex/linebreak.h"
#  include "tex/postlinebreak.h"
#  include "tex/scanning.h"
#  include "tex/buildpage.h"
#  include "tex/maincontrol.h"
#  include "tex/dumpdata.h"
#  include "tex/mainbody.h"
#  include "tex/extensions.h"
#  include "tex/texnodes.h"

#  include "tex/texmath.h"
#  include "tex/mlist.h"
#  include "tex/primitive.h"
#  include "tex/commands.h"
#  include "tex/equivalents.h"

/**********************************************************************/

#  include "tex/filename.h"

/**********************************************************************/

extern halfword new_ligkern(halfword head, halfword tail);
extern halfword handle_ligaturing(halfword head, halfword tail);
extern halfword handle_kerning(halfword head, halfword tail);

halfword lua_hpack_filter(
    halfword head_node, scaled size, int pack_type, int extrainfo, int d, halfword a);
void lua_node_filter(
    int filterid, int extrainfo, halfword head_node,
    halfword * tail_node);
halfword lua_vpack_filter(
    halfword head_node, scaled size, int pack_type, scaled maxd, int extrainfo, int d, halfword a);
void lua_node_filter_s(
    int filterid, int extrainfo);
int lua_linebreak_callback(
    int is_broken, halfword head_node, halfword * new_head);
int lua_appendtovlist_callback(
    halfword box, int location, halfword prev_depth, boolean is_mirrored,
    halfword * result, int * next_depth, boolean * prev_set);

void copy_late_lua(pointer r, pointer p);
void copy_user_lua(pointer r, pointer p);
void free_late_lua(pointer p);
void free_user_lua(pointer p);
void show_late_lua(pointer p);

void load_tex_patterns(int curlang, halfword head);
void load_tex_hyphenation(int curlang, halfword head);

/* lua/llualib.c */

void dump_luac_registers(void);
void undump_luac_registers(void);

/* lua/ltexlib.c */
void luacstring_start(int n);
void luacstring_close(int n);
int luacstring_cattable(void);
int luacstring_input(halfword *n);
int luacstring_partial(void);
int luacstring_final_line(void);

/* lua/luanode.c */
int visible_last_node_type(int n);
void print_node_mem_stats(void);

/* lua/ltexiolib.c */
void flush_loggable_info(void);

/* lua/luastuff.w  */
void luafunctioncall(int slot);
void luabytecodecall(int slot);

/* lua/luastuff.c */
void luatokencall(int p, int nameptr);

extern void check_texconfig_init(void);

scaled divide_scaled(scaled s, scaled m, int dd);
scaled divide_scaled_n(double s, double m, double d);

#  include "tex/texdeffont.h"
#  include "luatexcallbackids.h"

extern boolean get_callback(lua_State * L, int i);

/* Additions to texmfmp.h for pdfTeX */

/* mark a char in font */
#  define pdf_mark_char(f,c) set_char_used(f,c,true)

/* test whether a char in font is marked */
#  define pdf_char_marked char_used

#  define voidcast(a) (void *)(a)
#  define fixmemcast(a) (smemory_word *)(a)

extern void do_vf(internal_font_number tmp_f);

/* This routine has to return four values.  */
#  define	dateandtime(i,j,k,l) get_date_and_time (&(i), &(j), &(k), &(l))
extern void get_date_and_time(int *, int *, int *, int *);

/* Get high-res time info. */
#  define seconds_and_micros(i,j) get_seconds_and_micros (&(i), &(j))
extern void get_seconds_and_micros(int *, int *);

/* This routine has to return a scaled value. */
extern int getrandomseed(void);

/* Copy command-line arguments into the buffer, despite the name.  */
extern void topenin(void);

/* Can't prototype this since it uses poolpointer and ASCIIcode, which
   are defined later in mfd.h, and mfd.h uses stuff from here.  */
/* Therefore the department of ugly hacks decided to move this declaration
   to the *coerce.h files. */
/* extern void calledit (); */

/* Set an array size from texmf.cnf.  */
/*extern void setupboundvariable(integer *, const_string, integer);*/

/* here  are a few functions that used to be in coerce.h */

extern str_number getjobname(str_number);
extern str_number makefullnamestring(void);

extern string dump_name;
extern const_string c_job_name;

extern halfword *check_isnode(lua_State * L, int i);
extern void lua_nodelib_push_fast(lua_State * L, halfword n);

extern halfword list_node_mem_usage(void);

extern halfword *check_isnode(lua_State * L, int ud);

extern extinfo *get_charinfo_vert_variants(charinfo * ci);
extern extinfo *get_charinfo_hor_variants(charinfo * ci);
extern void set_charinfo_hor_variants(charinfo * ci, extinfo * ext);
extern void set_charinfo_vert_variants(charinfo * ci, extinfo * ext);

extern extinfo *copy_variants(extinfo * o);

extern int program_name_set;    /* in lkpselib.c */

extern int kpse_available(const char * m); /* in texfileio.w */


#endif                          /* PTEXLIB_H */
