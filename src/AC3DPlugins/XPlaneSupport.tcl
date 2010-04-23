# Copyright (c) 2007, Laminar Research.
#
# Permission is hereby granted, free of charge, to any person obtaining a 
# copy of this software and associated documentation files (the "Software"), 
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
# THE SOFTWARE.



#Import combo-box from AC3D for datarefs
catch {namespace import combobox::*}

#These constants cannot be changed - they match mirror constants inside the C plugin; changing them will probably cause hard crashes.
# Note: the key-frame RFC doesn't post a max number of keyframes, but more than 50 seems like a lot.
# Note: the sub-panel RFC does postulate a max-region count of 4!
set MAX_KEYFRAMES 50
set MAX_SEL 5
set SUBPANEL_DIM 4

#These turn on editing features that are based on posted or in-progress RFCs -- X-Plane doesn't support these yet!
set USE_KEYFRAMES 1
set USE_PANEL_EDIT 1

set IPHONE 0


##########################################################################################################################################################
# UTILS
##########################################################################################################################################################

proc xplane_dir_eval { cmd_str mask_str } {
	set dir [get_exportfolder "directory"]
	if {[string length $dir] > 0} {
		foreach filename [lsort [glob -dir $dir $mask_str]] {
			eval $cmd_str
		}
	}
}

proc make_labeled_entry { path name var width } {
	set varl [join [list $path "." $var ".l"] "" ]
	set vare [join [list $path "." $var ".e"] "" ]
	set varc [join [list $path "." $var ] "" ]
	frame $varc
	label $varl -text "$name"
	entry $vare -textvariable $var -width $width
	pack $varl $vare -side left -anchor nw
	pack $varc -side top -anchor nw
}

proc make_labeled_entry_pair { path name1 var1 name2 var2 } {
	set varl1 [join [list $path "." $var1 ".l1"] "" ]
	set varl2 [join [list $path "." $var1 ".l2"] "" ]
	set vare1 [join [list $path "." $var1 ".e1"] "" ]
	set vare2 [join [list $path "." $var1 ".e2"] "" ]
	set varc [join [list $path "." $var1 ] "" ]
	frame $varc
	label $varl1 -text "$name1"
	label $varl2 -text "$name2"
	entry $vare1 -textvariable $var1 -width 7
	entry $vare2 -textvariable $var2 -width 7
	pack $varl1 $vare1 $varl2 $vare2 -side left -anchor nw
	pack $varc -side top -anchor nw
}

proc packtext { w t } {
	pack $w
	$w.l configure -text $t
}

##########################################################################################################################################################
# BULK CONVERSION
##########################################################################################################################################################

proc xplane_convert_dir {} {
#	xplane_dir_eval "ac3d clear_all;ac3d load_ac \$filename;set filename \[string replace \$filename \[string last . \$filename\] end \".obj\"\];ac3d exporter_write_file OBJ8Save \$filename" "*.ac"
	xplane_dir_eval {ac3d clear_all;ac3d load_ac $filename;set filename [string replace $filename [string last . $filename] end ".obj"];ac3d exporter_write_file OBJ8Save $filename} "*.ac"
}

proc xplane_update_dir {} {
#	xplane_dir_eval "ac3d clear_all;ac3d load_ac \$filename;set filename \[string replace \$filename \[string last . \$filename\] end \".obj\"\];ac3d exporter_write_file OBJ8Save \$filename" "*.ac"
#	xplane_dir_eval {ac3d clear_all;ac3d load_ac $filename;set filename [string replace $filename [string last . $filename] end ".obj"];ac3d exporter_write_file OBJ8Save $filename} "*.ac"

	set strarg "ac3d clear_all;
					ac3d load_ac \$filename;
					ac3d xplane_update_selection;
					ac3d save_ac \$filename"
	xplane_dir_eval $strarg "*.ac"


}


##########################################################################################################################################################
# TEXTURE COORDINATE REMAPPING!
##########################################################################################################################################################

ac3d add_pref window_geom_xplane_rescale_dialog ""
ac3d add_pref window_geom_xplane_seltex_dialog ""

proc xplane_tex_select_dialog {} {
	global stex_s1
	global stex_t1
	global stex_s1
	global stex_t2

	if ![winfo exists .xp_seltex] {
		set stex_s1 0
		set stex_t1 0
		set stex_s2 1
		set stex_t2 1

		new_toplevel_tracked .xp_seltex "Rescale texture coordinates" prefs_window_geom_xplane_seltex_dialog
		label	.xp_seltex.s1_stex_label -text "Select Left:"
		spinbox .xp_seltex.s1_stex_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable stex_s1 -width 15
		label	.xp_seltex.t1_stex_label -text "Select Bottom:"
		spinbox .xp_seltex.t1_stex_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable stex_t1 -width 15
		label	.xp_seltex.s2_stex_label -text "Select Right:"
		spinbox .xp_seltex.s2_stex_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable stex_s2 -width 15
		label	.xp_seltex.t2_stex_label -text "Select Top:"
		spinbox .xp_seltex.t2_stex_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable stex_t2 -width 15
		frame	.xp_seltex.buttons
		button	.xp_seltex.buttons.apply -text "Remap Selected" -command {
			set strarg "\"$stex_s1 $stex_t1 $stex_s2 $stex_t2 \""
			eval ac3d xplane_select_tex $strarg
			ac3d redraw_all
		}
		pack	.xp_seltex.buttons.apply -side left

		grid 	.xp_seltex.s1_stex_label .xp_seltex.s1_stex_spinbox -sticky news
		grid 	.xp_seltex.t1_stex_label .xp_seltex.t1_stex_spinbox -sticky news
		grid 	.xp_seltex.s2_stex_label .xp_seltex.s2_stex_spinbox -sticky news
		grid 	.xp_seltex.t2_stex_label .xp_seltex.t2_stex_spinbox -sticky news
		grid 	.xp_seltex.buttons -columnspan 4 -sticky ns

		grid	columnconfigure .xp_seltex { 1 3 } -weight 1 -minsize 40

	}

	wm deiconify .xp_seltex
	raise        .xp_seltex
}








