#include "test.h"

static void dump_outline(FT_Outline *outline) {
    int i,j, t;
    FT_Vector *vp;
    printf("%d points, %d contours\n\n", outline->n_points,outline->n_contours);
    j = 0;
    for (i = 0; i < outline->n_points; i++) {
        vp = &outline->points[i];
        t = outline->tags[i];
        printf("%i: %i %i %c\n", i, vp->x, vp->y, "+*##"[t & 3]);
        if (i == outline->contours[j]) {
            j++;
            putchar('\n');
        }
    }
}
