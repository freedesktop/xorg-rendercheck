/*
 * $Id$
 *
 * Copyright © 2004 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <err.h>

#include "rendercheck.h"

color4d colors[] = {
	{1.0, 1.0, 1.0, 1.0},
	{1.0, 0, 0, 1.0},
	{0, 1.0, 0, 1.0},
	{0, 0, 1.0, 1.0},
	{0.5, 0, 0, 0.25},
	{0.5, 0, 0, .5},
	{0.0, .5, 1.0, .5},
	{0.0, .5, 1.0, 0}
};

int num_colors = sizeof(colors) / sizeof(colors[0]);

struct op_info ops[] = {
	{PictOpClear, "Clear"},
	{PictOpSrc, "Src"},
	{PictOpDst, "Dst"},
	{PictOpOver, "Over"},
	{PictOpOverReverse, "OverReverse"},
	{PictOpIn, "In"},
	{PictOpInReverse, "InReverse"},
	{PictOpOut, "Out"},
	{PictOpOutReverse, "OutReverse"},
	{PictOpAtop, "Atop"},
	{PictOpAtopReverse, "AtopReverse"},
	{PictOpXor, "Xor"},
	{PictOpAdd, "Add"},
	{PictOpSaturate, "Saturate"},
	{PictOpDisjointClear, "DisjointClear"},
	{PictOpDisjointSrc, "DisjointSrc"},
	{PictOpDisjointDst, "DisjointDst"},
	{PictOpDisjointOver, "DisjointOver"},
	{PictOpDisjointOverReverse, "DisjointOverReverse"},
	{PictOpDisjointIn, "DisjointIn"},
	{PictOpDisjointInReverse, "DisjointInReverse"},
	{PictOpDisjointOut, "DisjointOut"},
	{PictOpDisjointOutReverse, "DisjointOutReverse"},
	{PictOpDisjointAtop, "DisjointAtop"},
	{PictOpDisjointAtopReverse, "DisjointAtopReverse"},
	{PictOpDisjointXor, "DisjointXor"},
	{PictOpConjointClear, "ConjointClear"},
	{PictOpConjointSrc, "ConjointSrc"},
	{PictOpConjointDst, "ConjointDst"},
	{PictOpConjointOver, "ConjointOver"},
	{PictOpConjointOverReverse, "ConjointOverReverse"},
	{PictOpConjointIn, "ConjointIn"},
	{PictOpConjointInReverse, "ConjointInReverse"},
	{PictOpConjointOut, "ConjointOut"},
	{PictOpConjointOutReverse, "ConjointOutReverse"},
	{PictOpConjointAtop, "ConjointAtop"},
	{PictOpConjointAtopReverse, "ConjointAtopReverse"},
	{PictOpConjointXor, "ConjointXor"},
};

int num_ops = sizeof(ops) / sizeof(ops[0]);

#define round_pix(pix, mask) \
	((double)((int)(pix * (mask ) + .5)) / (double)(mask))

void
color_correct(picture_info *pi, color4d *color)
{
	if (!pi->format->direct.redMask) {
		color->r = 0.0;
		color->g = 0.0;
		color->b = 0.0;
	} else {
		color->r = round_pix(color->r, pi->format->direct.redMask);
		color->g = round_pix(color->g, pi->format->direct.greenMask);
		color->b = round_pix(color->b, pi->format->direct.blueMask);
	}
	if (!pi->format->direct.alphaMask)
		color->a = 1.0;
	else
		color->a = round_pix(color->a, pi->format->direct.alphaMask);
}

void
get_pixel(Display *dpy, picture_info *pi, int x, int y, color4d *color)
{
	XImage *image;
	unsigned long val;
	unsigned long rm, gm, bm, am;

	image = XGetImage(dpy, pi->d, x, y, 1, 1, 0xffffffff, ZPixmap);

	val = *(unsigned long *)image->data;

	rm = pi->format->direct.redMask << pi->format->direct.red;
	gm = pi->format->direct.greenMask << pi->format->direct.green;
	bm = pi->format->direct.blueMask << pi->format->direct.blue;
	am = pi->format->direct.alphaMask << pi->format->direct.alpha;
	if (am != 0)
		color->a = (double)(val & am) / (double)am;
	else
		color->a = 1.0;
	if (rm != 0) {
		color->r = (double)(val & rm) / (double)rm;
		color->g = (double)(val & gm) / (double)gm;
		color->b = (double)(val & bm) / (double)bm;
	} else {
		color->r = 0.0;
		color->g = 0.0;
		color->b = 0.0;
	}
}

int
eval_diff(char *name, color4d *expected, color4d *test, int x, int y,
    Bool verbose)
{
	double rscale, gscale, bscale, ascale;
	double rdiff, gdiff, bdiff, adiff, diff;

	/* XXX: Need to be provided mask shifts so we can produce useful error
	 * values.
	 */
	rscale = 1.0 * (1 << 5);
	gscale = 1.0 * (1 << 6);
	bscale = 1.0 * (1 << 5);
	ascale = 1.0;
	rdiff = fabs(test->r - expected->r) * rscale;
	bdiff = fabs(test->g - expected->g) * gscale;
	gdiff = fabs(test->b - expected->b) * bscale;
	adiff = fabs(test->a - expected->a) * ascale;
	/*rdiff = log2(1.0 + rdiff);
	gdiff = log2(1.0 + gdiff);
	bdiff = log2(1.0 + bdiff);
	adiff = log2(1.0 + adiff);*/
	diff = max(max(max(rdiff, gdiff), bdiff), adiff);
	if (diff > 3.0) {
		printf("%s test error of %.4f at (%d, %d) --\n"
		    "got:       %.2f %.2f %.2f %.2f\n"
		    "expected:  %.2f %.2f %.2f %.2f\n", name, diff, x, y,
		    test->r, test->g, test->b, test->a,
		    expected->r, expected->g, expected->b, expected->a);
		return FALSE;
	} else if (verbose) {
		printf("%s test succeeded at (%d, %d) with %.4f: "
		    "%.2f %.2f %.2f %.2f\n", name, x, y, diff,
		    expected->r, expected->g, expected->b, expected->a);
	}
	return TRUE;
}

