#include <display/sw_draw.h>
#include <display/sw_rotate.h>
#include <assert.h>
#ifdef CONFIG_GUI_API_BROM
#include <brom_interface.h>
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/*
 * compute x range in pixels inside the src image
 *
 * The routine will update the range value [*x_min, *x_max].
 *
 * @param x_min address of minimum range value in pixels
 * @param x_max address of maximum range value in pixels
 * @param img_w source image width in pixels
 * @param img_h source image height in pixels
 * @param start_x X coord of start point in fixedpoint-16, corresponding to the original *x_min value
 * @param start_y Y coord of start point in fixedpoint-16, corresponding to the original *x_min value
 * @param dx_x X coord of point delta in x direction in fixedpoint-16
 * @param dx_y Y coord of point delta in x direction in fixedpoint-16
 *
 * @return N/A
 */
static inline void sw_transform_compoute_x_range(
		int32_t *x_min, int32_t *x_max, int16_t img_w, int16_t img_h,
		int32_t start_x, int32_t start_y, int32_t dx_x, int32_t dx_y)
{
	const int32_t img_w_m1 = FIXEDPOINT16(img_w - 1);
	const int32_t img_h_m1 = FIXEDPOINT16(img_h - 1);
	int x_1, x_2;

	/* FIXME: the compiler seems to divide towards to zero. */
	if (dx_x != 0) {
		/*
			* floor(Δx * dx_x + start_x) >= FIXEDPOINT16(0)
			* ceil(Δx * dx_x + start_x) <= img_w_m1
			*/
		if (dx_x > 0) {
			x_1 = (FIXEDPOINT16(0) - start_x + dx_x - 1) / dx_x;
			x_2 = (img_w_m1 - start_x) / dx_x;
		} else {
			x_2 = (FIXEDPOINT16(0) - start_x) / dx_x;
			x_1 = (img_w_m1 - start_x + dx_x + 1) / dx_x;
		}

		*x_min = MAX(*x_min, x_1);
		*x_max = MIN(*x_max, x_2);
	} else if (start_x < FIXEDPOINT16(0) || start_x > img_w_m1) {
		*x_max = *x_min - 1;
		return;
	}

	if (dx_y != 0) {
		/*
			* floor(Δy * dx_y + start_y) >= FIXEDPOINT16(0)
			* ceil(Δy * dx_y + start_y) <= img_h_m1
			*/
		if (dx_y > 0) {
			x_1 = (FIXEDPOINT16(0) - start_y + dx_y - 1) / dx_y;
			x_2 = (img_h_m1 - start_y) / dx_y;
		} else {
			x_2 = (FIXEDPOINT16(0) - start_y) / dx_y;
			x_1 = (img_h_m1 - start_y + dx_y + 1) / dx_y;
		}

		*x_min = MAX(*x_min, x_1);
		*x_max = MIN(*x_max, x_2);
	} else if (start_y < FIXEDPOINT16(0) || start_y > img_h_m1) {
		*x_max = *x_min - 1;
		return;
	}
}

void sw_transform_config(int16_t img_x, int16_t img_y,
		int16_t pivot_x, int16_t pivot_y, uint16_t angle,
		uint16_t zoom_x, uint16_t zoom_y, uint16_t zoom_bits,
		sw_matrix_t *matrix)
{
#ifdef CONFIG_GUI_API_BROM
	if (zoom_x == zoom_y && zoom_x == (1 << zoom_bits) && img_x == 0 && img_y == 0) {
		p_brom_libgui_api->p_sw_rotate_configure(
				0, 0, img_x, img_y, pivot_x, pivot_y, angle, matrix);
		return;
	}
#endif

	uint16_t revert_angle = 3600 - angle;

	assert(angle <= 3600);

	/* coordinates in the destination coordinate system */
	matrix->tx = PX_FIXEDPOINT16(0);
	matrix->ty = PX_FIXEDPOINT16(0);
	matrix->dxdx = FIXEDPOINT16(1);
	matrix->dydx = FIXEDPOINT16(0);
	matrix->dxdy = FIXEDPOINT16(0);
	matrix->dydy = FIXEDPOINT16(1);

	/* rotate back to the source coordinate system */
	sw_transform_point32(&matrix->tx, &matrix->ty, matrix->tx, matrix->ty,
			PX_FIXEDPOINT16(img_x + pivot_x), PX_FIXEDPOINT16(img_y + pivot_y),
			revert_angle, zoom_x, zoom_y, zoom_bits);
	sw_transform_point32(&matrix->dxdx, &matrix->dydx, matrix->dxdx, matrix->dydx,
			FIXEDPOINT12(0), FIXEDPOINT12(0), revert_angle, zoom_x, zoom_y, zoom_bits);
	sw_transform_point32(&matrix->dxdy, &matrix->dydy, matrix->dxdy, matrix->dydy,
			FIXEDPOINT12(0), FIXEDPOINT12(0), revert_angle, zoom_x, zoom_y, zoom_bits);

	/* map to the source pixel coordinate system */
	matrix->tx -= PX_FIXEDPOINT16(img_x);
	matrix->ty -= PX_FIXEDPOINT16(img_y);
}

