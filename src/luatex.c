/* luatex.c: Hand-coded routines for TeX or Metafont in C.  Originally
   written by Tim Morgan, drawing from other Unix ports of TeX.  This is
   a collection of miscellany, everything that's easier (or only
   possible) to do in C.

   This file is public domain.  */

/* This file is used to create texextra.c etc., with this line
   changed to include texd.h or mfd.h.  The ?d.h file is what
   #defines TeX or MF, which avoids the need for a special
   Makefile rule.  */

/* We |#define DLLPROC| in order to build LuaTeX as DLL for W32TeX.  */

#define DLLPROC dllluatexmain

#include "ptexlib.h"
#include "luatex.h"
#include "lua/luatex-api.h"

#define TeX

/*
    The version number can be queried with \.{\\luatexversion} and the revision with
    with \.{\\luatexrevision}. Traditionally the revision can be any character and
    pdf\TeX\ occasionally used no digits. Here we still use a character but we will
    stick to "0" upto "9" so users can expect a number represented as string.
*/

int luatex_version = 113;
int luatex_revision = '2';
const char *luatex_version_string = "1.13.2";
const char *engine_name = my_name;

#ifdef _MSC_VER
#undef timezone
#endif

#include <time.h>               /* For `struct tm'.  */
#if defined (HAVE_SYS_TIME_H)
#  include <sys/time.h>
#elif defined (HAVE_SYS_TIMEB_H)
#  include <sys/timeb.h>
#endif

#if defined(__STDC__)
#  include <locale.h>
#endif

#include <signal.h>             /* Catch interrupts.  */
#include <errno.h>

/*
    Shell escape.

    If shellenabledp == 0, all shell escapes are forbidden.

    If (shellenabledp == 1 && restrictedshell == 0), any command is allowed for a
    shell escape.

    If (shellenabledp == 1 && restrictedshell == 1), only commands given in the
    configuration file as shell_escape_commands =
    kpsewhich,ebb,extractbb,mpost,metafun,... (no spaces between commands) in
    texmf.cnf are allowed for a shell escape in a restricted form: command name
    and arguments should be separated by a white space. The first word should be
    a command name. The quotation character for an argument with spaces,
    including a pathname, should be ". ' should not be used. Internally, all
    arguments are quoted by ' (Unix) or " (Windows) before calling the system()
    function in order to forbid execution of any embedded command.

    If the --shell-escape option is given, we set shellenabledp = 1 and
    restrictedshell = 0, i.e., any command is allowed.

    If the --no-shell-escape option is given, we set shellenabledp = -1 (and
    restrictedshell is irrelevant).

    If none of these option are given, there are three cases:

    (1) In the case where shell_escape = y or shell_escape = t or shell_escape =
        1 it becomes shellenabledp = 1 and restrictedshell = 0, that is, any
        command is allowed.
    (2) In the case where shell_escape = p it becomes shellenabledp = 1 and
        restrictedshell = 1, that is, restricted shell escape is allowed.
    (3) In all other cases, shellenabledp = 0, that is, shell escape is
        forbidden. The value of restrictedshell is irrelevant if shellenabledp ==
        0.
*/

#ifdef TeX

/*
    Called from maininit. Not static because also called from
    luatexdir/lua/luainit.c.

    In order to avoid all kind of time code in the backend code we use a
    function. The start time can be overloaded in several ways:

    (1) By setting the environmment variable SOURCE_DATE_EPOCH. This will
    influence the tex parameters, random seed, pdf timestamp and pdf id that is
    derived from the time. This variable is consulted when the kpse library is
    enabled which is analogue to other properties.

    (2) By setting the texconfig.start_time variable (as with other variables we
    use the internal name there). This has the same effect as (1) and is provided
    for when kpse is not used to set these variables or when an overloaded is
    wanted. This is analogue to other properties.

    When an utc time is needed one can provide the flag --utc. This property is
    independent of this time hackery. This flag has a corresponding texconfig
    option use_utc_time.

    To some extend a cleaner solution would be to have a flag that disables all
    variable data in one go (like filenames and so) but we just follow the method
    implemented in pdftex where primitives are used tod disable it.

*/

static int start_time = -1;

