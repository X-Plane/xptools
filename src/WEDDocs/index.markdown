Title:  WorldEditor Manual  
Template: manual
Home: http://developer.x-plane.com/docs/scenery/  
Version: 2.3

## Introduction and Setup ##

**About This Manual**

This is version 2.0 of the manual for WorldEditor. To use this manual, you can jump to a section by clicking its title in the table of contents on the side.  Clicking on terms highlighted in blue [like this][1] will take you directly to the relevant text or section. To search for a specific term or set of words, press "ctrl" ("command" on a Mac) + "f" to type the term and be taken to it anywhere in the document. For a PDF version of this manual, use an HTML to PDF converter such as [pdfcrowd.com][2].

### Introduction to Scenery Development in X-Plane ###

#### What the Scenery System Does ####

Scenery in the X-Plane simulator can include essentially everything outside the aircraft. X-Plane is designed specifically to enable users to create and modify scenery themselves. This means that, with a little ambition, a home user with no programming experience could design, say, a realistic version of their home town. This model of My Town, USA could then be easily incorporated into the X-Plane simulator so that upon flying from Neighboring Town, USA into My Town, the scenery seamlessly and transparently moves into the super realistic scenery. These scenery packages can be even be distributed on the Internet so that anyone using the X-Plane desktop simulator can download and install them.

#### What the Scenery is Made Of ####

Scenery in the X-Plane desktop simulator is made up of both scenery files (apt.dat and DSF files) and "resources", text files that describe the various entities referenced in the scenery package. This includes object files for describing buildings, network files for describing road patterns, forest files for describing vegetation, and so on.

In our scenery system, the world is divided into 1 degree latitude by 1 degree longitude tiles, each one of which is defined by one file. Custom scenery is stored in packages, or folders which contain all relevant files. Objects (in the form of OBJ files) can be placed at any location. These objects are most commonly buildings, but they could be houses, airplanes, or even people---X-Plane doesn't know the difference. In addition to these custom objects, custom terrain textures may be used to create [orthophoto][3]-style scenery.

#### Available Tools ####

The X-Plane scenery development kit contains the following:

*   A set of open-source, cross-platform tools for creating scenery
*   Specifications for all X-Plane-specific file formats
*   Documentation for the tools, including tutorials

The new scenery generation tools make extensive use of open source libraries; in order to comply with those licenses and to give back to the open source communities that make the new scenery possible, all of the new scenery tools have been released in source code as well as binaries. If you are a programmer interested in working with the scenery, we recommend working within the source code bases for these tools, as they already solve a number of problems relating to in-memory storage and processing of the new scenery.

##### The Scenery Creation Community & Additional Resources #####


[X-Plane.org][4] has web-based forums, including a forum dedicated to [scenery creation][5]. Additional guides, tutorials, tips and tricks can also be found on the [X-Plane Developer][7] site, including a [13 part video tutorial][8] on airport creation.

To file any reports of bugs or problems with WorldEditor (or any of the other scenery tools), visit [the scenery tools bug base][9].

##### Future Compatibility #####

The X-Plane 8/9/10/11 scenery file formats differ from the old X-Plane 7 formats in that they are open-ended; they can represent almost any configuration of scenery as long as a tool can create it. With X-Plane 7, to implement new features with the scenery, the format had to change. Since X-Plane 8, the format can represent almost anything. This means that the format will not change as we develop new scenery technology, all newer versions of X-plane can read older format files, and many existing sceneries can still be used in newer X-plane versions. Also, third party programmers will be able to design new scenery creation tools without being limited by our file formats, and it may even be possible to convert scenery from other flight simulators.

### What WorldEditor Is, and What It Can Do ###

WorldEditor (or WED) is the scenery creation and editing tool for the X-Plane flight simulator. It is designed to be a graphical tool for editing scenery overlays. It is able to:

*   create full custom airports,
*   create off-airport full custom overlay scenery,
*   customize a default X-Plane airport using built-in airport elements,
*   customize the air traffic control flow at an airport,
*   up- and download default X-Plane airports to the [X-Plane Scenery Gateway][6]
*   output scenery or airport data files which can be shared with the community.

WED is _not_ used to edit base terrain meshes. Although these files also share the suffix `.dsf`, these files are different in that they give
shape to terrain in X‑Plane. To create these files, use [MeshTool][10], a command-line tool to build base meshes from raw data. WED is also not used to edit or create 3-D models of aircraft, buildings, or other objects in the world. For information on using 3-D modeling programs such as AC3D or Blender to create X-Plane objects, see the [X-Plane Developer site][11], or [download the Plane Maker manual][12].

#### Downloading and Installing WorldEditor ####

To use WorldEditor, do the following:

1.  [Visit the X-Plane Developer WorldEditor page][13] and download the version of WED for your operating system. Save it to a location you will be able to find it, like the Desktop.

2.  Extract the WED executable from the ZIP file you just downloaded and save it somewhere you will be able to find it later (you can put it in your X-Plane directory, but this is not strictly necessary). WED is a single-executable file only, so no installer or other special installation procedures are required.

3.  Launch WED by double-clicking on it.

4. If this is your first first time launching WorldEditor, you must point the application to your X-Plane installation. To do this, click the **Choose X-Plane Folder** button in the bottom right of the window as seen in [Figure 1][14]. In the new window that opens, navigate to your X-Plane folder, and click **Select** or **OK**.

![WED first start](images/New_20/Uninitialized.png)

<p class="cap"><strong>Figure 1</strong>: A first launch of WorldEditor, with all options disabled except for "Choose X-Plane Folder"</p>

WED will verify the selected folder as a valid X‑Plane installation by testing for the presence of the "Resources/default scenery/" and "Custom Scenery/" folders inside that location. After that the remaining “New” and “Open” buttons will be enabled, and WED is ready to be used.

### X-Plane Scenery Concepts ###

X-Plane scenery comes in "packs." A scenery pack is simply a self-contained folder with all elements of a scenery. These can include:

*   DSF files, which contain the location of 3D scenery elements,
*   apt.dat files, which contain airport taxiway and ATC shapes,
*   art assets, which define how things look, and
*   an optional library.txt file, which makes these art assets available for use by other scenery packs as well.

For more information on the contents of a scenery package, see the appendix [Anatomy of the X-Plane Scenery System][15].

WorldEditor creates its own file, called `earth.wed.xml`, in the folder that represents your scenery pack. All of your work in WED is saved in this file. You then "export" your work, which creates the final scenery in a format usable by X-Plane. This workflow is quite similar to creating on a multi-layer Photoshop document, then saving a copy as a PNG for use outside Photoshop (emailing, posting to a website, etc.). Just like in Photoshop, the format used for _editing_ a scenery file includes more information (and takes up more space) than the format used as output for an end-user.

WED's own files have the advantage of letting WED save scenery information that isn't normally present in a scenery package: hierarchy information, object names, and window positions. However, it has a few major impacts on your work:

1.  You must "build" your scenery pack (which exports all of your work to the X-Plane file formats) before you will see anything in X-Plane. Doing a normal "save" in WED won't create real X-Plane scenery. To export a scenery pack, open the File menu and click **Export Scenery Pack**, or press Ctrl+B on the keyboard (Command+B on a Mac).  In addition, you can export directly to the Airport Scenery Gateway. We discuss this more in the section [Exporting the Scenery][16].

2.  Exporting scenery does a "compile" of your scenery pack with WED's latest work---this can overwrite and destroy existing scenery if you already have a package in your Custom Scenery folder with the same name (e.g., if you have previously exported this scenery package).

3.  Because WED _only_ opens earth.wed.xml files, if you have created scenery packs in other programs like Overlay Editor, or if you want to open scenery packs for which you don't have the earth.wed.xml file, that scenery must be _imported_ before you can edit them. See the section [Synchronizing with Other Editors][17] for details on how to do this.

4.  You must set an export target.  Basically the export target sets the oldest version of X-Plane that can use the output scenery pack (though you also have the option of setting the Global Airport Database as your target.)  When the export target is set older, the relevant new features in WED are then _not_ exported.  For example, if you set an export target of X-Plane 10.50, then you will not see any ground support vehicles in any version of X-Plane, as these features were not compatible with that X-Plane version.  

	The **Validate** command, which you can access either through the file 	menu, or by pressing Shift-Ctrl-V on the keyboard (Shift-Command-V on a 	Mac) checks the WED file based on the current export target.

	Note that the export target of "Gateway" is special: these sceneries are always intended and exported for the latest version of X-Plane supported by your WED version and are subject to additional validation checks only applicable for sceneries intended for upload to the Airport Scenery Gateway.

## Using the WorldEditor Interface ##

### The Scenery Package List ###

![WED 2 Package Chooser](images/New_20/LibraryChooser.png)

<p class="cap"><strong>Figure 2</strong>: The scenery package list, visible when launching WED</p>