void sw_transform_rgb565_over_rgb565(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
#ifdef CONFIG_GUI_API_BROM
	p_brom_libgui_api->p_sw_rotate_rgb565_over_rgb565(
			dst, src, dst_stride, src_w, src_h, x, y, w, h, matrix);
#else
	uint16_t * dst16 = dst;
	uint16_t src_pitch = src_w * 2;
	uint16_t src_bytes_per_pixel = 2;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint16_t *tmp_dst = dst16;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;

			*tmp_dst = bilinear_rgb565_fast_m6(*(uint16_t*)src1,
					*(uint16_t*)src2, *(uint16_t*)src3, *(uint16_t*)src4,
					x_frac >> 10, y_frac >> 10, 6);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst16 += dst_stride;
	}
#endif /* CONFIG_GUI_API_BROM */
}

void sw_transform_rgb565_over_argb8888(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
	uint32_t * dst32 = dst;
	uint16_t src_pitch = src_w * 2;
	uint16_t src_bytes_per_pixel = 2;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint32_t *tmp_dst = dst32;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;
			uint16_t c16;

			c16 = bilinear_rgb565_fast_m6(*(uint16_t*)src1,
					*(uint16_t*)src2, *(uint16_t*)src3, *(uint16_t*)src4,
					x_frac >> 10, y_frac >> 10, 6);

			*tmp_dst = ((c16 & 0x1f) << 3) | (c16 & 0x07) |
					((c16 & 0x07e0) << 5) | ((c16 & 0x60) << 3) |
					((c16 & 0xf800) << 8) | ((c16 & 0x3800) << 5) | 0xFF000000;

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst32 += dst_stride;
	}
}

void sw_transform_argb8565_over_rgb565(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
	uint16_t * dst16 = dst;
	uint16_t src_pitch = src_w * 3;
	uint16_t src_bytes_per_pixel = 3;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint16_t *tmp_dst = dst16;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;
			uint8_t result[3];

			bilinear_argb8565_fast_m6(result, src1, src2, src3, src4,
					x_frac >> 10, y_frac >> 10, 6);

			*tmp_dst = blend_argb8565_over_rgb565(*tmp_dst, result);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst16 += dst_stride;
	}
}

void sw_transform_argb8565_over_argb8888(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
	uint32_t * dst32 = dst;
	uint16_t src_pitch = src_w * 3;
	uint16_t src_bytes_per_pixel = 3;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint32_t *tmp_dst = dst32;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;
			uint8_t result[3];

			bilinear_argb8565_fast_m6(result, src1, src2, src3, src4,
					x_frac >> 10, y_frac >> 10, 6);

			*tmp_dst = blend_argb8565_over_argb8888(*tmp_dst, result);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst32 += dst_stride;
	}
}

void sw_transform_argb6666_over_rgb565(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
	uint16_t * dst16 = dst;
	uint16_t src_pitch = src_w * 3;
	uint16_t src_bytes_per_pixel = 3;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint16_t *tmp_dst = dst16;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;
			uint8_t result[3];

			bilinear_argb6666_fast_m6(result, src1, src2, src3, src4,
					x_frac >> 10, y_frac >> 10, 6);

			*tmp_dst = blend_argb6666_over_rgb565(*tmp_dst, result);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst16 += dst_stride;
	}
}