int get_start_time(void) {
    if (start_time < 0) {
        start_time = time((time_t *) NULL);
    }
    return start_time;
}

/*
    This one is called as part of the kpse initialization which only happens when
    this library is enabled.
*/

void init_start_time(void) {
}

/*
    This one is used to fetch a value from texconfig which can also be used to
    set properties. This might come in handy when one has other ways to get date
    info in the pdf file.
*/

void set_start_time(int s) {
    if (s >= 0) {
        start_time = s ;
    }
}

#endif

/* What we were invoked as and with. */

char **argv;
int argc;

/* The C version of what might wind up in |TEX_format_default|.  */

string dump_name;

/* The C version of the jobname, if given. */

const_string c_job_name;

const char *luatex_banner;

#ifdef _MSC_VER
/* Invalid parameter handler */
static void myInvalidParameterHandler(const wchar_t * expression,
    const wchar_t * function,
    const wchar_t * file,
    unsigned int line,
    uintptr_t pReserved)
{
/*
    printf(L"Invalid parameter detected in function %s."
        L" File: %s Line: %d\n", function, file, line);
    printf(L"Expression: %s\n", expression);

    I return silently to avoid an exit with the error 0xc0000417 (invalid
    parameter) when we use non-embedded fonts in luatex-ja, which works without
    any problem on Unix systems. I hope it is not dangerous.
*/
   return;
}
#endif

/*
    The entry point: set up for reading the command line, which will happen in
    `topenin', then call the main body.
*/

int
#if defined(DLLPROC)
DLLPROC (int ac, string *av)
#else
main (int ac, string *av)
#endif
{
#  ifdef __EMX__
    _wildcard(&ac, &av);
    _response(&ac, &av);
#  endif

#  ifdef WIN32
#    ifdef _MSC_VER
    _set_invalid_parameter_handler(myInvalidParameterHandler);
#    endif
    _setmaxstdio(2048);
/*
    We choose to crash for fatal errors:

    SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
*/
    setmode(fileno(stdin), _O_BINARY);
#  endif

    lua_initialize(ac, av);

#  ifdef WIN32
    if (ac > 1) {
        char *pp;
        if ((strlen(av[ac-1]) > 2) && isalpha(av[ac-1][0]) && (av[ac-1][1] == ':') && (av[ac-1][2] == '\\')) {
            for (pp=av[ac-1]+2; *pp; pp++) {
            if (IS_KANJI(pp)) {
                pp++;
                continue;
            }
            if (*pp == '\\')
                *pp = '/';
            }
        }
    }
#  endif

    /*
        Call the real main program.
    */
    main_body();

    return EXIT_SUCCESS;
}


/*
    This is supposed to ``open the terminal for input'', but what we really do is
    copy command line arguments into TeX's or Metafont's buffer, so they can
    handle them. If nothing is available, or we've been called already (and
    hence, argc==0), we return with `last=first'.
*/

void topenin(void)
{
    int i;

    buffer[first] = 0; /* In case there are no arguments. */

    if (optind < argc) {
        /* We have command line arguments. */
        int k = first;
        for (i = optind; i < argc; i++) {
            char *ptr = &(argv[i][0]);
            /*
                We cannot use strcat, because we have multibyte UTF-8 input.
            */
            while (*ptr) {
                buffer[k++] = (packed_ASCII_code) * (ptr++);
            }
            buffer[k++] = ' ';
        }
        /* Don't do this again. */
        argc = 0;
        buffer[k] = 0;
    }

    /*
        Find the end of the buffer looking at spaces and newlines.
    */

    for (last = first; buffer[last]; ++last);

    /*
        We conform to the way Web2c does handle trailing tabs and spaces. This
        decade old behaviour was changed in September 2017 and can introduce
        compatibility issues in existing workflows. Because we don't want too
        many differences with upstream TeXlive we just follow up on that patch
        and it's up to macro packages to deal with possible issues (which can be
        done via the usual callbacks. One can wonder why we then still prune
        spaces but we leave that to the reader.
    */

    /*  Patched original comment:

        Make `last' be one past the last non-space character in `buffer',
        ignoring line terminators (but not, e.g., tabs).  This is because
        we are supposed to treat this like a line of TeX input.  Although
        there are pathological cases (SPC CR SPC CR) where this differs
        from input_line below, and from previous behavior of removing all
        whitespace, the simplicity of removing all trailing line terminators
        seems more in keeping with actual command line processing.
    */

    /*
        The IS_SPC_OR_EOL macro deals with space characters (SPACE 32) and
        newlines (CR and LF) and no longer looks at tabs (TAB 9).

    */

#define IS_SPC_OR_EOL(c) ((c) == ' ' || (c) == '\r' || (c) == '\n')
    for (--last; last >= first && IS_SPC_OR_EOL (buffer[last]); --last);
    last++;
    /*
        One more time, this time converting to TeX's internal character
        representation.
    */
}