void
argb_fill(Display *dpy, picture_info *p, int x, int y, int w, int h, float a,
    float r, float g, float b)
{
	XRenderColor rendercolor;

	rendercolor.red = r * 65535;
	rendercolor.green = g * 65535;
	rendercolor.blue = b * 65535;
	rendercolor.alpha = a * 65535;

	XRenderFillRectangle(dpy, PictOpSrc, p->pict, &rendercolor, x, y, w, h);
}

void
begin_test(Display *dpy, picture_info *win)
{
	int i, j, src, dst, mask;
	int num_dests, num_formats;
	picture_info *dests, *pictures_1x1, *pictures_10x10, picture_3x3;

	num_dests = 3;
	dests = (picture_info *)malloc(num_dests * sizeof(dests[0]));
	if (dests == NULL)
		errx(1, "malloc error");

	dests[0].format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
	dests[1].format = XRenderFindStandardFormat(dpy, PictStandardRGB24);
	dests[2].format = XRenderFindStandardFormat(dpy, PictStandardA8);
	/*
	dests[3].format = XRenderFindStandardFormat(dpy, PictStandardA4);
	dests[4].format = XRenderFindStandardFormat(dpy, PictStandardA1);
	*/

	for (i = 0; i < num_dests; i++) {
		dests[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0),
		    win_width, win_height, dests[i].format->depth);
		dests[i].pict = XRenderCreatePicture(dpy, dests[i].d,
		    dests[i].format, 0, NULL);

		dests[i].name = (char *)malloc(20);
		if (dests[i].name == NULL)
			errx(1, "malloc error");
		describe_format(dests[i].name, 20, dests[i].format);
	}

	num_formats = 3;

	pictures_1x1 = (picture_info *)malloc(num_colors * num_formats *
	    sizeof(picture_info));
	if (pictures_1x1 == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_colors * num_formats; i++) {
		XRenderPictureAttributes pa;
		color4d *c = &colors[i / num_formats];

		/* The standard PictFormat numbers go from 0 to 4 */
		pictures_1x1[i].format = XRenderFindStandardFormat(dpy,
		    i % num_formats);
		pictures_1x1[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0), 1,
		    1, pictures_1x1[i].format->depth);
		pa.repeat = TRUE;
		pictures_1x1[i].pict = XRenderCreatePicture(dpy,
		    pictures_1x1[i].d, pictures_1x1[i].format, CPRepeat, &pa);

		pictures_1x1[i].name = (char *)malloc(20);
		if (pictures_1x1[i].name == NULL)
			errx(1, "malloc error");
		sprintf(pictures_1x1[i].name, "1x1R ");
		describe_format(pictures_1x1[i].name +
		    strlen(pictures_1x1[i].name), 20 -
		    strlen(pictures_1x1[i].name), pictures_1x1[i].format);

		argb_fill(dpy, &pictures_1x1[i], 0, 0, 1, 1,
		    c->a, c->r, c->g, c->b);

		pictures_1x1[i].color = *c;
		color_correct(&pictures_1x1[i], &pictures_1x1[i].color);
	}

	pictures_10x10 = (picture_info *)malloc(num_colors * num_formats *
	    sizeof(picture_info));
	if (pictures_10x10 == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_colors * num_formats; i++) {
		XRenderPictureAttributes pa;
		color4d *c = &colors[i / num_formats];

		/* The standard PictFormat numbers go from 0 to 4 */
		pictures_10x10[i].format = XRenderFindStandardFormat(dpy,
		    i % num_formats);
		pictures_10x10[i].d = XCreatePixmap(dpy, RootWindow(dpy, 0), 10,
		    10, pictures_10x10[i].format->depth);
		pa.repeat = TRUE;
		pictures_10x10[i].pict = XRenderCreatePicture(dpy,
		    pictures_10x10[i].d, pictures_10x10[i].format, 0, NULL);

		pictures_10x10[i].name = (char *)malloc(20);
		if (pictures_10x10[i].name == NULL)
			errx(1, "malloc error");
		sprintf(pictures_10x10[i].name, "10x10 ");
		describe_format(pictures_10x10[i].name +
		    strlen(pictures_10x10[i].name), 20 -
		    strlen(pictures_10x10[i].name), pictures_10x10[i].format);

		argb_fill(dpy, &pictures_10x10[i], 0, 0, 10, 10,
		    c->a, c->r, c->g, c->b);

		pictures_10x10[i].color = *c;
		color_correct(&pictures_10x10[i], &pictures_10x10[i].color);
	}

	picture_3x3.d = XCreatePixmap(dpy, RootWindow(dpy, 0), 3, 3, 32);
	picture_3x3.format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
	picture_3x3.pict = XRenderCreatePicture(dpy, picture_3x3.d,
	    picture_3x3.format, 0, NULL);
	picture_3x3.name = "3x3 sample picture";
	for (i = 0; i < 9; i++) {
		int x = i % 3;
		int y = i / 3;
		color4d *c = &colors[i % num_colors];

		argb_fill(dpy, &picture_3x3, x, y, 1, 1, c->a, c->r, c->g, c->b);
	}

	printf("Beginning testing of filling of 1x1R pictures\n");
	for (i = 0; i < num_colors * num_formats; i++) {
		fill_test(dpy, win, &pictures_1x1[i]);
	}

	printf("Beginning testing of filling of 10x10 pictures\n");
	for (i = 0; i < num_colors * num_formats; i++) {
		fill_test(dpy, win, &pictures_10x10[i]);
	}

	printf("Beginning dest coords test\n");
	/* 0 and num_formats should result in ARGB8888 red on ARGB8888 white. */
	dstcoords_test(dpy, win, &dests[0], &pictures_1x1[0],
	    &pictures_1x1[num_formats]);

	printf("Beginning src coords test\n");
	srccoords_test(dpy, win, &pictures_1x1[0], FALSE);

	printf("Beginning mask coords test\n");
	srccoords_test(dpy, win, &pictures_1x1[0], TRUE);

	printf("Beginning transformed src coords test\n");
	trans_coords_test(dpy, win, &pictures_1x1[0], FALSE);

	printf("Beginning transformed mask coords test\n");
	trans_coords_test(dpy, win, &pictures_1x1[0], TRUE);

	for (i = 0; i < num_ops; i++) {
	    for (j = 0; j <= num_dests; j++) {
		picture_info *pi;

		if (j != num_dests)
			pi = &dests[j];
		else
			pi = win;
		printf("Beginning %s blend test on %s\n", ops[i].name,
		    pi->name);

		for (src = 0; src < num_colors * num_formats; src++) {
			for (dst = 0; dst < num_colors; dst++) {
				blend_test(dpy, win, pi, i,
				    &pictures_1x1[src], &pictures_1x1[dst]);
				blend_test(dpy, win, pi, i,
				    &pictures_10x10[src], &pictures_1x1[dst]);
			}
		}
	    }
	}

	for (i = 0; i < num_ops; i++) {
	    for (j = 0; j <= num_dests; j++) {
		picture_info *pi;

		if (j != num_dests)
			pi = &dests[j];
		else
			pi = win;
		printf("Beginning %s composite mask test on %s\n", ops[i].name,
		    pi->name);

		for (src = 0; src < num_colors; src++) {
		    for (mask = 0; mask < num_colors; mask++) {
			for (dst = 0; dst < num_colors; dst++) {
				composite_test(dpy, win, pi, i,
				    &pictures_10x10[src], &pictures_10x10[mask],
				    &pictures_1x1[dst], FALSE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_1x1[src], &pictures_10x10[mask],
				    &pictures_1x1[dst], FALSE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_10x10[src], &pictures_1x1[mask],
				    &pictures_1x1[dst], FALSE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_1x1[src], &pictures_1x1[mask],
				    &pictures_1x1[dst], FALSE, TRUE);
			}
		    }
		}
	    }
	}

	for (i = 0; i < num_ops; i++) {
	    for (j = 0; j <= num_dests; j++) {
		picture_info *pi;

		if (j != num_dests)
			pi = &dests[j];
		else
			pi = win;
		printf("Beginning %s composite CA mask test on %s\n",
		    ops[i].name, pi->name);

		for (src = 0; src < num_colors; src++) {
		    for (mask = 0; mask < num_colors; mask++) {
			for (dst = 0; dst < num_colors; dst++) {
				composite_test(dpy, win, pi, i,
				    &pictures_10x10[src], &pictures_10x10[mask],
				    &pictures_1x1[dst], TRUE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_1x1[src], &pictures_10x10[mask],
				    &pictures_1x1[dst], TRUE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_10x10[src], &pictures_1x1[mask],
				    &pictures_1x1[dst], TRUE, TRUE);
				composite_test(dpy, win, pi, i,
				    &pictures_1x1[src], &pictures_1x1[mask],
				    &pictures_1x1[dst], TRUE, TRUE);
			}
		    }
		}
	    }
	}
}