void sw_transform_argb6666_over_argb8888(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
	uint32_t * dst32 = dst;
	uint16_t src_pitch = src_w * 3;
	uint16_t src_bytes_per_pixel = 3;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint32_t *tmp_dst = dst32;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;
			uint8_t result[3];

			bilinear_argb6666_fast_m6(result, src1, src2, src3, src4,
					x_frac >> 10, y_frac >> 10, 6);

			*tmp_dst = blend_argb6666_over_argb8888(*tmp_dst, result);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst32 += dst_stride;
	}
}

void sw_transform_argb8888_over_rgb565(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
#ifdef CONFIG_GUI_API_BROM
	p_brom_libgui_api->p_sw_rotate_argb8888_over_rgb565(
			dst, src, dst_stride, src_w, src_h, x, y, w, h, matrix);
#else
	uint16_t * dst16 = dst;
	uint16_t src_pitch = src_w * 4;
	uint16_t src_bytes_per_pixel = 4;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint16_t *tmp_dst = dst16;

		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;

			uint32_t color = bilinear_argb8888_fast_m8(*(uint32_t*)src1,
					*(uint32_t*)src2, *(uint32_t*)src3, *(uint32_t*)src4,
					x_frac >> 8, y_frac >> 8, 8);

			*tmp_dst = blend_argb8888_over_rgb565(*tmp_dst, color);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst16 += dst_stride;
	}
#endif /* CONFIG_GUI_API_BROM */
}

void sw_transform_argb8888_over_argb8888(void *dst, const void *src,
		uint16_t dst_stride, uint16_t src_w, uint16_t src_h,
		int16_t x, int16_t y, uint16_t w, uint16_t h,
		const sw_matrix_t *matrix)
{
#ifdef CONFIG_GUI_API_BROM
	p_brom_libgui_api->p_sw_rotate_argb8888_over_argb8888(
			dst, src, dst_stride, src_w, src_h, x, y, w, h, matrix);
#else
	uint32_t * dst32 = dst;
	uint16_t src_pitch = src_w * 4;
	uint16_t src_bytes_per_pixel = 4;
	int32_t src_coord_x = matrix->tx +
			y * matrix->dxdy + x * matrix->dxdx;
	int32_t src_coord_y = matrix->ty +
			y * matrix->dydy + x * matrix->dydx;

	for (int j = h; j > 0; j--) {
		int32_t p_x = src_coord_x;
		int32_t p_y = src_coord_y;
		uint32_t *tmp_dst = dst32;
		int x1 = 0, x2 = w - 1;

		sw_transform_compoute_x_range(&x1, &x2, src_w, src_h,
				p_x, p_y, matrix->dxdx, matrix->dydx);
		if (x1 > x2) {
			goto next_line;
		} else if (x1 > 0) {
			p_x += matrix->dxdx * x1;
			p_y += matrix->dydx * x1;
			tmp_dst += x1;
		}

		for (int i = x2 - x1; i >= 0; i--) {
			int x = FLOOR_FIXEDPOINT16(p_x);
			int y = FLOOR_FIXEDPOINT16(p_y);
			int x_frac = p_x - FIXEDPOINT16(x);
			int y_frac = p_y - FIXEDPOINT16(y);
			uint8_t *src1 = (uint8_t *)src + y * src_pitch + x * src_bytes_per_pixel;
			uint8_t *src2 = src1 + src_bytes_per_pixel;
			uint8_t *src3 = src1 + src_pitch;
			uint8_t *src4 = src2 + src_pitch;

			uint32_t color = bilinear_argb8888_fast_m8(*(uint32_t*)src1,
					*(uint32_t*)src2, *(uint32_t*)src3, *(uint32_t*)src4,
					x_frac >> 8, y_frac >> 8, 8);

			*tmp_dst = blend_argb8888_over_argb8888(*tmp_dst, color);

			tmp_dst += 1;
			p_x += matrix->dxdx;
			p_y += matrix->dydx;
		}

next_line:
		src_coord_x += matrix->dxdy;
		src_coord_y += matrix->dydy;
		dst32 += dst_stride;
	}
#endif /* CONFIG_GUI_API_BROM */
}