/*
    Normalize quoting of filename -- that is, only quote if there is a space, and
    always use the quote-name-quote style.
*/

string normalize_quotes(const_string name, const_string mesg)
{
    boolean quoted = false;
    boolean must_quote = (strchr(name, ' ') != NULL);
    /* Leave room for quotes and NUL. */
    string ret = (string) xmalloc((unsigned) strlen(name) + 3);
    string p;
    const_string q;
    p = ret;
    if (must_quote)
        *p++ = '"';
    for (q = name; *q; q++) {
        if (*q == '"')
            quoted = !quoted;
        else
            *p++ = *q;
    }
    if (must_quote)
        *p++ = '"';
    *p = '\0';
    if (quoted) {
        fprintf(stderr, "! Unbalanced quotes in %s %s\n", mesg, name);
        uexit(1);
    }
    return ret;
}


/*
    All our interrupt handler has to do is set TeX's or Metafont's global
    variable `interrupt'; then they will do everything needed.
*/

#ifdef WIN32
/* Win32 doesn't set SIGINT ... */
static BOOL WINAPI catch_interrupt(DWORD arg)
{
    switch (arg) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        interrupt = 1;
        return TRUE;
    default:
        /* No need to set interrupt as we are exiting anyway */
        return FALSE;
    }
}
#else /* not WIN32 */
static void catch_interrupt(int arg)
{
    (void) arg;
    interrupt = 1;
#  ifdef OS2
    (void) signal(SIGINT, SIG_ACK);
#  else
    (void) signal(SIGINT, catch_interrupt);
#  endif /* not OS2 */
}
#endif /* not WIN32 */

/*
    Besides getting the date and time here, we also set up the interrupt handler,
    for no particularly good reason. It's just that since the `fix_date_and_time'
    routine is called early on (section 1337 in TeX, ``Get the first line of
    input and prepare to start''), this is as good a place as any.
*/

void get_date_and_time(int *minutes, int *day, int *month, int *year)
{
    time_t myclock = get_start_time();
    struct tm *tmptr ;
    if (utc_option) {
        tmptr = gmtime(&myclock);
    } else {
        tmptr = localtime(&myclock);
    }

    *minutes = tmptr->tm_hour * 60 + tmptr->tm_min;
    *day = tmptr->tm_mday;
    *month = tmptr->tm_mon + 1;
    *year = tmptr->tm_year + 1900;

    {
#ifdef SA_INTERRUPT

        /*
            Under SunOS 4.1.x, the default action after return from the signal
            handler is to restart the I/O if nothing has been transferred. The
            effect on TeX is that interrupts are ignored if we are waiting for
            input. The following tells the system to return EINTR from read() in
            this case. From ken@cs.toronto.edu.
        */

        struct sigaction a, oa;

        a.sa_handler = catch_interrupt;
        sigemptyset(&a.sa_mask);
        sigaddset(&a.sa_mask, SIGINT);
        a.sa_flags = SA_INTERRUPT;
        sigaction(SIGINT, &a, &oa);
        if (oa.sa_handler != SIG_DFL)
            sigaction(SIGINT, &oa, (struct sigaction *) 0);
#else /* no SA_INTERRUPT */
#  ifdef WIN32
        SetConsoleCtrlHandler(catch_interrupt, TRUE);
#  else /* not WIN32 */
        void(*old_handler) (int);

        old_handler = signal(SIGINT, catch_interrupt);
        if (old_handler != SIG_DFL)
            signal(SIGINT, old_handler);
#  endif /* not WIN32 */
#endif /* no SA_INTERRUPT */
    }
}

/*
    Getting a high resolution time.
*/

