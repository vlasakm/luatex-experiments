#include "ptexlib.h"

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
