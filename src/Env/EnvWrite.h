/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef _EnvWrite_h_
#define _EnvWrite_h_

#define	WRITE_ERR		1002

int		EnvWrite(const char * inFileName);

void	GetNthVertex(
					long		inH,
					long		inV,
					double *	outLatitude,
					double *	outLongitude,
					float *		outElevation,
					short *		outLandUse,
					int *		outCustom,
					short *		outRotation,
					short *		outTextureScale,
					short *		outTextureXOffset,
					short *		outTextureYOffset,
					short *		outBodyID);

int		GetNthObject(
					long		inNth,
					long *		outKind,
					float *		outLatitude,
					float *		outLongitude,
					float *		outElevationHeading,
					char *		outName);

int		GetNthRoadSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthTrailSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthTrainSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthPowerSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthTaxiwaySegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthRiverSegment(
					long		inNth,
					double *	outLatitude,
					double *	outLongitude,
					int *		outLast);

int		GetNthTexture(
					long		inNth,
					char *		outName);

#endif