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

/* Test a composite of a given operation, source, mask, and destination picture.
 * Fills the window, and samples from the 0,0 pixel corner.
 */
Bool
composite_test(Display *dpy, picture_info *win, picture_info *dst,
	       const int *op, int num_op,
	       const picture_info **src_color, int num_src,
	       const picture_info **mask_color, int num_mask,
	       const picture_info **dst_color, int num_dst,
	       Bool componentAlpha, Bool print_errors)
{
	color4d expected, tested, tdst, tmsk;
	char testname[40];
	int i, s, m, d, iter;

	for (d = 0; d < num_dst; d++) {
	    tdst = dst_color[d]->color;
	    color_correct(dst, &tdst);

	    for (m = 0; m < num_mask; m++) {
		XImage *image;

		if (componentAlpha) {
		    XRenderPictureAttributes pa;

		    pa.component_alpha = TRUE;
		    XRenderChangePicture(dpy, mask_color[m]->pict,
					 CPComponentAlpha, &pa);
		}

		for (iter = 0; iter < pixmap_move_iter; iter++) {
		    XRenderComposite(dpy, PictOpSrc,
				     dst_color[d]->pict, 0, dst->pict,
				     0, 0,
				     0, 0,
				     0, 0,
				     num_op, num_src);
		    for (s = 0; s < num_src; s++) {
			for (i = 0; i < num_op; i++)
			    XRenderComposite(dpy, ops[op[i]].op,
					     src_color[s]->pict,
					     mask_color[m]->pict,
					     dst->pict,
					     0, 0,
					     0, 0,
					     i, s,
					     1, 1);
		    }
		}

		if (componentAlpha) {
		    XRenderPictureAttributes pa;

		    pa.component_alpha = FALSE;
		    XRenderChangePicture(dpy, mask_color[m]->pict,
					 CPComponentAlpha, &pa);
		}

		image = XGetImage(dpy, dst->d,
				  0, 0, num_op, num_src,
				  0xffffffff, ZPixmap);
		copy_pict_to_win(dpy, dst, win, win_width, win_height);

		if (componentAlpha &&
		    mask_color[m]->format->direct.redMask == 0) {
		    /* Ax component-alpha masks expand alpha into
		     * all color channels.
		     * XXX: This should be located somewhere generic.
		     */
		    tmsk.a = mask_color[m]->color.a;
		    tmsk.r = mask_color[m]->color.a;
		    tmsk.g = mask_color[m]->color.a;
		    tmsk.b = mask_color[m]->color.a;
		} else
		    tmsk = mask_color[m]->color;

		for (s = 0; s < num_src; s++) {
		    for (i = 0; i < num_op; i++) {
			get_pixel_from_image(image, dst, i, s, &tested);

			do_composite(ops[op[i]].op,
				     &src_color[s]->color, &tmsk, &tdst,
				     &expected, componentAlpha);
			color_correct(dst, &expected);

			snprintf(testname, 40,
				 "%s %scomposite", ops[op[i]].name,
				 componentAlpha ? "CA " : "");
			if (!eval_diff(testname, &expected, &tested, 0, 0,
				       is_verbose && print_errors)) {
			    if (print_errors)
				printf("src color: %.2f %.2f %.2f %.2f\n"
				       "msk color: %.2f %.2f %.2f %.2f\n"
				       "dst color: %.2f %.2f %.2f %.2f\n",
				       src_color[s]->color.r,
				       src_color[s]->color.g,
				       src_color[s]->color.b,
				       src_color[s]->color.a,
				       mask_color[m]->color.r,
				       mask_color[m]->color.g,
				       mask_color[m]->color.b,
				       mask_color[m]->color.a,
				       dst_color[d]->color.r,
				       dst_color[d]->color.g,
				       dst_color[d]->color.b,
				       dst_color[d]->color.a);
			    printf("src: %s, mask: %s, dst: %s\n",
				   src_color[s]->name,
				   mask_color[m]->name,
				   dst->name);
			    XDestroyImage(image);
			    return FALSE;
			} else if (is_verbose) {
			    printf("src: %s, mask: %s, dst: %s\n",
				   src_color[s]->name,
				   mask_color[m]->name,
				   dst->name);
			}
		    }
		}
		XDestroyImage(image);
	    }
	}

	return TRUE;
}