By clicking any row from the table visible in [Figure 2](#thescenerypackagelist), then clicking the **Open Scenery Package** button, you can open an existing scenery package. Alternatively, you can use the **New Scenery Package** button to create a new, empty custom scenery pack.

The first column of the table can indicate a library or scenery that is disabled in the `scenery_packs.ini` configuration file. X-Plane will not display such sceneries, but WED will still allow these to be edited. Disabled libraries will not be used by neither X-Plane nor WED.

The second column shows some information about the contents of each scenery:

-   “WED Airport” indicates a earth.wed.xml file is included in the pack and the scenery is ready for immediate editing in WED.
-   “Airport” indicates there are one or more X‑Plane airport definitions included in the pack. But the contents of the scenery pack will need to be “imported” into WED before the contents are visible or editable in WED.
-   If the middle column is empty, the pack may contain base mesh scenery, not editable by WED, or may be an empty pack / folder created by the “New” button.
-   A “Library” usually only contains art asset definitions intended for use by other scenery packs. These packs are not editable in WED.

The third column is the name of the folder that holds the scenery pack or library. The name has no relevance to X-Plane or WED and can be changed any time without affecting the scenery packs functionality.

Note that WED does *not* support windows shortcuts. Scenery packs not actually present in the 'Custom Scenery' folder must be referred to by [symbolic links](https://en.wikipedia.org/wiki/Symbolic_link) instead to be visible and useable in WED. See also the [Windows blog](https://blogs.windows.com/windowsdeveloper/2016/12/02/symlinks-windows-10/) or [Howto Geek](https://www.howtogeek.com/howto/16226/complete-guide-to-symbolic-links-symlinks-on-windows-or-linux/) how to create symbolic links in windows.

The line at the very top of [Figure 2](#thescenerypackagelist) shows the X-Plane folder last selected by the **Choose X-Plane Folder** button as well as the version information from the last time X-Plane was run in that folder.

### The WED Scenery Editing Window ###

<img id="wedwindow" src="images/New_20/MapWindow.jpg" alt="WED Window" style="min-width:1000px; max-width=1000px" height="599px" usemap="#wedwindowareas">

<map name="wedwindowareas">
  <area shape="rect" coords="  0, 60,240,350" title="Library Pane" href="#thelibrarypane">
  <area shape="rect" coords="  0,370,240,599" title="Library Preview Pane" href="#thelibrarypreviewpane">
  <area shape="rect" coords="255, 60,325,599" title="Toolbar" href="#thetoolbar">
  <area shape="rect" coords="  0,  0,500, 50" title="Tool Defaults" href="#thetooldefaultspane">
  <area shape="rect" coords="340, 60,740,599" title="Map Pane" href="#themappane">
  <area shape="rect" coords="760, 60,999,260" title="Hierachy Pane" href="#thehierarchypane">
  <area shape="rect" coords="760,320,999,599" title="Attributes Pane" href="#theattributespane">
  <area shape="rect" coords="760,270,999,310" title="Editing Tabs" href="#theeditingtabs">
</map>

<p class="cap"><strong>Figure 3</strong>: A typical WorldEditor window, click on the image for more information</p>

1. Library Pane - items available for placing in the scenery
2. Library Preview Pane - preview of item selected in Library pane
3. Toolbar
4. Tool Defaults
5. Map Pane
6. Hierachy Pane - items already placed in scenery
7. Attributes Pane - properties of items selected in the Map / Hierachy pane
8. Editing Tabs

All panes are user adjustable in size by dragging the dividers in between them. You can reset all panes to a usefull size very similar to [Figure 3](#wedwindow) by clicking on "Restore Frames in the View menu.

Additionally the left side panes (labeled 1+2 in [Figure 3](#wedwindow)) and right side panes (labeled 6-8 in [Figure 3](#wedwindow)) can be made to automatically open and close by moving the mouse to the side of the WED Window:

If you set either pane vertical divider so the side panes are fairly narrow (less than 100 pixels wide), but *not* completely closed - move the mouse over that narrow area and the corresponding side pane will expand automatically to a preset, usefull size. Move the mouse back over the center panes (labeled 3-5 in [Figure 3](#wedwindow)) and the side pane will return immediately to its narrow size.

### The Map Pane ###

The map pane, numbered 5 in [Figure 3](#wedwindow) above, takes up the largest portion of the window by default. This is the pane that gives a top-down view of the scenery package as it stands. By mousing over the map pane and scrolling with your mouse, you can zoom in or out (note that the zoom will center around wherever your mouse is located---if you place it in the bottom right and scroll up, you will zoom in _toward_ the bottom right of the window).

You can pan around in the map pane around by dragging it with the mouse while holding down the *right* mouse key.

If you ever lose your place in this view, you can click View from the menu and select Zoom World (to zoom out to where you can see the whole planet) or Zoom Package (to fit the view to the scenery package itself).

In this pane, you can visually add, move, and remove elements from the scenery package. The effect of clicking on an element in the map pane depends on what tool is selected in the toolbar.

In the right top corner of the map pane is a set of buttons to "tilt" the map pane, i.e. show the 3D elements viewed from an oblique angle. This is also known as a "birds eye view" and it allows to see the vertical walls of buildings and get a sense of the height of items.

### The Toolbar ###

<div class="floatRight" style="width:170px">
<img src="images/New_20/map_tools.jpg" alt="The toolbar" width="158px" class="aligncenter" id="figtoolbar" /><br />
<p class="cap text_left"><strong>Figure 4</strong>: The tools in the toolbar, numbered in correspondance with the list to the left</p>
</div>

The toolbar, numbered 3 in [Figure 3](#wedwindow) above and seen in [Figure 4][23] to the right, selects the "tool" currently in use. Different tools are able to modify different types of things in the map pane. These are as follows:

1.  **Vertex tool**   Shortcut key: **v**  
    Used to select and manipulate vertices or any point-type entity
    (such as runway endpoints, points in a facade, 3-D objects, object
    headings, etc.). You can either click the point directly, or drag-click
    to create a box within which all points will be selected.
    Depending on the point’s type, holding Alt when you click may change
    the tool’s behavior (e.g., an Alt+click on a point in a facade
    allows you to create a curve from that point).

2.  **Marquee tool** Shortcut key: **m**  
    Used to drag a rectangle to select whole entities (i.e., all the points
    that make it up) or groups of entities. Or click on an entity to select all
    parts of it.

    The two above tools differ in that the vertex tool is intended to
    manipulate individual points and point-type entities, while the marquee
    tool is made to move, rotate, and scale area-type entities or groups of
    items.

3.  **Runway tool**  Shortcut key: **r**  
    Creates runways, which can optionally include blastpads and displaced
    thresholds. Click somewhere on the map to set the first endpoint of
    the runway, then click again to set the second endpoint.

4.  **Sealane tool** Shortcut key: **s**  
    Creates sealanes (to be placed on water) that seaplanes take off and land
    from, lined with buoys.

5.  **Helipad tool**  Shortcut key: **h**  
    Creates helipads.

6. **Taxiway tool** Shortcut key: **t**  
    Creates taxiways via closed Bezier paths. These are functional
    as well as visible scenery elements that define areas that aircraft are
    intended to move on. For all other visual surface markings use tool #22
    "Polygon tool". 

7.  **Taxiline tool**  Shortcut key: **l**  
    Creates line markings on taxi- and runways, optionally accompanied by
    light fixtures. Use tool #21 "Line tool" for all other line markings.

8.  **Hole tool** Shortcut key: **k**  
    Creates holes in any existing polygon or taxiway. Often used to cut out
    pavement to make the underlying terrain, e.g. airport grass visible.

9.  **Light fixture tool** Shortcut key: **f**  
    Places runway related light fixtures such as PAPI/VASI or wigwags.

10. **Sign tool** Shortcut key: **g**  
    Places 3D run- or taxiway signs.

11. **Airport beacon tool**  
    Places the rotating airport beacon.

12. **Windsock tool**  
    Places windsocks.

13. **Tower viewpoint tool**  
    Used to set control tower viewpoints (the point from which users in
    X‑Plane will see their aircraft when they select the Tower Viewpoint
    from the View menu).

14. **Ramp start tool** Shortcut key: **o**  
    Places locations that can be selected as start locations at airport. They
    are additionally used by AI traffic as parking locations as well as the
    "Draw parked aircraft" simulator feature to show aircraft parked
    permanently on the ground.

15. **Boundary tool**  
    Creates boundaries around airports. These do not immediately create
    visible changes in sceneries, but define the area that will be kept free
    of 3D autogen in *future* base mesh sceneries. Wirhin those boundaries
    the terrain will be made relatively smooth to facilitate operating aircraft
    on. The boundary should, at a minimum, enclose all Run- and Taxiways.

16. **Taxi route tool**  
    Used to define the paths which ATC will use to direct traffic around
    the airport. These routes are used for all of  
    * progressive taxi instruction for the users aircraft
    * directing AI aircraft between parking positions and the runway
    * ground service vehicles moving on the airport

17. **Facade tool**  
    Used to draw facade boundaries (from `.fac` files) using Bezier
    curves. For more information on facades, see the section [Adding
    Facades](#addingfacades) later in this manual.

18. **Object tool**   
    Places an object (a `.obj` art asset or `.agp` group of art assets)
    of the type currently selected in the library pane (numbered 1
    in [Figure 3](#wedwindow) and described [below](#thelibrarypane)).
    For more information on objects, see the section [Adding Objects and
    Auto-Generating Scenes](#addingobjectsandauto-generatingscenes)
    later in this manual.

19. **Forest tool**  
    Used to draw forested regions, which may be filled with trees in
    X‑Plane depending on the user’s forest density settings. As well as
    lines of trees or locations of individual trees. The type of trees that
    will be used depends on the `.for` file resource you specify.

20. **String tool**  
    Used to place object strings (a number of otherwise standard `.obj`
    files placed repeatedly along a line, as defined in a `.str` file).

21. **Line tool**  
    Creates miscellaneous lines or linear surface markings, such as
    road markings, sidewalks or narrow surface features, using Bezier
    curves. Visial appearance is defined by `.lin` files.

22. **Polygon tool**  
    Used for drawing miscellaneous polygons (from `.pol` files—often
    simple textures).

23. **Exclusion tool**  
    Used to draw “exclusion zones,” which are lat-lon rectangles that
    prevent elements of lower-priority tiles (e.g., X-Plane’s
    autogenerated cities or forests) from being loaded in the area.
    Excluded types must be selected from the drop down menu. For
    example, if an overlay tile contained placements for custom
    buildings for Manhattan, the author would also create an exclusion
    zone around Manhattan and select “Objects” from Exclusions menu to
    prevent the default buildings (that ship with X-Plane) from
    appearing there.

24. **Truck Parking tool**  
    Used to place location where ground service vehicle are parked while
    waiting to calls to service aircraft.

25. **Truck destination tool**  
    Used to place desination to create additional random ground service
    vehicle movements, aka "joyrides".

For more detail on the tools described above, see the chapter [Editing Using the Map Tools][29].

### The Tool Defaults Pane ###

Numbered 4 in [Figure 3](#wedwindow), the tool defaults pane changes the settings for the currently selected tool. The available settings vary by the tool. For instance, the vertex tool's only option is whether or not to snap to vertices (that is, to "jump" to existing points when dragging some vertex). The runway tool, on the other hand, can specify the runway's surface and shoulder material (concrete, grass, snow, etc.), the roughness of the runway, the presence of centerline lights, and so on. For many tools, you can change the default resource by selecting that tool, then clicking an asset in the library pane (described [below][31]) that is usable by that tool.

Before using any tool, set up the preferences first and you won't have to edit later.  When you change the defaults for a tool, WED will remember those changes so that the next time you use the tool, it will be set up the same way. This saves time when drawing many similar types of things.

### The Library Pane ###

The library pane, numbered 1 in [Figure 3](#wedwindow), displays art assets currently available for inclusion in the scenery. Here, you can browse through the files in the X-Plane library using their virtual paths. (For our purposes, a full understanding of the library system is not required, but for further reading, see the appendix [About the X-Plane Library System][33].) Selecting an asset will also select an appropriate tool---for instance, if you select a `.obj` file, the object tool will become active, and if you select a `.for` file, the forest tool will become active.

If you are looking for a specific asset, you can search the library using the "Filter" & "Choose Pack" boxes at the top of the pane. Click the "Choose Pack" box to display a drop down list of available library packs, and select the pack you'd like to search within.  Next, type a search term in the "filter" box to see all available objects within the pack that include that term. For instance, if you were looking for a specific ATC tower, you might type "control\_tower," with the "airport scenery" pack selected.  The library pane would show the Classic\_Tower\_1 object, among others. 

You may also search the library using the special search operators `$` and `^`. `$` is an end-of-line indicator, while `^` goes at the beginning of a search. For example, searching for "Yellow" in the hierarchy search pane would return results with that word *anywhere* in the name or attribute. Searching for "Yellow$" on the other hand would only match items *ending* in "Yellow," while searching "^Yellow" would match anything *starting* with "Yellow."

### The Library Preview Pane ###

The library preview pane, numbered 2 in [Figure 3](#wedwindow), shows an accurate, real-time preview of some (but not all) art assets. When you select a `.pol` file in the library pane, the pane will be filled with the texture defined by that polygon asset. When you select a '.lin', '.for', `.obj` or `.agp` file in the library pane, a 3-D preview of the object or scene will appear in the preview pane. By clicking and dragging this preview, you can change your perspective, and you can zoom in using the scroll wheel on your mouse.

Rotate the preview object to see all wall types available for the facade.
This works even if there are more than 4 wall types-just keep spinning !
Facades also can change in appearance depending on the height and width
of any given wall. Drag the mouse holding the right button (as opposed
to the left button, which is used to rotate the object) to change the
height and width of the preview object and note the changes in appearance.

If there is  more than one variant of the .fac file, you can also use the button in the preview pane to toggle through the options. Keep in mind that X-Plane will select one at random every time scenery is loaded--there is no way to specify a variant the simulator should always use, so make sure your scenery design looks acceptable with _any_ of these variants.

### The Hierarchy Pane ###

![The hierarchy pane in WED][image-24]

<p class="cap"><strong>Figure 5</strong>: The hierarchy pane in WED</p>

The hierarchy pane, numbered 6 in [Figure 3](#wedwindow) and shown enlarged in [Figure 5][36], shows every element currently in the scenery package.

Using this pane, you can select an element or group by clicking on it. This will highlight that element in both the hierarchy and map panes. By double-clicking on an entity in the hierarchy, you can rename it.

By clicking and dragging an entity in this pane, you can change the order in which objects are drawn: objects and groups higher in the hierarchy will be drawn on top of those lower in the hierarchy whenever they overlap. For instance, in [Figure 5][37], the "Runways" group is higher than the "Taxiways and Tarmac" group, so all elements within "Runways" will be drawn over elements contained in "Taxiways and Tarmac" _if_ they happen to overlap.

To place one or more entities in a group, select them and press Ctrl+G on the keyboard (Command+G on Macs), or select Group from the Edit menu. To ungroup elements (that is, remove a group, leaving all entities the group previously contained at the same "level" as the group was), select a group label and press Ctrl+Shift+G (Command+Shift+G on Macs), or select Ungroup from the Edit menu.

By clicking the lock icon, you can lock an element or group to prevent further visual editing. Note that is applies only to editing within the map pane; changing the element's properties in the editing tabs (beneath the hierarchy pane, described [below](#theeditingtabs) is still possible. Note also that this property applies recursively: if a group in which an element resides is locked, that element will also be locked.

Next to the lock icon is an icon that looks like an eye. This toggles the visibility of an element or group. Invisible elements will not be exported for use in X-Plane, and will of course not be visible in the map pane.

To delete entities, select them in the hierarchy and press the Backspace or Delete key on the keyboard, or select Clear from the Edit menu.

### The Attributes Pane ###

![The editing tabs in WED][images/intro/editing_tabs.png]

<p class="cap"><strong>Figure 6</strong>: The editing tabs and attributes pane in WED</p>

The attributes pane, numbered 7 in [Figure 3](#wedwindow) and seen in [Figure 6][40], contains a number of editing tabs, which allow for editing of elements based on their type. The first tab, labeled Selection, shows in full detail whatever scenery element is currently selected. Because of this, it changes the properties displayed in order to match the currently selected element.

### The Editing Tabs ###

The tabs reduce clutter by selectively limiting the visible entities in the map pane. They also lock the hidden objects so they cannot be moved on the map by mistake when in a tab mode.

The Selection tab makes all types of scenery items selectable and shows everything, except for some clearances around ATC routes and road networks.

The Pavement tab allows you to access only the underlying ground cover of the airport, with other details removed. Only runways, taxiways, draped polygons and orthophotos are unlocked and visible in this mode.

The Taxi Routes tab allows you to easily edit ATC operations at an airport. Only ramp starts and ATC taxi routes are unlocked, but most pavement, such as taxiways, runways, facades, and orthophotos are visible but locked from editing.

The Lights and Markings tab allows editing of non-sign details on top of base pavements. Line and string files, taxiway lines, light fixtures and windsocks are unlocked in this mode, with underlying pavement visible but locked.

The 3D Objects tab allows you to edit 3D clutter and buildings, such as facades, forests, and objects. Underlying pavement is visible but locked again in this mode.

The Exclusion and Boundary tab allows fine editing of exclusion zones and airport boundaries, with most airport features visible but locked in this mode.

The Texture tab is for texture mapping a custom draped polygon file named "Draped Orthophoto." To add these to the scenery, select a .pol file in the library pane, then select **Use Texture Map** in the tool defaults pane. Some .pol art assets (such as the DrapedRunwaySigns.pol) come with special pre-defined "subtextures" that can be pre-selected by a mouse click on the relevant part of the texture in the library preview pane.

After drawing your shape with the polygon tool, use the bounding box in the texture tab to select what part of the original image is displayed in the shape. Use the Snap to Vertices options for the most precision. Note that this tab only works with custom draped polygons that have been drawn with the Texture Map box checked. Custom scenery that use custom textures or or .pol files other than those included in the Laminar Research default library are not eligible to upload to the Airport Scenery Gateway.

### The Taxi Sign Editor ###

The taxi sign editor is "what you see is what you get" (WYSIWYG).

![taxi sign editor](images/intro/taxi_sign_editor.png)

Signs in the hierarchy are shown as a rendered preview of what the sign will look like. When you click on the line, you open the editor with two "text" fields where signs can be directly typed, and a palette where signs can be added by clicking. If the raw definition code is showing, this indicates the taxi sign fails to meet the syntax rules as outlined on page 13 of the [apt.dat specification](http://developer.x-plane.com/wp-content/uploads/2017/01/XP-APT850-Spec.pdf). The fastest fix may be to clear the existing sign text completely and create new text with the WYSIWYG editor.

### Preferences ###

![preferences window](images/New_20/PreferencesWindow.jpg)

<p class="cap"><strong>Figure 7</strong>: The options in the preferences window</p>

WED has globalized preference settings that will apply to every scenery pack. This menu is found under File > Preferences (or WED > Preferences on Mac OS). Here you can change between meters and feet, or how the coordinates are displayed in the map (although you can only enter coordinates as decimals still). The size of all text in WED can be specified as well. Most tables and menus will adjust to this size instantly, but some may require a restart of WED for best appearance. See the section "[Using Orthophotos for Scenery and Guides][]" for information on how to use the Tile Server Custom URL field.

## Creating Airports and Overlay Scenery ##

This tutorial steps through the creation of an airport from scratch, including runways, taxiways, air traffic control frequencies, and more. It also discusses adding [orthophotos][42] and objects to the scenery. Finally, it details exporting the package for use in X-Plane. 

Alternatively, see WorldEditor in action by watching the [13 WED tutorials][43] on YouTube that show how to create a simple airport.

**Note**: Before beginning, make sure you are familiar with the basics of the WED interface, discussed in the section [Using the WorldEditor Interface][44] above.

### Creating a New Package ###

If you have not installed WorldEditor yet, do so according to the section [Downloading and Installing WorldEditor][45] above.

To begin, we will create a new scenery package in WorldEditor. Launch WED and click the **New Scenery Package** button, as seen in the image below.

![Clicking the New Scenery Package button from the WorldEditor launch window](images/intro/new-package.jpg)

<p class="cap"><strong>Figure 8</strong>: Clicking the New Scenery Package button from the WorldEditor launch window</p>

Click on the new airport in the list and type a name for it (this name is unimportant except as an identifier for your own use). With the name entered, click the **Open Scenery Package** button. After a moment, the WED drafting window will appear, showing a new, empty scenery package.

Now is a good time to save your work, and to get into the habit of saving often!

### Setting Up the Basic Airport Information ###

We will now set up the airport that X-Plane will use. A scenery package does not _need_ to be associated with an airport, but it is generally the case that people create scenery packages around some airport.

You have two options regarding the airport data used in your package: you can either use a default airport (and modify it if necessary), or create a new one from scratch. The airport data included with X-Plane by default is generally of high quality, even in cases where only the main runways are included, so it is most common to import the existing data.

#### Importing Airport Data ####

Scenery artists can import airport data from the Airport Scenery Gateway database as well. First, select the option "Import from Airport Scenery Gateway" from within the File menu. The window that opens will download the airports currently available in the database, so you must be connected to the Internet to use the feature. 

You can search by ICAO in the Filter field, or scroll through the list to see if any scenery for an airport exists. Click "ICAO" to sort the list by identifier or "Name" for alphabetical sort. Data in the "Checked Out Until" and "Artist" columns indicates other Gateway artists reporting via the Airport Scenery Gateway website that they are currently working on updates for that airport. Such tags will not affect the ability to down or even upload modified airports, but can be used to avoid duplicate efforts.

Highlight the airport line then click **Next** to see all of the submissions of that airport in the database. Notes are included for each submission letting you know what user submitted it, how old it is, its status (such as whether it's the recommended version), and any comments included by the creator. Pick which version(s) you'd like to import and click the **Import Pack(s)** button. 

Multiple submissions of the same airport can be imported at one time, however we suggest that you only import the version labeled "Recommended" to avoid confusion between packs, and to work with the most complete data for the airport. Alternately multiple airports can be highlighted in the initial selection list, in which case the recommended version for each airport is imported without further user input.

You can import airport data saved on your computer via the File menu > **Import apt.dat**. Navigate to the apt.dat file you wish to import from in the next window. Then WED will ask which airports within the file to actually import. Type the ICAO identifier of your airport in the text box labeled Filter (found at the top of the dialog box), click your airport to select it (or hold Control/Command and click multiple airports to select them all), and click Import. At this point, all existing data on your airport is present in your project.

In most scenery packs, additional 2-D and 3-D airport scenery is stored in DSF files, which have to be imported separately with the "Import DSF" function in the File menu. Note the DSF format is a lossy, compressed format (similar to .jpg images), so the precision is less than when importing from the Airport Scenery Gateway or using an earth.wed.xml file, if available. This imported DSF data will show up in a separate group named after the path of the DSF file imported. At this point it is **not** associated with any airport previously imported from the apt.dat file. This group must be dragged in the Hierarchy Pane into the hierarchy of the intended airport to restore all functionality of this scenery in X-Plane.

#### Creating an Airport From Scratch ####

If you choose to create an airport from scratch rather than importing one from existing airport data, open the Airport menu and click **Create Airport**. A new group will appear in the hierarchy paned labeled "unnamed entity." Click twice on this name in order to rename it to match your airport's name.

With the newly created airport selected, we can enter some basic information about it in the editing pane (beneath the hierarchy pane). Using [the Airnav database][46], you can find the airport's elevation, ICAO identifier, and so on. (Note that, by default, the elevation is in feet above mean sea level. You can change these measurements to meters by changing your preferences.)

![setting editing pane fields](images/updates/editing_airport.png)

<p class="cap"><strong>Figure 9</strong>: Setting the field elevation, ATC presence, and ICAO identifier for an example airport</p>

#### Airport information basics ####

Whether you are creating an airport from scratch or importing an existing one, there are some data fields that should be filled out for all airports, such as the name, type, elevation, ATC, and ICAO. The scenery ID field relates to Gateway imports and should usually be left alone.

![1.5 metadata](images/updates/metadata.png)

You can specify whether base mesh flattening will be applied to only this airport with the "Always Flatten" option. In addition, up to 11 "Meta Tags" can define many properties important for ATC and navigation functions associated with a given airport. These can be automatically filled in for new airports by choosing **Update Metadata** from the Airport menu. You can manually add metadata fields by choosing **Add metadata** then the appropriate option if the automatic update doesn't return any results.

The most frequently edited meta tags are the "ICAO code," "FAA code" and "Local Code" meta tags. The airport selection windows in X-Plane, all GPS and FMC navigation use these meta tags _only_ to display the airport's identifier. The "Airport ID" property in WED or as used on the Airport Scenery Gateway is no longer used to determine the airport identifier in the X-Plane user interface and may therefore differ from the Airport ID. If an ICAO or otherwise locally issued identifier changes, the relevant meta tags are to be edited to reflect the new identifier, while the Airport ID in WED, on the Gateway and in all scenery files must <em>never</em> change once assigned.

#### Setting the Default Airport for Editing ####

An important concept in WED is the idea of the "current airport." The current airport is the one named in the upper left of the map pane, as illustrated in [Figure 10][47]. If there are no airports in the scenery package, there is no current airport. The current airport is the airport with which all new scenery elements (runways, objects, etc.) will be associated by default. Thus, if you have 3 airports in your scenery package and you draw a new runway, that runway will appear within the hierarchy of your _current_ airport. Note, however, that it is _the hierarchy pane_ which ultimately determines what airport a scenery element will belong to. If you create an entity at the wrong airport, you can always drag it in the hierarchy pane so that it belongs to the right airport.

To switch between current airports, select the airport you would like to edit from the hierarchy pane, then select **Edit Airport (airport name)** from under the Airport drop down menu. 

![The current airport listed in the upper left of the map pane][images/updates/current_airport.jpg]

<p class="cap"><strong>Figure 10</strong>: The current airport listed in the upper left of the map pane (in this case, KOJC/Olathe-Johnson County Executive)</p>

### Using Orthophotos for Scenery and Guides ###

When drawing runways, adding lighting, and so on (all discussed in the rest of this chapter), it may be helpful to have a real-world photo to guide you. Since X-Plane global scenery includes all roads, coastlines, and other data from OpenStreetMap data, WED can display these maps automatically by choosing View menu > Slippy Maps > OpenStreetMap. The maps are downloaded directly from the web, so this feature requires internet connectivity. Once loaded, the maps are cached on disk for at least one month to support continuing their use while offline.

![OSM KSEA example](images/New_20/OSM-map.jpg)

<p class="cap"><strong>Figure 10</strong>: The OpenStreetMap data underlying the KSEA demo area</p>

Another very popular option is to trace features from satellite imagery. This is supported by choosing View menu > Slippy Maps > **ESRI Imagery**. This imagery is also data is cached on disk and available for offline use the same way as the OpenStreetMap data. 

If you would prefer to use slippy maps of your own choosing, enter the tile URL (using [syntax as shown in this article](https://wiki.openstreetmap.org/wiki/Tile_servers)) in the "Tile Server Custom URL" field in the Preferences window. Please note that using this feature will **legally prevent sharing the scenery you create, under any circumstances**. Use this feature at your own risk as many services (Google Maps for example) strictly prohibit using their data unless it is **purchased** from them.

A third option is to download or create with any suitable 3rd party application georeferenced images in the GeoTiff format. For these use View menu > Pick Overlay Image... to import. These images will be placed into the correct position automatically, but if for some reason the georeferencing data can not be read, the image is placed in order to completely and exactly fill the current map window in either width or height, preserving the image's aspect ration. If this happens, the image corner coordinates can still be entered manually by selecting the reference image's corner and entering the Longitude/Latitude properties for each.

All of the above methods are for images that are only visible in WED to assist with scenery creation and layout. If instead you want your images to be exported as part of the scenery pack, you would need to import orthophotos using the File menu > **Import Orthophoto** option. If you do not want to use orthophotos in your scenery, you can instead skip to the section "[Adding and Modifying Runways or Sealanes][]" below.

Orthophotos are real-world aerial photographs which overlay the terrain in X-Plane, and are seen in scenery packages like those of [RealScenery][48]), and to use them you must first download some high quality orthophotos of the area. 

For scenery in the US, it's easy to obtain public domain orthophotos that may be used freely in our scenery; you can download them from the [USGS Seamless Server][49]. Outside the US, it may be more difficult----copyrights on most imagery like this will prevent you from using or distributing the images with your scenery.

**Note**: When downloading orthoimagery other from any service other than the USGS, their individual copyrights must be respected. Therefore, it is recommended that only images from the USGS be used in WorldEditor. Additionally, scenery that uses orthophotos may not be uploaded to the Airport Scenery Gateway.

<!--For help using the [Seamless server][49], see the appendix [Using the USGS Seamless Server][50].-->

From here on, we will assume that you have either downloaded orthophotos from the Seamless server, or that you have similar files from some other resource. The advantage to using the [Seamless server][50] is that you can download image files that have their geographic coordinates embedded in the files themselves.

Be sure to put the orthophoto image files you downloaded in your custom scenery pack folder.

#### Inserting Orthophotos ####

Adding orthophoto scenery is as simple as importing and, if no georeferencing information is included, positioning the image, customizing the rest of your airport's features, and exporting the scenery pack. WorldEditor will automatically take care of any necessary image resizing and splitting large images into smaller textures without resolution loss, as well as the creation of image format and draped polygon files. 

Supported image file types are: .bmp, .dds, .jpg, .png, and .tif. The recommended maximum image size is up to 16,384 pixels per side. Note that while WED can accept images of any size and shape, some resizing or transformation may occur if the source image is not already to the power of 2, or basically square. 

![orthophoto inserted](images/New_20/importh_orthophoto.jpg)

<p class="cap"><strong>Figure 11</strong>: An orthophoto inserted into a WED project</p>

To get started, open the File menu and select the **Import Orthophoto** option. Note that the image file must be located inside the scenery pack's folder as all output files created by WED will be placed in the same location. Navigate to this folder and select the orthophoto files you downloaded previously. Assuming the coordinate information in the files is correct (and that the file type includes embedded coordinates), WED will automatically place the images where they should be. 

Use the Marquee tool to select the newly inserted images, then give them a meaningful name in the hierarchy pane.

If your image did not include coordinate information, and you need to fine-tune the placement of your images, use the vertex tool to highlight the corners of you images. With a corner selected, you can use the Selection tab (in the bottom right of the window) to tune its latitude or longitude coordinates, along with its other properties. Alternatively, you could select your image's corners and drag them to move them, holding the control key to preserve the aspect ratio if desired. This is, for obvious reasons, not nearly as accurate as putting in the exact coordinates that you want for the image.

With the image's position perfected, you should be able to zoom in on your runways and see the X-Plane runway aligned (nearly) perfectly with the runway in the photo.

You may want to group all your overlay orthophotos if you are using more than one. To do so, select them all by holding down the Ctrl key (Windows) or Command key (Mac) and clicking the orthophotos in turn. Open the Edit menu and click **Group**, or Ctrl or Command + G. Once they are grouped and in place, you can keep them from being accidentally moved by clicking the lock image next to them.

Finally, to make sure that your overlay images don't hide other parts of the runway, select the group containing all overlay images, open the Edit menu again, and select **Move Last**. 

WED will check the last modification date of the referenced image file every time the scenery pack is exported. If the original image is newer than the .dds created from it (e.g. because it was edited with an external photo editor), WED will re-create the .dds files as needed. If the corresponding .pol files are edited after initial creation by WED they will never be overwritten either, so as to preserve any manual edits applied to them.

### Adding and Modifying Runways or Sealanes ###

To add a runway, simply select the runway tool (numbered 2 in [Figure 4][51]), then click twice in the map pane to visually set the two endpoints of the runway. For the sake of consistency, click first on the northern or western end (depending on the runway's orientation) and clicking second on the southern or eastern end. Note that for now, this does not have to be very precise---we'll clean it up momentarily to make it pin-point accurate. (Note that sealanes are added identically to runways, but they use the sealane tool rather than the runway tool.)

After your second mouse click, the green drawing line will turn into an orange outline, and the runway will appear in the object hierarchy pane, with a long list of attributes below it.

After having placed the runway roughly where it belongs, use [the Airnav database][52] or another reference to manually specify the exact endpoints of the runway, as well as its other properties. With the runway selected (either because you just drew it or because you used the marquee tool and clicked on it), click the Selection tab (found among the editing tabs, described [above][53]). You should begin by setting the "Latitude 1" and "Longitude 1" attributes---these are the coordinates of the first end you drew previously, which should have been the northern or western end of the runway. Then, input the "Latitude 2" and "Longitude 2" coordinates. (Note that the Latitude/Longitude Center, Heading, and Length attributes will all be calculated automatically from these.)

Beyond these attributes, the order in which you input the data does not matter. At the very least, you should specify the runway width, the surface, and, of course, the name. Be sure to enter the name as low/high numbers. For example, 18/36 is a legal name while 36/18 is not. 

Note that, for all attributes with "1" and "2" variants, the "1" end is the end you clicked first when drawing.

Some important attributes whose significance may not be obvious are:

*   Roughness: this specifies how rough the runway is (and how much it bumps the plane around) when taxiing. This is on a scale from 0.0 to 1.0. A runway in good condition should have a roughness of about 0.25.
*   Displaced Threshold 1 and 2: these specify how far from the end of the runway an aircraft is allowed to touch down, measured in feet or meters depending on your settings in preferences. By default, the threshold is the end of the runway, so this is set to 0.0.
*   Shoulder: this sets the surface type of the runway shoulder, which is a small section of pavement beyond the runway found mostly in large airports.
*   Edge lights: runway edge lights are classified by the intensity of the light they produce. They can be High Intensity Runway Lights (HIRL), Medium Intensity Runway Lights (MIRL), or Low Intensity Runway Lights (LIRL), or the runway may have no edge lights at all.
*   Markings 1 and 2: these specify the type of markings on each end of runway. Read more about these [on Wikipedia][54].
*   Blast pad 1 and 2: these specify the length (in either meters or feet) of the blast pad. A blast pad is an area of pavement at the end of the runway constructed to keep dirt and grass from being blown around in the [jet blast][55] created by a large aircraft taking off.
*   Approach lights 1 and 2: many configurations exist for a runway's Approach Lighting System (ALS). You can read about them [on Wikipedia][56].
*   REIL 1 and 2: the Runway End Identifier Lights.
*   TDZ lights 1 and 2: the Touch Down Zone lights.
* Distance Signs: the white on black signs indicating the runway length remaining in 1000 foot increments. Note these signs may be placed automatically by X-Plane where taxiways enter the runway. This is visible by watching the small black locations indicated when **Toggle Preview** is checked in the View menu. In such cases, best practice is to manually place _all_ such distance signs using the Taxi Sign function, and using the indicated positions as a guide. Once done, uncheck this Distance Signs option for the runway to leave only the manually placed (and conflict free) signs in place.

Having finished modifying a runway visually, you may want to lock it (as described in the section on [The Hierarchy Pane][57] above). Another good idea is to very its alignment with any existing X-Plane NAVAIDs, such as ILS, VOR and NDB systems. This can be done by checking the **Navaids** option under the View menu to display all such installations as defined in the default NavData of the current X-Plane install.

![navaid overlay](images/New_20/navaid_overlay.png)

<p class="cap"><strong>Figure 12</strong>: The NAVAID overlay showing ILS localizer, Glideslope and Inner Marker correctly aligned with runway</p>

Note this NAVAID layer is read-only--in other words it cannot be edited in WED. To request a fix to any discrepancies observed, file a bug report in the "NAVAIDs" tab on the Airport Scenery Gateway "[Report a Problem](https://gateway.x-plane.com/bugs)" page.

If a runway has been modeled using an OBJ or some other specific technique that replaces X-Plane's runway, a transparent surface can be selected from the drop down menu in the editing pane. This will allow users to get all of the menu items and approach lighting associated with a real runway, while still being able to set the surface physics and graphics themselves (using a draped polygon, with its physics specified in its .pol file). This is an advanced scenery design technique and not allowed on airports submitted to the Airport Scenery Gateway.

### Adding and Modifying Taxiways ###

The taxiway tool is used to draw taxiways and other pavement (including aprons, parking lots, and so on) using Bezier polygons. Taxiways are much more than just "colored areas" in the scenery--they also create smooth surfaces, are shown in the Airport diagram of the X-Plane map, affect how runway lights are shown, and more. They are functional elements of the airport and should not be used for anything but surfaces where aircraft do actually taxi. They are not to be used for roads, building footprints or other paved, grey surfaces--use polygon (.pol) files for these features.

Before beginning, it is important to ensure that, when drawing these polygons, there is only _one_ enclosed area per polygon---that is, make sure that the outline does not cross over itself at any point. To ensure the taxiway aligns correctly with the runway or other pavement when rendered in X-Plane, be sure the two surfaces overlap a bit or that the points snap together. (See the [Taxiway Shapes article](http://developer.x-plane.com/2007/11/taxiway-shapes/) for more information.)

With this in mind, select the taxiway tool from the toolbar and create a rough outline of a section of pavement. At this point, you should not be making the shapes especially detailed---use a small number of nodes, with the intention of cleaning the shape up later. Finish the shape by hitting the "Enter" key or clicking on the first node you placed, which will have a circle around it. If at any point you need to delete/remove a single vertex, you can use the vertex tool to select that vertex and hit the delete key.  The vertex will be deleted and a straight line will be drawn between the two remaining vertices on either side of the deleted vertex.

Perhaps the easiest way to handle the airport's taxiways is to outline _all_ the taxiways and other pavement in your object, then use the hole tool to cut out all the area that was selected which _isn't_ pavement. If you are using an orthophoto, it will be helpful to change the transparency of the taxiway in order to see where the holes should be drawn. To do this, click the View menu and select "Pavement Transparency." Set the taxiways to 50% transparency for a good balance.

For instance, on the left in [Figure 13][58], the taxiway has just been drawn---it covers a lot of area that it shouldn't. Then, on the right, holes have been cut in it so that it doesn't cover the grassy areas around it. It still isn't pretty, but we'll clean it up soon to more closely match the real taxiways. 

For information on how to use the hole tool, see the section "[Cutting Holes in Bezier Shapes][]." 

<img src="http://www.x-plane.com/images/WED/tutorial/taxiways.jpg" alt="The rough outlined taxiways, before and after cutting holes in them (left and right, respectively)" width="830px" class="centerMe" />

<p class="cap text_left"><strong>Figure 13</strong>: The rough outlined taxiways, before and after cutting holes in them (left and right, respectively)</p>

Drawing one large taxiway and cutting holes will not work if you need markings on your (true) taxiways, though---in that case, you'll need separate entities for each taxiway.

#### Smoothing the Curves ####

Now that we have a very basic outline of our pavement drawn, we need to modify it to follow the contours of the actual airport's curves. We'll do this by turning the node near a corner into a curving node in the Bezier path, which will cause the line connected to it to curve around it. To convert a plain node to a curving one, hold down the Alt key and drag the mouse away from the point. After you let go of both the mouse and the Alt key, you can click and drag the outside arrows to further tune the curve.

You may need place an extra node between two points for the purpose of creating this curving node. To split the line connecting two points and place a node between them, highlight the points using the vertex or marquee tool, open the Edit menu, and click Split. Alternatively, you can highlight the points and press Ctrl+E in Windows, or Command+E on a Mac.

Repeat this process for each curve around the pavement.

For information on switching between the different types of nodes, see the chapter [Bezier Path Tools][59].

### Creating Markings, Signs and Lighting ###

#### Runway Lines and Markings ####

Now we will add markings to our runways, taxiways, and other pavement. Markings come in two varieties: perimeter markings (found around the outline of taxiways), and overlay markings (like taxilines, ILS, and hold short markings). The latter are essentially stand-alone lines, so they may also be useful for coping with complicated taxiway layouts

Note that whenever you select a tool that supports markings (e.g., the taxiway and taxiline tools), you can set that tool's default markings (using the tool defaults bar at the top of the window). When you select a default marking in this pull-down menu, that marking will be applied to that tool until you change it. So if you were going to draw taxilines, you would select the taxiline tool, set the **Markings** drop-down to "Double Solid Yellow (Black)," then begin drawing the shape.

You can, however, draw the shape first and add the markings later. This is easily accomplished by selecting an entity (like a taxiway) with either the vertex tool or the marquee tool, then going to the attributes pane and setting the Line Attributes, Light Attributes, or both, under the Selection tab. When you do this, the markings will be applied to the entire shape. This is generally not what is desired, though, and you'll need to remove the markings from some of the segments for taxiway intersections and the like.

To remove the markings from one section of the taxiway, select a single node using the vertex tool. Then, in the attributes pane Selection tab, set its Line Attributes to none. The line connecting this node to whichever node was drawn after it will no longer have that attribute. For instance, if you selected Node 9 and set its Line Attributes to none, the line connecting it to Node 10 would have no markings.

#### Adding Ramp Starts ####

Adding ramp starts via the the ramp start tool will create the list of ramp starts seen in the Global Airports screen of X-Plane. These are places you can start your flight from, where parked aircraft are drawn, or AI aircraft may taxi to and from during their operations.

Select your ramp start type: gate, tie down, or hangar. You can only have one of these checked at a time, but all types are displayed in the Ramp Start list of the Airport Map in X-Plane. AI aircraft can use the gate and tie down types only, and push back services are only available at gate type ramp starts. Misc (checked by default) and hangar types are only usable by the sim pilot. 

The equipment type field tells X-Plane what type of aircraft would use this ramp in the real world. This is mostly used for spawning the correct type of AI planes. Pick at least one type of plane from the list.

Specifying the size, operation type, and airline fields will help X-Plane populate airports with the most accurate mix of AI aircraft and static objects. Note that changing the Size field will change the size of the yellow icon. This is to help visualize how much space will be taken up by the largest aircraft of that category. Be sure to provide plenty of clearance--if any part of the icon is covered, there will not be enough space there in X-Plane and taxiing aircraft will become stuck when trying to pass. Keep in mind that viewing the layout in the "Taxi+Flow" editing tab will make these issues easier to spot.

If you are importing an airport from an earlier version of WED, use the "Upgrade Ramps" option under the airport menu to automatically convert old style ramps to the new format compatible with X-Plane 10.50.

#### Creating Taxi Signs ####

A runway or taxi sign is simply a [directional point entity][90], which means it is placed and rotated just like other directional point entities. To place a new sign, select the taxi sign tool from the toolbar, then click in the map pane wherever the sign should be located. The new sign is represented as a rectangular icon with an arrow emerging from one side to indicate its heading---the front of the sign is the side with the arrow emerging from it. Note that you can rotate the sign as you create it by clicking and dragging, or you can change its heading later using the vertex tool.

Once the sign is positioned, click on the line in the hierarchy to open the sign editor and change the Name parameter to specify what the sign actually says. You can type directly on the line or click on individual icons in the editor. With the sign editor, you cannot create an invalid sign--the editor knows the syntax rules. If you see pure text, the sign is already invalid and you will need to delete the offending part or redo it. (Use the Validate option to point out sign syntax errors.)

![taxi sign editor](images/intro/taxi_sign_editor.png)

The sign specification used by X-Plane is shared by the FAA, and it is largely in line with other aviation authorities worldwide. Scenery artists may find it helpful to review "[ICAO Recommended Airport Signs, Runway and Taxiway Markings][89]," which outlines how real-world taxi signs are positioned and designed.

#### Creating Windsocks, Light Fixtures, and Airport Beacons ####

Windsocks, light fixtures, and airport beacons are all placed on the airport like runway signs (described [above][61])---click to place them, and, in the case of the light fixtures, drag the cursor to change their orientation. You can use the marquee tool to change their position, and, in the case of light fixtures, you can use the vertex tool to change their heading.

### Adding and Modifying Helipads ###

To create a helipad, simply select the helipad tool, then click and drag in the map pane to both set the pad's location and its heading. After placing a helipad, you can set the helipad's name, surface type, marking type, size, and so on using the attributes pane (either the selection tab or the helipads tab, depending on your preferred workflow).

### Creating Airport Boundaries ###

Airport boundaries define the edges of the area considered an airport. These are used primarily when new global terrain is generated---they enable our scenery generation algorithms to flatten the appropriate airport terrain, and to apply appropriate land use textures. The elevation of the airport area is pre-processed to remove bumps and radar spikes (flattened), and the terrain type is set to airport grass.

Guidelines for drawing the airport boundary:

*   The airport boundary should match a visual boundary between cleared airport area and surrounding terrain.
*   The airport boundary should include any airport-related buildings, so that the elevation near terminals is well controlled.
*   The airport boundary does not need to be very detailed---the boundary is slightly blended in the DSF file.
[Figure 14][62] shows a possible boundary for BWI. Note that the forest areas are probably on the property of the airport legally, but are not part of the airport boundary in the diagram so that the trees are not replaced with grass when the airport is built.

![An example of determining an boundary][image-28]

[cap]**Figure 14**: An example of determining an boundary[/cap]

To draw an airport boundary, select the boundary tool and click at each corner around the airport. This is the area that will be flattened in X-Plane and filled with airport grass the next time global scenery is cut. The boundary has no impact on the way your scenery is shown in X-Plane--it is only used when submitted to the Gateway and only for a future version of X-Plane. When drawing the outline, be sure to go in one direction---either all clockwise or all counter-clockwise. When you have placed the last corner, press the Enter key to commit the points to the boundary. At this point, the boundary will appear in the object hierarchy pane; you can then name it there.

If your points aren't in the places you would like, you can use the vertex tool to click the points and drag them.

### Adding Objects and Auto-Generating Scenes ###

For the purpose of this tutorial, we will assume the objects to be used are already created and saved as single object `.obj` files, or composed into `.agp` scene files that may contain several objects, trees, facades, and even draped textures. X-Plane's built-in library contains a great variety of objects. Note that scenery that uses third party resources (such as OpenSceneryX) cannot be uploaded or shared on the Airport Scenery Gateway.

To add an object to your scenery, first find it in the library browser (the pane on the right side of the screen). Selecting an object there will also select the Object tool from the toolbar. Having selected the object, click in the map pane wherever the object should go, or click and drag your mouse around to both place the object and set its heading. If you want to change the object's heading after placing it, use the vertex tool, and if you want to move the object itself, use the marquee tool to drag it around.

For each object, you can choose at what object density (set in X-Plane's rendering options) that object will be visible. In the object's "show with" field (visible in the attributes pane), you can choose to always have X-Plane draw the object (by selecting "default") or have X-Plane only draw the object at the highest level of object density (by selecting "too many"), or somewhere in between.

![Adding an object to a scenery package][image-29]

<p class="cap"><strong>Figure 15</strong>: Adding an object to a scenery package</p>

Note that in many cases, your drawn objects don't have to match the objects in the orthophoto, due to the fact that X-Plane will draw concrete pads where they would be in real life, and these pads will go on top of the orthophoto.

### Drawing Object Strings and Line Markings ###

Object strings and line markings are drawn by default as [open Bezier paths][63]. To draw one, select the resource you would like to draw from the library pane (possibly by typing `str` or `lin` in the library pane's Filter box), then, click in the map pane to place each vertex of the line.  To change a string or line into a closed Bezier path (a ring), select the string in the hierarchy pane and check the box labeled "closed." 

The `.lin` files that come as part of the default library are identical in appearance to "Airport Line Markings", but differ in the way they are stored in scenery files.

Airport Line Markings are intended for aircraft movement assistance, such as taxiway center or edge lines, runway hold markings, etc. They can have lights associated with them and they are written into the apt.dat file. They will **not** however, align exactly with polygon shapes. They also take up 5 to 7 times as much space in scenery files and end up in a global database: _all_ airport line markings at _all_ airports have to be parsed by the simulator _every_ time X-Plane starts. 

Multi-purpose lines (`.lin`), on the other hand, render faster and therefore should be used in the majority of cases. They will align exactly with a polygon and are subject to the same compression when saved to the .dsf file. 

### Drawing Draped Polygons ###

To add a draped polygon (a surface that covers the X-Plane terrain and takes its shape) to your scenery, first either select the polygon tool or find the resource you would like to use in the library pane (perhaps by typing `pol` in the library pane's Filter). Then, click in the map pane to draw the polygon as a [Bezier path][64].

### Adding Facades ###

A facade in WED is essentially an image wrapped around a polygon at a specified height. This is used to build a simple building or fence quickly and easily. Users specify only the shape of the structure at its base, its height, and the `.fac` file to use.

To draw a facade, select the facade tool from the toolbar (or select a `.fac` file from the library browser), and set the height of the structure in the tool defaults setting pane if desired. Depending on what type of .fac file is used, the tool will automatically draw open or closed Bezier paths for fences or buildings, respectively.

Use the facade tool to trace the outline of the scenery element you would like to draw. You can modify the points later using the vertex tool---remember that holding Alt and clicking on a point with the vertex tool will change it to a point of curvature in the Bezier path. After the facade is outlined, give it a name and change its height in the attribute pane as needed.

### Drawing Forests ###

Overlay scenery created in WorldEditor can specify forested regions with a high degree of specificity. Using the forest tool, you can draw the outline of the forested region (using only plain nodes, with straight line segments). Before or after a forest is drawn (by using the tool defaults or attributes panes, respectively), you can set the following properties for it:

*   Density, set as a ratio (where 0.0 is bare and 1.0 is full density),
*   Resource, specifying what `.for` file will be used to fill the forested region (and thus how the flora will appear in X-Plane), and
*   Fill mode, specifying whether the forested region will be filled with flora (in "area" mode), lined with flora (in "linear" mode), or set only at the vertices you have drawn (in "point" mode).

The default library has a number of forests types under the `lib/g8/` path available. These carry names that specify the type and climate zone a given flora type will used for by the autogen system. Matching this climate zone to the location where a your scenery is located (using a `_cld_wet` option at an airport in Alaska, or `_vhot_dry` in the Sahara, for example) will ensure the forests match with the autogenerated scenery all around the airport and provide a more consistent overall appearance. 

### Creating Airport Traffic Flow Information ###

An airport traffic flow defines a particular configuration of runways. Most large airports will have several "flows" depending on weather or time conditions, or type of aircraft, so WED allows you to create more than one flow as well.

An ATC traffic flow starts with an ATC flow item, which defines the flow's name, its visibility/ceiling requirements, and its pattern runway. It can also specify the following rules:

* One or more runway uses, which define which exact runways are in use when a given flow is active. As only one flow is active at any given time, _all_ runways that are simultaneously in use need to have a runway use rule in that flow. Note that a runway can be used more than once, so a flow could have runway 4L for arrivals for turboprops as well as 4L for departures for props. If no runway uses are provided, X-Plane's artificial intelligence ATC will pick a runway from **any** runway in existence at that airport.
* Zero or more time rules that define a set of times when the flow may be used. If no time rules are provided, the flow can be used at any time, i.e. 24 hours a day. Thus, in most cases there is no need to specify any time rules at all.
* Zero or more wind rules that define the wind limitations on the flow. If any wind rule meets the current weather, the flow can be used, so you can define a wide angle range at low wind speed and a narrow angle range at high wind speed. For example, the first flow should always include a wind rule allowing it to be used in light / variable winds from any direction (0-359 degrees). 

The last of all flows specified should never include any wind, time or visibility rules. This is the fallback flow if no other matching flow's rules meet the actual wind or weather conditions. (Note there is currently no option to force an airport to be closed due to the wind or weather rules preventing operation of any runway.) If there is a certain weather scenario not covered by any specified flow and its rules, X-Plane will **always** choose the last flow specified, whether that makes realistic sense or not.

To create a traffic flow, select your airport in the hierarchy pane, open the Airport menu, and click **Create Airport Flow**. Then, after selecting that flow in the hierarchy pane, open the Airport menu again and select **Create Runway Use Rule**, **Create Runway Time Rule**, or **Create Runway Wind Rule**. These rules specify under what conditions the runway should be used. X-Plane will try to use the rules in order (from top to bottom), so be sure to order multiple flows from most specific to least. It also a good idea to have a generic, catch-all flow that can be used in all conditions in case the condition-specific flows overlook a possible scenario.

Note that the taxi routes (detailed below) generated by X-Plane's artificial intelligence are _not_ part of the flow----these routes do not change with flow. 

For a step-by-step walkthrough of creating traffic flows, see the [four part video tutorial][65] on the X-Plane YouTube channel. See the article "[ATC Flow Authoring in WED](http://developer.x-plane.com/?article=atc-flow-authoring-in-wed)" for more information on how to set each field of the ATC flow.


### Specifying ATC Taxi Networks ###

Taxi networks provide guidelines on how to get from one part of an airport to another. If the airport does not have a hand-drawn route, X-Plane auto-generates a taxi network when it starts.  These networks tend to have problems and are not very precise. Specifying a taxi network in WorldEditor will keep the AI-controlled aircraft moving in a reasonable manner around an airport, instead of going through buildings or off the pavement. They also contain important data such as taxiway names and hold short information.

To create taxiway routings for the AI aircraft, use the taxi routes tool and click to trace the path the aircraft should take. To do this efficiently, you may want to preset the properties of the taxi route tool using the tool defaults bar at the top of the window; then, you can draw all departure paths together, all paths for a specific runway together, and so on. If you do not set the properties that way, or need to change them later, you can select the taxiway(s) in the hierarchy pane, and then make your changes in the attributes pane below it. (Note that there are additional fields in the attributes pane, such as latitude and longitude, that should not be edited.)

Picking a runway in the Departure and Arrival fields indicates the segment is in a hot zone for that runway. Setting the ILS field indicates that segment is in an ILS precision area. The Size field determines the largest AI aircraft that is legally allowed to use the taxi route. This prevents aircraft with large wingspans from taxiing along routes that have limited clearance. Most taxiways should be specified large enough for the largest aircraft that could use a given airport--which should match the size of the largest ramp start specified at that airport. The slop field indicates how close your point must be to another taxiway section in order to snap to it. Larger numbers allow the paths to snap together from farther away. The other preset fields are self-explanatory. 

When viewed in the ATC Taxi + Flow tab, taxi routes are color-coded for better visualization in the map pane. Runway segments are blue (or purple), basic route segments are yellow, hot zones are red, and ILS precision areas are orange. 

![taxi route colors](http://developer.x-plane.com/wp-content/uploads/2016/08/wed_color_coding.jpg)

**Figure 16**: Color coding of ATC taxi route segments

In addition, changing the Size field will change the width of the band. This is to help visualize how much space will be taken up by the largest aircraft of that category. Be sure to provide plenty of clearance--if any part of the band is overlapping, there will not be enough space there for the plane to pass safely in X-Plane.

WED includes a lot of validation to help catch any errors with your taxi networks because they are very complicated systems to design, but also very important. They're the only way ATC understands how to direct AI aircraft around the airport. Keep the taxi network limited to the preferred and safe routes that aircraft would take at any airport. The ATC will _not_ be able to distinguish rarely or never used taxi routes from the preferred one. Instead, it will always take the shortest and straightest path from a runway to the assigned parking position and vice versa.

See the article "[ATC Taxi Route Authoring](http://developer.x-plane.com/?article=atc-taxi-route-authoring)" for an in-depth guide to designing correct taxi routes. A [five part video tutorial][66] is also available on the X-Plane Official YouTube channel. 

### Specifying Service Vehicle Networks ###

Scenery artists can customize routes for airport service vehicles to use in X-Plane 11. Ground traffic vehicles are AI-controlled, and each vehicle spends most of its time at specific "parking locations." When called to duty it heads to either an aircraft's location or a "truck destination." Service vehicles will randomly visit "truck destinations" in order to create more traffic and activity at airports. All service vehicles return to "parking locations" after they complete their service.

The three tools used in creating service vehicle networks are:

- Truck Parking tool
- Truck Destination tool
- ATC Taxi Routes tool

![example routes](images/WED_16/truck_routes.png)

<p class="cap"><strong>Figure 17</strong>: Example of truck routes, parking spots and destinations at KSEA, as seen in the ATC Taxi + Flow tab view.</p>

The available types of service vehicles are:

- Fuel, for both jet and prop
- Crew cars
- Catering
- Baggage
- Ground power units (GPU)
- Push back trucks

Note that, in order for an aircraft to receive service, ports must have been specified in [Plane Maker](http://developer.x-plane.com/manuals/planemaker/).

To begin laying down service vehicle networks, first select the Truck Parking tool and click in the map pane to place locations where the service vehicles will park when not in use. Only one type of vehicle is allowed per spot, which is determined by the drop down. Place parking locations near aircraft when possible so the vehicles do not have to drive a long way. Service vehicles will avoid collisions with other service vehicles and the aircraft they are servicing, but not other objects. (The awareness to avoid buildings and other objects may be available in the future.) If service vehicles need to cover a long distance however, they will use ground truck taxi routes when available.

Drawing the ground truck routes works exactly like drawing ATC taxi routes. Select the ATC Taxi Routes tool and pick "Ground Trucks" from the "Allowed Vehicles" field in the tool bar, then click in the map pane to place the points which outline a route. Service vehicle routes ignore the settings in the Runway, Departure, Arrival, ILS, Size and Name fields, but _can_ be marked one way. 

Unlike ATC taxi routes, service vehicle routes can be multiple separate, unconnected networks. Service vehicle routes are not allowed to cross or be too close to any runway, so stand-alone networks separated by runways may be necessary at many airports. Ground routes may be placed through terminal buildings if, in real life, there are paths through the area that X-Plane is currently incapable of modeling.

Service vehicles are good at maneuvering near their docking locations and need room to do so. This means service vehicle routes should _not_ be drawn close to ramp starts. Leave room around the ramp starts, especially the right side, to allow for this.

![space to maneuver](images/WED_16/GndTraf_Maneuver.jpg)

<p class="cap"><strong>Figure 18</strong>: Space for the service vehicles to maneuver around the right (starboard) side of aircraft is indicated by the purple striped area.</p>

The truck destination tool is used to place a location that will randomly call the specified service vehicles. Pick all the types of vehicles that will be allowed to use it in the drop down of the toolbar, then click in the map pane to place the destinations. Vehicles will drive to the spot and park there briefly in order to create more traffic activity at the airport. 


### Exporting the Scenery ###

When you have finished customizing the airport, open the File menu and select **Validate**. This command will check the WED file for errors based on the current export target. You can change the export target by selecting **Target X-Plane Version** from under the File menu. The Scenery Gateway validation target is a much more stringent validation process. This is intended to make sure the scenery is useable with low end or older computers, and to ensure no 3rd party add on libraries are required. Remember that selecting an older version will disable newer features, such as ground traffic which does not exist in versions prior to X-Plane 11.00. 

If there any scenery errors when you validate or export your scenery, WED will show you a window of all issues, which can range from warnings to major errors that prevent an export. Clicking on one of the lines in this pop up box will allow you to go directly to that issue, or you may select multiple issues at once to fix them all in one action. A "validation_report.txt" file will also be placed in the scenery folder. Use either of these resources to correct the errors before completing the export.

If no errors are present, select **Export Scenery Pack** from the File menu. If orthophotos were used, the .dsf and .pol file will be automatically created and placed in the scenery folder. The new scenery will be visible the next time you load the area in X-Plane. 

All the files that the scenery package depends on are should be its folder (or referenced in the X-Plane library). To share the scenery pack with a friend (or with the world via the Internet), just zip up your package folder (located in X-Plane's Custom Scenery folder). Instruct the other user(s) to unzip the folder into their Custom Scenery folder, after which the scenery will appear in X-Plane on their computer as it does on yours. Beware, however, that external resources will not appear for other users unless they have those same resources installed. Thus, if you use a third party product such as OpenSceneryX in your scenery, be sure to tell your users to install it as well. 

#### Airport Scenery Gateway ####

You also have the option to upload your files directly to the [Airport Scenery Gateway][67]. This is a community-driven effort to collect airport data into a global airport database and will be a collection of all airport layouts authored by the X-Plane community over the years. Future releases of X-Plane will periodically share Gateway submissions with all X-Plane users.

**Note:** Scenery elements or library items from 3rd party addons such as OpenSceneryX, or any draped orthophotos, are not compatible with the Airport Scenery Gateway.

To upload a scenery package to the Gateway:

1. Register for free as a new artist on the Gateway to establish your Gateway login credentials. 

2. Ensure that all buildings and other 3D objects are inside the folder in the WED hierarchy that represents the airport (just like runways must be). It's also a good idea to view your scenery in X-Plane first, with object settings at max to catch any problems.

3. Set the "Target X-Plane Version" in the File menu to "Airport Scenery Gateway."

4. Select File->Validate to run a validation pass on your airport. Any errors that would prevent Gateway upload will be located.

5. Select the airport you want to upload in the hierarchy. (Note that you can only upload one airport at a time.)

6. Select File -> Export to Airport Scenery Gateway.

7. Input your username, password, and text description.

8. Click "Upload."

To track your airport's progress on the Gateway:

1. Log in to the Gateway. Artists must be logged in to see submissions until they have been accepted or approved by a moderator.

2. Click on the "Airport Scenery" menu option.

3. Search for your airport using the ICAO code.

4. Select the airport from the search results grid to view all submissions that exist for this airport.

5. Confirm that your submission currently has a status of "Uploaded." This means the airport has been received by the Gateway but has not yet been acknowledged by the moderator. The moderator's primary responsibility is to ensure the airport looks sensible, and no objects block the taxi routes, ramps, or runways. As multiple submissions appear for a given airport, the moderator will gradually raise the bar to determine the "Recommended" submission.

6. Check back periodically to follow the progress of your airport submission on the Gateway. Airports may have the following status:
	* **Uploaded** (Not yet acknowledged by the moderator.)
	* **Accepted** (This airport submission has been acknowledged by the moderator, but not yet evaluated.)
	* **Approved** (This airport submission has been approved as suitable for inclusion in a future release of X-Plane. "Approved" submissions may be downloaded from the Gateway by X-Plane users. However, if there are multiple approved submissions for the same airport, only the "Recommended" submission is a candidate to appear in a future release of X-Plane.)
	* **Recommended** (This airport submission has been recommended for inclusion in a future release of X-Plane. Airport submissions that are "Recommended" at the moment of capture into the next release of X-Plane will appear.)
	* **See Comments** (This airport submission could not be approved by the moderator. The reason for this can be viewed by clicking the moderator comment button that applies to this submission.)

For more information, to register as an artist, or to download existing files, check out the [Airport Scenery Gateway website][68]. 

## Editing Using the Map Tools ##

Tools used in WorldEditor can be categorized as follows:

- [Selection Tools][]
	- Marquee tool
	- Vertex tool
- [Entity Creation Tools][]
	- [Point Tools][]
		- [Directional Point Tools][]
			- Helipad tool
			- Taxi sign tool
			- Light fixture tool
			- Ramp start tool
			- Object tool
		- [Non-Directional Point Tools][]
			- Tower viewpoint tool
			- Airport beacon tool
			- Windsock tool
	- [Linear Tools][]
		- Runway tool
		- Sealane tool
		- Exclusion tool
		- Taxi routes tool
	- [Bezier Path Tools][]
		- Taxiway tool
		- Hole tool
		- Taxiline tool
		- Boundary tool
		- Facade tool
		- Forest tool
		- String tool
		- Line tool
		- Polygon tool

### Selection Tools ###

When working with WED, you will be doing lots of selecting. There are two tools used for selecting entities: the vertex and marquee tools. Both of these tools can select entities by two means:

1.  By single-clicking on an entity, or
2.  By clicking and dragging a rectangle around an entity.

Selecting things with the marquee tool will result in a bounding box onscreen, which can then be manipulated (i.e., scaled), moved, and rotated using modifier keys in combination with cursor movements. For instance, holding down the Alt/Option key with some items selected with the marquee tool will allow you to scale and rotate the object. Selecting things with the vertex tool will allow you to edit entities on a point level---that is, you can move vertices and bezier handles.

There is some overlap in these tools depending on what you are selecting and what you want to do. For instance, you can use the vertex tool to select one end of a runway and move it, but you may also select that same end of a runway using the marquee tool. In a similar manner, you can select a single vertex of a taxiway with the marquee tool and move it, but not edit its shape via bezier handles (for that, you would have to use the vertex tool). A good rule of thumb is that you use the marquee tool to move, rotate, and scale selected items, and you use the vertex tool to manipulate individual points and entities.

Holding modifier keys change the way selection operates in the following ways when using the vertex tool:

* Shift key: add these items to any existing selection.
* Shift key + dragging the perimeter of a linear or polygonal feature will move and scale I "proportionally"--polygons will grow and shrink evenly around all sides, while lines move sideways to create new lines perfectly parallel to their previous location. 

![duplicating lines](images/New_20/line_modifier.jpg)

<p class="cap"><strong>Figure 19</strong>: Using Modifier keys to duplicate lines</p>

* CTRL/Command key: toggle the newly selected item's status. Unselected entities will be added to the existing selection, already selected entities will be removed from the existing selection.

When using the marquee tool, holding modifier key(s) does the following:

* Alt/Option: rotate the selection
* CTRL/Command key: resize the selection while preserving the aspect ratio
* SHIFT + CTRL/Command: rescale and preserve the location of the center of the selection


### Entity Creation Tools ###

With the exception of the selection tools, all tools in WED are used to create objects from points, lines, and bezier paths. With this in mind, we will consider the tools by type.

#### Point Tools ####

##### Directional Point Tools #####

Directional point tools include the following:

*   Helipad tool
*   Taxi sign tool
*   Light fixture tool
*   Ramp start tool
*   Object tool

A directional point tool will allow you to place an entity and set its direction by clicking and dragging when placing the entity. If a single click with no dragging is used to place a directional entity, the entity will be placed with a heading of 0.00 degrees. The heading can be changed by either

*   Selecting the entity and typing a numerical heading in the attributes pane, or
*   Using the marquee or vertex tool to select and graphically rotate the entity by holding down the option key and clicking and dragging one of the corners of the bounding box.

Note that the helipad tool is not a "pure" point tool, as helipads can be resized and stretched. However, stretching a helipad to a non-square shape is not recommended, as it will result in a distorted texture in X-Plane.

##### Non-Directional Point Tools #####

Non-directional point tools include the following:

*   Tower viewpoint tool
*   Airport beacon tool
*   Windsock tool

Non-directional entities are simply placed by clicking in the map window where you want the entity to be. Each type of entity has its own set of unique attributes viewable and modifiable in the attributes pane.

#### Linear Tools ####

Linear tools include the following:

*   Runway tool
*   Sealane tool
*   Exclusion tool
-   Taxi route tool

A linear tool is defined by two endpoints, though the scenery entities created may later be scaled and rotated as though they were defined as a box. Placement of runways and sealanes can be accomplished with either

*   two single mouse clicks, one for each end of the entity, or
*   a single mouse click to establish one end and then clicking and dragging the second end graphically.

If you click and drag for the first point of a runway or sealane, a little crosshair cursor will appear which can then move around for exact placement of the first point. Releasing the mouse button will then allow you to click for the second point as usual.

Exclusion zones are drawn somewhat differently: simply click once and drag to the opposite corner in order to draw an exclusion "box." Use the vertex or marquee tools to fine-tune the size of this box.

You cannot click and drag with the taxi routes tool; you must click to place the first point, then click again each time to place the next segment. Press enter when the line is complete to stop adding points. To approximate curves with this tool, use multiple short segments. AI aircraft will automatically smooth any sharp angles or crudely drawn taxi routes in the simulator. Similar to the Bezier path tools described below, a new point can be added to a taxi route by selecting the vertex tool, then Alt (Option on a Mac) clicking on the path.

#### Bezier Path Tools ####

<div class="floatRight" style="width:360px">
<img src="http://www.x-plane.com/images/WED/bezier/open-closed.jpg" alt="Open and closed bezier paths" width="350px" class="centerMe" id="openclosed" />
<p class="cap text_left"><strong>Figure 20</strong>: Open and closed bezier paths</p>
</div>

Bezier path tools include the following:

- Taxiway tool
- Hole tool
- Taxiline tool
- Boundary tool
- Facade tool
- Forest tool
- String tool
- Line tool
- Polygon tool

Bezier tools are used to create freeform shapes. These shapes are commonly called Bezier curves or Bezier paths.

<div class="floatRight" style="width:360px">
<img src="http://www.x-plane.com/images/WED/bezier/crossing.jpg" alt="A normal Bezier path (filled, on the left) compared to a Bezier path crossing over itself (unfilled, on the right)" width="350px" class="centerMe" id="crossing" /><br/>
<p class="cap text_left"><strong>Figure 21</strong>: A normal Bezier path (filled, on the left) compared to a Bezier path crossing over itself (unfilled, on the right)</p>
</div>

Drawing Bezier shapes may seem a bit foreign for the uninitiated, but with practice, it can become second-nature. There are some important concepts to know in order to successfully work with Bezier shapes. The first is that a Bezier shape may be open or closed. [Figure 20][69] shows an example of open and closed Bezier paths. In WED, open Bezier paths are used only for the taxiline, taxi route, line, and string tools. Closed Bezier paths are used for the taxiway, hole, boundary, facade, forest, and polygon tools.

A closed bezier path is also called a "ring." It is important to note that a closed Bezier path may *not* cross over on itself. [Figure 21][70] shows two closed Bezier paths. You'll notice that the path that crosses over itself has no fill (it is not solid), indicating a problem that will cause the entity not to render in X-Plane. 

##### Components of a Bezier Path #####

<div class="floatRight" style="width:360px">
<img src="http://www.x-plane.com/images/WED/bezier/parts_of_path.jpg" alt="A node, segment, and control handles illustrated in a Bezier path" width="350px" class="centerMe" id="components" /><br />
<p class="cap text_left"><strong>Figure 22</strong>: A node, segment, and control handles illustrated in a Bezier path</p>
</div>

In order to learn to work with Bezier path, you need to know the pieces that make up a Bezier path. [Figure 22][71] shows the primary parts of a Bezier path. The most fundamental part of the path is the **node**, represended as a round dot or triangle, depending on the node's type. Attached to nodes are **control handles**, which are small triangles at the end of a line. A node can have 0, 1, or 2 control handles. The path between two nodes is called a **segment**. The location of two connected nodes' control handles determines the shape of the segment between them. For instance, two nodes that each have no control handles will have a straight segment between them. All segments together compost the **bezier path**.

A node can have a few different configurations, as follows:

*   Plain nodes have no control handles and are primarily used for sharp corners; the segments on both sides of a plain node will be straight. These nodes are represented by an upside-down triangle.
*   Single-handle nodes have a control handle on one side of the node. The segment on one side of the node will be straight and the segment on the other side will be curved.
*   Normal nodes have two control handles which are exactly opposite one another. Moving or extending one control handle causes the other to change correspondingly. This node is commonly used in the middle of a curve.
*   Split (or "broken") nodes have two control handles, but each handle can be moved or extended independently, thus changing the curve on either side of the node in different ways.

##### Creating Shapes #####

With the knowledge of the four node types, the next step is to string those nodes and control handles together in such a way as to create the shapes you want. A bezier path is created by selecting a Bezier tool clicking to outline your shape; a node will be added for each click (or click-and-drag) operation. A Bezier path may have as many nodes as needed to create the shape.

There are two ways to "complete" a shape (i.e., tell WED you are finished adding nodes)---either double-click when creating the final node, or change to another tool, in which case the last node you added will be the last point. Once the shape has been completed, you can edit it with the vertex tool.

<div class="floatRight" style="width:360px">
<img src="http://www.x-plane.com/images/WED/bezier/different_config_same_shape.png" alt="Different configurations of Bezier nodes resulting in the same shape" width="350px" class="aligncenter" id="diffconfigs" /><br />
<p class="cap text_left"><strong>Figure 23</strong>: Different configurations of Bezier nodes resulting in the same shape</p>
</div>

It is very common when drawing Bezier paths to work with all the node types. There is no one particular way to draw a path. [Figure 23][72] illustrates that two similar shapes can be drawn with completely different types of nodes. With a little practice, you'll soon get the feel for how you want to create your shapes.

When drawing curves, it is extremely common to convert between node types while in the middle of drawing a path. When in the process of drawing a path, you may only create plain nodes (by single-clicking) or normal nodes (by clicking and dragging). You must place either of these two types of nodes first, (optionally) convert them to a different node type, then continue drawing the path. Note that you cannot convert a node to a single-handle node while in the process of drawing the path. Once the path has been completed, however, you may convert between all types.

To convert from one type of node to another, use the modifier keys Shift, Ctrl (Command on Macs), and Alt keys in combination with single clicking or clicking and dragging. [Figure 24[73] lists the keystroke combinations to convert between the node types.

![Chart for converting between Bezier node types in WED][image-30]

<p class="cap"><strong>Figure 24</strong>: Chart for converting between Bezier node types in WED</p>

##### Adding Nodes #####

Once a closed Bezier path has been drawn, you may want to add new nodes to the existing shape. To do this, use the vertex tool and select the two nodes on either side of the point where you would like to add a node. With the two nodes selected, open the Edit menu and click **Split**, or press Ctrl+E on Windows or Command+E on Macs. A new node will be created in the middle of the selected nodes.

##### Cutting Holes in Bezier Shapes #####

<div class="floatRight" style="width:360px">
<img src="http://www.x-plane.com/images/WED/bezier/hole.jpg" alt="A comparison of a properly drawn hole (which is contained entirely within its parent shape) and an improperly drawn one (which crosses the boundary of its parent shape)" width="350px" class="centerMe" id="holetool" />
<p class="cap text_left"><strong>Figure 25</strong>: A comparison of a properly drawn hole (which is contained entirely within its parent shape) and an improperly drawn one (which crosses the boundary of its parent shape)</p>
</div>

The hole tool is a Bezier type tool and is used to create holes inside of existing Bezier shapes. The most important thing to keep in mind when using the hole tool is that the Bezier hole must be entirely contained within its parent shape. [Figure 25][74] show an example of this, where moving the hole outside of its parent entity will cause the shapes to lose their fill texture. A hole must be associated with another shape---this is done by selecting the parent shape (e.g., by using the marquee tool) before using the hole tool. You may select a shape using either the vertex tool or marquee tool. When the shape is outlined in orange, you may use the hole tool.

Since the hole is attached to a parent shape, moving the parent shape will move the hole also. However, you may select the hole with the marquee tool and move it withing its parent shape to relocate it, or select it with the vertex tool to reshape the hole.

##### Transforming and Rotating Shapes #####

After having created a shape, you can manipulate it: you can stretch it, scale it, or, in some cases, rotate it. To do so, use the marquee tool and select the entity. When you do so, a bounding box appears around the entity. By clicking and dragging one of the box's nodes, you can stretch the shape. By holding down the Alt key, you can cause the bounding box's nodes to become rotation nodes; by clicking and dragging a node, you can rotate the shape.

##### Texturing the Shape #####

Once a shape is created, in some cases, such as with shapes made with the taxiway or polygon tool, you can specify what kind of surface the it has. The shape's texture has two properties: type and direction. The type of texture is specified in the attributes pane using the pull-down menu for the Surface property, or by choosing a library file in the Resource field. Typical surfaces are asphalt, grass, dirt, water, and so on. The texture's heading can be set using the Heading field in the attributes pane. Or, to adjust the texture heading graphically, select the shape using the vertex tool, then hold down the Shift key and click and drag within the shape. As you drag the mouse, the texture will update in real time and you can align the texture as you like.

## Advanced Topics ##

### Synchronizing with Other Editors ###

In order to work with data from another program, you should do a one-time import of the files from that program, then work in WED only until you export your file for use in X-Plane. You can export the scenery (by opening the File menu and clicking **Export Scenery Pack**) as many times as you like, but you must re-import files from other programs any time you modify them, such as any time you modify an image file in an image editing program. Note that WED will not monitor your files for duplicates, so you might unintentionally import a copy of a file you already have (such as a DSF).

There is one important disadvantage of repeated import/export cycles: the DSF file format is a highly compressed data format so scenery elements' coordinates will lose precision with every export. Other WED-specific information such as groups or names names given to specific entities are not stored in DSF files at all, so this information is permanently lost. This are some of the reasons we **strongly** recommend always using the option to import airports directly from the Scenery Gateway--this is equivalent of receiving the full precision and information stored in WED's proprietary earth.wed.xml files

Note also that WorldEditor will not automatically import projects from older versions. You can, however, import your project manually. Simply point WED's opening screen to the appropriate X-Plane folder and choose the applicable scenery pack from the Custom Scenery folder.


### Replacing Default Airports ###

The X-Plane scenery system uses globally unique identifiers called Airport IDs to identify every individual airport. These are also used to determine if any  airport in Custom Scenery should remove, or "exclude," the corresponding airport included in default scenery or otherwise installed. These identifiers are separate from, and in many cases _different_ than, the more commonly known ICAO codes. To make this mechanism work, every airport must use the same Airport ID as X-Plane uses for its own airport in the same place. Set the WED property "Airport ID" in the airport's properties according to the section "[Creating an Airport From Scratch][]."

Some older sceneries may use different Airport IDs. To find the correct, current Airport ID, look up the airport on the Airport Scenery Gateway or **Toggle NAVAIDS** from under the View menu.

![navaid overlay at KSEA](images/New_20/navaid_overly.png)

<p class="cap"><strong>Figure 26</strong>: The NAVAID overlay layer at KSEA</p>

This layer will show VFR-map-style airport, heliport and seaport icons along with the assigned Airport ID for each.

### Improving Performance of WorldEditor ###

If you have performance issues with WED, you can turn off the visual preview of objects. To do so, open the View menu and click **Toggle Preview**.

### Recovering from a Crash ###

In the event that WED crashes, users can easily recover their work from a back up file that is automatically generated after every save. First, in the Custom Scenery folder, naviagate to the folder for the scenery that caused the crash. Inside the folder will be two .xml files: "earth.wed.bak.xml" and "earth.wed.xml." Delete the file "earth.wed.xml," then rename the other file to "earth.wed.xml." Upon restart, the scenery file will be restored to the last saved version from before the crash.

### Troubleshooting and How to File a Bug Report ###

As a general rule of thumb, the first thing you should do after encountering any problems is update to the latest version. If you are running the latest version and still have problems, you can check for problem files by manually running the installer found on the X-Plane website. Select "Update X-Plane," pick which copy you'd like to update and click the "continue" button. The installer will scan your installation to see if any of the default files are missing or altered, and allow you to restore them.

For additional help, first search for a solution on the [X-Plane Q & A site](http://questions.x-plane.com/). You can also ask your question on the site if it has not been covered already. Questions are answered by Laminar Research team members and knowledgeable community members. The site also feature commenting, voting, notifications, points and rankings. 

When sending a bug report, please include as much information as possible----anything that the X‑Plane development crew might need to know in order to reproduce the malfunction. This includes (but is not limited to) the following information:

1.  The file `WED_log.txt` found in the same directory as the WED executable.
    This file includes all hardware and software version information.
2.  All exact steps (as specific and step-by-step as possible) required
    to reproduce the problem.

Additionally, before filing a bug report, please:

1. Be sure you are using the latest version of WED (this includes making sure you aren't using an outdated shortcut).
2. Be sure you understand the feature you are reporting a bug on.
3. Ask on the [X-Plane Q & A site](http://questions.x-plane.com/) if you are not sure whether you have a bug or a tech support problem.
4. Attach the appropriate 'earth.wed.xml' file when filing the report, as well as PNG screenshots for any visual problems. 

To file a bug report, please visit the [Airport Scenery Gateway][75] and create a [free account][76]. WED bugs should be filed on the "Scenery Tools" tab on the [Report A Problem page][77]. All bugs are public and ongoing progress or comments can be tracked on this page.

## Menu Reference ##

### The File Menu ###

| Menu item                 | Description                                           |
| ------------------------- | ------------------------------------------------------|
| New Package               | Creates a new package in the package list             |
| Open Package              | Opens the selected package for editing                |
| Change X-System Folder    | Changes which X-System folder WED is editing          |
| Close                     | Closes the current scenery pack                       |
| Save                      | Saves the current sessions                            |
| Revert to Saved           | Reloads the package from disk, going back to the last saved version. Note that you can undo a revert-to-saved by using the "undo" function under the Edit menu. 
| Validate                  | Checks a scenery pack for errors and problems.        |
| Target X-Plane Version | Sets the oldest version of X-Plane that can use the scenery pack. Note: setting this to older versions may not allow newer WED features to export. |
| Import apt.dat            | Imports an `apt.dat` file. This brings up a list of airports in the apt.dat file to select from. Note that the default `apt.dat` is located in `X-Plane 10/Resources/default scenery/default apt dat/Earth nav data/`. You will have the option of only importing *some* of the airports contained in the file. Hold Ctrl and click (Command+click on Macs) to select multiple airports, then hit the **Import** button.  |
| Import DSF                | Imports a DSF file from disk.                         |
| Import Orthophoto | Imports selected file from disk. Note building a scenery pack will turn this file into orthophoto scenery. |
| Import from Airport Scenery Gateway | Imports an `apt.dat` file, as described above, from the Airport Scenery Gateway database instead of the local disk. |
| Export apt.dat            | Exports only `apt.dat` data into a single `apt.dat` file. |
| Export Scenery Pack       | Exports the entire scenery pack, DSFs and `apt.dat`s into your scenery pack, so you can fly it in X-Plane. |
| Export to Airport Scenery Gateway | Automatically exports and uploads the selected scenery pack to the Gateway |
| Exit (Windows/Linux only) | Quits WED.                                            |

### The Edit Menu ###

| Menu item                 | Description                                           |
| ------------------------- | ------------------------------------------------------|
| Undo, Redo                | WED supports multiple levels of undo/redo for map items.  Undo/redo is not available for text editing; instead, simply finish the text edit and then undo. |
| Cut, Copy, Paste          | Only available for text editing.                      |
| Clear                     | Deletes the selection                                 |
| Duplicate                 | Duplicates the selection. Note that this works for hierarchy items, so you can duplicate whole groups of items in the hierarchy pane. |
| Group, Ungroup            | These group the selection in the hierarchy pane, or break apart any group that is selected. |
| Split                     | Introduces split points into any side of a polygon or line whose end-points are selected. |
| Align 	| Align nodes into a straight line. |
| Match Bezier handles | Automatically match a line or polygon to the shape of another one. |
| Othogonalize	|	Straightens all sides of a polygon to make exact rectangles, etc. |
| Make Regular Poly | Makes polygons into rotationally or axially symmetrical shapes. |
| Merge                     | Merges two selected ATC network nodes into a single node, connecting the incoming routes of both. |
| Reverse                   | Reverses the winding direction of a polygon. Note that if a polygon is right-side out and is reversed, it will be inside-out and stop rendering. |
| Rotate                    | Rotates the order of sides on a polygon, which can change the position of facade sides and markings. |
| Crop Unselected           | Deletes every unselected element from the WED project.  Parents and children of the selected elements in the hierarchy are kept. |
| Convert to | Convert taxiways, draped polygons, airport line markings or lines into another type of entity of this list. |
| Move First, Move Up, Move Down, Move Last | Change the ordering of the selected object in the hierarchy pane (e.g., selecting move first will cause the selected element to be highest in whatever group it is in).  |
| Break Apart APG's        | Break apart .agp groups into individual objects, if all .obj referenced are public. |

### The View Menu ###

| Menu item                 | Description                                           |
| ------------------------- | ------------------------------------------------------|
| Zoom World                | Zooms out to see the entire world.                    |
| Zoom Package              | Zooms to show only the contents of the currently open package. |
| Zoom Selection            | Zooms in to see the selected items filling the map pane. |
| Show Line Markings        | Shows a simple color preview of taxiway and line markings for `apt.dat` files. |
| Show Vertices             | Shows each vertex as a small dot.                     |
| Pavement Transparency     | Selects a level of transparency for all apt.dat pavement. |
| Object Density            | Shows a preview of the objects that a user would see at a given object density (set in the Rendering Options window in X-Plane). |
| Pick Overlay Image        | Creates a new overlay image from disk for reference purposes only. |
| Toggle World Map          | Turns the low-resolution background world map on or off. When checked, the background map is enabled. |
| Toggle Navaids            | Toggles background layer with airport icons and NAVAID locations. |
| Slippy Map | Toggles OpenStreetMap or ESRI map information. |
| Toggle Preview            | Toggles semi-realistic preview of objects, AGPs, facades and most other art assets. When checked, these previews are enabled. |
| Restore Frames            | Resets the frames of the WED editing window to their default position. |

### Select Menu ###

| Menu item                 | Description                                           |
| ------------------------- | ------------------------------------------------------|
| Select All, Select None   | Select every object in the package or none of them.   |
| Select Parent/Select Children | Select the parent or children of the current selection in the hierarchy. |
| Select Polygon            | Given a selected vertex, selects the polygon that contains it. |
| Select Vertices           | Given a selected polygon, selects its vertices.       |
| Select Connected 	| Selects disjointed parts of an ATC network. | 
| Select Degenerate Edges  | Selects ATC edges of zero length (i.e., edges between two points that are on top of one another).  These are good edges to delete or the pack will not pass validation. |
| Select Double Nodes       | Selects any ATC nodes that overlap each other.  These are good nodes to merge or the pack may not pass validation. |
| Select Crossing Edges     | Selects ATC edges that cross each other.  These edges should be split or the pack may not pass validation. |
| Select Local Objects/Library Objects/Laminar Library Objects/Third Party Library Objects/Missing Objects | Selects objects meeting the criteria |

### Airport Menu ###

| Menu item                 | Description                                           |
| ------------------------- | ------------------------------------------------------|
| Create Airport            | Creates a new airport within the package.             |
| Create ATC frequency      | Creates a new ATC frequency for the current airport.  |
| Create Airport Flow       | Creates a new, empty airport flow for the current airport.   |
| Create Runway Use         | Adds a runway use rule to the selected flow.          |
| Create Runway Time Rule   | Adds a new time rule limitation to the current flow.  |
| Create Runway Wind Rule   | Adds a new wind rule limitation to the current flow.  |
| Add Metadata	| Add metadata fields such as City, Country, etc. |
| Update Metadata | Downloads new, missing metadata (if available) for the airport. |
| Edit Airport *[airport]*  | Changes the current airport to the selected one.      |
| Upgrade Ramps | Auto-update pre X-Plane 10.50 style ramp starts with useful defaults. |
| Align Airports | Mass-move airports to bring runways into CIFP compliance. |
| Replace Vehicle Objects   | Replace static ground service .objs with X-Plane 11 ground service functions. |

## Property Reference ##

The following is a list of object types with descriptions of the properties associated with them. Note that all objects have an additional Name property, which simply identifies them in the hierarchy, and many have latitude, longitude, or heading fields which are self-explanatory.

| Property name   | Description                                                    |
| --------------- | -------------------------------------------------------------- |
|                               **Airport**                                       ||
| Type            | Seaports and Heliport do not support "Always flatten"          |
| Field Elevation | Elevation to use when Airport terrain is flattened at run-time |
| Has ATC         | Property specifying ATC availability (X-Plane 9 only)          |
| Airport ID      | Handle used to idenify and disambuguate airport sceneries. NOT used in user facing displays in X-Plane 10.45 and later |
| Always Flatten  | Flatten terrain completely to elev. specified in Field Elevation |
| Left Hand Driving | Ground Service Vehicles drive on left side of centerline     |
| Scenery ID      | For Scenery Gateway internal revision control purposes, only.  |
|                               **Airport Beacon**                                ||
| Type            | The type of beacon at this location.                    |
|                             **Airport Boundary**                                ||
| Markings           | The pavement line markings attached to this boundary, set on a per-point basis.     |
| Lights          | The lights attached to this boundary, set on a per-point basis.    |
|                             **Airport Lines**                                   ||
| Lines           | The pavement line markings attached to this line, which may be set either at the level of a line or its points.                 |
| Lights          | The airport lights attached to this line, which may be set either at the level of a line or its points.                 |
|                             **Airport Sign**                                    ||
| Name            | Interpreted as a sign code for these entities. See the `apt.dat` [1000 file specification][78] for the meaning of the sign codes.   |
| Heading | Specifies the direction the front of the sign faces. |
| Size | Size of the sign, defined as one of several preset sizes. |
|                             **ATC Flow** (X-Plane 10 only)                      ||
| METAR ICAO            | The METAR ICAO code of the airport whose weather determines the use of this flow. For smaller airports, their flow may be dictated by a nearer larger airport, so this ICAO may not be the ICAO of the airport that contains the flow!     |
| Minimum ceiling | The minimum ceiling in feet/meters at which this flow can be used. |
| Minimum visibility | The minimum visibility in sm at which this flow can be used. |
| Pattern runway | The runway to be used for pattern operations.                   |
| Pattern direction | Defines whether the airport uses left or right traffic.      |
|                             **ATC Frequency**                                   ||
| Type           | The type of frequency (e.g. ground, delivery, etc.)             |
| Frequency      | The actual frequency or 8.33kHz channel number, in MHz.         |
|                             **ATC Runway Use** (X-Plane 10 only)                ||
| Runway         | The runway to use                                               |
| Departure Frequency | The actual departure frequency, in MHz. |
| Traffic type   | A set of aircraft categories (e.g., prop, jet) that can operate on the runway.  |
| Operations     | The type of operation (e.g. departure, arrival) that can operate on the runway. |
| Legal on-course hdg min/max | This defines the range of headings that the first waypoint for the departure can still be in to receive this runway. |
| ATC assigned heading min/max | Defines the range of headings ATC can give the aircraft on takeoff from this runway.  |
|                             **ATC Time Rule** (X-Plane 10 only)                ||
| Start time     | The earliest time this flow can be used, in Greenwich mean time (GMT), or Zulu time.   |
| End time       | The latest time this flow can be used, in Greenwich mean time (GMT), or Zulu time.    |
|                             **ATC Wind Rule** (X-Plane 10 only)                ||
| METAR ICAO           | The ICAO of the airport whose wind is being measured. Like ceilings, the airport whose weather determines flow may not be the same as the airport you are currently working with; small airports must often do what big airports dictate.    |
|Direction from/to | Defines the range the wind must come from for the flow to be used. |
| Max speed (knots) | The maximum wind speed for which this rule is in effect. |
|                             **Draped Orthophoto**                        ||
| Resource       | The file name of the `.pol` file that defines the texture for the draped orthophoto                 |
| Texture Top, Bottom, Left, Right| UV texture bounds used for automatic re-calculation of texture UV coordinates when polygon geometric shape changes. 1,0,0,1 results in the full texture being stretched to cover polygon shape. 0,0,0,0 or Top==Bottom or Left==Right disables automatic coordinate calculation |
|                             **Exclusion Zone**                                  ||
| Exclusions | These define what types of 3-D entities are excluded below this rectangle. The OBJ exclusion excludes objects and auto-generated X-Plane 10 elements.  |
|                             **Facade**                                          ||
| Height | Height of the facade in X-Plane |
| Resource       | The file name of the `.fac` file that defines the look of this facade.   |
| Pick walls     | Check this box to define the wall types on a per-node basis (available in X-Plane 10 only).    |
| Show with     |  The minimum rendering settings at which this object is guaranteed to appear. By picking higher rendering settings in this popup, you allow X-Plane to drop your facade when the user's rendering settings are low.  If you pick default, your facade will always appear.  |
|                            **Forest Placement**                                ||
| Density        | The vegetation density in this forest stand as a ratio, where 0.0 is none and 1.0 is maximum density.  |
| Resource       | The name of the `.for` file that defines the look of this forest. |
| Fill mode      | Defines whether the forest has trees filling the area, around the edges (linear), or just at corners (points)    |
|                             **Helipad**                                         ||
| Width/Length | Defines the size and shape of the helipad; square by default. |
| Surface        | The appearance of the surface of the pad, such as concrete or asphalt.  |
| Markings       | Markings on the helipad itself. Only "default" currently available.                                |
| Shoulder       | The type of shoulder around the helipad.                         |
| Roughness      | A ratio of roughness, currently ignored by X-Plane.              |
| Lights    | The type of lights around the edge of the pad.                   |
|                             **Light Fixture**                                   ||
| Type           | The type of lighting fixture.  Non-operational fixtures like apron lights can be placed as library elements.     |
| Angle          | The glide slope that shows correct descent for PAPIs and VASIs.  |
|                             **Line Placement**                                   ||
| Resource       | The path to the `.lin` file that defines the look of this line.  |
| Closed         | When checked, WED will automatically close the line to make a loop.  |
| = Airport Line | Virtual property to invoke the Line Style Selector GUI, offering to select a resource that matches the appearance of the choices available for Airport Line Markings |
|                             **Object Placement**                                 ||
| Elevation Mode  | Select to place the object either at ground level "None", at an height relative to ground level "set_AGL" or at an absolute elevation "set_MSL". Set_MSL is available starting with XP10, set_AGL requires XP11.50 or later. |
| Elevation      | Height in MSL or AGL as per Elevation Mode property.             |
| Resource       | The path to the `.obj` or `.agp` file that defines the look of this object placement.  |
| Show level     | The minimum rendering settings at which this object is guaranteed to appear.  By picking higher rendering settings in this menu, you allow X-Plane to not display your object when the user's rendering settings are low.  If you pick "default", your facade will always appear.  |
|                             **Polygon Placement**                                ||
| Heading        | The heading, in degrees, to which the draped polygon's texture is rotated. |
| Resource       | The path to the `.pol` file that defines the look of this draped polygon. |
|                             **Ramp Position**                                    ||
| Ramp start type      | The type of parking spot (available in X-Plane 10 only).          |
| Equipment      | Selects all airplane classes that can legally park at this spot (available in X-Plane 10 only).  |
| Size           | Largest size of aircraft to use the position (X-Plane 10.45+ only) |
| Ramp Operation Type | Types of aircraft that park to use the position (X-Plane 10.45+ only) |
| Airlines       | List of 3-letter airline codes to specify liveries for parked aircraft (X-Plane 10.45+ only) |
|                             **Runway**                                           ||
| Width | Change the default width. |
| Surface        | The material the runway itself is built out of.                  |
| Shoulder       | The material the shoulder of the runway is built out of, if there is one. |
| Roughness      | A ratio for how bumpy the runway is (currently ignored by X-Plane). |
| Centerline lights  | A check box to enable centerline lights on the runway.           |
| Edge lights    | The type of edge lights.  Note that X-Plane does not vary brightness between MIRL, HIRL and LIRL.  |
| Distance signs | Check this box to have X-Plane generate distance remaining signs every 1000 feet. |
| Displaced threshold (at each end) | The number of meters to displace the landing threshold from the end of the runway.  |
| Blast pad  (at each end) | The number of meters of blast pad next to the runway. |
| Markings (at each end) | The touch down markings for this end of the runway.     |
| Approach lights  (at each end) | The type of approach lights for this end of the runway. |
| TDZ lights  (at each end) | Check this to enable touch down zone lights on this end of the runway. |
| REIL lights  (at each end) | The type of runway end identifier lights---pick unidirectional, omnidirectional, or none. |
|                             **Sealane**                                         ||
| Width | Change the default width. |
| Show buoys          | Check this to have X-Plane generate buoys along the sealane.    |
|                             **String Placement**                                ||
| Resource       | The path to the `.str` file that defines the look of this object string. |
| Spacing        | The distance between object placements in meters.               |
| Closed         | Check this to make a closed loop rather than an open line.      |
|                             **Taxi Route**                                   ||
| Split | Cannot (and should not) be changed. |
| One way        | Check this if traffic on this route should only flow in one direction.  Be careful with this property, as it puts pressure on ATC to make circuitous routes.  |
| Size           | Maximum size / wingspan of aircraft per [ICAO classification](https://developer.x-plane.com/article/atc-taxi-route-authoring) allowed to traveling the segment |
| Allowed Vehicles | Type of traffic handled by route: Aircraft or Ground Vehicles |
| Runway         | Select which runway the ATC taxi route is on.                |
| Departures     | A set of all runways that this route intersects with for the purpose of departures. This can include an intersection between the route and the airspace after the departure end of the runway.  |
| Arrivals       | A set of all runways that this route intersects with for the purpose of arrivals. This can include an intersection of the route with the airspace before the touchdown zone or after the departure end.      |
| ILS precision area | A set of all runways whose ILS precision areas intersects this route.  |
|                             **Taxiway**                                         ||
| Surface        | The material that makes up the surface of this taxiway.         |
| Roughness      | A ratio for how bumpy the runway is (currently ignored by X-Plane). |
| Texture heading        | The direction of the "grain" of the surface, in true degrees.   |
| Line attributes          | The pavement line markings attached to this taxiway---may be set on the entire taxiway or on segments by selecting individual points.    |
| Light attributes         | The airport lights attached to this taxiway---may be set on the entire taxiway or on its points.  |
|                             **Tower Viewpoint**                                 ||
| Height         | The viewpoint's height above the ground when using the tower view in X-Plane.  Note that this simply controls where the camera is placed in tower view mode---it does not create a control tower.  Place a control tower object using a library object. |
|                             **Windsock**                                        ||
| Lit            | Check this for the windsock to be lit at night.                |

## Appendix: Anatomy of the X-Plane Scenery System ##

This is a broad overview of the architecture of X-Plane's scenery system. It provides a road map for authors on how the various components fit together.

### Scenery Packages ###

In X-Plane, all scenery content comes in a scenery package. A scenery package is a folder with a predefined file organization.

Custom scenery is typically distributed as a single scenery package. A scenery package can contain art assets for the library, tiles, airport data, or any combination of the above.

Scenery packages are installed in the "Custom Scenery" folder. Scenery packages have a priority order, defined by their file name alphabetically; X-Plane also stores the default "global scenery" in packages (stored in the Global Scenery folder) and the sim's built-in art assets are stored in packages in `Resources/default scenery/`. All third party scenery should be packaged and installed in the "Custom Scenery" folder.

See the "Expanding X-Plane" section of [the X-Plane 10 manual][79] for information on how to download and install a custom scenery package.

#### Components of a Scenery Package ####

A scenery package is simply a folder that contains scenery. Within that package may be many files, including:

*   Art assets, which are textures, meshes, or other definitions used to visualize scenery.
*   Tiles, which define what sorts of things are on a small portion of the Earth.
*   A library file, which forms a "master list" of all art assets for use in all scenery packages.
*   An airport data file, which defines the layout of one or more airports.

### Scenery Tiles (DSF) ###

X-Plane divides the planet into thousands of 1x1 degree "tiles," which are "cut" along latitude and longitude lines. For a given area of the earth, tile files are loaded to build the local contents of the planet. There are two kinds of tiles: base mesh and overlay.

X-Plane normally maps a 3x2 tile region (3 east-west, 2 north-south) to provide about 200x200 km of loaded scenery at any one time.

There are two kinds of tile formats: DSF (distribution scenery format) is the current tile format, supported in X-Plane 8, 9, and 10. ENV (environment file) is the legacy tile format supported in X-Plane 6 and 7 (and supported but deprecated in versions 8 and 9). The rest of this article will refer only to DSFs. Any new scenery you create should use DSFs.

Tiles are named by the latitude and longitude of their southwest corner. They live in a folder that defines a 10x10 block of tiles, defined by its southwest corner, and that folder in turn lives in a per-planet folder, which lives in your custom scenery package. For example, consider the following hierarchy:

- X-Plane Install/
	- Custom Scenery/
		- My Awesome Boston Scenery/
			- Earth nav data/
				- +40-080/
					- +42-071.dsf
					- +42-072.dsf

"My Awesome Boston Scenery" is the name of the package. Note that there can be no sub-folders introduced in this particular layout---layout of DSF tiles is fixed.

DSFs are binary files. As such, they contain the coordinate and location data for a tile but not the art assets. For example, in an orthophoto scenery package---that is, scenery where the ground texture is taken from a real photo---the shape of the mesh (the elevations) will be in the DSF, but the image of the orthophoto will be in separate art asset files.

Tile files can contain:

*   Base mesh triangles (base mesh tiles only).
*   Beach placements, defined by 3-d polygonal outlines of the land-water polygon (base mesh tiles only).
*   Road grids, defined as a connected set of line segments with road type information.
*   Model placements, defined as points and rotations.
*   Facades (that is, buildings extruded from a footprint), defined as a polygonal footprint + height.
*   Forests of trees, defined as a polygonal footprint + density information.
*   Draped polygons, defined by polygon, for overlaying orthophotos or other repeating textures, like pavement.
*   Draped painted lines, defined by a polygon path, typically used for custom taxiway lines, etc.
*   Strings of objects, defined by a path, often used for adding a series of runway lights.

In all cases, the DSF file contains the coordinate location information and a reference to an art asset file that is stored separately.

#### Base Mesh Tiles ####

For any given tile, only one tile file can provide the base mesh, or ground. The base mesh is a textured triangle mesh used to model mountains, water, land, etc. Base mesh tiles can also contain all of the components of overlay files if desired.

For further reading, see the [DSF File Specification][80] and [DSF Usage by X-Plane][81] articles on the X-Plane Developer Site.

#### Overlay Tiles ####

A tile file can be marked as an overlay---in this case, its contents are superimposed over other tile files. Overlay files cannot contain any base mesh information. An overlay file is often used to add additional details. For example, an overlay tile could add the buildings of an airport.

Overlay tiles are loaded in priority order. An overlay can contain "exclusion zones," which are lat-lon rectangles that prevent elements of lower-priority tiles from being loaded. For example, if an overlay tile contained placements for custom buildings for Manhattan, the author would also create an exclusion zone around Manhattan that would prevent the default buildings (that ship with X-Plane) from appearing there.

<!--For further reading, see the [About Overlays](http://wiki.x-plane.com/About_Overlays) article on the X-Plane Wiki.-->

### Art Assets and the Library ###

All art assets are stored in scenery packages, but never directly inside a DSF. Typically an art asset is built from a combination of text files and image files. X-Plane supports DDS, PNG, and BMP; BMP is not recommended. DDS is the preferred format for scenery use. The text files often contain information on how to use the texture and other useful properties. For example, the ".net" file format, which defines a road network, indicates the shape of roads to be built, the physical properties of the roads (e.g., how bumpy they are), how to map the texture to the roads, and where to place cars on those roads.

A quick cheat sheet to art asset file types:

*   `.ter` – A terrain definition; the file lists physical properties, scaling information, and texture files.
*   `.obj` – OBJect---that is, a 3-d model/mesh. This is not the same format as an Alias/Wavefront OBJ files.
*   `.fac` – Facade definition; the file describes how to extrude a building from a footprint using a texture.
*   `.for` – Forest definition; the file describes how to build trees from a texture for placement within a polyogn.
*   `.bch` – Beach definition; the file describes how to tile a beach from a texture.
*   `.net` – Road network definition; the file describes how to build 3-d roads and place traffic for several road types from several textures.
*   `.pol` – Draped polygon; describes how to texture a polygon that is "draped" over the existing mesh using a texture.
*   `.str` – Object string; describes how to place a number of OBJ files along a line.
*   `.lin` – Line; describes how to paint a line along the mesh.

Note that `.pol` (draped polygons) can be used in an overlay, while `.ter` (base mesh terrain) can only be used in base meshes. Base mesh performance is significantly faster than draped polygons; draped polygons should only be used to customize _small_ areas of the world, like an airport surface area. These files are not appropriate for large-scale orthophoto scenery!

Art asset "primary" files (e.g. OBJ, FAC, etc) can be in any location within a scenery package---they are defined by relative paths. So you can organize your art assets as desired. The dependent ("secondary") files like DDS textures should be in the same folder as the art asset. An example:

- X-Plane Install/
	- Custom Scenery/
		- My Awesome Boston Scenery/
			- Earth nav data/
				- +40-080/
					- +42-071.dsf (references kbos/hangar.obj)
					- +42-072.dsf (references bos/fenway.obj)
				- kbos
					- hangar.obj (references hangar.dds) 
					- hangar.dds
				- bos
					- fenway.obj (references fenway.dds)
					- fenway.dds

In this layout, the .obj files can be moved anywhere within My Awesome Boston Scenery as long as the relative paths in the DSF are adjusted.

#### Objects ####

OBJ files are the main way to get 3-d models into X-Plane. They are used for both scenery models (e.g. buildings) and the 3-d visualization of airplanes. An OBJ is a text file, but typically they are edited using a 3-d editor like AC3D or Blender and exported. Objects can be animated, and have a number of specialized features that can be used only in airplanes.

For further reading, see the [OBJ8 Specification][82] article on the X-Plane Developer site.

#### Polygons ####

The polygon definitions in a DSF can sometimes have holes and sometimes they can be bezier curves; what is allowed depends on the art asset type. For example, bezier curves are allowed for draped polygons but not facades; holes are allowed for forests but not facades.

For further reading, see the [DSF Usage by X-Plane][83] article.

#### Road Networks ####

Road networks may be specified in overlay files, but they require MSL elevation (that is, their altitude is predefined). The lack of on-the-fly elevation adjustment for overlay road networks is a problem for making add-on road packs; fixing this limitation is an area of ongoing development.

For further reading, see the [.net File Specification][84] article on the X-Plane Developer site.

#### Library ####

The library provides a way to locate art assets by a virtual file system, rather than by the physical file system. The library allows one scenery package to "publish" an art asset (by providing it with a virtual file name in the library) and another scenery pack to use that art asset (by referring to it by its virtual file path).

The library system is useful for a few reasons:

*   It hides the actual layout of art assets from third parties, breaking a dependency. Laminar Research can (and does) rebuild our art assets periodically; this does not affect third parties because we map our assets to the legacy virtual paths.
*   It allows scenery packages to share art resources independently.
*   It allows scenery packages to augment, replace or customize the art assets that ship with X-Plane.
*   It allows a many-to-many relationship between art assets and usage. For example, we map many car .obj files to one virtual path, allowing for a variety of traffic. We map a particular building to both legacy and current virtual paths, so that we can provide legacy art asset support without having to duplicate the files on disk.

Note that image files are not shared via the library system; instead the text file that references them is shared.

Art assets do not have to be placed into the library system---a package can simply access them by relative file paths. Or, a package can put its art assets into the library and then reference them by virtual path, allowing for customization by other packages.

Art assets are placed in the library by putting a file called "library.txt" into the root of your package, like this:

- X-Plane Install/
	- Custom Scenery/
		- My Awesome Buildings/
			- library.txt (maps houses/house1.obj to lib/g8/buildings/60\_30.obj and houses/house2.obj to lib/g8/buildings/60\_30.obj)
			- houses
				- house1.obj (references house.dds)
				- house2.obj (references house.dds)
				- house.dds

Note that in this case, another scenery package that references houses/house1.obj will _not_ use these files. That is because scenery packages can only use each other's art assets via virtual path. A scenery pack that references "lib/g8/buildings/60\_30.obj" (a virtual path) might end up with either house1.obj or house2.obj.

Library files let you specify the replacement semantics for art assets–that is, a library file can provide additional variants of an art asset or replace the art asset completely. Libraries can specify what tiles (in latitude/longitude) can use the art asset for 'regionalization'.

For further reading, see the articles [Airport Customization Tutorial][85], [Library File Format Specification][86], and [Tutorial on Customizing X-Plane's Default Scenery Artwork][87].

### Airport Data ###

The data that describes the layout of an airport are stored in apt.dat files. There is one apt.dat file per package per planet, like this:

- X-Plane Install/
	- Custom Scenery/
		- My Awesome Boston Scenery/
			- Earth nav data/
				- apt.dat

The apt.dat file can contain one or more airports; each one fully replaces the previous airport's definition. (That is, you cannot simply add a runway to the existing KBOS, you must fully re-specify KBOS).

You do not need to provide tiles for the airports you cover, or vice versa.

Airport data does not affect the terrain type below the airport; airports in the default scenery that X-Plane ships have grass underneath them because the base mesh creation process added the grass. If you add a new airport, you may need to include an overlay tile with .pol placements that drape new terrain over the old; X-Plane's art assets for airport grass are all exported into the library for this purpose.
<!--Robin Peel maintains the file format and the master database of airports that form the "default" scenery for X-Plane. For further reading, see Robin Peel's [X-Plane Airport &amp; Navigation Data home page](http://data.x-plane.com/).-->

#### Airport vs. DSF Overlays ####

Airport definitions do not use any specific art assets; the file format models airport data and lets X-Plane use built-in art assets to draw.

However, all of the types of drawing that are possible in an airport (pavement, lines, lights) can be created using DSF overlay data. The intention is for authors who want to truly customize the look of their airport to use DSF overlays.

*   `.pol` files have the same functionality as taxiway and runway pavement.
*   `.lin` files have the same functionality as taxiway lines.
*   `.str` files have the same functionality as taxiway lights.
*   `.obj` files are actually used to build the lights that are placed in an airport–each runway light is actually an OBJ!

Some of X-Plane's art assets for airports are available via the library.

For further reading, see the [Advanced Airport Customization][88] article on the X-Plane Developer Site.

### Types of Custom Scenery ###

Here are some examples of scenery add-ons:

*   A customized airport. The package contains OBJs for the various buildings, an apt.dat file with a new layout, and an overlay DSF placing the objects.
*   A new base mesh. The package contains a base mesh DSF and .ter files for the orthophotos. Because of the amount of data required to create a base mesh, this is for advanced scenery authors only.
*   Regionalized Buildings. The package contains only OBJ files for customized buildings and a library.txt file to place them in the library.

## Appendix: About the X-Plane Library System ##

Since Version 8, X-Plane has employed a library system to locate the bitmaps, models, and other graphic resources it depends on. You can use the library system to replace virtually any part of the artwork for the global scenery.

### Libraries and Scenery Packages ###

Any X-Plane scenery package can be a library. To be a library, a scenery package must have a text file called `library.txt` directly inside it. This text file contains a list of all of the resources the library shares with X-Plane.

X-Plane scenery packages can be in either the Custom Scenery folder or the Default Scenery folder. The Custom Scenery folder is for add-on scenery packages that are installed by the user; the Default Scenery folder is for the scenery packages that come from Laminar Research and are installed with the simulator.

**Note**: The X-Plane updater/installer will sometimes upgrade the default scenery packages that come with the sim. It will not alter custom scenery packages. For this reason, all add-ons should be placed in the Custom Scenery folder!

The library.txt file maps a _file path_ for a resource inside the package to a _virtual path_ that scenery can use. File paths are the actual locations of the objects on disk. Virtual paths are used by the DSFs that need artwork.

### How X-Plane Locates Objects, Terrain, and other Graphic Resources ###

As X-Plane loads scenery, it must locate each object, terrain definition, etc. First X-Plane looks in each scenery package, in alphabetical order, first in custom scenery, then in the default scenery folder. If X-Plane finds a virtual path in a library.txt file in a package, it uses the object or terrain in that package located by the file path that is associated with it.

If X-Plane cannot find the object as a virtual path in any library, it then looks in the scenery package the DSF is located in for an actual file. A few things to note about this:

*   X-Plane will only look in a scenery package other than the one a DSF lives in for files in the library.txt. It does not actually look at the files in the package, only the library.txt file. So if you do not include a library.txt file, no other package will use your objects/terrains/etc.
*   X-Plane matches virtual paths, not real file paths, when looking up files in the library. This means you can name your customized artwork anything you want---you only need to match virtual paths.
*   Because you can map any virtual path to any file path in your library.txt file, you can use one object or terrain for many virtual path entries without having to duplicate the actual files.
*   You can also map one virtual path to multiple file paths. For objects, X-Plane will randomly pick one of the objects to provide more variation.

### Customizing Objects ###

You can customize the objects used in the global scenery. The virtual paths for the objects in the global scenery are named by their size and the terrain they are used with. The size can be thought of as a maximum limit as you make objects; this is how much space the scenery-creation tools reserved for each object.

There are two separate sets of objects for the global scenery---one for the US and one for the rest of the world. Because the US has much higher road density, the US objects are all relatively small. World objects are much larger.

The global scenery objects are _not_ meant to be one building each. For example, a 200x30 meter object in the US can be 7 houses next to each other, a shopping mall with a parking lot, or one large building.

### Customizing Terrain Textures ###

You can customize the terrain textures used in X-Plane. This is done by providing new "terrain info" (`.ter`) files. If you do not provide `.ter` files, X-Plane will not use your textures. X-Plane loads your .ter files and from there will pick a PNG in your package. If you override a `.ter` file, you must provide _all_ of the textures for that `.ter` file in your package.

Night lighting is _not_ automatic for scenery; you must provide a TEXTURE\_LIT line in your .ter file to enable night lighting. The lit texture may have any name; we recommend using the \_LIT suffix for clarity, but any name will work.

Someday X-Plane may feature seasonal textures; when this happens, the TEXTURE\_LIT command will allow one .ter with multiple daytime seasonal textures to share a single night lighting map.

### Customizing Roads ###

X-Plane uses exactly one road definition (a `.net` file) for all roads in a given DSF. One `.net` file may define many road types, so subtypes within the road file are used to create variety. This means you must include all of the textures and objects for your entire set of roads.

### Customizing Beaches ###

All default beaches in X-Plane exist in one image file and are referenced via one `.bch` file. So you must provide all beach definitions if you are going to provide any. Only one beach definition (a `.bch` file) may be used per DSF. Subtypes within the beach are used to create variety.


###### links and images ######

[1]:	#gettingstarted
[2]:	http://pdfcrowd.com/
[3]:	http://en.wikipedia.org/wiki/Orthophoto "The Orthophoto article on Wikipedia"
[4]:	http://forums.x-plane.org
[5]:	http://forums.x-plane.org/index.php?/forums/forum/7-scenery-development-forum/
[6]:	https://gateway.x-plane.com
[7]:	http://developer.x-plane.com/docs/scenery/
[8]:	https://www.youtube.com/playlist?list=PLMlRNmHeyivDv8OLFArflWLt_ZQJWD1oU
[9]:	https://gateway.x-plane.com/bugs
[10]:	http://developer.x-plane.com/tools/ "Download MeshTool at the X-Plane Developer site"
[11]:	http://developer.x-plane.com/ "X-Plane Developer"
[12]:	http://www.x-plane.com/files/manuals/Plane_Maker_manual.pdf "Download the Plane Maker Manual (PDF)"
[13]:	http://developer.x-plane.com/tools/worldeditor/ "WorldEditor Download page"
[14]:	#choosefolder
[15]:	#appendix:anatomyofthex-planescenerysystem "Jump to Anatomy of the X-Plane Scenery System"
[16]:	#exportingthescenery
[17]:	#synchronizingwithothereditors
[18]:	#wedwindow
[19]:	#packlist
[20]:	#packlist
[21]:	#wedwindow
[22]:	#wedwindow
[23]:	#figtoolbar
[24]:	#figtoolbar
[25]:	#wedwindow
[26]:	#thelibrarypane
[27]:	#addingobjectsandauto-generatingscenes
[28]:	#addingfacades
[29]:	#editingusingthemaptools
[30]:	#wedwindow
[31]:	#thelibrarypane
[32]:	#wedwindow
[33]:	#appendix:aboutthex-planelibrarysystem "Jump to the appendix "
[34]:	#wedwindow
[35]:	#wedwindow
[36]:	#hierarchy
[37]:	#hierarchy
[38]:	#theeditingtabsandtheattributespane
[39]:	#wedwindow
[40]:	#editing
[41]:	#editing
[42]:	http://en.wikipedia.org/wiki/Orthophoto "The Orthophoto article on Wikipedia"
[43]:	https://www.youtube.com/playlist?list=PLMlRNmHeyivDv8OLFArflWLt_ZQJWD1oU
[44]:	#usingtheworldeditorinterface
[45]:	#downloadingandinstallingworldeditor
[46]:	http://www.airnav.com/airports/ "The Airnav (Airport Navigation) Database"
[47]:	#currentapt
[48]:	http://www.realscenery.com/
[49]:	http://viewer.nationalmap.gov/viewer/ "The USGS Seamless Server, source of high-quality, public domain orthophotos for the United States"
[50]:	http://viewer.nationalmap.gov/viewer/ "The USGS Seamless Server, source of high-quality, public domain orthophotos for the United States"
[51]:	#figtoolbar
[52]:	http://www.airnav.com/airports/ "The Airnav (Airport Navigation) Database"
[53]:	#theeditingtabsandtheattributespane
[54]:	http://en.wikipedia.org/wiki/Runway#Runway_markings
[55]:	http://en.wikipedia.org/wiki/Jet_blast
[56]:	http://en.wikipedia.org/wiki/Approach_Lighting_System
[57]:	#thehierarchypane
[58]:	#taxisolid
[59]:	#bezierpathtools
[60]:	#appendix:definingtaxisigns
[61]:	#creatingrunwaysigns
[62]:	#exboundary
[63]:	#bezierpathtools
[64]:	#bezierpathtools
[65]:	https://www.youtube.com/playlist?list=PLMlRNmHeyivDK6at6tOG4jx8HhSlNfJCx
[66]:	https://www.youtube.com/playlist?list=PLMlRNmHeyivDExXiX09nq9toVVplBVxtD
[67]:	http://gateway.x-plane.com/
[68]:	http://gateway.x-plane.com/
[69]:	#openclosed
[70]:	#crossing
[71]:	#components
[72]:	#diffconfigs
[73]:	#conversion_chart
[74]:	#holetool
[75]:	http://gateway.x-plane.com/
[76]:	http://gateway.x-plane.com/register
[77]:	http://gateway.x-plane.com/bugs
[78]:	http://data.x-plane.com/file_specs/XP%20APT1000%20Spec.pdf
[79]:	http://www.x-plane.com/manuals/desktop/#expandingx-plane "Download the X-Plane 10 Desktop Manual"
[80]:	http://developer.x-plane.com/?article=dsf-file-format-specification "The DSF File Format Specification on the X-Plane Developer Site"
[81]:	http://developer.x-plane.com/?article=dsf-usage-in-x-plane "DSF Usage by X-Plane on the X-Plane Developer Site"
[82]:	http://developer.x-plane.com/?article=obj8-file-format-specification
[83]:	http://developer.x-plane.com/?article=dsf-usage-in-x-plane "DSF Usage by X-Plane on the X-Plane Developer Site"
[84]:	http://developer.x-plane.com/?article=vector-network-net-file-format-specification
[85]:	http://developer.x-plane.com/?article=airport-customization
[86]:	http://developer.x-plane.com/?article=library-library-txt-file-format-specification
[87]:	http://wiki.x-plane.com/Tutorial:_Customizing_X-Plane%27s_Default_Scenery_Artwork
[88]:	http://developer.x-plane.com/?article=advanced-airport-creation
[89]:	http://ww1.jeppesen.com/documents/aviation/business/ifr-paper-services/airport-signs.pdf
[90]:	#directionalpointtools

[image-24]:	http://www.x-plane.com/images/WED/intro/hierarchy.jpg
[image-28]:	http://www.x-plane.com/images/WED/tutorial/example_boundary.jpg
[image-29]:	http://developer.x-plane.com/wp-content/uploads/2014/12/rotate_obj.jpg
[image-30]:	http://www.x-plane.com/images/WED/bezier/conversion_chart.png
