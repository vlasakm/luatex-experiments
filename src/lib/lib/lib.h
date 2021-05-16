/* lib.h: declare prototypes for functions in lib.a.
  Public domain. */

#ifndef LIB_LIB_H
#define LIB_LIB_H

/* openclose.c */
extern void close_file (FILE *);
extern void recorder_change_filename (string);
extern void recorder_record_input (const_string);
extern void recorder_record_output (const_string);

extern string fullnameoffile;
extern boolean recorder_enabled;
extern string output_directory;

#endif /* not LIB_LIB_H */