proc xplane_tex_rescale_dialog {} {
	global old_s1
	global old_t1
	global old_s1
	global old_t2
	global new_s1
	global new_t1
	global new_s1
	global new_t2

	if ![winfo exists .xp_rescale] {
		set old_s1 0
		set old_t1 0
		set old_s2 1
		set old_t2 1

		set new_s1 0
		set new_t1 0
		set new_s2 1
		set new_t2 1

		new_toplevel_tracked .xp_rescale "Rescale texture coordinates" prefs_window_geom_xplane_rescale_dialog
		label	.xp_rescale.s1_old_label -text "Old Left:"
		spinbox .xp_rescale.s1_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_s1 -width 15
		label	.xp_rescale.s1_new_label -text "New Left:"
		spinbox .xp_rescale.s1_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_s1 -width 15

		label	.xp_rescale.t1_old_label -text "Old Bottom:"
		spinbox .xp_rescale.t1_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_t1 -width 15
		label	.xp_rescale.t1_new_label -text "New Bottom:"
		spinbox .xp_rescale.t1_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_t1 -width 15

		label	.xp_rescale.s2_old_label -text "Old Right:"
		spinbox .xp_rescale.s2_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_s2 -width 15
		label	.xp_rescale.s2_new_label -text "New Right:"
		spinbox .xp_rescale.s2_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_s2 -width 15

		label	.xp_rescale.t2_old_label -text "Old Top:"
		spinbox .xp_rescale.t2_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_t2 -width 15
		label	.xp_rescale.t2_new_label -text "New Top:"
		spinbox .xp_rescale.t2_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_t2 -width 15

		frame	.xp_rescale.buttons
		button	.xp_rescale.buttons.apply -text "Remap Selected" -command {
			set strarg "\"$old_s1 $old_t1 $old_s2 $old_t2 $new_s1 $new_t1 $new_s2 $new_t2 \""
			eval ac3d xplane_rescale_tex $strarg
			ac3d redraw_all
		}
		pack	.xp_rescale.buttons.apply -side left

		button	.xp_rescale.buttons.dir -text "Remap Directory" -command {
			set strarg "ac3d clear_all;
							ac3d load_ac \$filename;
							ac3d xplane_rescale_tex \"$old_s1 $old_t1 $old_s2 $old_t2 $new_s1 $new_t1 $new_s2 $new_t2\";
							ac3d save_ac \$filename"
			xplane_dir_eval $strarg "*.ac"
			ac3d redraw_all
		}
		pack	.xp_rescale.buttons.dir -side left
		
		grid 	.xp_rescale.s1_old_label .xp_rescale.s1_old_spinbox .xp_rescale.s1_new_label .xp_rescale.s1_new_spinbox -sticky news
		grid 	.xp_rescale.t1_old_label .xp_rescale.t1_old_spinbox .xp_rescale.t1_new_label .xp_rescale.t1_new_spinbox -sticky news
		grid 	.xp_rescale.s2_old_label .xp_rescale.s2_old_spinbox .xp_rescale.s2_new_label .xp_rescale.s2_new_spinbox -sticky news
		grid 	.xp_rescale.t2_old_label .xp_rescale.t2_old_spinbox .xp_rescale.t2_new_label .xp_rescale.t2_new_spinbox -sticky news
		grid 	.xp_rescale.buttons -columnspan 4 -sticky ns

		grid	columnconfigure .xp_rescale { 1 3 } -weight 1 -minsize 40

	}

	wm deiconify .xp_rescale
	raise        .xp_rescale
}


##########################################################################################################################################################
# TEXTURE COORDINATE REMAPPING!
##########################################################################################################################################################

ac3d add_pref window_geom_xplane_rescale_keyframe ""

proc xplane_keyframe_rescale_dialog {} {
	global lo
	global hi

	if ![winfo exists .xp_rescale_keyframe] {
		set old_lo 0
		set old_hi 1
		set new_lo 1
		set new_hi 0

		new_toplevel_tracked .xp_rescale_keyframe "Rescale texture coordinates" prefs_window_geom_xplane_rescale_keyframe
		label	.xp_rescale_keyframe.lo_old_label -text "Old Low Value:"
		spinbox .xp_rescale_keyframe.lo_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_lo -width 15
		label	.xp_rescale_keyframe.lo_new_label -text "New Low Value:"
		spinbox .xp_rescale_keyframe.lo_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_lo -width 15
		label	.xp_rescale_keyframe.hi_old_label -text "Old High Value:"
		spinbox .xp_rescale_keyframe.hi_old_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable old_hi -width 15
		label	.xp_rescale_keyframe.hi_new_label -text "New High Value:"
		spinbox .xp_rescale_keyframe.hi_new_spinbox -from 0.0 -increment 0.125 -to 1.0 -textvariable new_hi -width 15

		button	.xp_rescale_keyframe.apply -text "Remap Selected" -command {
			ac3d xplane_rescale_keyframe $old_lo $new_lo $old_hi $new_hi
			ac3d redraw_all
		}
		pack	.xp_rescale_keyframe.apply -side left
		
		grid 	.xp_rescale_keyframe.lo_old_label .xp_rescale_keyframe.lo_old_spinbox -sticky news
		grid 	.xp_rescale_keyframe.lo_new_label .xp_rescale_keyframe.lo_new_spinbox -sticky news
		grid 	.xp_rescale_keyframe.hi_old_label .xp_rescale_keyframe.hi_old_spinbox -sticky news
		grid 	.xp_rescale_keyframe.hi_new_label .xp_rescale_keyframe.hi_new_spinbox -sticky news
		grid 	.xp_rescale_keyframe.apply -columnspan 2 -sticky ns

		grid	columnconfigure .xp_rescale_keyframe { 1 3 } -weight 1 -minsize 40

	}

	wm deiconify .xp_rescale_keyframe
	raise        .xp_rescale_keyframe
}

##########################################################################################################################################################
# PREFS DIALOG
##########################################################################################################################################################

ac3d add_pref window_geom_xplane_prefs_dialog ""

proc xplane_prefs_dialog {} {

	global xplane_layer_group_options
#	global prefs_xplane_default_layer_group
	
	if ![winfo exists .xp_prefs] {

#		if { [set prefs_xplane_default_layer_group] == "NULL" } {
#			set prefs_xplane_default_layer_group "none"
#		}

		new_toplevel_tracked .xp_prefs "X-Plane export prefs" prefs_window_geom_xplane_prefs_dialog

		label		.xp_prefs.layer_btn_label -text "Default Layer Group:"
		menubutton .xp_prefs.layer_btn -menu .xp_prefs.layer_btn.menu -direction flush -textvariable prefs_xplane_default_layer_group -padx 30 -pady 5
		menu .xp_prefs.layer_btn.menu
		foreach item $xplane_layer_group_options {
			.xp_prefs.layer_btn.menu add radiobutton -label $item -variable prefs_xplane_default_layer_group
		}

		label		.xp_prefs.layer_offset_label -text "Default Offset:"
		spinbox		.xp_prefs.layer_offset -from -5 -increment 1 -to 5 -textvariable prefs_xplane_default_layer_offset
		
#		checkbutton .xp_prefs.apt_lights		-variable prefs_xplane_export_airport_lights -text "Export as airport light"
		checkbutton .xp_prefs.triangles			-variable prefs_xplane_export_triangles	  -text "Export Geometry"
		
		label	.xp_prefs.default_lod_label -text "Default LOD:"
		spinbox .xp_prefs.default_lod_value -from 0 -increment 100 -to 1000 -textvariable prefs_xplane_default_LOD

		label	.xp_prefs.export_prefix_label -text "Bulk Export Prefix:"
		entry	.xp_prefs.export_prefix -textvariable prefs_xplane_export_prefix

		label	.xp_prefs.texture_prefix_label -text "Texture Export Prefix:"
		entry	.xp_prefs.texture_prefix -textvariable prefs_xplane_texture_prefix


		grid	.xp_prefs.layer_btn_label .xp_prefs.layer_btn
		grid	.xp_prefs.layer_offset_label .xp_prefs.layer_offset

		grid	x .xp_prefs.triangles -sticky news
		grid	.xp_prefs.default_lod_label	.xp_prefs.default_lod_value -sticky news
		grid	.xp_prefs.export_prefix_label .xp_prefs.export_prefix
		grid	.xp_prefs.texture_prefix_label .xp_prefs.texture_prefix

	}

	wm deiconify			.xp_prefs
	raise			        .xp_prefs
}


##########################################################################################################################################################
# OBJECT INSPECTOR
##########################################################################################################################################################

proc update_listbox_sel { lb tv } {
	global $tv
	set temp [$lb curselection]
	if [llength $temp] {
		set $tv [$lb get [lindex $temp 0]]
	}
}

