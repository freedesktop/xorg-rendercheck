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

#include <X11/extensions/Xrender.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef struct _color4d
{
	double r, g, b, a;
} color4d;

typedef struct _picture_info {
	Drawable d;
	Picture pict;
	XRenderPictFormat *format;
	char *name;		/* Possibly, some descriptive name. */
	color4d color;		/* If a 1x1R pict, the (corrected) color.*/
} picture_info;

/* tests.c */
void
begin_test(Display *dpy, picture_info *win);

/* ops.c */
void
do_composite(int op, color4d *src, color4d *mask, color4d *dst, color4d *result,
    Bool componentAlpha);
