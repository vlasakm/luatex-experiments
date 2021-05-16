/* openclose.c: open and close files for TeX, Metafont, and BibTeX.

   Written 1995 Karl Berry.  Public domain.  */

#include <w2c/config.h>
#include "lib.h"
#include <kpathsea/c-pathch.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/variable.h>
#include <kpathsea/absolute.h>
#ifdef PTEX
#include <ptexenc/ptexenc.h>
#endif

/* The globals we use to communicate.  */
extern string nameoffile;
extern unsigned namelength;

/* Define some variables. */
/* For "file:line:error" style error messages. */
string fullnameoffile;       /* Defaults to NULL.  */
static string recorder_name; /* Defaults to NULL.  */
static FILE *recorder_file;  /* Defaults to NULL.  */
/* For the filename recorder. */
boolean recorder_enabled;    /* Defaults to false. */
/* For the output-dir option. */
string output_directory;     /* Defaults to NULL.  */


/* Helpers for the filename recorder... */
/* Start the recorder */
static void
recorder_start(void)
{
    /* Alas, while we'd like to use mkstemp it is not portable,
       and doing the autoconfiscation (and providing fallbacks) is more
       than we want to cope with.  So we have to be content with using a
       default name.  Throw in the pid so at least parallel builds might
       work (Debian bug 575731).  */
    string cwd;
    char pid_str[MAX_INT_LENGTH];

    /* Windows (MSVC) seems to have no pid_t, so instead of storing the
       value returned by getpid() we immediately consume it.  */
    sprintf (pid_str, "%ld", (long) getpid());
    recorder_name = concat3(kpse_program_name, pid_str, ".fls");
    
    /* If an output directory was specified, use it instead of cwd.  */
    if (output_directory) {
      string temp = concat3(output_directory, DIR_SEP_STRING, recorder_name);
      free(recorder_name);
      recorder_name = temp;
    }
    
    recorder_file = xfopen(recorder_name, FOPEN_W_MODE);
    
    cwd = xgetcwd();
    fprintf(recorder_file, "PWD %s\n", cwd);
    free(cwd);
}

/* Change the name of the recorder file after we know the log file to
   the usual thing -- no pid integer and the document file name instead
   of the program name.  Unfortunately, we have to explicitly take
   -output-directory into account (again), since the NEW_NAME we are
   called with does not; it is just the log file name with .log replaced
   by .fls.  */

void
recorder_change_filename (string new_name)
{
   string temp = NULL;
   
   if (!recorder_file)
     return;

   /* On windows, an opened file cannot be renamed. */
#if defined(_WIN32)
   fclose (recorder_file);
#endif /* _WIN32 */

   /* If an output directory was specified, use it.  */
   if (output_directory) {
     temp = concat3(output_directory, DIR_SEP_STRING, new_name);
     new_name = temp;
   }

   /* On windows, renaming fails if a file with new_name exists. */
#if defined(_WIN32)
   remove (new_name);
#endif /* _WIN32 */

   rename(recorder_name, new_name);
   free(recorder_name);
   recorder_name = xstrdup(new_name);

   /* reopen the recorder file by FOPEN_A_MODE. */
#if defined(_WIN32)
   recorder_file = xfopen (recorder_name, FOPEN_A_MODE);
#endif /* _WIN32 */

   if (temp)
     free (temp);
}

/* helper for recorder_record_* */
static void
recorder_record_name (const_string prefix, const_string name)
{
    if (recorder_enabled) {
        if (!recorder_file)
            recorder_start();
        fprintf(recorder_file, "%s %s\n", prefix, name);
        fflush(recorder_file);
    }
}

/* record an input file name */
void
recorder_record_input (const_string name)
{
    recorder_record_name ("INPUT", name);
}

/* record an output file name */
void
recorder_record_output (const_string name)
{
    recorder_record_name ("OUTPUT", name);
}


/* Close F.  */

void
close_file (FILE *f)
{
  /* If F is null, just return.  bad_pool might close a file that has
     never been opened.  */
  if (!f)
    return;
    
#ifdef PTEX
#ifdef WIN32
  clear_infile_enc (f);
  if (fclose (f) == EOF) {
#else
  if (nkf_close (f) == EOF) {
#endif
#else
  if (fclose (f) == EOF) {
#endif
    /* It's not always nameoffile, we might have opened something else
       in the meantime.  And it's not easy to extract the filenames out
       of the pool array.  So just punt on the filename.  Sigh.  This
       probably doesn't need to be a fatal error.  */
    perror ("fclose");
  }
}
