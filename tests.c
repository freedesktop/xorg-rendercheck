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

#include "rendercheck.h"

extern int win_width, win_height;
extern int pixmap_move_iter;
extern Bool is_verbose;

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

struct op_info {
	int op;
	char *name;
} ops[] = {
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

/* Originally a bullseye, this pattern has been modified so that flipping
 * will result in errors as well.
 */
int target_colors[5][5] = {
	{1, 1, 1, 1, 1},
	{1, 0, 0, 0, 1},
	{1, 1, 1, 0, 1},
	{1, 0, 0, 1, 1},
	{1, 1, 1, 1, 1},
};

int dot_colors[5][5] = {
	{7, 7, 7, 7, 7},
	{7, 7, 7, 7, 7},
	{7, 0, 7, 7, 7},
	{7, 7, 7, 7, 7},
	{7, 7, 7, 7, 7},
};

#define round_pix(pix, mask) \
	((double)((int)(pix * (mask ) + .5)) / (double)(mask))

static void
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

static void
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

static int
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

static Bool
fill_test(Display *dpy, picture_info *win, picture_info *src)
{
	color4d tested;

	get_pixel(dpy, src, 0, 0, &tested);
	/* Copy the output to the window, so the user sees something visual. */
	XRenderComposite(dpy, PictOpSrc, src->pict, 0, win->pict, 0, 0, 0, 0,
	    0, 0, win_width, win_height);

	return eval_diff("fill", &src->color, &tested, 0, 0, is_verbose);
}

static Bool
destcoords_test(Display *dpy, picture_info *win, picture_info *dst,
    picture_info *bg, picture_info *fg)
{
	color4d expected, tested;
	int x, y;
	Bool failed = FALSE;

	XRenderComposite(dpy, PictOpSrc, bg->pict, 0, dst->pict, 0, 0, 0, 0,
	    0, 0, win_width, win_height);
	XRenderComposite(dpy, PictOpSrc, fg->pict, 0, dst->pict, 0, 0, 0, 0,
	    1, 1, 1, 1);
	/* Copy the output to the window, so the user sees something visual. */
	XRenderComposite(dpy, PictOpSrc, dst->pict, 0, win->pict, 0, 0, 0, 0,
	    0, 0, win_width, win_height);

	for (x = 0; x < 3; x++) {
		for (y = 0; y < 3; y++) {
			get_pixel(dpy, dst, x, y, &tested);
			if (x == 1 && y == 1)
				expected = fg->color;
			else
				expected = bg->color;
			color_correct(dst, &expected);
			if (!eval_diff("dst coords", &expected, &tested, x, y,
			    is_verbose))
				failed = TRUE;
		}
	}

	return !failed;
}

static Bool
srccoords_test(Display *dpy, picture_info *win, picture_info *src,
    picture_info *white, Bool test_mask)
{
	color4d expected, tested;
	int i;
	XRenderPictureAttributes pa;
	Bool failed = FALSE;
	int tested_colors[5][5];

	for (i = 0; i < 25; i++) {
		char name[20];
		int x = i % 5, y = i / 5;

		if (!test_mask)
			XRenderComposite(dpy, PictOpSrc, src->pict, 0,
			    win->pict, x, y, 0, 0, 0, 0, 1, 1);
		else {
			/* Using PictOpSrc, color 0 (white), and component
			 * alpha, the mask color should be written to the
			 * destination.
			 */
			pa.component_alpha = TRUE;
			XRenderChangePicture(dpy, src->pict, CPComponentAlpha,
			    &pa);
			XRenderComposite(dpy, PictOpSrc, white->pict, src->pict,
			    win->pict, 0, 0, x, y, 0, 0, 1, 1);
			pa.component_alpha = FALSE;
			XRenderChangePicture(dpy, src->pict, CPComponentAlpha,
			    &pa);
		}
		get_pixel(dpy, win, 0, 0, &tested);

		expected = colors[target_colors[x][y]];
		color_correct(win, &expected);
		if (tested.r == 1.0) {
			if (tested.g == 1.0 && tested.b == 1.0)
				tested_colors[x][y] = 0;
			else if (tested.g == 0.0 && tested.b == 0.0)
				tested_colors[x][y] = 1;
			else tested_colors[x][y] = 9;
		} else
			tested_colors[x][y] = 9;

		if (test_mask)
			snprintf(name, 20, "mask coords");
		else
			snprintf(name, 20, "src coords");
		if (!eval_diff(name, &expected, &tested, x, y,
		    is_verbose))
			failed = TRUE;
	}
	if (failed) {
		int j;

		printf("expected vs tested:\n");
		for (i = 0; i < 5; i++) {
			for (j = 0; j < 5; j++)
				printf("%d", target_colors[j][i]);
			printf(" ");
			for (j = 0; j < 5; j++)
				printf("%d", tested_colors[j][i]);
			printf("\n");
		}
	}

	return !failed;
}

/* Scale the source image (which contains a single white pixel on a background of
 * transparent) and check coordinates.
 */
static Bool
trans_coords_test(Display *dpy, picture_info *win, picture_info *src,
    picture_info *white, Bool test_mask)
{
	color4d expected, tested;
	int x, y;
	XRenderPictureAttributes pa;
	Bool failed = FALSE;
	int tested_colors[40][40], expected_colors[40][40];
	XTransform t;

	t.matrix[0][0] = 1.0; t.matrix[0][1] = 0.0; t.matrix[0][2] = 0.0;
	t.matrix[1][0] = 0.0; t.matrix[1][1] = 1.0; t.matrix[1][2] = 0.0;
	t.matrix[2][0] = 0.0; t.matrix[2][1] = 0.0; t.matrix[2][2] = 8.0;
	XRenderSetPictureTransform(dpy, src->pict, &t);

	if (!test_mask)
		XRenderComposite(dpy, PictOpSrc, src->pict, 0,
		    win->pict, 0, 0, 0, 0, 0, 0, 40, 40);
	else {
		XRenderComposite(dpy, PictOpSrc, white->pict, src->pict,
		    win->pict, 0, 0, 0, 0, 0, 0, 40, 40);
	}

	for (x = 0; x < 40; x++) {
	    for (y = 0; y < 40; y++) {
		int src_sample_x, src_sample_y;

		src_sample_x = x / 8;
		src_sample_y = y / 8;
		expected_colors[x][y] = dot_colors[src_sample_x][src_sample_y];

		get_pixel(dpy, win, x, y, &tested);

		if (tested.r == 1.0 && tested.g == 1.0 && tested.b == 1.0) {
			tested_colors[x][y] = 0;
		} else if (tested.r == 0.0 && tested.g == 0.0 &&
		    tested.b == 0.0) {
			tested_colors[x][y] = 7;
		} else {
			tested_colors[x][y] = 9;
		}
		if (tested_colors[x][y] != expected_colors[x][y])
			failed = TRUE;
	    }
	}

	if (failed) {
		printf("%s transform coordinates test failed.\n",
		    test_mask ? "mask" : "src");
		printf("expected vs tested:\n");
		for (y = 0; y < 40; y++) {
			for (x = 0; x < 40; x++)
				printf("%d", expected_colors[x][y]);
			printf(" ");
			for (x = 0; x < 40; x++)
				printf("%d", tested_colors[x][y]);
			printf("\n");
		}
		printf(" vs tested (same)\n");
		for (y = 0; y < 40; y++) {
			for (x = 0; x < 40; x++)
				printf("%d", tested_colors[x][y]);
			printf("\n");
		}
	}
	t.matrix[0][0] = 1.0; t.matrix[0][1] = 0.0; t.matrix[0][2] = 0.0;
	t.matrix[1][0] = 0.0; t.matrix[1][1] = 1.0; t.matrix[1][2] = 0.0;
	t.matrix[2][0] = 0.0; t.matrix[2][1] = 0.0; t.matrix[2][2] = 1.0;
	XRenderSetPictureTransform(dpy, src->pict, &t);

	return !failed;
}

static Bool
blend_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *dst_color)
{
	color4d expected, tested, tdst;
	char testname[20];
	int i;

	for (i = 0; i < pixmap_move_iter; i++) {
		XRenderComposite(dpy, PictOpSrc, dst_color->pict, 0, dst->pict, 0, 0,
		    0, 0, 0, 0, win_width, win_height);
		XRenderComposite(dpy, ops[op].op, src_color->pict, 0, dst->pict, 0, 0,
		    0, 0, 0, 0, win_width, win_height);
	}
	get_pixel(dpy, dst, 0, 0, &tested);
	/* Copy the output to the window, so the user sees something visual. */
	if (win != dst)
		XRenderComposite(dpy, PictOpSrc, dst->pict, 0, win->pict, 0, 0,
		    0, 0, 0, 0, win_width, win_height);

	tdst = dst_color->color;
	color_correct(dst, &tdst);
	do_composite(ops[op].op, &src_color->color, NULL, &tdst, &expected,
	    FALSE);
	color_correct(dst, &expected);

	snprintf(testname, 20, "%s blend", ops[op].name);
	if (!eval_diff(testname, &expected, &tested, 0, 0, is_verbose)) {
		printf("src color: %.2f %.2f %.2f %.2f\n"
		    "dst color: %.2f %.2f %.2f %.2f\n",
		    src_color->color.r, src_color->color.g,
		    src_color->color.b, src_color->color.a,
		    dst_color->color.r, dst_color->color.g,
		    dst_color->color.b, dst_color->color.a);
		printf("src: %s, dst: %s\n", src_color->name, dst->name);
		return FALSE;
	} else if (is_verbose) {
		printf("src: %s, dst: %s\n", src_color->name, dst->name);
	}
	return TRUE;
}

