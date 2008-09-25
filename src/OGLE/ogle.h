/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef OGLE_H
#define OGLE_H

/*

	OGLE COORDINATE SYSTEM

	Each handle requires two rectangles:
	- Drawing coordinates are the coordinate system that OGLE needs to pass to the callbacks to put
	  text on the screen. +X = right and +Y = up.
	- The visible boundary defines how much of the text can be seen currently, in drawing coordinates.
	- The logical boundary defines the entire size of the document, in drawing coordinates.
	- Neither rectangle needs to be aligned with the origin of the drawing coordinate system in any way.

	Scrolling deltas are always positive numbers expressed from the lower left corner of the logical
	view to the lower left corner of the visible view.  Typically these will always be positive, but if
	the visible view is taller than the logical view, the vertical scroll position can go negative to
	allow this.




*/


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************************
 * C DATATYPES
 *******************************************************************************************/

/* Special keys */
enum {
	ogle_DeleteBack 	= 8,
	ogle_Left 			= 28,
	ogle_Right 			= 29,
	ogle_Up 			= 30,
	ogle_Down 			= 31
};


struct OGLE_Rec;
typedef OGLE_Rec *		OGLE_Handle;

/*******************************************************************************************
 * C CALLBACKS
 *******************************************************************************************/

struct OGLE_Callbacks {

	/********************** GEOMETRY CALLBACKS *********************/

	/* Called to determine the visible bounds in OpenGL space of the editor. */
	void			(* GetVisibleBounds_f)(
							OGLE_Handle		handle,
							float			bounds[4]);

	/* Called to determine the logical bounds in OpenGL space of the editor. */
	void			(* GetLogicalBounds_f)(
							OGLE_Handle		handle,
							float			bounds[4]);

	/* Called to attempt to change the physical bounds of the editor.  You do
	 * not HAVE to accept this change-request; OGLE will re-request the logical
	 * bounds to figure out what really happened.   This is called both when the
	 * total text grows and shrinks, so if you accept this your right-side scrollbar
	 * can be good. */
	void			(* SetLogicalHeight_f)(
							OGLE_Handle		handle,
							float			height);

	/* Requests scrolling to a position - again OGLE will re-request bounds to see
	 * what really happened.  Coodrinates are the off logcal->physical lower left
	 * corner vector, so the values are always positive. */
	void			(* ScrollTo_f)(
							OGLE_Handle		handle,
							float			where[2]);

	/********************** RAW TEXT DATA *********************/


	void			(* GetText_f)(
							OGLE_Handle		handle,
							const char **	start_p,
							const char **	end_p);
	void			(* ReplaceText_f)(
							OGLE_Handle		handle,
							int				offset1,
							int				offset2,
							const char *	t1,
							const char *	t2);

	/********************** FONT MANAGEMENT *********************/

	float			(* GetLineHeight_f)(
							OGLE_Handle 	handle);
	float			(* MeasureString_f)(
							OGLE_Handle 	handle,
							const char * 	tStart,
							const char * 	tEnd);
	int				(* FitStringFwd_f)(
							OGLE_Handle 	handle,
							const char * 	tStart,
							const char * 	tEnd,
							float 			space);
	int				(* FitStringRev_f)(
							OGLE_Handle 	handle,
							const char * 	tStart,
							const char * 	tEnd,
							float 			space);
	void			(* DrawString_f)(
							OGLE_Handle		handle,
							const char *	tStart,
							const char *	tEnd,
							float			x,
							float			y);
	void			(* DrawSelection_f)(
							OGLE_Handle		handle,
							float			bounds[4]);

	/********************** MISC *********************/

	const char *	(* WordBreak_f)(
							OGLE_Handle		handle,
							const char *	t1,
							const char *	t2);

};

/*******************************************************************************************
 * C FUNCTION CALLS
 *******************************************************************************************/

OGLE_Handle		OGLE_Create(
						OGLE_Callbacks *	callbacks,
						void *				ref);

void			OGLE_Destroy(
						OGLE_Handle 		handle);

void *			OGLE_GetRef(
						OGLE_Handle			handle);

void			OGLE_Draw(
						OGLE_Handle			handle,
						int					draw_caret);

void			OGLE_Key(
						OGLE_Handle			handle,
						char				key,
						int					extend);