void get_seconds_and_micros(int *seconds, int *micros)
{
#if defined (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = (int)tv.tv_sec;
    *micros = (int)tv.tv_usec;
#elif defined (HAVE_FTIME)
    struct timeb tb;
    ftime(&tb);
    *seconds = tb.time;
    *micros = tb.millitm * 1000;
#else
    time_t myclock = time((time_t *) NULL);
    *seconds = (int) myclock;
    *micros = 0;
#endif
}

/*
    Generating a better seed numbers
*/

int getrandomseed(void)
{
#if defined (HAVE_GETTIMEOFDAY)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int)(tv.tv_usec + 1000000 * tv.tv_usec);
#elif defined (HAVE_FTIME)
    struct timeb tb;
    ftime(&tb);
    return (tb.millitm + 1000 * tb.time);
#else
    time_t myclock = get_start_time((time_t *) NULL);
    struct tm *tmptr ;
    if (utc_option) {
        tmptr = gmtime(&myclock);
    } else {
        tmptr = localtime(&myclock);
    }
    return (tmptr->tm_sec + 60 * (tmptr->tm_min + 60 * tmptr->tm_hour));
#endif
}

/*
    Read a line of input as efficiently as possible while still looking like
    Pascal. We set `last' to `first' and return `false' if we get to eof.
    Otherwise, we return `true' and set last = first + length(line except
    trailing whitespace).
*/

boolean input_line(FILE * f)
{
    int i = EOF;

#ifdef WIN32
    if (f != Poptr && fileno (f) != fileno (stdin)) {
        long position = ftell (f);
        if (position == 0L) {
            /* Detect and skip Byte order marks.  */
            int k1 = getc (f);

            if (k1 != 0xff && k1 != 0xfe && k1 != 0xef)
                rewind (f);
            else {
                int k2 = getc (f);
                if (k2 != 0xff && k2 != 0xfe && k2 != 0xbb)
                    rewind (f);
                else if ((k1 == 0xff && k2 == 0xfe) || /* UTF-16(LE) */
                         (k1 == 0xfe && k2 == 0xff))   /* UTF-16(BE) */
                    ;
                else {
                    int k3 = getc (f);
                    int k4 = getc (f);
                    if (k1 == 0xef && k2 == 0xbb && k3 == 0xbf &&
                        k4 >= 0 && k4 <= 0x7e) /* UTF-8 */
                        ungetc (k4, f);
                    else
                        rewind (f);
                }
            }
        }
    }
#endif
    /*
        Recognize either LF or CR as a line terminator.
    */
    last = first;
    while (last < buf_size && (i = getc(f)) != EOF && i != '\n' && i != '\r')
        buffer[last++] = (packed_ASCII_code) i;

    if (i == EOF && errno != EINTR && last == first)
        return false;

    /*
        We didn't get the whole line because our buffer was too small.
    */
    if (i != EOF && i != '\n' && i != '\r') {
        fprintf(stderr, "! Unable to read an entire line---bufsize=%u.\n",
                (unsigned) buf_size);
        fputs("Please increase buf_size in texmf.cnf.\n", stderr);
        uexit(1);
    }

    buffer[last] = ' ';
    if (last >= max_buf_stack)
        max_buf_stack = last;

    /*
        If next char is LF of a CRLF, read it.
    */
    if (i == '\r') {
        while ((i = getc(f)) == EOF && errno == EINTR);
        if (i != '\n')
            ungetc(i, f);
    }

    /*
        Trim trailing space character (but not, e.g., tabs). We can't have line
        terminators because we stopped reading at the first \r or \n. TeX's rule
        is to strip only trailing spaces (and eols). David Fuchs mentions that
        this stripping was done to ensure portability of TeX documents given the
        padding with spaces on fixed-record "lines" on some systems of the time,
        e.g., IBM VM/CMS and OS/360.
    */
    while (last > first && buffer[last - 1] == ' ')
        --last;

    /*
        Don't bother using xord if we don't need to.
    */

    return true;
}

/*
    Get the job name to be used, which may have been set from the command line.
*/

str_number getjobname(str_number name)
{
    str_number ret = name;
    if (c_job_name != NULL)
        ret = maketexstring(c_job_name);
    return ret;
}
