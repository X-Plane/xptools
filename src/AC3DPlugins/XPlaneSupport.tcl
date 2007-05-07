catch {namespace import combobox::*}

set MAX_KEYFRAMES 50
set MAX_SEL 5

set USE_KEYFRAMES 1

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

proc make_labeled_entry { path name var } {
	set varl [join [list $path "." $var ".l"] "" ]
	set vare [join [list $path "." $var ".e"] "" ]
	set varc [join [list $path "." $var ] "" ]
	frame $varc
	label $varl -text "$name"
	entry $vare -textvariable $var -width 10
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


##########################################################################################################################################################
# BULK CONVERSION
##########################################################################################################################################################

proc xplane_convert_dir {} {
#	xplane_dir_eval "ac3d clear_all;ac3d load_ac \$filename;set filename \[string replace \$filename \[string last . \$filename\] end \".obj\"\];ac3d exporter_write_file OBJ8Save \$filename" "*.ac"
	xplane_dir_eval {ac3d clear_all;ac3d load_ac $filename;set filename [string replace $filename [string last . $filename] end ".obj"];ac3d exporter_write_file OBJ8Save $filename} "*.ac"
}


##########################################################################################################################################################
# TEXTURE COORDINATE REMAPPING!
##########################################################################################################################################################

ac3d add_pref window_geom_xplane_rescale_dialog ""

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
# PREFS DIALOG
##########################################################################################################################################################

ac3d add_pref window_geom_xplane_prefs_dialog ""

proc xplane_prefs_dialog {} {

	global xplane_layer_group_options
#	global prefs_x-plane_default_layer_group
	
	if ![winfo exists .xp_prefs] {

#		if { [set prefs_x-plane_default_layer_group] == "NULL" } {
#			set prefs_x-plane_default_layer_group "none"
#		}

		new_toplevel_tracked .xp_prefs "X-Plane export prefs" prefs_window_geom_xplane_prefs_dialog

		label		.xp_prefs.layer_btn_label -text "Default Layer Group:"
		menubutton .xp_prefs.layer_btn -menu .xp_prefs.layer_btn.menu -direction flush -textvariable prefs_x-plane_default_layer_group -padx 30 -pady 5
		menu .xp_prefs.layer_btn.menu
		foreach item $xplane_layer_group_options {
			.xp_prefs.layer_btn.menu add radiobutton -label $item -variable prefs_x-plane_default_layer_group
		}

		label		.xp_prefs.layer_offset_label -text "Default Offset:"
		spinbox		.xp_prefs.layer_offset -from -5 -increment 1 -to 5 -textvariable prefs_x-plane_default_layer_offset
		
#		checkbutton .xp_prefs.apt_lights		-variable prefs_x-plane_export_airport_lights -text "Export as airport light"
		checkbutton .xp_prefs.triangles			-variable prefs_x-plane_export_triangles	  -text "Export Geometry"
		
		label	.xp_prefs.default_lod_label -text "Default LOD:"
		spinbox .xp_prefs.default_lod_value -from 0 -increment 100 -to 1000 -textvariable prefs_x-plane_default_LOD

		label	.xp_prefs.export_prefix_label -text "Bulk Export Prefix:"
		entry	.xp_prefs.export_prefix -textvariable prefs_x-plane_export_prefix

		label	.xp_prefs.texture_prefix_label -text "Texture Export Prefix:"
		entry	.xp_prefs.texture_prefix -textvariable prefs_x-plane_texture_prefix


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

proc recurse_menu { base path } {
	set path_parent [lrange $path 0 end-1]
	
	set widget_parent $base.[join $path_parent ".m"]
	set widget_itself $base.[join $path ".m"]
	
	if ![llength $path_parent] {
		set widget_parent $base
	}
	
	if ![winfo exists $widget_parent] {
		if [llength $path_parent] {
			recurse_menu $base $path_parent
		}
	}
	
	menu $widget_itself	
	$widget_parent add cascade -label [lindex $path end] -menu $widget_itself		
}

proc build_popup { popup textvar use_arrays } {
	menu $popup.test_menu
	$popup.test_menu add command -label "none" -command "set $textvar none"

	catch {
		set fi [open "plugins/datarefs.txt" r]
		gets $fi line	
		while { [gets $fi line] >= 0 } {
			if [llength $line] {
			   set dref_fullname [lindex $line 0]
			   set dref_dtype [lindex $line 1]
			   set dref_dtype_l [split $dref_dtype "\[\]"]
			   set array_count -1
			   if { [llength $dref_dtype_l] > 1 } {
					set array_count [lindex $dref_dtype_l 1]
			   }
				
				if {$use_arrays == 0} {
					set array_count -1
				}
			   
			   if { $array_count > 0 && $array_count <= 10 } {
					for {set x 0} {$x<$array_count} {incr x} {
						set dref_path [split $dref_fullname "/"]
						lappend dref_path $x
						set dref_name [lindex $dref_path end]

						set dref_parents [lrange $dref_path 0 end-1]   
						set widget_parent [join $dref_parents ".m"]
						if ![winfo exists $popup.test_menu.$widget_parent] {
							recurse_menu $popup.test_menu $dref_parents
						}				   
						$popup.test_menu.$widget_parent add command -label $dref_name -command "set $textvar $dref_fullname\\\[$x\\\]"
					}
			   } else {
					set dref_path [split $dref_fullname "/"]
					set dref_name [lindex $dref_path end]			   
					set dref_parents [lrange $dref_path 0 end-1]   
					set widget_parent [join $dref_parents ".m"]			   
					if ![winfo exists $popup.test_menu.$widget_parent] {
						recurse_menu $popup.test_menu $dref_parents
					}
					$popup.test_menu.$widget_parent add command -label $dref_name -command "set $textvar $dref_fullname"
			   }
			}
		}
		close $fi   
	} msg		
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
			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]			
			xplane_light_sync $x $container
		}
		if {$sel_type == 2} { 
			pack $container.obj 
			global xplane_obj_name$x
			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]
#			pack forget $container.obj.anim_type_btn
#			if {$anim_type == 1} { pack $container.obj.anim_type_btn -anchor nw }
			xplane_obj_sync $x $container
		}
		if {$sel_type == 3} { 
			pack $container.grp 
			global xplane_obj_name$x
			set xplane_obj_name$x [ac3d object_get_name [lindex [ac3d get_selected_objects] $x] ]
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
	global xplane_anim_type$idx
	global xplane_blend_enable$idx
	global xplane_anim_keyframe_count$idx
	global MAX_KEYFRAMES
	pack forget $container.obj.none
	pack forget $container.obj.trans
	pack forget $container.obj.rotate
	pack forget $container.obj.static
	pack forget $container.obj.show
	pack forget $container.obj.hide
	pack forget $container.obj.dref_btn
	
	if { [set xplane_anim_type$idx] == "no animation"} { pack $container.obj.none }
	if { [set xplane_anim_type$idx] == "rotate"} { 
		pack $container.obj.rotate
		for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
			pack forget $container.obj.rotate.xplane_anim_value$x$idx
		}
		for {set x 0} {$x< [set xplane_anim_keyframe_count$idx] } {incr x} {
			pack $container.obj.rotate.xplane_anim_value$x$idx
		}
		pack $container.obj.dref_btn
	}
	if { [set xplane_anim_type$idx] == "translate"} { 
		pack $container.obj.trans
		for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
			pack forget $container.obj.trans.xplane_anim_value$x$idx
		}
		for {set x 0} {$x< [set xplane_anim_keyframe_count$idx]} {incr x} {
			pack $container.obj.trans.xplane_anim_value$x$idx
		}
		pack $container.obj.dref_btn		
	}
	if { [set xplane_anim_type$idx] == "static"} { 
		pack $container.obj.static 
	}
	if { [set xplane_anim_type$idx] == "show"  } { 
		pack $container.obj.show 
		pack $container.obj.dref_btn
	}
	if { [set xplane_anim_type$idx] == "hide"  } { 
		pack $container.obj.hide 
		pack $container.obj.dref_btn
	}
	
	pack forget $container.obj.none.blend_level	
	if { [set xplane_blend_enable$idx] == 0} { pack $container.obj.none.blend_level }
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
			global xplane_anim_value$x$idx
			global xplane_anim_angle$x$idx
		}
	}
	
	global xplane_hard_surface_options
	global xplane_light_options
	global xplane_layer_group_options
	global USE_KEYFRAMES
	
	if ![winfo exists .xp_view] {

		new_toplevel_tracked .xp_view "X-Plane Properties" prefs_window_geom_xplane_inspector
		
		for {set idx 0} {$idx<$MAX_SEL} {incr idx} {

			global xplane_anim_type$idx
			global xplane_poly_os$idx
			global xplane_blend_enable$idx
			global xplane_blend_level$idx
			global xplane_hard_surf$idx

			global xplane_light_type$idx
			global xplane_light_red$idx
			global xplane_light_green$idx
			global xplane_light_blue$idx
			global xplane_light_alpha$idx
			global xplane_light_size$idx
			global xplane_light_s1$idx
			global xplane_light_s2$idx
			global xplane_light_t1$idx
			global xplane_light_t2$idx
			global xplane_light_dataref$idx
		
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
			global xplane_obj_name$idx
			label $container.light.name -textvariable xplane_obj_name$idx
			pack $container.light.name_label $container.light.name -anchor nw

			menubutton $container.light.light_type_btn -menu $container.light.light_type_btn.menu -direction flush  -textvariable xplane_light_type$idx -padx 30 -pady 5
			menu $container.light.light_type_btn.menu
			foreach light $xplane_light_options {
				$container.light.light_type_btn.menu add radiobutton -label $light -variable xplane_light_type$idx -command xplane_light_sync_all
			}		
			pack $container.light.light_type_btn -anchor nw

			labelframe $container.light.rgb -text "RGB:"
				make_labeled_entry $container.light.rgb "Red:" xplane_light_red$idx
				make_labeled_entry $container.light.rgb "Green:" xplane_light_green$idx
				make_labeled_entry $container.light.rgb "Blue:" xplane_light_blue$idx
			pack $container.light.rgb

			labelframe $container.light.dataref -text "Dataref:"
				make_labeled_entry $container.light.dataref "Red:" xplane_light_red$idx
				make_labeled_entry $container.light.dataref "Green:" xplane_light_green$idx
				make_labeled_entry $container.light.dataref "Blue:" xplane_light_blue$idx
				make_labeled_entry $container.light.dataref "Alpha:" xplane_light_alpha$idx
				make_labeled_entry $container.light.dataref "Size:" xplane_light_size$idx
				make_labeled_entry $container.light.dataref "S1:" xplane_light_s1$idx
				make_labeled_entry $container.light.dataref "T1:" xplane_light_t1$idx
				make_labeled_entry $container.light.dataref "S2:" xplane_light_s2$idx
				make_labeled_entry $container.light.dataref "T2:" xplane_light_t2$idx
				menubutton $container.light.dataref.dref_btn -menu $container.light.dataref.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_light_dataref$idx
				build_popup $container.light.dataref.dref_btn xplane_light_dataref$idx 0
				pack $container.light.dataref.dref_btn
				
			pack $container.light.dataref

			labelframe $container.light.smoke_black -text "Black Smoke Puff:"
				make_labeled_entry $container.light.smoke_black "Puff size:" xplane_light_smoke_size$idx
			pack $container.light.smoke_black

			labelframe $container.light.smoke_white -text "White Smoke Puff:"
				make_labeled_entry $container.light.smoke_white "Puff size:" xplane_light_smoke_size$idx
			pack $container.light.smoke_white

			#-------------------------------------- OBJECTS --------------------------------------
			
			label $container.obj.name_label -text "Name:"
			global xplane_obj_name$idx
			label $container.obj.name -textvariable xplane_obj_name$idx
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
					$container.obj.none.hard_surf_btn.menu add radiobutton -label $surf -variable xplane_hard_surf$idx
				}
				pack $container.obj.none.hard_surf_label $container.obj.none.hard_surf_btn
				checkbutton $container.obj.none.use_materials -text "Use AC3D Materials" -variable xplane_use_materials$idx
				pack $container.obj.none.use_materials
				checkbutton $container.obj.none.blend_enable -text "Blending" -variable xplane_blend_enable$idx -command "xplane_obj_sync_all"
				pack $container.obj.none.blend_enable
				frame $container.obj.none.blend_level
					make_labeled_entry $container.obj.none.blend_level "blend cutoff" xplane_blend_level$idx
				pack $container.obj.none.blend_level
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
#				make_labeled_entry $container.obj.rotate "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.rotate.dref_btn -menu $container.obj.rotate.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.rotate.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.rotate.dref_btn
			pack $container.obj.rotate

			labelframe $container.obj.trans -text "Translation:"
				for {set x 0} {$x<$MAX_KEYFRAMES} {incr x} {
					make_labeled_entry $container.obj.trans "value $x" xplane_anim_value$x$idx
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
				make_labeled_entry $container.obj.trans "anchor" xplane_anim_keyframe_root$idx
#				make_labeled_entry $container.obj.trans "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.trans.dref_btn -menu $container.obj.trans.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.trans.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.trans.dref_btn
			pack $container.obj.trans

			labelframe $container.obj.static -text "Static Translation:"
				make_labeled_entry $container.obj.static "low value" xplane_anim_value0$idx
				make_labeled_entry $container.obj.static "high value" xplane_anim_value1$idx
#				make_labeled_entry $container.obj.static "dataref" xplane_anim_dataref$idx
			pack $container.obj.static

			labelframe $container.obj.show -text "Show:"
				make_labeled_entry $container.obj.show "low value" xplane_anim_value0$idx
				make_labeled_entry $container.obj.show "high value" xplane_anim_value1$idx
#				make_labeled_entry $container.obj.show "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.show.dref_btn -menu $container.obj.show.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.show.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.show.dref_btn
			pack $container.obj.show

			labelframe $container.obj.hide -text "Hide:"
				make_labeled_entry $container.obj.hide "low value" xplane_anim_value0$idx
				make_labeled_entry $container.obj.hide "high value" xplane_anim_value1$idx
#				make_labeled_entry $container.obj.hide "dataref" xplane_anim_dataref$idx
#				menubutton $container.obj.hide.dref_btn -menu $container.obj.hide.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
#				build_popup $container.obj.hide.dref_btn xplane_anim_dataref$idx
#				pack $container.obj.hide.dref_btn
			pack $container.obj.hide
			
			menubutton $container.obj.dref_btn -menu $container.obj.dref_btn.test_menu -direction flush -padx 30 -pady 5 -textvariable xplane_anim_dataref$idx
			build_popup $container.obj.dref_btn xplane_anim_dataref$idx 1
			pack $container.obj.dref_btn

			#-------------------------------------- GROUP --------------------------------------

			global xplane_obj_name$idx
			label $container.grp.name_label -text "Name:"
			label $container.grp.name -textvariable xplane_obj_name$idx
			grid $container.grp.name_label $container.grp.name -sticky nw

			labelframe $container.grp.lod -text "LOD:"
				make_labeled_entry $container.grp.lod "Near LOD:" xplane_lod_near$idx
				make_labeled_entry $container.grp.lod "Far LOD:" xplane_lod_far$idx
			grid $container.grp.lod -columnspan 2 -sticky nw

			labelframe $container.grp.layer_group -text "Layer Group:"

				menubutton $container.grp.layer_group.layer_btn -menu $container.grp.layer_group.layer_btn.menu -direction flush -textvariable xplane_layer_group$idx -padx 30 -pady 5
				menu $container.grp.layer_group.layer_btn.menu
				foreach item $xplane_layer_group_options {
					$container.grp.layer_group.layer_btn.menu add radiobutton -label $item -variable xplane_layer_group$idx
				}
				pack $container.grp.layer_group.layer_btn -anchor nw
				make_labeled_entry $container.grp.layer_group "Group Offset:" xplane_layer_group_offset$idx
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


set xplane_light_options [list none "black smoke" "white smoke" rgb custom \
	headlight taillight \
	airplane_landing airplane_beacon airplane_nav_tail airplane_nav_left airplane_nav_right airplane_strobe \
	ship_nav_left ship_nav_right ship_mast_obs ship_mast_grn ship_nav_tail ship_mast_powered \
	carrier_datum carrier_waveoff carrier_meatball1 carrier_meatball2 carrier_meatball3	carrier_meatball4 carrier_meatball5	carrier_mast_strobe	carrier_deck_blue_s	carrier_deck_blue_w \
	carrier_deck_blue_n	carrier_deck_blue_e	carrier_pitch_lights carrier_foul_line_red carrier_foul_line_white carrier_center_white	carrier_edge_white carrier_thresh_white \
	frigate_SGSI_lo	frigate_SGSI_on	frigate_SGSI_hi	frigate_deck_green oilrig_deck_blue \
	town_light_60 town_light_90 town_light_150 town_light_180 town_light_220 town_light_280 0town_light_330 town_light_350 town_light_omni \
	town_tiny_light_60 town_tiny_light_90 town_tiny_light_150 town_tiny_light_180 town_tiny_light_220 town_tiny_light_280 town_tiny_light_330 town_tiny_light_350 town_tiny_light_omni \
	obs_strobe_day obs_strobe_night obs_red_day obs_red_night]
	
set xplane_hard_surface_options [list none object water concrete asphalt grass dirt gravel lakebed snow shoulder blastpad]
set xplane_layer_group_options [list none terrain beaches shoulders taxiways runways markings airports roads objects light_objects cars]

trace add variable select_info write xplane_inspector_update
for {set idx 0} {$idx<$MAX_SEL} {incr idx} {
	trace add variable xplane_anim_keyframe_count$idx write xplane_inspector_update
}

##########################################################################################################################################################
# ANIMATION BAR
##########################################################################################################################################################

proc clean_anim {} {
	if [winfo exists .xp_anim] {
		set children [winfo children .xp_anim.drefs]
		foreach c $children {
			destroy $c
		}
	}
}

proc sync_dataref { dref name now minv maxv } {
	if [winfo exists .xp_anim] {

		if ![winfo exists .xp_anim.drefs.label_$dref] {
			label .xp_anim.drefs.label_$dref -text $dref
			scale .xp_anim.drefs.$dref -command "ac3d xplane_set_anim_now $dref" -from 0 -to 360 -orient horiz -length 150 -width 10 -resolution 0
			button .xp_anim.drefs.sel_$dref -command "ac3d xplane_anim_select $dref" -text "Select"
			grid .xp_anim.drefs.label_$dref .xp_anim.drefs.$dref .xp_anim.drefs.sel_$dref -sticky news
		}

		.xp_anim.drefs.label_$dref configure -text $name
		.xp_anim.drefs.$dref configure -from $minv -to $maxv
		.xp_anim.drefs.$dref set $now
	}
}

proc xplane_anim_sync {} {
	global xplane_anim_enable
	ac3d xplane_set_anim_enable $xplane_anim_enable
}

ac3d add_pref window_geom_xplane_anim ""

proc xplane_anim_window {} {
	global xplane_anim_enable
	
	if ![winfo exists .xp_anim] {
		new_toplevel_tracked .xp_anim "X-Plane Animation" prefs_window_geom_xplane_anim
		checkbutton .xp_anim.enable -text "Show Animation" -variable xplane_anim_enable		
		button	.xp_anim.sync -text "Resync" -command "ac3d xplane_resync_anim"
		grid  .xp_anim.enable .xp_anim.sync

		frame .xp_anim.drefs
		grid .xp_anim.drefs -sticky news
		
		set xplane_anim_enable 1
		
		xplane_anim_sync
		ac3d xplane_resync_anim
	}
	wm deiconify		.xp_anim
	raise				.xp_anim
}

proc xplane_anim_update { name1 name2 op } {
	xplane_anim_sync
}

trace add variable xplane_anim_enable write xplane_anim_update

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
.mbar.xplane.menu add command -label "Calculate X-Plane LOD" -command "ac3d xplane_calc_lod"
# No make LOD group cmd needed - LOD grop is a group
#.mbar.xplane.menu add command -label "Make LOD Group" -command "ac3d xplane_make_named_group LOD"
.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Remap Texture Coordinates..." -command "xplane_tex_rescale_dialog"
.mbar.xplane.menu add command -label "Change Texture..." -command "ac3d xplane_change_texture"
.mbar.xplane.menu add command -label "Make Transparent" -command "ac3d xplane_make_transparent"
.mbar.xplane.menu add command -label "Make Night Lighting" -command "ac3d xplane_make_night"
.mbar.xplane.menu add command -label "Pseudo-Cylindrical UV Remap" -command "ac3d xplane_do_uvmap"
.mbar.xplane.menu add command -label "Copy UV Map" -command "ac3d xplane_uv_copy"
.mbar.xplane.menu add command -label "Paste UV Map" -command "ac3d xplane_uv_paste"
.mbar.xplane.menu add command -label "Reload All Textures" -command "ac3d xplane_reload_texes"
.mbar.xplane.menu add separator
.mbar.xplane.menu add command -label "Make Animation Group" -command "ac3d xplane_make_anim_group"
.mbar.xplane.menu add command -label "Make Translation" -command "ac3d xplane_make_anim_typed translate"
.mbar.xplane.menu add command -label "Make Rotation" -command "ac3d xplane_make_anim_typed rotate"
.mbar.xplane.menu add command -label "Make Show" -command "ac3d xplane_make_anim_typed show"
.mbar.xplane.menu add command -label "Make Hide" -command "ac3d xplane_make_anim_typed hide"
# no need to bake static translations - it's done for us
#.mbar.xplane.menu add command -label "Bake Static Transitions" -command "ac3d xplane_bake_static"
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
.mbar.xplane.menu add command -label "Update from old plugin" -command "ac3d xplane_update_selection"



##########################################################################################################################################################
# DATAREFS
##########################################################################################################################################################