static Bool
composite_test(Display *dpy, picture_info *win, picture_info *dst, int op,
    picture_info *src_color, picture_info *mask_color, picture_info *dst_color,
    Bool componentAlpha, Bool print_errors)
{
	color4d expected, tested, tdst, tmsk;
	char testname[40];
	XRenderPictureAttributes pa;
	Bool success = TRUE;

	if (componentAlpha) {
		pa.component_alpha = TRUE;
		XRenderChangePicture(dpy, mask_color->pict, CPComponentAlpha,
		    &pa);
	}
	XRenderComposite(dpy, PictOpSrc, dst_color->pict, 0, dst->pict, 0, 0,
	    0, 0, 0, 0, win_width, win_height);
	XRenderComposite(dpy, ops[op].op, src_color->pict, mask_color->pict,
	    dst->pict, 0, 0, 0, 0, 0, 0, win_width, win_height);
	get_pixel(dpy, dst, 0, 0, &tested);
	/* Copy the output to the window, so the user sees something visual. */
	if (win != dst)
		XRenderComposite(dpy, PictOpSrc, dst->pict, 0, win->pict, 0, 0,
		    0, 0, 0, 0, win_width, win_height);
	if (componentAlpha) {
		pa.component_alpha = FALSE;
		XRenderChangePicture(dpy, mask_color->pict, CPComponentAlpha,
		    &pa);
	}

	if (componentAlpha && mask_color->format->direct.redMask == 0) {
		/* Ax component-alpha masks expand alpha into all color
		 * channels.  XXX: This should be located somewhere generic.
		 */
		tmsk.a = mask_color->color.a;
		tmsk.r = mask_color->color.a;
		tmsk.g = mask_color->color.a;
		tmsk.b = mask_color->color.a;
	} else
		tmsk = mask_color->color;

	tdst = dst_color->color;
	color_correct(dst, &tdst);
	do_composite(ops[op].op, &src_color->color, &tmsk, &tdst,
	    &expected, componentAlpha);
	color_correct(dst, &expected);

	snprintf(testname, 40, "%s %scomposite", ops[op].name,
	    componentAlpha ? "CA " : "");
	if (!eval_diff(testname, &expected, &tested, 0, 0, is_verbose &&
	    print_errors)) {
		if (print_errors)
			printf("src color: %.2f %.2f %.2f %.2f\n"
			    "msk color: %.2f %.2f %.2f %.2f\n"
			    "dst color: %.2f %.2f %.2f %.2f\n",
			    src_color->color.r, src_color->color.g,
			    src_color->color.b, src_color->color.a,
			    mask_color->color.r, mask_color->color.g,
			    mask_color->color.b, mask_color->color.a,
			    dst_color->color.r, dst_color->color.g,
			    dst_color->color.b, dst_color->color.a);
		printf("src: %s, mask: %s, dst: %s\n", src_color->name,
		    mask_color->name, dst->name);
		success = FALSE;
	} else if (is_verbose) {
		printf("src: %s, mask: %s, dst: %s\n", src_color->name,
		    mask_color->name, dst->name);
	}
	return success;
}