void			OGLE_Click(
						OGLE_Handle			handle,
						float				x,
						float				y,
						int					extend);

void			OGLE_Drag(
						OGLE_Handle			handle,
						float				x,
						float				y);


void			OGLE_ReplaceText(
						OGLE_Handle			handle,
						int					offset1,
						int					offset2,
						const char *		t1,
						const char *		T2);

void			OGLE_GetSelection(
						OGLE_Handle			handle,
						int *				offset1,
						int	*				offset2);

void			OGLE_SetSelection(
						OGLE_Handle			handle,
						int					offset1,
						int					offset2);

void			OGLE_RevealSelection(
						OGLE_Handle			handle);

void			OGLE_Repaginate(
						OGLE_Handle			handle);


#ifdef __cplusplus
}


/*******************************************************************************************
 * C++ WRAPPER OBJECT
 *******************************************************************************************/


class	OGLE {
public:
								 OGLE();
	virtual						~OGLE();

			void			Draw(
									int					draw_caret);

			void			Key(
									char				key,
									int					extend);

			void			Click(
									float				x,
									float				y,
									int					extend);

			void			Drag(
									float				x,
									float				y);

			void			DoReplaceText(
									int					offset1,
									int					offset2,
									const char *		t1,
									const char *		t2);

			void			GetSelection(
									int *				offset1,
									int	*				offset2);

			void			SetSelection(
									int					offset1,
									int					offset2);

			void			RevealSelection(void);

			void			Repaginate(void);

protected:

	virtual	void			GetVisibleBounds(
								float			bounds[4])=0;
	virtual	void			GetLogicalBounds(
								float			bounds[4])=0;
	virtual	void			SetLogicalHeight(
								float 			height)=0;
	virtual	void			ScrollTo(
								float			where[2])=0;
	virtual	void			GetText(
								const char **	start_p,
								const char **	end_p)=0;
	virtual	void			ReplaceText(
								int				offset1,
								int				offset2,
								const char *	t1,
								const char *	t2)=0;
	virtual	float			GetLineHeight(void)=0;
	virtual	float			MeasureString(
								const char * 	tStart,
								const char * 	tEnd)=0;
	virtual	int				FitStringFwd(
								const char * 	tStart,
								const char * 	tEnd,
								float 			space)=0;
	virtual	int				FitStringRev(
								const char * 	tStart,
								const char * 	tEnd,
								float 			space)=0;
	virtual	void			DrawString(
								const char *	tStart,
								const char *	tEnd,
								float			x,
								float			y)=0;
	virtual	void			DrawSelection(
								float			bounds[4])=0;
	virtual	const char *	WordBreak(
								const char *	t1,
								const char *	t2)=0;

private:

	static	void			GetVisibleBoundsCB(
								OGLE_Handle		handle,
								float			bounds[4]);
	static	void			GetLogicalBoundsCB(
								OGLE_Handle		handle,
								float			bounds[4]);
	static	void			SetLogicalHeightCB(
								OGLE_Handle		handle,
								float			height);
	static	void			ScrollToCB(
								OGLE_Handle		handle,
								float			where[2]);
	static	void			GetTextCB(
								OGLE_Handle		handle,
								const char **	start_p,
								const char **	end_p);
	static	void			ReplaceTextCB(
								OGLE_Handle		handle,
								int				offset1,
								int				offset2,
								const char *	t1,
								const char *	t2);
	static	float			GetLineHeightCB(
								OGLE_Handle 	handle);
	static	float			MeasureStringCB(
								OGLE_Handle 	handle,
								const char * 	tStart,
								const char * 	tEnd);
	static	int				FitStringFwdCB(
								OGLE_Handle 	handle,
								const char * 	tStart,
								const char * 	tEnd,
								float 			space);
	static	int				FitStringRevCB(
								OGLE_Handle 	handle,
								const char * 	tStart,
								const char * 	tEnd,
								float 			space);
	static	void			DrawStringCB(
								OGLE_Handle		handle,
								const char *	tStart,
								const char *	tEnd,
								float			x,
								float			y);
	static	void			DrawSelectionCB(
								OGLE_Handle		handle,
								float			bounds[4]);
	static	const char *	WordBreakCB(
								OGLE_Handle		handle,
								const char *	t1,
								const char *	t2);

	OGLE_Handle		mHandle;


};


#endif


#endif /* OGLE_H */
