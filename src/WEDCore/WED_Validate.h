/* 
 * Copyright (c) 2013, Laminar Research.
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

#ifndef WED_Validate_h
#define WED_Validate_h

#include "WED_Entity.h"
#include "WED_TaxiRoute.h"


class	IResolver;
class	WED_Thing;
class	WED_Airport;

//Keep this enum strictly organized by alphabetical order and sub catagory. Make this collection easily grep-able
enum validate_error_t
{
	err_airport_elements_outside_hierarchy,
	err_airport_impossible_size,
	err_airport_no_boundary,
	err_airport_no_icao,
	err_airport_no_name,
	err_airport_no_rwys_sealanes_or_helipads,
	err_airport_metadata_invalid,
	err_airport_no_runway_matching_cifp,
	err_airport_runway_matching_cifp_mislocated,
	err_apt_boundary_bez_curve_used,
	err_atc_taxi_routes_only_for_gte_xp10,
	err_atc_rule_wind_blank_ICAO_for_METAR,
	err_atc_rule_wind_invalid_directions,
	err_atc_rule_wind_invalid_speed,
	err_atc_rule_time_invalid_times,
	err_atcrwy_centerline_not_parallel_centerline,
	err_atcrwy_centerline_taxiroute_segment_off_center,
	err_atcrwy_centerline_too_sharp_turn,
	err_atcrwy_connectivity_forms_loop,
	err_atcrwy_connectivity_n_split,
	err_atcrwy_connectivity_not_continous,
	err_atcrwy_hotzone_taxi_route_too_close,
	err_atcrwy_taxi_route_does_not_span_enough_rwy,
	err_atcrwy_taxi_route_node_out_of_bounds,
	err_atcrwy_taxi_route_node_within_bounds_but_not_connected,
	err_atcrwy_truck_route_too_close_to_runway,
	err_duplicate_name,
	err_flow_blank_ICAO_for_METAR,
	err_flow_blank_name,
	err_flow_flows_only_for_gte_xp10,
	err_flow_no_arr_or_no_dep_runway,
	err_flow_has_opposite_arrivals,
	err_flow_has_opposite_departures,
	err_flow_pattern_runway_not_in_airport,
	err_flow_visibility_negative,
	err_freq_airport_has_gnd_or_del_but_no_tower,
	err_freq_could_not_find_at_least_one_valid_freq_for_group,
	err_freq_del_grnd_twr_in_civilian_band_must_be_on_25khz_spacing,
	err_freq_duplicate_freq,
	err_freq_not_between_0_and_1000_mhz,
	err_gateway_orthophoto_cannot_be_exported,
	err_gateway_resource_not_in_default_library,
	err_gateway_resource_private_or_depricated,
	err_gis_poly_facade_custom_wall_choice_only_for_gte_xp10,
	err_gis_poly_facades_may_not_have_holes,
	err_gis_poly_facades_curved_only_for_gte_xp10, //gte is greater than or equal to
	err_gis_poly_line_and_point_forests_only_for_gte_xp10,
	err_gis_poly_linear_feature_at_least_two_points,
	err_gis_poly_linear_feature_at_least_three_points,
	err_gis_poly_self_intersecting,
	err_gis_poly_wound_clockwise,
	err_gis_poly_zero_length_side,
	err_heli_name_does_not_start_with_h,
	err_heli_name_illegal_characters,
	err_heli_name_longer_than_allowed,
	err_heli_name_none,
	err_heli_not_adequetely_long,
	err_heli_not_adequetely_wide,
	err_orthophoto_bad_uv_map,
	err_ramp_airlines_contains_non_lowercase_letters,
	err_ramp_airlines_is_not_in_groups_of_three,
	err_ramp_airlines_is_not_spaced_correctly,
	err_ramp_airlines_not_group_of_three_letters,
	err_ramp_airlines_no_valid_airline_codes,
	err_ramp_op_type_and_airlines_only_allowed_at_gates_and_tie_downs,
	err_ramp_start_must_have_at_least_one_equip,
	err_ramp_start_with_specific_traffic_and_types_only_for_gte_xp10,
	err_resource_cannot_be_found,
	err_resource_does_not_have_correct_file_type,
	err_rwy_end_outside_of_map,
	err_rwy_end_not_matching_cifp,
	err_rwy_misaligned_with_name,
	err_rwy_must_be_reversed_to_match_name,
	err_rwy_name_high_illegal_characters,
	err_rwy_name_high_illegal_end_number,
	err_rwy_name_high_illegal_suffix,
	err_rwy_name_high_name_empty,
	err_rwy_name_low_illegal_characters,
	err_rwy_name_low_illegal_end_number,
	err_rwy_name_low_illegal_suffix,
	err_rwy_name_low_name_empty,
	err_rwy_name_mismatched_runway_numbers,
	err_rwy_name_not_recipricol_high_end,
	err_rwy_name_not_recipricol_low_end,
	err_rwy_name_reversed_runway_numbers_low_snd,  //and should be second
	err_rwy_name_suffix_only_on_one_end,
	err_rwy_name_suffixes_match,
	err_rwy_overlapping_displaced_thresholds,
	err_rwy_surface_illegal_roughness,
	err_rwy_surface_water_not_valid,
	err_rwy_unrealistically_small,
	err_rwy_use_must_have_at_least_one_equip,
	err_rwy_use_must_have_at_least_one_op,
	err_rwy_use_no_runway_selected,
	err_sign_error, //See the WED_Sign_Parser.h
	err_taxi_route_has_hot_zones_not_present,
	err_taxi_route_not_joined_to_dest_route,
	err_taxi_route_set_to_runway_not_present,
	err_taxi_route_zero_length,
	err_taxiway_hole_does_not_have_at_least_3_sides,
	err_taxiway_outer_boundary_does_not_have_at_least_3_sides,
	err_taxiway_surface_water_not_valid_type,
	err_truck_dest_must_have_at_least_one_truck_type_selected,
	err_truck_parking_cannot_have_negative_car_count,
	err_truck_parking_car_count_exceeds_max,
	err_truck_parking_no_ground_taxi_routes
};

// The validation error record stores a single validation problem for reporting.
// This consists of:
// - One error message loosely describing what went wrong and
// - An error code defined in the validation_error_t enum	
// - One or more objects participating in the problem.  At least one object is mandatory.
// - If the objects are within an airport, the parent airport that is now effectively invalid/unexportable.

struct	validation_error_t {

	validation_error_t() : airport(NULL) { }
	
	// This constructor creates a validation error with a single object ("who") participating.  Due to C++ weirdness
	// we have to template; the assumption is that "who" is a WED_Thing derivative.
	template <typename T>
	validation_error_t(const string& m, validate_error_t error_code, T * who, WED_Airport * a) : msg(m), airport(a) { bad_objects.push_back(who); }

	// This constructor takes an arbitrary container of ptrs to WED_Thing derivatives and builds a single validation
	// failure with every object listed.
	template <typename T>
	validation_error_t(const string& msg, validate_error_t error_code, const T& container, WED_Airport * airport);
	

	WED_Airport *		airport;		// NULL if error is in an object outside ANY airport
	vector<WED_Thing *>	bad_objects;	// object(s) involved in the validation error - at least one required.
	string				msg;
	validate_error_t	err_code;
};

typedef vector<validation_error_t> validation_error_vector;

// Collection primitives - these recursively walk the composition and pull out all entities of a given type.
bool	WED_ValidateApt(IResolver * resolver, WED_Thing * root = NULL);	// if root not null, only do this sub-tree

template <typename T>
validation_error_t::validation_error_t(const string& m, validate_error_t error_code, const T& container, WED_Airport * a) :
	msg(m), err_code(error_code), airport(a)
{
	copy(container.begin(), container.end(), back_inserter(bad_objects));
}

#endif
