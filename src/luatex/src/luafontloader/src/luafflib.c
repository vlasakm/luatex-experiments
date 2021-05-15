/* luafflib.c

   Copyright 2007-2010 Taco Hoekwater <taco@luatex.org>

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

/**
*  @desc Support interface for fontforge 20070607
*  @version  1.0
*  @author Taco Hoekwater
*/

#include "pfaedit.h"
#include "ustring.h"
#include "lib/lib.h"            /* web2c's lib, for recorder_record_input */

#include "ffdummies.h"
#include "splinefont.h"

extern void normal_error(const char *t, const char *p);
extern void formatted_error(const char *t, const char *fmt, ...);

static void ff_do_cff(SplineFont * sf, char *filename, unsigned char **buf,
                      int *bufsiz)
{
    FILE *f;
    int32 *bsizes = NULL;
    int flags = ps_flag_nocffsugar + ps_flag_nohints;
    EncMap *map;

    map = EncMap1to1(sf->glyphcnt);

    if (WriteTTFFont
        (filename, sf, ff_cff, bsizes, bf_none, flags, map, ly_fore)) {
        /* success */
        f = fopen(filename, "rb");
        recorder_record_input(filename);
        readbinfile(f, buf, bufsiz);
        /*fprintf(stdout,"\n%s => CFF, size: %d\n", sf->filename, *bufsiz); */
        fclose(f);
        return;
    }
    formatted_error("fontloader","%s to CFF conversion failed", sf->filename);
}

/* exported for writecff.c */

int ff_createcff(char *file, unsigned char **buf, int *bufsiz)
{
    SplineFont *sf;
    int k;
    char s[] = "tempfile.cff";
    int openflags = 1;
    int notdefpos = 0;
    sf = ReadSplineFont(file, openflags);
    if (sf) {
        /* this is not the best way. nicer to have no temp file at all */
        ff_do_cff(sf, s, buf, bufsiz);
        for (k = 0; k < sf->glyphcnt; k++) {
            if (sf->glyphs[k] && strcmp(sf->glyphs[k]->name, ".notdef") == 0) {
                notdefpos = k;
                break;
            }
        }
        remove(s);
        EncMapFree(sf->map);
        SplineFontFree(sf);
    }
    return notdefpos;
}

int ff_get_ttc_index(char *ffname, char *psname)
{
    SplineFont *sf;
    int i = 0;
    int openflags = 1;
    int index = -1;

    sf = ReadSplineFontInfo((char *) ffname, openflags);
    if (sf == NULL) {
        normal_error("fontloader","font loading failed unexpectedly");
    }
    while (sf != NULL) {
        if (strcmp(sf->fontname, psname) == 0) {
            index = i;
        }
        i++;
        sf = sf->next;
    }
    if (index>=0)
        return (i-index-1);
    return -1;
}
