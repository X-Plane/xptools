#include "MacImageToClipboard.h"

bool MacImageToClipboard(const unsigned char * rgb, int width, int height)
{
		QDErr			qderr;
		GWorldPtr		old_world = NULL, gworld = NULL;
		GDHandle		old_gd = NULL;
		PixMapHandle	pixmap;
		Rect			bounds;
		bool			result = false;
		unsigned char *	rgba;
		long			rowbytes_src, rowbytes_dst, pixel_src, pixel_dst;
		int				x, y;
		PicHandle		pic_h = NULL;
		ScrapRef		scrap;		

	/* First we need to create an off-screen graphics environment.  We'll copy our
	 * OpenGL image into that environment (which is XRGB).  That environment is
	 * a Mac structure and can thus be sent to the clipboard. 
	 *
	 * XRGB??!  Well, Mac QuickDraw uses 32-bits for an RGB pixel, with the first 
	 * byte unussed.  So it's like ARGB but ignored. */
		
	SetRect(&bounds, 0, 0, width, height);	
	qderr = NewGWorld(&gworld, 32, &bounds, NULL, NULL, 0);
	if (qderr != noErr) goto bail;
	
	GetGWorld(&old_world, &old_gd);
	SetGWorld(gworld, NULL);
	
	/* Find the memory for the Mac drawing environment.  It will always be XRGB format
	 * since we asked for 32-bits deep. */
	
	pixmap = GetGWorldPixMap(gworld);
	rgba = (unsigned char *) GetPixBaseAddr(pixmap);
	rowbytes_dst = GetPixRowBytes(pixmap);
	rowbytes_src = width * 3;
	pixel_dst = 4;
	pixel_src = 3;
	
	/* Perform a copy from RGB to XRGB.  We also take this change to invert the image
 	 * vertically since the origin is the upperleft for the Mac. */

	for (y = 0; y < height; ++y)
	for (x = 0; x < width; ++x)
	{
		const unsigned char *	srcp = rgb + x * pixel_src + (height - y - 1) * rowbytes_src;
		unsigned char *	dstp = rgba + x * pixel_dst + y * rowbytes_dst;
		
		dstp[1] = srcp[0];
		dstp[2] = srcp[1];
		dstp[3] = srcp[2];
	}

	/* We now create a Mac 'picture' from the offscreen by copying the image onto itself. */

	pic_h = OpenPicture(&bounds);
	if (pic_h == NULL) goto bail;
	
	CopyBits(GetPortBitMapForCopyBits(gworld), GetPortBitMapForCopyBits(gworld), &bounds, &bounds, srcCopy, NULL);
	
	ClosePicture();

	/* Given a picture, we can then send it to the scrap book. */

	if (ClearCurrentScrap() != noErr) 
		goto bail;
	if (GetCurrentScrap(&scrap) != noErr) 
		goto bail;

	HLock((Handle) pic_h);

	if (PutScrapFlavor( scrap, kScrapFlavorTypePicture, kScrapFlavorMaskNone, GetHandleSize((Handle) pic_h), *pic_h) != noErr) 
		goto bail;
	
	result = true;
	
bail:

	if (pic_h)
		KillPicture(pic_h);
		
	if (old_world)
		SetGWorld(old_world, old_gd);

	if (gworld)
		DisposeGWorld(gworld);
	return result;
}
