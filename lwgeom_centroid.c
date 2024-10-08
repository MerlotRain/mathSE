/**
 * Copyright (c) 2023-present Merlot.Rain
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "liblwgeom_internel.h"
#include <math.h>
#include <assert.h>

struct nv__centriod {
	POINT2D p_cent_sum;
	size_t pt_num;

	POINT2D l_cent_sum;
	double total_length;

	double total_area;
	double total_ax;
	double total_ay;
};

void
nv__centriod_single(const LWGEOM *obj, struct nv__centriod *centriod)
{
	assert(obj);
	if (lwgeom_dim_geometry(obj) == 0)
	{
		centriod->p_cent_sum.x += obj->pp[0];
		centriod->p_cent_sum.y += obj->pp[1];
		centriod->pt_num += 1;
	}
	else if (lwgeom_dim_geometry(obj) == 1)
	{
		size_t npts = obj->npoints;
		double line_len = 0.0;
		for (size_t i = 0; i < npts - 1; ++i)
		{
			double x1 = lwgeom_get_x(obj, i);
			double y1 = lwgeom_get_y(obj, i);
			double x2 = lwgeom_get_x(obj, i + 1);
			double y2 = lwgeom_get_y(obj, i + 1);
			double segment_len = LW_POINTDISTANCE(x1, y1, x2, y2);
			if (segment_len == 0.0)
				continue;

			line_len += segment_len;
			double midx = (x1 + x2) * 0.5;
			double midy = (y1 + y2) * 0.5;
			centriod->l_cent_sum.x += segment_len * midx;
			centriod->l_cent_sum.y += segment_len * midy;
		}
		centriod->total_length += line_len;
		if (line_len == 0.0 && npts > 0)
		{
			centriod->p_cent_sum.x += obj->pp[0];
			centriod->p_cent_sum.y += obj->pp[1];
			centriod->pt_num += 1;
		}
	}
	else if (lwgeom_dim_geometry(obj) == 2)
	{
		double area = lwgeom_prop_area(obj);
		double tx = 0.0;
		double ty = 0.0;
		for (int i = 0; i < obj->npoints; ++i)
		{
			tx += lwgeom_get_x(obj, i);
			ty += lwgeom_get_y(obj, i);
		}
		centriod->total_area += area;
		centriod->total_ax += (tx / obj->npoints);
		centriod->total_ay += (ty / obj->npoints);
	}
}

void
nv_prop_geo_centriod(const LWGEOM *obj, double *xy)
{
	assert(obj);
	struct nv__centriod centriod;
	if (obj->ngeoms == 1)
	{
		nv__centriod_single(obj, &centriod);
	}
	else
	{
		for (int i = 0; i < obj->ngeoms; ++i)
		{
			nv__centriod_single(obj->geoms[i], &centriod);
		}
	}

	if (lwgeom_dim_geometry(obj) == 0)
	{
		xy[0] = centriod.p_cent_sum.x / centriod.pt_num;
		xy[1] = centriod.p_cent_sum.y / centriod.pt_num;
	}
	else if (lwgeom_dim_geometry(obj) == 1)
	{
		xy[0] = centriod.l_cent_sum.x / centriod.total_length;
		xy[1] = centriod.l_cent_sum.y / centriod.total_length;
	}
	else if (lwgeom_dim_geometry(obj) == 2)
	{
		xy[0] = centriod.total_area / centriod.total_ax;
		xy[1] = centriod.total_area / centriod.total_ay;
	}
}