void
begin_test(Display *dpy, picture_info *win)
{
	int i, j, src, dst, mask;
	int num_dests, num_formats;
	picture_info *dests, *pictures_1x1, *pictures_10x10, picture_3x3;
	picture_info picture_target, picture_dot;

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
		XRenderColor rendercolor;

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

		rendercolor.red = colors[i / num_formats].r * 65535;
		rendercolor.green = colors[i / num_formats].g * 65535;
		rendercolor.blue = colors[i / num_formats].b * 65535;
		rendercolor.alpha = colors[i / num_formats].a * 65535;
		XRenderFillRectangle(dpy, PictOpSrc, pictures_1x1[i].pict,
		    &rendercolor, 0, 0, 1, 1);

		pictures_1x1[i].color.r = colors[i / num_formats].r;
		pictures_1x1[i].color.g = colors[i / num_formats].g;
		pictures_1x1[i].color.b = colors[i / num_formats].b;
		pictures_1x1[i].color.a = colors[i / num_formats].a;
		color_correct(&pictures_1x1[i], &pictures_1x1[i].color);
	}

	pictures_10x10 = (picture_info *)malloc(num_colors * num_formats *
	    sizeof(picture_info));
	if (pictures_10x10 == NULL)
		errx(1, "malloc error");

	for (i = 0; i < num_colors * num_formats; i++) {
		XRenderPictureAttributes pa;
		XRenderColor rendercolor;

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

		rendercolor.red = colors[i / num_formats].r * 65535;
		rendercolor.green = colors[i / num_formats].g * 65535;
		rendercolor.blue = colors[i / num_formats].b * 65535;
		rendercolor.alpha = colors[i / num_formats].a * 65535;
		XRenderFillRectangle(dpy, PictOpSrc, pictures_10x10[i].pict,
		    &rendercolor, 0, 0, 10, 10);

		pictures_10x10[i].color.r = colors[i / num_formats].r;
		pictures_10x10[i].color.g = colors[i / num_formats].g;
		pictures_10x10[i].color.b = colors[i / num_formats].b;
		pictures_10x10[i].color.a = colors[i / num_formats].a;
		color_correct(&pictures_10x10[i], &pictures_10x10[i].color);
	}

	picture_3x3.d = XCreatePixmap(dpy, RootWindow(dpy, 0), 3, 3, 32);
	picture_3x3.format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
	picture_3x3.pict = XRenderCreatePicture(dpy, picture_3x3.d,
	    picture_3x3.format, 0, NULL);
	picture_3x3.name = "3x3 sample picture";
	for (i = 0; i < 9; i++) {
		XRenderColor color;

		color.red = colors[i % num_colors].r * 65535;
		color.green = colors[i % num_colors].g * 65535;
		color.blue = colors[i % num_colors].b * 65535;
		color.alpha = colors[i % num_colors].a * 65535;
		XRenderFillRectangle(dpy, PictOpSrc, picture_3x3.pict, &color,
		    i % 3, i / 3, 1, 1);
	}
	picture_target.d = XCreatePixmap(dpy, RootWindow(dpy, 0), 5, 5, 32);
	picture_target.format = XRenderFindStandardFormat(dpy,
	    PictStandardARGB32);
	picture_target.pict = XRenderCreatePicture(dpy, picture_target.d,
	    picture_target.format, 0, NULL);
	picture_target.name = "target picture";
	for (i = 0; i < 25; i++) {
		int x = i % 5;
		int y = i / 5;
		XRenderColor color;
		int tc;

		tc = target_colors[x][y];

		color.red = colors[tc].r * 65535;
		color.green = colors[tc].g * 65535;
		color.blue = colors[tc].b * 65535;
		color.alpha = colors[tc].a * 65535;
		XRenderFillRectangle(dpy, PictOpSrc, picture_target.pict,
		    &color, x, y, 1, 1);
	}
	picture_dot.d = XCreatePixmap(dpy, RootWindow(dpy, 0), 5, 5, 32);
	picture_dot.format = XRenderFindStandardFormat(dpy,
	    PictStandardARGB32);
	picture_dot.pict = XRenderCreatePicture(dpy, picture_dot.d,
	    picture_dot.format, 0, NULL);
	picture_dot.name = "dot picture";
	for (i = 0; i < 25; i++) {
		int x = i % 5;
		int y = i / 5;
		XRenderColor color;
		int tc;

		tc = dot_colors[x][y];

		color.red = colors[tc].r * 65535;
		color.green = colors[tc].g * 65535;
		color.blue = colors[tc].b * 65535;
		color.alpha = colors[tc].a * 65535;
		XRenderFillRectangle(dpy, PictOpSrc, picture_dot.pict,
		    &color, x, y, 1, 1);
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
	destcoords_test(dpy, win, &dests[0], &pictures_1x1[0],
	    &pictures_1x1[num_formats]);

	printf("Beginning src coords test\n");
	srccoords_test(dpy, win, &picture_target, &pictures_1x1[0], FALSE);

	printf("Beginning mask coords test\n");
	srccoords_test(dpy, win, &picture_target, &pictures_1x1[0], TRUE);

	printf("Beginning transformed src coords test\n");
	trans_coords_test(dpy, win, &picture_dot, &pictures_1x1[0], FALSE);

	printf("Beginning transformed mask coords test\n");
	trans_coords_test(dpy, win, &picture_dot, &pictures_1x1[0], TRUE);

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
