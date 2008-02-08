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
#ifndef _EnvParser_h_
#define _EnvParser_h_

#define	BAD_FORMAT_ERR		1001
#define	CANNOT_OPEN_ERR		1002

#ifdef __cplusplus
extern "C" {
#endif

int	ReadEnvFile(const char * envFileName);

const char *	StripFileName(const char * inFilePath, char inSeparator);

/* Define these routines yourself! */

void	AcceptDimensions(int inH, int inV);
void	AcceptVertex(
					int		inH,
					int		inV,
					double	inLatitude,
					double	inLongitude,
					float	inElevation,
					short	inLandUse,
					int		inCustom,
					short	inRotation,
					short	inTextureScale,
					short	inTextureXOffset,
					short	inTextureYOffset,
					short	inBodyID);
					
void	AcceptObject(
					long	inKind,
					float	inLatitude,
					float	inLongitude,
					float	inElevationHeading,
					char *	inName);
					
void	AcceptRoadSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);

void	AcceptTrailSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);

void	AcceptTrainSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);

void	AcceptElectricSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);

void	AcceptTaxiwaySegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);
					
void	AcceptRiverSegment(
					double	inLatitude,
					double	inLongitude,
					int		isLast);
					
void	AcceptCustomTexture(
					short	inIndex,
					char *	inName);

#ifdef __cplusplus
}
#endif

#endif