proc refilter_listbox_dref { lb tv } {
	global all_datarefs
	global $tv
	set now [set $tv]
	set drefs [list none]
	$lb list delete 0 end

	if {$now == "none"} {
		set now ""
	}
	
	if [string length $now] {
		if { [string first "*" $now] == -1} {
			set now "*$now*"
		}
	} else {
		set now "*"
	}
	
	set now [string map { [ \\[ ] \\] } $now]

	set drefs [lsearch -all -inline $all_datarefs $now]
	$lb list insert end "none"
	foreach x $drefs {
		$lb list insert end $x
	}
}

proc refilter_listbox_cmnd { lb tv } {
	global all_cmnds
	global $tv
	set now [set $tv]
	set cmnds [list none]
	$lb list delete 0 end

	if {$now == "none"} {
		set now ""
	}
	
	if [string length $now] {
		if { [string first "*" $now] == -1} {
			set now "*$now*"
		}
	} else {
		set now "*"
	}
	
	set now [string map { [ \\[ ] \\] } $now]

	set cmnds [lsearch -all -inline $all_cmnds $now]
	$lb list insert end "none"
	foreach x $cmnds {
		$lb list insert end $x
	}
}


proc build_listbox_dref { listbox scrollbar textvar } {

	namespace import combobox::*
	combobox $listbox -editable true -textvariable $textvar -width 50 -opencommand "refilter_listbox_dref $listbox $textvar"
	pack $listbox

}

proc build_listbox_cmnd { listbox scrollbar textvar } {

	namespace import combobox::*
	combobox $listbox -editable true -textvariable $textvar -width 50 -opencommand "refilter_listbox_cmnd $listbox $textvar"
	pack $listbox

}


proc fetch_all_datarefs {} {
	global all_datarefs
		
	if {[catch {
		set fi [open "plugins/DataRefs.txt" r]
	}]} { 
		puts "Unable to open datarefs.txt"
		set all_datarefs ""	
		return 
	}

	gets $fi line	
	while { [gets $fi line] >= 0 } {

		set line [split $line "\t \""] 
	
		if [llength $line] {
		   set dref_fullname [lindex $line 0]
		   set dref_dtype [lindex $line 1]
		   set dref_dtype_l [split $dref_dtype "\[\]"]
		   set array_count -1
		   if { [llength $dref_dtype_l] > 1 } {
				set array_count [lindex $dref_dtype_l 1]
		   }
			
		   if { $array_count > 0 && $array_count <= 100 } {
				for {set x 0} {$x<$array_count} {incr x} {
					lappend all_datarefs "$dref_fullname\[$x\]"
				}
		   } else {
				lappend all_datarefs "$dref_fullname"
		   }
		   
		   lappend cats [string range $dref_fullname 0 [string last "/" $dref_fullname] ]
		}
	}
	
	set cats [lsort -unique $cats]
	foreach x $cats {
		set all_datarefs [linsert $all_datarefs 0 $x]
	}
	
	close $fi   
}


proc fetch_all_cmnds {} {
	global all_cmnds
		
	if {[catch {
		set fi [open "plugins/Commands.txt" r]
	}]} { 
		puts "Unable to open commands.txt\n"
		set all_cmnds ""	
		return 
	}

	gets $fi line	
	while { [gets $fi line] >= 0 } {
		if [llength $line] {
		   set cmnd_fullname [lindex $line 0]
			lappend all_cmnds "$cmnd_fullname"
		   
		   lappend cats [string range $cmnd_fullname 0 [string last "/" $cmnd_fullname] ]
		}
	}
	
	set cats [lsort -unique $cats]
	foreach x $cats {
		set all_cmnds [linsert $all_cmnds 0 $x]
	}
	
	close $fi   
}



proc xplane_inspector_sync {} {
	global MAX_SEL
	if ![winfo exists .xp_view] return
	
	ac3d xplane_editor_sync		
	
	set sel_count [ac3d xplane_get_sel_count]

	for {set x 0} {$x<$MAX_SEL} {incr x} {
		pack forget .xp_view.v$x
	}
	
	for {set x 0} {$x<$sel_count} {incr x} {
		set sel_type [ac3d xplane_get_sel_type $x]
		set anim_type [ac3d xplane_can_animate $x]
		
		set container .xp_view.v$x
		
		pack $container -side left -anchor nw

		pack forget $container.none
		pack forget $container.light
		pack forget $container.obj
		pack forget $container.grp
		pack forget $container.multi
		

		if {$sel_type == 0} { pack $container.none }
		if {$sel_type == 1} { 
			pack $container.light 
			global xplane_obj_name$x
#			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]			
			xplane_light_sync $x $container
		}
		if {$sel_type == 2} { 
			pack $container.obj 
			global xplane_obj_name$x
#			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]
#			pack forget $container.obj.anim_type_btn
#			if {$anim_type == 1} { pack $container.obj.anim_type_btn -anchor nw }
			xplane_obj_sync $x $container
		}
		if {$sel_type == 3} { 
			pack $container.grp 
			global xplane_obj_name$x
#			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]
		}
		if {$sel_type == 4} { pack $container.multi }
	}
}

proc xplane_light_sync { x container } {
	global xplane_light_type$x
	pack forget $container.light.rgb
	pack forget $container.light.dataref
	pack forget $container.light.smoke_black
	pack forget $container.light.smoke_white
	
	if { [set xplane_light_type$x] == "rgb"}		 { pack $container.light.rgb }
	if { [set xplane_light_type$x] == "custom"}		 { pack $container.light.dataref }
	if { [set xplane_light_type$x] == "black smoke"} { pack $container.light.smoke_black }
	if { [set xplane_light_type$x] == "white smoke"} { pack $container.light.smoke_white }
}

proc xplane_light_sync_all {} {
	global MAX_SEL
	for {set idx 0} {$idx < $MAX_SEL} {incr idx} {
		xplane_light_sync $idx .xp_view.v$idx
	}
}


