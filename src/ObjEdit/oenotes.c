TODO ON OBJEDIT:

Files:

	Specify texture by dragging into the window
	Merge object by dragging into the window
	Load new file by dragging into the window
	Create new LOD by dragging into the window	

Navigation -

	*** a way to change the size/split of the 4 windows (Mahesh seconds this)
	caption: zoom factor for texture
	margin around texture - clamp to it?

Selection:
	control key and more modifiers for selection?!?
	
Sel Cmds:
	select joined
	select opposite (if it exists)
	
Textures 

	Edit texture as a square
	Better constrained drag (squarify, align to nearby, etc.)
	(show special tex for "no texture")
	Show lit textures in texture win!??
	"Reset" texture should wrap a texture around the quads, not project in 3-d space.

Texture patches
	projecting a texture from a patch

Obj Processing
	quad merging
	tri->quad
	build strips
	
LOD
	A way to merge in new objs as LOD
	A way to set LOD
	A way to auto-recommend LOD
	A way to preview the LOD transitions.

Attributes, etc.
	Materials and color editing!
	Attribute Editing	

Hidden Faces, Locked faces, unselectable
	(allow hidden faces, delete hidden cmd)
	saving selections

Anthony's Requests:
	1. A way to specify the texture bitmap in the program.
	2. Performance.
	3. A way to work with the texture in tiling/repeating mode.
	4. A way to reversea hidden surface that goes the wrong way.
	5. Patches - generally need some ework.
	6. Projection - planar, spherical, cubic or cylindrical.
		Default should be some kind of alignment between the selection and the
		projecting sphere.
	7. Dragging S&T gets cumbersome

Anthony wants: scaling of all selected S&Ts as a whole.
Bias "reset" to current axis.

	- Make deformer appear in a useful place.	
	- Get a rect of pts that deforms them.
	- Fix font color ????	


TODO (obj edit soon):
-----
1. Add LOD control
2. Add Merging
3. Add vertex management via rotate, etc.
4. swatch placement is totally brain-dead!
DESIGN ISSUES:
1. can save without applying!
2. undo across multiple LODs?!?  can be confusing at best!



SERGIO'S OBJEDIT NOTES:
 - wants iso mode
 - use real open boxes
 - why don't textures open properly?
 - fix application of patches!!
 - resizing panes
 - make projector line up auomatically
รรร fix mouse zoom and lower camera FOV
***grouping and selection features
***texture rescaling to make more room in a bitmap.
***triangle merging
 
