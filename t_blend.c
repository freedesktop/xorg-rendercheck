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

/* Test a composite of a given operation, source, and destination picture.  */
Bool
blend_test(Display *dpy, picture_info *win, picture_info *dst,
	   const int *op, int num_op,
	   const picture_info **src_color, int num_src,
	   const picture_info **dst_color, int num_dst)
{
	color4d expected, tested, tdst;
	char testname[20];
	int i, j, k, y, iter;

	k = y = 0;
	while (k < num_dst) {
	    XImage *image;
	    int k0 = k;

	    for (iter = 0; iter < pixmap_move_iter; iter++) {
		k = k0;
		y = 0;
		while (k < num_dst && y + num_src < win_height) {
		    XRenderComposite(dpy, PictOpSrc,
				     dst_color[k]->pict, 0, dst->pict,
				     0, 0,
				     0, 0,
				     0, y,
				     num_op, num_src);
		    for (j = 0; j < num_src; j++) {
			for (i = 0; i < num_op; i++) {
			    XRenderComposite(dpy, ops[op[i]].op,
					     src_color[j]->pict, 0, dst->pict,
					     0, 0,
					     0, 0,
					     i, y,
					     1, 1);
			}
			y++;
		    }
		    k++;
		}
	    }

	    image = XGetImage(dpy, dst->d,
			      0, 0, num_ops, y,
			      0xffffffff, ZPixmap);
	    copy_pict_to_win(dpy, dst, win, win_width, win_height);

	    y = 0;
	    while (k0 < k) {
		tdst = dst_color[k0]->color;
		color_correct(dst, &tdst);

		for (j = 0; j < num_src; j++) {
		    for (i = 0; i < num_op; i++) {
			get_pixel_from_image(image, dst, i, y, &tested);

			do_composite(ops[op[i]].op,
				     &src_color[j]->color,
				     NULL,
				     &tdst,
				     &expected,
				     FALSE);
			color_correct(dst, &expected);

			snprintf(testname, 20, "%s blend", ops[op[i]].name);
			if (!eval_diff(testname, &expected, &tested, 0, 0, is_verbose)) {
			    char srcformat[20];

			    describe_format(srcformat, 20, src_color[j]->format);
			    printf("src color: %.2f %.2f %.2f %.2f (%s)\n"
				   "dst color: %.2f %.2f %.2f %.2f\n",
				   src_color[j]->color.r, src_color[j]->color.g,
				   src_color[j]->color.b, src_color[j]->color.a,
				   srcformat,
				   dst_color[k0]->color.r, dst_color[k0]->color.g,
				   dst_color[k0]->color.b, dst_color[k0]->color.a);
			    printf("src: %s, dst: %s\n", src_color[j]->name, dst->name);
			    return FALSE;
			} else if (is_verbose) {
			    printf("src: %s, dst: %s\n", src_color[j]->name, dst->name);
			}
		    }
		    y++;
		}
		k0++;
	    }

	    XDestroyImage(image);
	}

	return TRUE;
}
