#include "ptexlib.h"
#include <stdlib.h>

/*tex

    The function |divide_scaled| divides |s| by |m| using |dd| decimal digits of
    precision.

*/

scaled round_xn_over_d(scaled x, int n, unsigned int d)
{
    boolean positive = true;
    unsigned t, u, v;
    if (x < 0) {
        positive = !positive;
        x = -(x);
    }
    if (n < 0) {
        positive = !positive;
        n = -(n);
    }
    t = (unsigned) ((x % 0100000) * n);
    u = (unsigned) (((unsigned) (x) / 0100000) * (unsigned) n + (t / 0100000));
    v = (u % d) * 0100000 + (t % 0100000);
    if (u / d >= 0100000)
        arith_error = true;
    else
        u = 0100000 * (u / d) + (v / d);
    v = v % d;
    if (2 * v >= d)
        u++;
    if (positive)
        return (scaled) u;
    else
        return (-(scaled) u);
}



/*tex scaled value corresponds to 1truein (rounded!) */

scaled one_true_inch = (7227 * 65536 + 50) / 100;

/*tex scaled value corresponds to 1in (rounded to 4736287) */

scaled one_inch = (7227 * 65536 + 50) / 100;


boolean doing_leaders = false;  /* are we inside a leader box? */


/* From WEB2C */

void recorder_change_filename(char *filename)
{
	(void)filename;
}
void recorder_record_input(const char *input_filename)
{
	(void)input_filename;
}
void recorder_record_output(const char *output_filename)
{
	(void)output_filename;
}

char *fullnameoffile;
int recorder_enabled;
char *output_directory;

void
uexit (int unix_code)
{
	int final_code;

	if (unix_code == 0)
		final_code = EXIT_SUCCESS;
	else if (unix_code == 1)
		final_code = EXIT_FAILURE;
	else
		final_code = unix_code;

	exit(final_code);
}

void
usage (const char *str)
{
	fprintf (stderr, "Try `%s --help' for more information.\n", str);
	uexit (1);
}

void
usagehelp (const char **message, const char *bug_email)
{
	if (!bug_email)
		bug_email = "tex-k@tug.org";
	while (*message) {
		printf("%s\n", *message);
		++message;
	}
	printf("\nEmail bug reports to %s.\n", bug_email);
	uexit(0);
}

void *
xmalloc(size_t size)
{
	void *alloc = malloc(size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xmalloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

void *
xrealloc(void *old, size_t size)
{
	if (!old) {
		return xmalloc(size);
	}

	void *alloc = realloc(old, size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xrealloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

void *
xcalloc(size_t n, size_t size)
{
	void *alloc = calloc(n ? n : 1, size ? size : 1);
	if (!alloc) {
		fprintf(stderr, "xcalloc failed for %zu\n", size);
		exit(EXIT_FAILURE);
	}
	return alloc;
}

char *
xstrdup(const char *s)
{
	char *new = xmalloc(strlen(s) + 1);
	return strcpy(new, s);
}