proc xplane_obj_sync { idx container } {
	global xplane_manip_types

	global xplane_anim_type$idx
	global xplane_blend_enable$idx
	global xplane_hard_surf$idx
	global xplane_anim_keyframe_count$idx
	global xplane_mod_lit$idx
	global xplane_manip_type$idx
	
	global MAX_KEYFRAMES
	pack forget $container.obj.none
	pack forget $container.obj.trans
	pack forget $container.obj.rotate
	pack forget $container.obj.static
	pack forget $container.obj.show
	pack forget $container.obj.hide
	pack forget $container.obj.dref_list
	pack forget $container.obj.none.manip.xplane_manip_dx$idx
	pack forget $container.obj.none.manip.xplane_manip_dy$idx
	pack forget $container.obj.none.manip.xplane_manip_dz$idx
	pack forget $container.obj.none.manip.guess$idx
	pack forget $container.obj.none.manip.xplane_manip_v1_min$idx
	pack forget $container.obj.none.manip.xplane_manip_v1_max$idx
	pack forget $container.obj.none.manip.xplane_manip_v2_min$idx
	pack forget $container.obj.none.manip.xplane_manip_v2_max$idx
	pack forget $container.obj.none.manip.dref1
	pack forget $container.obj.none.manip.dref2
	pack forget $container.obj.none.manip.cmnd1
	pack forget $container.obj.none.manip.cmnd2
	pack forget $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
	pack forget $container.obj.none.manip.xplane_manip_tooltip$idx
	
	if { [set xplane_anim_type$idx] == "no animation"} { pack $container.obj.none }
	if { [set xplane_anim_type$idx] == "rotate"} { 
		pack $container.obj.rotate
		for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
			pack forget $container.obj.rotate.xplane_anim_value$x$idx
		}
		for {set x 0} {$x< [set xplane_anim_keyframe_count$idx] } {incr x} {
			pack $container.obj.rotate.xplane_anim_value$x$idx
		}
		pack $container.obj.dref_list
	}
	if { [set xplane_anim_type$idx] == "translate"} { 
		pack $container.obj.trans
		for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
			pack forget $container.obj.trans.xplane_anim_value$x$idx
		}
		for {set x 0} {$x< [set xplane_anim_keyframe_count$idx]} {incr x} {
			pack $container.obj.trans.xplane_anim_value$x$idx
		}
		pack $container.obj.dref_list		
	}
	if { [set xplane_anim_type$idx] == "static"} { 
		pack $container.obj.static 
	}
	if { [set xplane_anim_type$idx] == "show"  } { 
		pack $container.obj.show 
		pack $container.obj.dref_list
	}
	if { [set xplane_anim_type$idx] == "hide"  } { 
		pack $container.obj.hide 
		pack $container.obj.dref_list
	}
	
	pack forget $container.obj.none.blend_level	
	if { [set xplane_blend_enable$idx] == 0} { pack $container.obj.none.blend_level -after $container.obj.none.blend_enable }
	pack forget $container.obj.none.is_deck
	if { [set xplane_hard_surf$idx] != "none"} { pack $container.obj.none.is_deck -after $container.obj.none.hard_surf_btn }
	pack forget $container.obj.none.dref_list
	pack forget $container.obj.none.xplane_lit_v1$idx
	pack forget $container.obj.none.xplane_lit_v2$idx

	if { [set xplane_mod_lit$idx] != 0} { 
		pack $container.obj.none.dref_list -after $container.obj.none.mod_lit 
		pack $container.obj.none.xplane_lit_v2$idx -after $container.obj.none.mod_lit 
		pack $container.obj.none.xplane_lit_v1$idx -after $container.obj.none.mod_lit 
	}

	$container.obj.none.manip.type_btn configure -text [lindex $xplane_manip_types [set xplane_manip_type$idx]]

	# axis
	if { [set xplane_manip_type$idx] == 2} {
		packtext $container.obj.none.manip.xplane_manip_dx$idx "Axis (X Component)"
		packtext $container.obj.none.manip.xplane_manip_dy$idx "Axis (Y Component)"
		pack $container.obj.none.manip.xplane_manip_dz$idx $container.obj.none.manip.guess$idx		
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Min"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Max"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# axis_2d
	if { [set xplane_manip_type$idx] == 3} {
		packtext $container.obj.none.manip.xplane_manip_dx$idx "X Axis Length"
		packtext $container.obj.none.manip.xplane_manip_dy$idx "Y Axis Length"
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Min X"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Max X"
		packtext $container.obj.none.manip.xplane_manip_v2_min$idx "Min Y"
		packtext $container.obj.none.manip.xplane_manip_v2_max$idx "Max Y"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.dref2
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# command
	if { [set xplane_manip_type$idx] == 4} {
		pack $container.obj.none.manip.cmnd1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# command-axis
	if { [set xplane_manip_type$idx] == 5} {
		packtext $container.obj.none.manip.xplane_manip_dx$idx "Axis (X Component)"
		packtext $container.obj.none.manip.xplane_manip_dy$idx "Axis (Y Component)"
		pack $container.obj.none.manip.xplane_manip_dz$idx $container.obj.none.manip.guess$idx
		pack $container.obj.none.manip.cmnd1
		pack $container.obj.none.manip.cmnd2
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	#no-op is 6


	# dref-push
	if { [set xplane_manip_type$idx] == 7} {
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Up"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Down"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# dref-radio
	if { [set xplane_manip_type$idx] == 8} {
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Down"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# dref-toggle
	if { [set xplane_manip_type$idx] == 9} {
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Off"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "On"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# dref-delta
	if { [set xplane_manip_type$idx] == 10} {
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Click"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Hold"
		packtext $container.obj.none.manip.xplane_manip_v2_min$idx "Min"
		packtext $container.obj.none.manip.xplane_manip_v2_max$idx "Max"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}
	# dref-wrap
	if { [set xplane_manip_type$idx] == 11} {
		packtext $container.obj.none.manip.xplane_manip_v1_min$idx "Click"
		packtext $container.obj.none.manip.xplane_manip_v1_max$idx "Hold"
		packtext $container.obj.none.manip.xplane_manip_v2_min$idx "Min"
		packtext $container.obj.none.manip.xplane_manip_v2_max$idx "Max"
		pack $container.obj.none.manip.dref1
		pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					
		pack $container.obj.none.manip.xplane_manip_tooltip$idx
	}


}


proc xplane_obj_sync_all {} {
	global MAX_SEL
	for {set idx 0} {$idx < $MAX_SEL} {incr idx} {
		xplane_obj_sync $idx .xp_view.v$idx
	}
}

ac3d add_pref window_geom_xplane_inspector ""

proc xplane_inspector {} {
	global MAX_KEYFRAMES
	global MAX_SEL

	for {set idx 0} {$idx<$MAX_SEL} {incr idx} {
		for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
#			global xplane_anim_value$x$idx
#			global xplane_anim_angle$x$idx
		}
	}
	
	global xplane_cursor_options
	global xplane_hard_surface_options
	global xplane_light_options
	global xplane_layer_group_options
	global xplane_manip_types
	global USE_KEYFRAMES
	
	if ![winfo exists .xp_view] {

		new_toplevel_tracked .xp_view "X-Plane Properties" prefs_window_geom_xplane_inspector
		
		for {set idx 0} {$idx<$MAX_SEL} {incr idx} {

#			global xplane_anim_type$idx
#			global xplane_poly_os$idx
#			global xplane_blend_enable$idx
#			global xplane_blend_level$idx
#			global xplane_hard_surf$idx
#			global xplane_is_deck$idx
#			global xplane_mod_lit$idx
#			global xplane_wall$idx
#			global xplane_draw_disable$idx
#			global xplane_lit_dataref$idx
#
#			global xplane_light_type$idx
#			global xplane_light_red$idx
#			global xplane_light_green$idx
#			global xplane_light_blue$idx
#			global xplane_light_alpha$idx
#			global xplane_light_size$idx
#			global xplane_light_s1$idx
#			global xplane_light_s2$idx
#			global xplane_light_t1$idx
#			global xplane_light_t2$idx
#			global xplane_light_dataref$idx
		
			set container .xp_view.v$idx
			frame $container
			frame $container.none
			frame $container.light
			frame $container.obj
			frame $container.grp
			frame $container.multi
			
			#-------------------------------------- NONE --------------------------------------
			
			label $container.none.name -text "No selection"
			pack $container.none.name -side top -anchor nw

			#-------------------------------------- LIGHTS --------------------------------------

			label $container.light.name_label -text "Name:"
#			global xplane_obj_name$idx
			label $container.light.name -textvariable xplane_obj_name$idx
			pack $container.light.name_label $container.light.name -anchor nw

			menubutton $container.light.light_type_btn -menu $container.light.light_type_btn.menu -direction flush  -textvariable xplane_light_type$idx -padx 30 -pady 5
			menu $container.light.light_type_btn.menu
			foreach light $xplane_light_options {
				$container.light.light_type_btn.menu add radiobutton -label $light -variable xplane_light_type$idx -command xplane_light_sync_all
			}		
			pack $container.light.light_type_btn -anchor nw

			labelframe $container.light.rgb -text "RGB:"
				make_labeled_entry $container.light.rgb "Red:" xplane_light_red$idx 10
				make_labeled_entry $container.light.rgb "Green:" xplane_light_green$idx 10
				make_labeled_entry $container.light.rgb "Blue:" xplane_light_blue$idx 10
			pack $container.light.rgb

			labelframe $container.light.dataref -text "Dataref:"
				make_labeled_entry $container.light.dataref "Red:" xplane_light_red$idx 10
				make_labeled_entry $container.light.dataref "Green:" xplane_light_green$idx 10
				make_labeled_entry $container.light.dataref "Blue:" xplane_light_blue$idx 10
				make_labeled_entry $container.light.dataref "Alpha:" xplane_light_alpha$idx 10
				make_labeled_entry $container.light.dataref "Size:" xplane_light_size$idx 10
				make_labeled_entry $container.light.dataref "S1:" xplane_light_s1$idx 10
				make_labeled_entry $container.light.dataref "T1:" xplane_light_t1$idx 10
				make_labeled_entry $container.light.dataref "S2:" xplane_light_s2$idx 10 
				make_labeled_entry $container.light.dataref "T2:" xplane_light_t2$idx 10
				build_listbox_dref $container.light.dataref.dref_list $container.light.dataref.scroll xplane_light_dataref$idx				
				
			pack $container.light.dataref

			labelframe $container.light.smoke_black -text "Black Smoke Puff:"
				make_labeled_entry $container.light.smoke_black "Puff size:" xplane_light_smoke_size$idx 10
			pack $container.light.smoke_black

			labelframe $container.light.smoke_white -text "White Smoke Puff:"
				make_labeled_entry $container.light.smoke_white "Puff size:" xplane_light_smoke_size$idx 10
			pack $container.light.smoke_white

			#-------------------------------------- OBJECTS --------------------------------------
			
			label $container.obj.name_label -text "Name:"
#			global xplane_obj_name$idx
			entry $container.obj.name -textvariable xplane_obj_name$idx -width 20
			pack $container.obj.name_label $container.obj.name

			# Ben says: this would make a static label showing the animation type - not needed since the group that surrounds the animation type handles this.
#			label $container.obj.anim_type_btn -textvariable xplane_anim_type$idx -padx 30 -pady 5
			# This creates a popup letting us see the animation type.  We have this disabled because changing an animation's type will cause insanity.
#			menubutton $container.obj.anim_type_btn -menu $container.obj.anim_type_btn.menu -direction flush  -textvariable xplane_anim_type$idx -padx 30 -pady 5
#			menu $container.obj.anim_type_btn.menu
#			foreach anim_mode [list "no animation" "rotate" "translate" "static" "show" "hide" ] {
#				$container.obj.anim_type_btn.menu add radiobutton -label $anim_mode -variable xplane_anim_type$idx -command "xplane_obj_sync_all"
#			}		
#			pack $container.obj.anim_type_btn
			
			labelframe $container.obj.none -text "Object:"		
				label	$container.obj.none.poly_os_label -text "Polygon Offset:"
				spinbox $container.obj.none.poly_os_value -from 0 -increment 1 -to 5 -textvariable xplane_poly_os$idx
				pack	$container.obj.none.poly_os_label	$container.obj.none.poly_os_value
				label $container.obj.none.hard_surf_label -text "Surface:"
				menubutton $container.obj.none.hard_surf_btn -menu $container.obj.none.hard_surf_btn.menu -direction flush -textvariable xplane_hard_surf$idx -padx 30 -pady 5
				menu $container.obj.none.hard_surf_btn.menu
				foreach surf $xplane_hard_surface_options {
					$container.obj.none.hard_surf_btn.menu add radiobutton -label $surf -variable xplane_hard_surf$idx -command "xplane_obj_sync_all"
				}
				checkbutton $container.obj.none.is_deck -text "Deck" -variable xplane_is_deck$idx
				pack $container.obj.none.hard_surf_label $container.obj.none.hard_surf_btn 
				pack $container.obj.none.is_deck
				checkbutton $container.obj.none.use_materials -text "Use AC3D Materials" -variable xplane_use_materials$idx
				pack $container.obj.none.use_materials
				checkbutton $container.obj.none.blend_enable -text "Blending" -variable xplane_blend_enable$idx -command "xplane_obj_sync_all"
				pack $container.obj.none.blend_enable
				frame $container.obj.none.blend_level
					make_labeled_entry $container.obj.none.blend_level "blend cutoff" xplane_blend_level$idx 10
				pack $container.obj.none.blend_level
				
				checkbutton $container.obj.none.hard_wall -text "Wall" -variable xplane_wall$idx
				pack $container.obj.none.hard_wall
				checkbutton $container.obj.none.draw_disable -text "Disable Drawing" -variable xplane_draw_disable$idx
				pack $container.obj.none.draw_disable
				checkbutton $container.obj.none.mod_lit -text "Dynamic LIT" -variable xplane_mod_lit$idx -command "xplane_obj_sync_all"
				pack $container.obj.none.mod_lit
				build_listbox_dref $container.obj.none.dref_list $container.obj.none.scroll xplane_lit_dataref$idx
				make_labeled_entry $container.obj.none "v1" xplane_lit_v1$idx 10
				make_labeled_entry $container.obj.none "v2" xplane_lit_v2$idx 10				

				labelframe $container.obj.none.manip -text "Manipulators:"					

					label $container.obj.none.manip.type_label -text "Kind:"
					menubutton $container.obj.none.manip.type_btn -menu $container.obj.none.manip.type_btn.menu -direction flush -text "None" -padx 30 -pady 5
					menu $container.obj.none.manip.type_btn.menu
					for {set i 0} {$i< [llength $xplane_manip_types] } {incr i} {					
						$container.obj.none.manip.type_btn.menu add radiobutton -value $i -label [lindex $xplane_manip_types $i] -variable xplane_manip_type$idx -command "xplane_obj_sync_all"
					}
					pack $container.obj.none.manip.type_label $container.obj.none.manip.type_btn					
					

					make_labeled_entry $container.obj.none.manip "Dx:" xplane_manip_dx$idx 10
					make_labeled_entry $container.obj.none.manip "Dy:" xplane_manip_dy$idx 10
					make_labeled_entry $container.obj.none.manip "Axis (Z Component)" xplane_manip_dz$idx 10
					button $container.obj.none.manip.guess$idx -text "Guess" -command "ac3d xplane_guess_axis $idx"
					pack $container.obj.none.manip.guess$idx -side left -anchor nw

					make_labeled_entry $container.obj.none.manip "Min:" xplane_manip_v1_min$idx 10
					make_labeled_entry $container.obj.none.manip "Max:" xplane_manip_v1_max$idx 10
					make_labeled_entry $container.obj.none.manip "Min:" xplane_manip_v2_min$idx 10
					make_labeled_entry $container.obj.none.manip "Max:" xplane_manip_v2_max$idx 10

					label $container.obj.none.manip.cursor_label -text "Cursor:"
					menubutton $container.obj.none.manip.cursor_btn -menu $container.obj.none.manip.cursor_btn.menu -direction flush -textvariable xplane_manip_cursor$idx -padx 30 -pady 5
					menu $container.obj.none.manip.cursor_btn.menu
					foreach surf $xplane_cursor_options {
						$container.obj.none.manip.cursor_btn.menu add radiobutton -label $surf -variable xplane_manip_cursor$idx
					}
					pack $container.obj.none.manip.cursor_label $container.obj.none.manip.cursor_btn					

					build_listbox_dref $container.obj.none.manip.dref1 $container.obj.none.dref1_scroll xplane_manip_dref1$idx
					build_listbox_dref $container.obj.none.manip.dref2 $container.obj.none.dref2_scroll xplane_manip_dref2$idx
					build_listbox_cmnd $container.obj.none.manip.cmnd1 $container.obj.none.cmnd1_scroll xplane_manip_cmnd1$idx
					build_listbox_cmnd $container.obj.none.manip.cmnd2 $container.obj.none.cmnd2_scroll xplane_manip_cmnd2$idx
					make_labeled_entry $container.obj.none.manip "tooltip" xplane_manip_tooltip$idx 50
					
				pack $container.obj.none.manip
				
			pack $container.obj.none
			
			labelframe $container.obj.rotate -text "Rotation:"
				for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
					make_labeled_entry_pair $container.obj.rotate "value $x" xplane_anim_value$x$idx "angle $x" xplane_anim_angle$x$idx
					if {$USE_KEYFRAMES} {
						button $container.obj.rotate.xplane_anim_value$x$idx.delete -text "Delete" -command "ac3d xplane_delete_keyframe $x $idx"
						button $container.obj.rotate.xplane_anim_value$x$idx.add -text "Add" -command "ac3d xplane_add_keyframe $x $idx"
					}
					button $container.obj.rotate.xplane_anim_value$x$idx.go -text "Go" -command "ac3d xplane_set_anim_keyframe $x $idx"
					if {$USE_KEYFRAMES} {
						pack $container.obj.rotate.xplane_anim_value$x$idx.delete $container.obj.rotate.xplane_anim_value$x$idx.add $container.obj.rotate.xplane_anim_value$x$idx.go -side left -anchor nw
					} else {
						pack $container.obj.rotate.xplane_anim_value$x$idx.go -side left -anchor nw
					}
				}
				# This would make a dataref text field instead of popup menu
#				make_labeled_entry $container.obj.rotate "dataref" xplane_anim_dataref$idx 10
#				menubutton $container.obj.rotate.dref_btn -menu $container.obj.rotate.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.rotate.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.rotate.dref_btn
			pack $container.obj.rotate

			labelframe $container.obj.trans -text "Translation:"
				for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
					make_labeled_entry $container.obj.trans "value $x" xplane_anim_value$x$idx 10
					if {$USE_KEYFRAMES} {
						button $container.obj.trans.xplane_anim_value$x$idx.delete -text "Delete" -command "ac3d xplane_delete_keyframe $x $idx"
						button $container.obj.trans.xplane_anim_value$x$idx.add -text "Add" -command "ac3d xplane_add_keyframe $x $idx"
					}
					button $container.obj.trans.xplane_anim_value$x$idx.go -text "Go" -command "ac3d xplane_set_anim_keyframe $x $idx"
					if {$USE_KEYFRAMES} {						
						pack $container.obj.trans.xplane_anim_value$x$idx.delete $container.obj.trans.xplane_anim_value$x$idx.add $container.obj.trans.xplane_anim_value$x$idx.go -side left -anchor nw
					} else {
						pack $container.obj.trans.xplane_anim_value$x$idx.go -side left -anchor nw
					}
				}
				make_labeled_entry $container.obj.trans "anchor" xplane_anim_keyframe_root$idx 10
#				make_labeled_entry $container.obj.trans "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.trans.dref_btn -menu $container.obj.trans.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.trans.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.trans.dref_btn
			pack $container.obj.trans

			labelframe $container.obj.static -text "Static Translation:"
				make_labeled_entry $container.obj.static "low value" xplane_anim_value0$idx 10
				make_labeled_entry $container.obj.static "high value" xplane_anim_value1$idx 10
#				make_labeled_entry $container.obj.static "dataref" xplane_anim_dataref$idx
			pack $container.obj.static

			labelframe $container.obj.show -text "Show:"
				make_labeled_entry $container.obj.show "low value" xplane_anim_value0$idx 10
				make_labeled_entry $container.obj.show "high value" xplane_anim_value1$idx 10
#				make_labeled_entry $container.obj.show "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.show.dref_btn -menu $container.obj.show.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.show.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.show.dref_btn
			pack $container.obj.show

			labelframe $container.obj.hide -text "Hide:"
				make_labeled_entry $container.obj.hide "low value" xplane_anim_value0$idx 10
				make_labeled_entry $container.obj.hide "high value" xplane_anim_value1$idx 10
#				make_labeled_entry $container.obj.hide "dataref" xplane_anim_dataref$idx 10
#				menubutton $container.obj.hide.dref_btn -menu $container.obj.hide.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.hide.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.hide.dref_btn
			pack $container.obj.hide
		
			build_listbox_dref $container.obj.dref_list $container.obj.dref_scroll xplane_anim_dataref$idx

			#-------------------------------------- GROUP --------------------------------------

#			global xplane_obj_name$idx
			label $container.grp.name_label -text "Name:"
			label $container.grp.name -textvariable xplane_obj_name$idx
			grid $container.grp.name_label $container.grp.name -sticky nw

			labelframe $container.grp.lod -text "LOD:"
				make_labeled_entry $container.grp.lod "Near LOD:" xplane_lod_near$idx 10
				make_labeled_entry $container.grp.lod "Far LOD:" xplane_lod_far$idx 10
			grid $container.grp.lod -columnspan 2 -sticky nw

			labelframe $container.grp.layer_group -text "Layer Group:"

				menubutton $container.grp.layer_group.layer_btn -menu $container.grp.layer_group.layer_btn.menu -direction flush -textvariable xplane_layer_group$idx -padx 30 -pady 5
				menu $container.grp.layer_group.layer_btn.menu
				foreach item $xplane_layer_group_options {
					$container.grp.layer_group.layer_btn.menu add radiobutton -label $item -variable xplane_layer_group$idx
				}
				pack $container.grp.layer_group.layer_btn -anchor nw
				make_labeled_entry $container.grp.layer_group "Group Offset:" xplane_layer_group_offset$idx 10
			grid $container.grp.layer_group -columnspan 2 -sticky nw

			grid columnconfigure $container.grp 0 -weight 0
			grid columnconfigure $container.grp 1 -weight 1

			#-------------------------------------- MULTIPLE --------------------------------------

			label $container.multi.msg -text "Multiple objects selected of different types..."
			pack $container.multi.msg

			pack $container.none
			pack $container.light
			pack $container.obj
			pack $container.grp
			pack $container.multi

			pack $container
		}
		
		checkbutton .xp_view.multi_edit -variable xplane_multi_edit -text "Apply to All"
		pack .xp_view.multi_edit -side bottom -anchor sw
		xplane_inspector_sync
	}


	wm deiconify			.xp_view
	raise					.xp_view
}

proc xplane_inspector_update { name1 name2 op } {
	xplane_inspector_sync
}

if {$IPHONE} {
	set xplane_light_options [list none \
		rwy_ww rwy_wy rwy_yw rwy_yy \
		rwy_gr rwy_rg \
		rwy_xr rwy_rx \
		rwy_xw rwy_wx \
		rwy_papi_1 rwy_papi_2 rwy_papi_3 rwy_papi_4 \
		rwy_papi_rev_1 rwy_papi_rev_2 rwy_papi_rev_3 rwy_papi_rev_4 \
		airplane_landing airplane_nav_l airplane_nav_r airplane_nav_t airplane_strobe airplane_beacon ]

} else {
	set xplane_light_options [list none "black smoke" "white smoke" rgb custom \
		headlight taillight \
		airplane_landing airplane_taxi airplane_beacon airplane_nav_tail airplane_nav_left airplane_nav_right airplane_strobe \
		ship_nav_left ship_nav_right ship_mast_obs ship_mast_grn ship_nav_tail ship_mast_powered \
		carrier_datum carrier_waveoff carrier_meatball1 carrier_meatball2 carrier_meatball3	carrier_meatball4 carrier_meatball5	carrier_mast_strobe	carrier_deck_blue_s	carrier_deck_blue_w \
		carrier_deck_blue_n	carrier_deck_blue_e	carrier_pitch_lights carrier_foul_line_red carrier_foul_line_white carrier_center_white	carrier_edge_white carrier_thresh_white \
		frigate_SGSI_lo	frigate_SGSI_on	frigate_SGSI_hi	frigate_deck_green oilrig_deck_blue \
		town_light_60 town_light_90 town_light_150 town_light_180 town_light_220 town_light_280 0town_light_330 town_light_350 town_light_omni \
		town_tiny_light_60 town_tiny_light_90 town_tiny_light_150 town_tiny_light_180 town_tiny_light_220 town_tiny_light_280 town_tiny_light_330 town_tiny_light_350 town_tiny_light_omni \
		obs_strobe_day obs_strobe_night obs_red_day obs_red_night]
}
	
set xplane_hard_surface_options [list none object water concrete asphalt grass dirt gravel lakebed snow shoulder blastpad]
set xplane_layer_group_options [list none terrain beaches shoulders taxiways runways markings airports roads objects light_objects cars]
set xplane_cursor_options [list four_arrows hand button rotate_small rotate_small_left rotate_small_right rotate_medium rotate_medium_left rotate_medium_right rotate_large \
	rotate_large_left rotate_large_right up_down down up left_right right left  arrow]

set xplane_manip_types [list none panel axis axis_2d command command_axis no_op push radio toggle delta wrap]


trace add variable select_info write xplane_inspector_update
for {set idx 0} {$idx<$MAX_SEL} {incr idx} {
	trace add variable xplane_anim_keyframe_count$idx write xplane_inspector_update
}

##########################################################################################################################################################
# PANEL SUB-REGION SYSTEM
##########################################################################################################################################################

proc eval_panel_dialog {} {
	global SUBPANEL_DIM
	
	global prefs_xplane_enable_regions
	global prefs_xplane_region_count
	
	for {set x 0} {$x < $SUBPANEL_DIM} {incr x} {
		global prefs_xplane_region_l$x
		global prefs_xplane_region_r$x
		global prefs_xplane_region_b$x
		global prefs_xplane_region_t$x
	}
	if ![winfo exists .xp_panel] return
	
	pack forget .xp_panel.sub_label
	pack forget .xp_panel.sub_count
#	pack forget .xp_panel.go
	pack forget .xp_panel.notes
	for {set x 0} {$x < $SUBPANEL_DIM} {incr x} {
		pack forget .xp_panel.region$x
	}
	
	if {$prefs_xplane_enable_regions} {
		pack .xp_panel.sub_label
		pack .xp_panel.sub_count
		for {set x 0} {$x < $prefs_xplane_region_count} {incr x} {
			pack .xp_panel.region$x
		}			
#		pack .xp_panel.go		
		pack .xp_panel.notes
	}
}
proc sync_panel_dialog { name1 name2 op } {
	eval_panel_dialog
}

ac3d add_pref window_geom_xplane_panel_dialog ""

set init_panel_prefs 0

proc xplane_panel_dialog {} {
	global SUBPANEL_DIM
	global init_panel_prefs
	global prefs_xplane_region_count
	global prefs_xplane_enable_regions
	
	for {set x 0} {$x < $SUBPANEL_DIM} {incr x} {
		global prefs_xplane_region_l$x
		global prefs_xplane_region_r$x
		global prefs_xplane_region_b$x
		global prefs_xplane_region_t$x
	}

	if {$init_panel_prefs == 0 } {
		set init_panel_prefs 1
		for {set x 0} {$x < $SUBPANEL_DIM} {incr x} {
			trace add variable prefs_xplane_region_l$x write sync_panel_dialog
			trace add variable prefs_xplane_region_r$x write sync_panel_dialog
			trace add variable prefs_xplane_region_b$x write sync_panel_dialog
			trace add variable prefs_xplane_region_t$x write sync_panel_dialog
		}

		trace add variable prefs_xplane_enable_regions write sync_panel_dialog
		trace add variable prefs_xplane_region_count write sync_panel_dialog
	}


	
	if ![winfo exists .xp_panel] {

		new_toplevel_tracked .xp_panel "X-Plane Panel Properties" prefs_window_geom_xplane_panel_dialog
		
		checkbutton		.xp_panel.enable -variable prefs_xplane_enable_regions -text "Enable panel regions" -command "eval_panel_dialog"	
		pack			.xp_panel.enable
		label			.xp_panel.sub_label -text "Number of panel regions:"
		pack			.xp_panel.sub_label
		spinbox			.xp_panel.sub_count -from 1 -increment 1 -to $SUBPANEL_DIM -textvariable prefs_xplane_region_count -command "eval_panel_dialog"	
		pack			.xp_panel.sub_count
		for {set x 0} {$x < $SUBPANEL_DIM} {incr x} {
			labelframe .xp_panel.region$x -text "Region $x:"
		
			make_labeled_entry .xp_panel.region$x "Region $x left" prefs_xplane_region_l$x 10
			make_labeled_entry .xp_panel.region$x "Region $x bottom" prefs_xplane_region_b$x 10
			make_labeled_entry .xp_panel.region$x "Region $x right" prefs_xplane_region_r$x 10
			make_labeled_entry .xp_panel.region$x "Region $x top" prefs_xplane_region_t$x 10
			
			pack .xp_panel.region$x
		}
		
		label			.xp_panel.notes -text "Note: 0 0 is the lower left of the panel texture."
		pack			.xp_panel.notes

#		button	.xp_panel.go -command "ac3d xplane_make_subpanel" -text "Make Subpanel..."
#		pack	.xp_panel.go
	}

	wm deiconify			.xp_panel
	raise			        .xp_panel
	
	eval_panel_dialog
}



##########################################################################################################################################################
# ANIMATION BAR
##########################################################################################################################################################

proc clean_anim {} {
	global ANIM_INNER
	if [winfo exists .xp_anim] {
		set children [winfo children $ANIM_INNER]
		foreach c $children {
			destroy $c
		}
	}
}
proc kill_dataref { dref } {
	global ANIM_INNER
	destroy $ANIM_INNER.label_$dref
	destroy $ANIM_INNER.$dref
	destroy $ANIM_INNER.sel_$dref
}

proc sync_dataref { dref name now minv maxv } {
	global ANIM_INNER
	if [winfo exists .xp_anim] {

		if ![winfo exists $ANIM_INNER.label_$dref] {
			label $ANIM_INNER.label_$dref -text $dref
			scale $ANIM_INNER.$dref -command "ac3d xplane_set_anim_now $dref" -from 0 -to 360 -orient horiz -length 150 -width 10 -resolution 0
			button $ANIM_INNER.sel_$dref -command "ac3d xplane_anim_select $dref" -text "Select"
			grid $ANIM_INNER.label_$dref $ANIM_INNER.$dref $ANIM_INNER.sel_$dref -sticky news
		}

		$ANIM_INNER.label_$dref configure -text $name
		$ANIM_INNER.$dref configure -from $minv -to $maxv
		$ANIM_INNER.$dref set $now		
	}
}

proc xplane_anim_sync {} {
	global xplane_anim_enable
	global xplane_anim_invis
	ac3d xplane_set_anim_enable $xplane_anim_enable
	ac3d xplane_set_anim_list_invis $xplane_anim_invis
}

ac3d add_pref window_geom_xplane_anim ""

proc ScrolledVertCanvas_hack { c width height region } {
	frame $c
	canvas $c.canvas -width $width -height $height \
		-yscrollcommand "$c.yscroll set" \
		-scrollregion $region 

	scrollbar $c.yscroll -orient vertical  \
		-command "$c.canvas yview" 


	pack $c.yscroll -side right -fill y
	pack $c.canvas -side left -fill both -expand true

	set f [frame $c.canvas.f -bd 0]

	$c.canvas create window 0 0 -anchor nw -window $f

	return $f
}


proc xplane_anim_window {} {
	global xplane_anim_enable
	global xplane_anim_invis
	global ANIM_INNER

	if ![winfo exists .xp_anim] {
		new_toplevel_tracked .xp_anim "X-Plane Animation" prefs_window_geom_xplane_anim

		checkbutton .xp_anim.enable -text "Show Animation" -variable xplane_anim_enable		
		checkbutton .xp_anim.invis -text "List Invisible" -variable xplane_anim_invis
		button	.xp_anim.sync -text "Resync" -command "ac3d xplane_resync_anim"
		button  .xp_anim.sel_all -text "Select All Animation" -command "ac3d xplane_anim_select_all"
		grid .xp_anim.enable .xp_anim.invis .xp_anim.sync .xp_anim.sel_all -sticky nw

#		frame .xp_anim.drefs
		set ANIM_INNER [ ScrolledVertCanvas_hack .xp_anim.drefs 300 500 { 0 0 300 10000 } ]
		grid .xp_anim.drefs -columnspan 3 -sticky news
		 
		grid rowconfigure .xp_anim 0 -weight 0 -pad 0
		grid rowconfigure .xp_anim 1 -weight 100 -pad 0

		grid columnconfigure .xp_anim 1 -weight 0 -pad 0
		grid columnconfigure .xp_anim 1 -weight 100 -pad 0

		 
		set xplane_anim_enable 1
		set xplane_anim_invis 0
		
		xplane_anim_sync
		ac3d xplane_resync_anim
	}
	wm deiconify		.xp_anim
	raise				.xp_anim
}

proc xplane_anim_update { name1 name2 op } {
	xplane_anim_sync
}

set xplane_anim_enable 0
set xplane_anim_invis 0

trace add variable xplane_anim_enable write xplane_anim_update
trace add variable xplane_anim_invis write xplane_anim_update

#trace add variable unsaved_changes write "ac3d xplane_sync_panel"


##########################################################################################################################################################
# MENU BAR
##########################################################################################################################################################



.mbar add cascade -menu .mbar.xplane.menu -label "X-Plane" -underline 0
menubutton .mbar.xplane -text "X-Plane" -menu .mbar.xplane.menu -underline 0
if { $tcl_version >= 8 } {
} else {
	pack .mbar.xplane -after .mbar.tools
}
menu .mbar.xplane.menu -tearoff 0	
set UI(menu_xplane) .mbar.xplane.menu


.mbar.xplane.menu add command -label "X-Plane Object Properties..." -command "xplane_inspector"
if {$USE_PANEL_EDIT} {
  .mbar.xplane.menu add command -label "X-Plane Panel Properties..." -command "xplane_panel_dialog"
}
.mbar.xplane.menu add command -label "Select All Lights" -command "ac3d xplane_sel_lights"
.mbar.xplane.menu add command -label "Calculate X-Plane LOD..." -command "ac3d xplane_calc_lod"
.mbar.xplane.menu add command -label "Make LOD Group" -command "ac3d xplane_make_named_group LOD"
.mbar.xplane.menu add command -label "Calculate Batches For a Selection..." -command "ac3d xplane_optimize_selection 0"
.mbar.xplane.menu add command -label "Optimize Selection..." -command "ac3d xplane_optimize_selection 1"

.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Remap Texture Coordinates..." -command "xplane_tex_rescale_dialog"
.mbar.xplane.menu add command -label "Select by Texture Coordinates..." -command "xplane_tex_select_dialog"
.mbar.xplane.menu add command -label "Change Texture..." -command "ac3d xplane_change_texture"
.mbar.xplane.menu add command -label "Make Transparent" -command "ac3d xplane_make_transparent"
.mbar.xplane.menu add command -label "Make Night Lighting" -command "ac3d xplane_make_night"
if {$USE_KEYFRAMES} {
.mbar.xplane.menu add command -label "Pseudo-Cylindrical UV Remap" -command "ac3d xplane_do_uvmap"
.mbar.xplane.menu add command -label "Copy UV Map" -command "ac3d xplane_uv_copy"
.mbar.xplane.menu add command -label "Paste UV Map" -command "ac3d xplane_uv_paste"
}
.mbar.xplane.menu add command -label "Reload All Textures" -command "ac3d xplane_reload_texes"
.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Make Animation Group" -command "ac3d xplane_make_anim_group"
.mbar.xplane.menu add command -label "Make Translation" -command "ac3d xplane_make_anim_typed translate"
.mbar.xplane.menu add command -label "Make Rotation" -command "ac3d xplane_make_anim_typed rotate"
.mbar.xplane.menu add command -label "Make Show" -command "ac3d xplane_make_anim_typed show"
.mbar.xplane.menu add command -label "Make Hide" -command "ac3d xplane_make_anim_typed hide"
# no need to bake static translations - it's done for us
#.mbar.xplane.menu add command -label "Bake Static Transitions" -command "ac3d xplane_bake_static"

if {$USE_KEYFRAMES} {
.mbar.xplane.menu add command -label "Reverse Keyframes" -command "ac3d xplane_reverse_keyframe"
.mbar.xplane.menu add command -label "Rescale Keyframes..." -command xplane_keyframe_rescale_dialog
}

.mbar.xplane.menu add command -label "Animation Time..." -command "xplane_anim_window"
.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Make Trees" -command "ac3d xplane_make_tree"
.mbar.xplane.menu add command -label "Make One-sided" -command "ac3d xplane_make_onesided"
.mbar.xplane.menu add command -label "Make Up Normals" -command "ac3d xplane_make_upnormal"
.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Convert Directory to OBJ8..." -command "xplane_convert_dir"
.mbar.xplane.menu add command -label "Bulk Export..." -command "ac3d xplane_bulk_export"
.mbar.xplane.menu add command -label "By-Texture Export..." -command "ac3d xplane_tex_export"
.mbar.xplane.menu add command -label "X-Plane Export Settings..." -command "xplane_prefs_dialog"
.mbar.xplane.menu add command -label "Update workspace from old plugin" -command "ac3d xplane_update_selection"
.mbar.xplane.menu add command -label "Update Directory from old plugin..." -command "xplane_update_dir"


fetch_all_datarefs
fetch_all_cmnds

##########################################################################################################################################################
# DATAREFS
##########################################################################################################################################################

