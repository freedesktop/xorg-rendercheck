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
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

extern int num_ops;
extern int num_colors;
extern color4d colors[];

int is_verbose = FALSE; /* XXX: Make me a command line arg */

/* Number of times to repeat operations so that pixmaps will tend to get moved
 * into offscreen memory and allow hardware acceleration.
 */
int pixmap_move_iter = 1;

int win_width = 10;
int win_height = 10;

static int
bit_count(int i)
{
	int count;

	count = (i >> 1) & 033333333333;
	count = i - count - ((count >> 1) & 033333333333);
	count = (((count + (count >> 3)) & 030707070707) % 077);
	/* HAKMEM 169 */
	return count;
}

/* This is not complete, but decent enough for now.*/
void
describe_format(char *desc, int len, XRenderPictFormat *format)
{
	char ad[4] = "", rd[4] = "", gd[4] = "", bd[4] = "";
	int ac, rc, gc, bc;
	int ashift;

	ac = bit_count(format->direct.alphaMask);
	rc = bit_count(format->direct.redMask);
	gc = bit_count(format->direct.greenMask);
	bc = bit_count(format->direct.blueMask);

	if (ac != 0) {
		snprintf(ad, 4, "a%d", ac);
		ashift = format->direct.alpha;
	} else if (rc + bc + gc < format->depth) {
		/* There are bits that are not part of A,R,G,B. Mark them with
		 * an x.
		 */
		snprintf(ad, 4, "x%d", format->depth - (format->direct.red +
		    format->direct.green + format->direct.blue));
		if (format->direct.red == 0 || format->direct.green == 0)
			ashift = format->depth;
		else
			ashift = 0;
	} else
		ashift = 0;

	if (rc  != 0)
		snprintf(rd, 4, "r%d", rc);
	if (gc  != 0)
		snprintf(gd, 4, "g%d", gc);
	if (bc  != 0)
		snprintf(bd, 4, "b%d", bc);

	if (ashift > format->direct.red) {
		if (format->direct.red > format->direct.blue)
			snprintf(desc, len, "%s%s%s%s", ad, rd, gd, bd);
		else
			snprintf(desc, len, "%s%s%s%s", ad, bd, gd, rd);
	} else {
		if (format->direct.red > format->direct.blue)
			snprintf(desc, len, "%s%s%s%s", rd, gd, bd, ad);
		else
			snprintf(desc, len, "%s%s%s%s", bd, gd, rd, ad);
	}
}

int main(int argc, char **argv)
{
	Display *dpy;
	XEvent ev;
	int i, maj, min;
	XWindowAttributes a;
	picture_info window;
	char window_desc[20];

	dpy = XOpenDisplay(0);

	if (!XRenderQueryExtension(dpy, &i, &i))
		errx(1, "Render extension missing.");

	XRenderQueryVersion(dpy, &maj, &min);
	if (maj != 0 || min < 1)
		errx(1, "Render extension version too low (%d.%d).", maj, min);

	printf("Render extension version %d.%d\n", maj, min);

	/* Conjoint/Disjoint were added in version 0.2, so disable those ops if
	 * the server doesn't support them.
	 */
	if (min < 2) {
		printf("Server doesn't support conjoint/disjoint ops, disabling.\n");
		num_ops = PictOpSaturate;
	}

	window.d = XCreateSimpleWindow(dpy, RootWindow(dpy, 0), 0, 0, win_width,
	    win_height, 0, 0, WhitePixel(dpy, 0));
	XGetWindowAttributes(dpy, window.d, &a);
	window.format = XRenderFindVisualFormat(dpy, a.visual);
	window.pict = XRenderCreatePicture(dpy, window.d,
	    window.format, 0, 0);
	XSelectInput(dpy, window.d, ExposureMask);
	XMapWindow(dpy, window.d);

	describe_format(window_desc, 20, window.format);
	printf("Window format: %s\n", window_desc);

	/* We have to premultiply the alpha into the r, g, b values of the
	 * sample colors.  Render colors are premultiplied with alpha, so r,g,b
	 * can never be greater than alpha.
	 */
	for (i = 0; i < num_colors; i++) {
		colors[i].r *= colors[i].a;
		colors[i].g *= colors[i].a;
		colors[i].b *= colors[i].a;
	}

	while (XNextEvent(dpy, &ev) == 0) {
		if (ev.type == Expose && !ev.xexpose.count) {
			begin_test(dpy, &window);
			exit(0);
		}
	}
	return 0;
}
