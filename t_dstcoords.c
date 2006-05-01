/*
 * Copyright © 2005 Eric Anholt
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

#include "rendercheck.h"

/* Test destination coordinates by drawing a 3x3 picture offset one pixel.
 * XXX: This should be done with another operation, to catch issues with Render
 * acceleration in the non-CopyArea-equivalent case.
 */
Bool
dstcoords_test(Display *dpy, picture_info *win, picture_info *dst,
    picture_info *bg, picture_info *fg)
{
	color4d expected, tested;
	int x, y, i;
	Bool failed = FALSE;

	for (i = 0; i < pixmap_move_iter; i++) {
		XRenderComposite(dpy, PictOpSrc, bg->pict, 0, dst->pict, 0, 0,
		    0, 0, 0, 0, win_width, win_height);
		XRenderComposite(dpy, PictOpSrc, fg->pict, 0, dst->pict, 0, 0,
		    0, 0, 1, 1, 1, 1);
	}
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
