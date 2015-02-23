//	WinDjView
//	Copyright (C) 2004-2015 Andrew Zhezherun
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along
//	with this program; if not, write to the Free Software Foundation, Inc.,
//	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//	http://www.gnu.org/copyleft/gpl.html
//
//
//	Uses pnmscalefixed scaling algorithm from netpbm project
//	Copyright (C) 1989, 1991 by Jef Poskanzer
//
//  Uses a modificaiton of pnmscalefixed with subpixel scaling
//  written by Larry Davis.


#include "stdafx.h"
#include "Scaling.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define FULLSCALE  4096
#define SCALESHIFT 12
#define HALFSCALE  2048

void horizontal_scale(const GPixel* inputxelrow, GPixel* newxelrow, UINT cols, UINT newcols, UINT sxscale)
{
	// Take the input row inputxelrow[], which is 'cols' columns wide, and
	// scale it by a factor of 'sxcale', which is in SCALEths to create
	// the output row newxelrow[], which is 'newcols' columns wide.

	UINT r = 0, g = 0, b = 0;
	UINT newcol = 0;
	UINT fraccoltofill = FULLSCALE;  // Output column is "empty" now

	const GPixel* piP = inputxelrow;
	GPixel* piN = newxelrow;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		// Process one pixel from input ('inputxelrow')
		UINT fraccolleft = sxscale;

		// Output all columns, if any, that can be filled using information
		// from this input column, in addition what's already in the output column.
		while (fraccolleft >= fraccoltofill)
		{
			// Generate one output pixel in 'newxelrow'.  It will consist
			// of anything accumulated from prior input pixels in 'r','g',
			// and 'b', plus a fraction of the current input pixel.

			r += fraccoltofill * piP->r;
			g += fraccoltofill * piP->g;
			b += fraccoltofill * piP->b;
			piN->r = (BYTE) (r >> SCALESHIFT);
			piN->g = (BYTE) (g >> SCALESHIFT);
			piN->b = (BYTE) (b >> SCALESHIFT);
			r = g = b = 0;

			// Set up to start filling next output column
			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		// There's not enough left in the current input pixel to fill up
		// a whole output column, so just accumulate the remainder of the
		// pixel into the current output column.
		if (fraccolleft > 0)
		{
			r += fraccolleft * piP->r;
			g += fraccolleft * piP->g;
			b += fraccolleft * piP->b;

			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		// We ran out of input columns before we filled up the output
		// columns.  This would be because of rounding down.  For small
		// images, we're probably missing only a tiny fraction of a column,
		// but for large images, it could be multiple columns.

		// So we fake the remaining output columns by copying the rightmost
		// legitimate pixel.  We call this stretching.

		r += fraccoltofill * (piP - 1)->r + HALFSCALE; // for rounding
		g += fraccoltofill * (piP - 1)->g + HALFSCALE; // for rounding
		b += fraccoltofill * (piP - 1)->b + HALFSCALE; // for rounding

		piN->r = (BYTE) (r >>= SCALESHIFT);
		piN->g = (BYTE) (g >>= SCALESHIFT);
		piN->b = (BYTE) (b >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
}

void horizontal_scale_subpix(const GPixel* inputxelrow, GPixel* newxelrow, UINT cols, UINT newcols, UINT sxscale)
{
	// Take the input row inputxelrow[], which is 'cols' columns wide, and
	// scale it by a factor of 'sxcale', which is in SCALEths to create
	// the output row newxelrow[], which is 'newcols' columns wide.

	UINT r = 0, g = 0, b = 0;
	UINT newcol = 0;
	UINT fraccoltofill = FULLSCALE;  // Output column is "empty" now

	const GPixel* piP = inputxelrow;
	GPixel* piN = newxelrow;

	// Red
	fraccoltofill = FULLSCALE / 3;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		// Process one pixel from input ('inputxelrow')
		UINT fraccolleft = sxscale;

		// Output all columns, if any, that can be filled using information
		// from this input column, in addition what's already in the output column.
		while (fraccolleft >= fraccoltofill)
		{
			// Generate one output pixel in 'newxelrow'.  It will consist
			// of anything accumulated from prior input pixels in 'r','g',
			// and 'b', plus a fraction of the current input pixel.

			r += fraccoltofill * piP->r;
			piN->r = (BYTE) (r >> SCALESHIFT);
			r = g = b = 0;

			// Set up to start filling next output column
			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		// There's not enough left in the current input pixel to fill up
		// a whole output column, so just accumulate the remainder of the
		// pixel into the current output column.
		if (fraccolleft > 0)
		{
			r += fraccolleft * piP->r;

			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		// We ran out of input columns before we filled up the output
		// columns.  This would be because of rounding down.  For small
		// images, we're probably missing only a tiny fraction of a column,
		// but for large images, it could be multiple columns.

		// So we fake the remaining output columns by copying the rightmost
		// legitimate pixel.  We call this stretching.

		r += fraccoltofill * (piP - 1)->r + HALFSCALE; // for rounding

		piN->r = (BYTE) (r >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
	newxelrow->r += 2 * newxelrow->r;

	// Green
	fraccoltofill = (2 * FULLSCALE) / 3;
	piP = inputxelrow;
	piN = newxelrow;
	newcol = 0;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		// Process one pixel from input ('inputxelrow')
		UINT fraccolleft = sxscale;

		// Output all columns, if any, that can be filled using information
		// from this input column, in addition what's already in the output column.
		while (fraccolleft >= fraccoltofill)
		{
			// Generate one output pixel in 'newxelrow'.  It will consist
			// of anything accumulated from prior input pixels in 'r','g',
			// and 'b', plus a fraction of the current input pixel.

			g += fraccoltofill * piP->g;
			piN->g = (BYTE) (g >> SCALESHIFT);
			r = g = b = 0;

			// Set up to start filling next output column
			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		// There's not enough left in the current input pixel to fill up
		// a whole output column, so just accumulate the remainder of the
		// pixel into the current output column.
		if (fraccolleft > 0)
		{
			g += fraccolleft * piP->g;

			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		// We ran out of input columns before we filled up the output
		// columns.  This would be because of rounding down.  For small
		// images, we're probably missing only a tiny fraction of a column,
		// but for large images, it could be multiple columns.

		// So we fake the remaining output columns by copying the rightmost
		// legitimate pixel.  We call this stretching.

		g += fraccoltofill * (piP - 1)->g + HALFSCALE; // for rounding

		piN->g = (BYTE) (g >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
	newxelrow->g += newxelrow->g / 2;

	// Blue
	fraccoltofill = FULLSCALE;
	piP = inputxelrow;
	piN = newxelrow;
	newcol = 0;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		// Process one pixel from input ('inputxelrow')
		UINT fraccolleft = sxscale;

		// Output all columns, if any, that can be filled using information
		// from this input column, in addition what's already in the output column.
		while (fraccolleft >= fraccoltofill)
		{
			// Generate one output pixel in 'newxelrow'.  It will consist
			// of anything accumulated from prior input pixels in 'r','g',
			// and 'b', plus a fraction of the current input pixel.

			b += fraccoltofill * piP->b;
			piN->b = (BYTE) (b >> SCALESHIFT);
			r = g = b = 0;

			// Set up to start filling next output column
			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		// There's not enough left in the current input pixel to fill up
		// a whole output column, so just accumulate the remainder of the
		// pixel into the current output column.
		if (fraccolleft > 0)
		{
			b += fraccolleft * piP->b;

			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		// We ran out of input columns before we filled up the output
		// columns.  This would be because of rounding down.  For small
		// images, we're probably missing only a tiny fraction of a column,
		// but for large images, it could be multiple columns.

		// So we fake the remaining output columns by copying the rightmost
		// legitimate pixel.  We call this stretching.

		b += fraccoltofill * (piP - 1)->b + HALFSCALE; // for rounding

		piN->b = (BYTE) (b >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
}

void horizontal_scale(const BYTE* inputxelrow, BYTE* newxelrow, UINT cols, UINT newcols, UINT sxscale)
{
	UINT newcol = 0;
	UINT fraccoltofill = FULLSCALE;
	UINT r = 0;
	const BYTE* piP = inputxelrow;
	BYTE* piN = newxelrow;

	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		UINT fraccolleft = sxscale;
		while (fraccolleft >= fraccoltofill)
		{
			r += fraccoltofill * (*piP);
			*piN = (BYTE) (r >> SCALESHIFT);
			r = 0;

			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		if (fraccolleft > 0)
		{
			r += fraccolleft * (*piP);
			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		r += fraccoltofill * (*(piP - 1)) + HALFSCALE;
		*piN = (BYTE) (r >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
}

void horizontal_scale_subpix(const BYTE* inputxelrow, GPixel* newxelrow, UINT cols, UINT newcols, UINT sxscale)
{
	UINT newcol = 0;
	UINT fraccoltofill = FULLSCALE;
	UINT r = 0;
	const BYTE* piP = inputxelrow;
	GPixel* piN = newxelrow;

	// Red
	fraccoltofill = FULLSCALE / 3;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		UINT fraccolleft = sxscale;
		while (fraccolleft >= fraccoltofill)
		{
			r += fraccoltofill * (*piP);
			piN->r = (BYTE) (r >> SCALESHIFT);
			r = 0;

			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		if (fraccolleft > 0)
		{
			r += fraccolleft * (*piP);
			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		r += fraccoltofill * (*(piP - 1)) + HALFSCALE;
		piN->r = (BYTE) (r >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
	newxelrow->r += 2 * newxelrow->r;

	// Green
	fraccoltofill = (2 * FULLSCALE) / 3;
	piP = inputxelrow;
	piN = newxelrow;
	newcol = 0;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		UINT fraccolleft = sxscale;
		while (fraccolleft >= fraccoltofill)
		{
			r += fraccoltofill * (*piP);
			piN->g = (BYTE) (r >> SCALESHIFT);
			r = 0;

			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		if (fraccolleft > 0)
		{
			r += fraccolleft * (*piP);
			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		r += fraccoltofill * (*(piP - 1)) + HALFSCALE;
		piN->g = (BYTE) (r >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
	newxelrow->g += newxelrow->g / 2;

	// Blue
	fraccoltofill = FULLSCALE;
	piP = inputxelrow;
	piN = newxelrow;
	newcol = 0;
	for (UINT col = 0; col < cols; ++col, ++piP)
	{
		UINT fraccolleft = sxscale;
		while (fraccolleft >= fraccoltofill)
		{
			r += fraccoltofill * (*piP);
			piN->b = (BYTE) (r >> SCALESHIFT);
			r = 0;

			++piN;
			++newcol;
			fraccolleft -= fraccoltofill;
			fraccoltofill = FULLSCALE;
		}

		if (fraccolleft > 0)
		{
			r += fraccolleft * (*piP);
			fraccoltofill -= fraccolleft;
		}
	}

	while (newcol < newcols)
	{
		r += fraccoltofill * (*(piP - 1)) + HALFSCALE;
		piN->b = (BYTE) (r >>= SCALESHIFT);

		++piN;
		++newcol;
		fraccoltofill = FULLSCALE;
	}
}

GP<GPixmap> RescalePnm(GP<GPixmap> pSrc, UINT nWidth, UINT nHeight)
{
	GP<GPixmap> pGPixmap = GPixmap::create(nHeight, nWidth);

	UINT cols = pSrc->columns();
	UINT rows = pSrc->rows();
	UINT newcols = nWidth;
	UINT newrows = nHeight;

	// We round the scale factor down so that we never fill up the
	// output while (a fractional pixel of) input remains unused.
	// Instead, we will run out of input while some of the output is
	// unfilled.  We can address that by stretching, whereas the other
	// case would require throwing away some of the input.
	UINT sxscale = FULLSCALE * newcols / cols;
	UINT syscale = FULLSCALE * newrows / rows;

	GPixel* tempxelrow = (GPixel*) malloc(cols * sizeof(GPixel));
	unsigned int* rs = (UINT*) malloc(cols * sizeof(UINT) * 3);
	unsigned int* gs = rs + cols;
	unsigned int* bs = gs + cols;
	if (tempxelrow == NULL || rs == NULL)
	{
		if (tempxelrow != NULL)
			free(tempxelrow);
		if (rs != NULL)
			free(rs);
		return pGPixmap;
	}

	UINT rowsread = 0;
	UINT fracrowleft = syscale;
	UINT needtoreadrow = 1;
	UINT fracrowtofill = FULLSCALE;

	for (UINT col = 0; col < cols; ++col)
		rs[col] = gs[col] = bs[col] = HALFSCALE;

	int vertical_stretch = 0;
	GPixel* xelrow = 0;
	GPixel* xP;
	GPixel* nxP;

	for (UINT row = 0; row < newrows; ++row)
	{
		// First scale vertically from xelrow into tempxelrow.
		if (newrows == rows)
		{
			// shortcut vertical scaling if possible
			xelrow = (*pSrc)[rowsread];
			++rowsread;
		}
		else
		{
			while (fracrowleft < fracrowtofill)
			{
				if (needtoreadrow)
				{
					if (rowsread < rows)
					{
						xelrow = (*pSrc)[rowsread];
						++rowsread;
					}
				}

				UINT* prs = rs;
				UINT* pgs = gs;
				UINT* pbs = bs;
				UINT col;
				for (col = 0, xP = xelrow; col < cols; ++col, ++xP)
				{
					*prs++ += fracrowleft * xP->r;
					*pgs++ += fracrowleft * xP->g;
					*pbs++ += fracrowleft * xP->b;
				}

				fracrowtofill -= fracrowleft;
				fracrowleft = syscale;
				needtoreadrow = 1;
			}

			// Now fracrowleft is >= fracrowtofill, so we can produce a row.
			if (needtoreadrow)
			{
				if (rowsread < rows)
				{
					xelrow = (*pSrc)[rowsread];
					++rowsread;
					needtoreadrow = 0;
				}
				else
				{
					// We need another input row to fill up this output row,
					// but there aren't any more.  That's because of rounding
					// down on our scaling arithmetic.  So we go ahead with
					// the data from the last row we read, which amounts to
					// stretching out the last output row.
					vertical_stretch += fracrowtofill;
				}
			}

			UINT* prs = rs;
			UINT* pgs = gs;
			UINT* pbs = bs;
			UINT col;
			for (col = 0, xP = xelrow, nxP = tempxelrow; col < cols; ++col, ++xP, ++nxP)
			{
				UINT r = *prs + fracrowtofill * xP->r;
				UINT g = *pgs + fracrowtofill * xP->g;
				UINT b = *pbs + fracrowtofill * xP->b;
				nxP->r = (BYTE) (r >> SCALESHIFT);
				nxP->g = (BYTE) (g >> SCALESHIFT);
				nxP->b = (BYTE) (b >> SCALESHIFT);
				*prs++ = HALFSCALE;
				*pgs++ = HALFSCALE;
				*pbs++ = HALFSCALE;
			}

			fracrowleft -= fracrowtofill;
			if (fracrowleft == 0)
			{
				fracrowleft = syscale;
				needtoreadrow = 1;
			}
			fracrowtofill = FULLSCALE;
		}

		// Now scale tempxelrow horizontally into newxelrow & write it out.
		if (newcols == cols)
		{
			// shortcut X scaling if possible.
			memcpy((*pGPixmap)[row], xelrow, newcols*sizeof(GPixel));
		}
		else
		{
			horizontal_scale(tempxelrow, (*pGPixmap)[row], cols, newcols, sxscale);
		}
	}

	free(tempxelrow);
	free(rs);

	return pGPixmap;
}

GP<GPixmap> RescalePnm_subpix(GP<GPixmap> pSrc, UINT nWidth, UINT nHeight)
{
	GP<GPixmap> pGPixmap = GPixmap::create(nHeight, nWidth);

	UINT cols = pSrc->columns();
	UINT rows = pSrc->rows();
	UINT newcols = nWidth;
	UINT newrows = nHeight;

	// We round the scale factor down so that we never fill up the
	// output while (a fractional pixel of) input remains unused.
	// Instead, we will run out of input while some of the output is
	// unfilled.  We can address that by stretching, whereas the other
	// case would require throwing away some of the input.
	// Sub-pixel rendering eats up extra 2/3 of a pixel.
	// Round down to avoid overflows in horizontal_scale_subpix().
	UINT sxscale = (FULLSCALE * newcols - 2 * FULLSCALE / 3 - 1) / cols;
	UINT syscale = FULLSCALE * newrows / rows;

	GPixel* tempxelrow = (GPixel*) malloc(cols * sizeof(GPixel));
	unsigned int* rs = (UINT*) malloc(cols * sizeof(UINT) * 3);
	unsigned int* gs = rs + cols;
	unsigned int* bs = gs + cols;
	if (tempxelrow == NULL || rs == NULL)
	{
		if (tempxelrow != NULL)
			free(tempxelrow);
		if (rs != NULL)
			free(rs);
		return pGPixmap;
	}

	UINT rowsread = 0;
	UINT fracrowleft = syscale;
	UINT needtoreadrow = 1;
	UINT fracrowtofill = FULLSCALE;

	for (UINT col = 0; col < cols; ++col)
		rs[col] = gs[col] = bs[col] = HALFSCALE;

	int vertical_stretch = 0;
	GPixel* xelrow = 0;
	GPixel* xP;
	GPixel* nxP;

	for (UINT row = 0; row < newrows; ++row)
	{
		// First scale vertically from xelrow into tempxelrow.
		if (newrows == rows)
		{
			// shortcut vertical scaling if possible
			xelrow = (*pSrc)[rowsread];
			++rowsread;
		}
		else
		{
			while (fracrowleft < fracrowtofill)
			{
				if (needtoreadrow)
				{
					if (rowsread < rows)
					{
						xelrow = (*pSrc)[rowsread];
						++rowsread;
					}
				}

				UINT* prs = rs;
				UINT* pgs = gs;
				UINT* pbs = bs;
				UINT col;
				for (col = 0, xP = xelrow; col < cols; ++col, ++xP)
				{
					*prs++ += fracrowleft * xP->r;
					*pgs++ += fracrowleft * xP->g;
					*pbs++ += fracrowleft * xP->b;
				}

				fracrowtofill -= fracrowleft;
				fracrowleft = syscale;
				needtoreadrow = 1;
			}

			// Now fracrowleft is >= fracrowtofill, so we can produce a row.
			if (needtoreadrow)
			{
				if (rowsread < rows)
				{
					xelrow = (*pSrc)[rowsread];
					++rowsread;
					needtoreadrow = 0;
				}
				else
				{
					// We need another input row to fill up this output row,
					// but there aren't any more.  That's because of rounding
					// down on our scaling arithmetic.  So we go ahead with
					// the data from the last row we read, which amounts to
					// stretching out the last output row.
					vertical_stretch += fracrowtofill;
				}
			}

			UINT* prs = rs;
			UINT* pgs = gs;
			UINT* pbs = bs;
			UINT col;
			for (col = 0, xP = xelrow, nxP = tempxelrow; col < cols; ++col, ++xP, ++nxP)
			{
				UINT r = *prs + fracrowtofill * xP->r;
				UINT g = *pgs + fracrowtofill * xP->g;
				UINT b = *pbs + fracrowtofill * xP->b;
				nxP->r = (BYTE) (r >> SCALESHIFT);
				nxP->g = (BYTE) (g >> SCALESHIFT);
				nxP->b = (BYTE) (b >> SCALESHIFT);
				*prs++ = HALFSCALE;
				*pgs++ = HALFSCALE;
				*pbs++ = HALFSCALE;
			}

			fracrowleft -= fracrowtofill;
			if (fracrowleft == 0)
			{
				fracrowleft = syscale;
				needtoreadrow = 1;
			}
			fracrowtofill = FULLSCALE;
		}

		// Now scale tempxelrow horizontally into newxelrow & write it out.
		if (newcols == cols)
		{
			// shortcut X scaling if possible.
			memcpy((*pGPixmap)[row], xelrow, newcols*sizeof(GPixel));
		}
		else
		{
			horizontal_scale_subpix(tempxelrow, (*pGPixmap)[row], cols, newcols, sxscale);
		}
	}

	free(tempxelrow);
	free(rs);

	return pGPixmap;
}

GP<GBitmap> RescalePnm(GP<GBitmap> pSrc, UINT nWidth, UINT nHeight)
{
	const int align = 4;
	int border = ((nWidth + align - 1) & ~(align - 1)) - nWidth;
	GP<GBitmap> pGBitmap = GBitmap::create(nHeight, nWidth, border);
	pGBitmap->change_grays(256);

	UINT cols = pSrc->columns();
	UINT rows = pSrc->rows();
	UINT newcols = nWidth;
	UINT newrows = nHeight;

	UINT nPaletteEntries = pSrc->get_grays();

	BYTE grays[256];
	int color = 0xff0000;
	int decrement = color / (nPaletteEntries - 1);
	for (int i = nPaletteEntries - 1; i >= 0; --i)
	{
		grays[i] = (color >> 16);
		color -= decrement;
	}

	UINT sxscale = FULLSCALE * newcols / cols;
	UINT syscale = FULLSCALE * newrows / rows;

	BYTE* tempxelrow = (BYTE*) malloc(cols * sizeof(BYTE));
	UINT* rs = (UINT*) malloc(cols * sizeof(UINT));
	if (tempxelrow == NULL || rs == NULL)
	{
		if (tempxelrow != NULL)
			free(tempxelrow);
		if (rs != NULL)
			free(rs);
		return pGBitmap;
	}

	UINT rowsread = 0;
	UINT fracrowleft = syscale;
	UINT needtoreadrow = 1;
	UINT fracrowtofill = FULLSCALE;

	for (UINT col = 0; col < cols; ++col)
		rs[col] = HALFSCALE;

	int vertical_stretch = 0;
	BYTE* xelrow = 0;
	BYTE* xP;
	BYTE* nxP;

	for (UINT row = 0; row < newrows; ++row)
	{
		if (newrows == rows)
		{
			// shortcut vertical scaling if possible
			xelrow = (*pSrc)[rowsread];
			++rowsread;
		}
		else
		{
			while (fracrowleft < fracrowtofill)
			{
				if (needtoreadrow)
				{
					if (rowsread < rows)
					{
						xelrow = (*pSrc)[rowsread];
						++rowsread;
					}
				}

				UINT* prs = rs;
				UINT col;
				for (col = 0, xP = xelrow; col < cols; ++col, ++xP)
				{
					*prs += fracrowleft * grays[*xP];
					++prs;
				}

				fracrowtofill -= fracrowleft;
				fracrowleft = syscale;
				needtoreadrow = 1;
			}

			if (needtoreadrow)
			{
				if (rowsread < rows)
				{
					xelrow = (*pSrc)[rowsread];
					++rowsread;
					needtoreadrow = 0;
				}
				else
				{
					vertical_stretch += fracrowtofill;
				}
			}

			UINT* prs = rs;
			UINT col;
			for (col = 0, xP = xelrow, nxP = tempxelrow; col < cols; ++col, ++xP, ++nxP, ++prs)
			{
				UINT r = *prs + fracrowtofill * grays[*xP];
				*nxP = (BYTE) (r >> SCALESHIFT);
				*prs = HALFSCALE;
			}

			fracrowleft -= fracrowtofill;
			if (fracrowleft == 0)
			{
				fracrowleft = syscale;
				needtoreadrow = 1;
			}
			fracrowtofill = FULLSCALE;
		}

		if (newcols == cols)
		{
			// shortcut X scaling if possible
			memcpy((*pGBitmap)[row], xelrow, newcols*sizeof(BYTE));
		}
		else
		{
			horizontal_scale(tempxelrow, (*pGBitmap)[row], cols, newcols, sxscale);
		}
	}

	free(tempxelrow);
	free(rs);

	return pGBitmap;
}

GP<GPixmap> RescalePnm_subpix(GP<GBitmap> pSrc, UINT nWidth, UINT nHeight)
{
	GP<GPixmap> pGPixmap = GPixmap::create(nHeight, nWidth);

	UINT cols = pSrc->columns();
	UINT rows = pSrc->rows();
	UINT newcols = nWidth;
	UINT newrows = nHeight;

	UINT nPaletteEntries = pSrc->get_grays();

	BYTE grays[256];
	int color = 0xff0000;
	int decrement = color / (nPaletteEntries - 1);
	for (int i = nPaletteEntries - 1; i >= 0; --i)
	{
		grays[nPaletteEntries-i-1] = (color >> 16);
		color -= decrement;
	}

	// Sub-pixel rendering eats up extra 2/3 of a pixel.
	// Round down to avoid overflows in horizontal_scale_subpix().
	UINT sxscale = (FULLSCALE * newcols - 2 * FULLSCALE / 3 - 1) / cols;
	UINT syscale = FULLSCALE * newrows / rows;

	BYTE* tempxelrow = (BYTE*) malloc(cols * sizeof(BYTE));
	UINT* rs = (UINT*) malloc(cols * sizeof(UINT));
	if (tempxelrow == NULL || rs == NULL)
	{
		if (tempxelrow != NULL)
			free(tempxelrow);
		if (rs != NULL)
			free(rs);
		return pGPixmap;
	}

	UINT rowsread = 0;
	UINT fracrowleft = syscale;
	UINT needtoreadrow = 1;
	UINT fracrowtofill = FULLSCALE;

	for (UINT col = 0; col < cols; ++col)
		rs[col] = HALFSCALE;

	int vertical_stretch = 0;
	BYTE* xelrow = 0;
	BYTE* xP;
	BYTE* nxP;

	for (UINT row = 0; row < newrows; ++row)
	{
		if (newrows == rows)
		{
			// shortcut vertical scaling if possible
			xelrow = (*pSrc)[rowsread];
			++rowsread;
		}
		else
		{
			while (fracrowleft < fracrowtofill)
			{
				if (needtoreadrow)
				{
					if (rowsread < rows)
					{
						xelrow = (*pSrc)[rowsread];
						++rowsread;
					}
				}

				UINT* prs = rs;
				UINT col;
				for (col = 0, xP = xelrow; col < cols; ++col, ++xP)
				{
					*prs += fracrowleft * grays[*xP];
					++prs;
				}

				fracrowtofill -= fracrowleft;
				fracrowleft = syscale;
				needtoreadrow = 1;
			}

			if (needtoreadrow)
			{
				if (rowsread < rows)
				{
					xelrow = (*pSrc)[rowsread];
					++rowsread;
					needtoreadrow = 0;
				}
				else
				{
					vertical_stretch += fracrowtofill;
				}
			}

			UINT* prs = rs;
			UINT col;
			for (col = 0, xP = xelrow, nxP = tempxelrow; col < cols; ++col, ++xP, ++nxP, ++prs)
			{
				UINT r = *prs + fracrowtofill * grays[*xP];
				*nxP = (BYTE) (r >> SCALESHIFT);
				*prs = HALFSCALE;
			}

			fracrowleft -= fracrowtofill;
			if (fracrowleft == 0)
			{
				fracrowleft = syscale;
				needtoreadrow = 1;
			}
			fracrowtofill = FULLSCALE;
		}

		/*
		// TODO: convert to RGB without subpixel rendering...
		if (newcols == cols)
		{
			// shortcut X scaling if possible
			memcpy((*pGBitmap)[row], xelrow, newcols*sizeof(BYTE));
		}
		else
		*/
		{
			horizontal_scale_subpix(tempxelrow, (*pGPixmap)[row], cols, newcols, sxscale);
		}
	}

	free(tempxelrow);
	free(rs);

	return pGPixmap;
}
