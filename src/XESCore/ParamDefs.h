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
#if !EXTRACT_TOKENS
#include "EnumSystem.h"
#endif

#if EXTRACT_TOKENS
/************** PARAMETER DEFS *******************
 * These indicate indices for looking up metadata
 * in various data structures.
 *************************************************/

	TOKEN(NO_VALUE)

	/* CONTINUOUS PARAM (DEM) TYPES */

	/* These are continuous data DEMs. */
	TOKEN(dem_Elevation)			//	Height above sea level, meters from a DEM (raw elevation)
	TOKEN(dem_Bathymetry)			// Height of water bottom, raw.

	TOKEN(dem_Temperature)			//	Average Temperature Data
	TOKEN(dem_TemperatureRange)		//	Average Temperature Range over a year Data
	TOKEN(dem_Rainfall)				//	Average Rainfall Data
	TOKEN(dem_Biomass)				//	Biomass an area can support

	TOKEN(dem_Slope)				//	Incline of slope in degrees: 1-cos(angle), e.g. 0 for flat, 1 for sloped.
	TOKEN(dem_SlopeHeading)			//	Angle of slope: cos(heading), e.g. 1 = north, -1 = south, 0 = east.
	TOKEN(dem_RelativeElevation)	//	Comparison of this point to local min/max as a ratio
	TOKEN(dem_ElevationRange)		//	Total span of min/max in this area.

	TOKEN(dem_UrbanDensity)			//	Human settlement density
//	TOKEN(dem_UrbanPropertyValue)	//	Human settlement density
	TOKEN(dem_UrbanRadial)			//	DEM of urban radial stuff
	TOKEN(dem_UrbanTransport)		//	DEM of possible transportation stuff
	TOKEN(dem_UrbanSquare)			//	Boolean DEM - is it square? (Contains 1.0 for square, 2.0 for irregular urban)
//	TOKEN(dem_VegetationDensity)	//	Vegetation density ratio

	TOKEN(dem_TemperatureSeaLevel)	//	Average Temperature Data corrected for height

	TOKEN(dem_HydroDirection)		// Direction of rainfall per DEM point
	TOKEN(dem_HydroQuantity)		// Quantity of water transit per DEM point
	TOKEN(dem_HydroElevation)		// Water level or NO_VALUE for dry land for hydro reprocessing

	/* These are enum DEMs. */
//	TOKEN(dem_OrigLandUse)			//	Standard land use codes, USGS or XP6/7
	TOKEN(dem_LandUse)				//	Standard land use codes, USGS or XP6/7
	TOKEN(dem_Climate)				//	Type of climate
//	TOKEN(dem_NudeColor)			//	Color of nude terrain (geology)
	TOKEN(dem_ForestType)			//	Forest token type...

	/* Boolean DEMs. */

	TOKEN(dem_Wizard)				//	Derived dem for the purpose of illustrating stuff.

	/* HALFEDGE PARAMS */

	TOKEN(he_IsUnderpassing)
	TOKEN(he_TIGER_TLID)
	TOKEN(he_IsRiver)
	TOKEN(he_IsDryRiver)
	TOKEN(he_MustBurn)				// Indicates that the half-edge must be turned into a triangle border.
	TOKEN(he_Bridge)				// Indicates that this is a bridge in the original vector data, not an error in water-road merging.

	/* AREA PARAMS */

	/* POINT FEATURE PARAMS */

	TOKEN(pf_Height)

	/* POLYGONAL (AREA) FEATURE PARAMS */

	TOKEN(af_Height)				// Highest legal auto-gen obj we can put here
	TOKEN(af_HeightObjs)			// Height of tallest obj in face

	TOKEN(af_WaterArea)				// square area in sq meters
	TOKEN(af_WaterOpen)				// 1 = this water reaches the end of the DSF - COULD be an ocean), 0 = fully enclosed within DSF
	TOKEN(af_OriginCode)			// Origin of data code -- who did we get this from?	
	TOKEN(af_Zoning)
	
	// These tokens are derived 
	TOKEN(af_UrbanAverage)
	TOKEN(af_ForestAverage)
	TOKEN(af_SlopeMax)
	TOKEN(af_Relief)
	TOKEN(af_AreaMeters)
	TOKEN(af_Cat1)				// Dominant land class
	TOKEN(af_Cat1Rat)
	TOKEN(af_Cat2)				// Dominant land class
	TOKEN(af_Cat2Rat)
	TOKEN(af_Cat3)				// Dominant land class
	TOKEN(af_Cat3Rat)
	
/*******************NETWORK TYPE CODES***********************************
 * These types define network features on a map.
 ************************************************************************/


	TOKEN(road_Unknown)

	TOKEN(road_Start_Highway)
		TOKEN(road_MotorwayOneway)
	TOKEN(road_End_Highway)

	TOKEN(road_Start_MainDrag)
		TOKEN(road_Primary)
		TOKEN(road_PrimaryOneway)
		TOKEN(road_Secondary)
		TOKEN(road_SecondaryOneway)
		TOKEN(road_Tertiary)
		TOKEN(road_TertiaryOneway)
	TOKEN(road_End_MainDrag)

	TOKEN(road_Start_LocalRoad)
		TOKEN(road_Local)
		TOKEN(road_LocalOneway)
	TOKEN(road_End_LocalRoad)

	TOKEN(road_Start_Access)
		TOKEN(road_Ramp)
	TOKEN(road_End_Access)

	TOKEN(road_Start_Walkway)
		TOKEN(walk_Trail)
	TOKEN(road_End_Walkway)

	TOKEN(train_Start)
		TOKEN(train_RailwayOneway)
	TOKEN(train_End)

	TOKEN(powerline_Generic)
	TOKEN(dam_Generic)


/*******************FEATURE CLASS CODES**********************************
 * Features are source phenomenon, in other words, stuff cataloged in
 * data sets like hospitals, shopping malls, jails, etc.  Features fall
 * into two rough catagories: generic landmarks are features that are
 * identifiable among themselves (e.g. a campground doesn't look like
 * anything other than a campground).  Generic features are not as easily
 * identified; the visual representation may not be unique to the code.
 * (A mid-rise residential feature may not look that different from other
 * mid-size buildings, depending on the locality).
 *
 * It is important to remember that features do NOT specify a visual
 * encoding.  A military base may be visually represented with different
 * technology based on climate and global position.  A medical center may
 * change in appearance based on urban density, etc.
 *
 * In a few cases, the feature code itself implies parameters that would
 * otherwise be taken from continuous data.  For example,
 * feat_ResidentialLowValueComplex implies low economic value (a grubby
 * housing complex) regardless of the economic or density data.  These
 * overrides are provided to allow features to keep their fidelity.  For
 * example, a 'poor house' in US data might be mapped to a low value
 * residential feature so that even if the economic indicators say otherwise,
 * a low-value graphic will be used.  Where no continuous data is implied,
 * the feature should be subject to local constraints.
 *
 * Features may be point, polygonal, or area in our representation scheme,
 * but these feature codes are NOT divided up by these storage-types.
 * The comments below (e.g. "Area-type features") are more to help visualize
 * what we are getting at...we expect a military base to cover an area and
 * not just be a single building.  The military base could be encoded as a
 * GT polygon (or several), polygons, or points; it is up to the compiler
 * to do a best-effort to fit a feature no matter what form it is in.
 ************************************************************************/

	TOKEN(feat_Unknown)

	/* Generic landmarks - these landmarks are clearly identifiable by
	   TOKEN(nature) but are not differentiable from each other. */

	/* Area-type features */
	TOKEN(feat_MilitaryBase)
	TOKEN(feat_TrailerPark)
	TOKEN(feat_Campground)
	TOKEN(feat_Marina)
	TOKEN(feat_GolfCourse)
	TOKEN(feat_Cemetary)
	TOKEN(feat_Airport)

	/* Building-type features */
	TOKEN(feat_MedicalCenter)
	TOKEN(feat_EducationalCenter)
	TOKEN(feat_Jail)
	TOKEN(feat_Religious)
	TOKEN(feat_PostOffice)
	TOKEN(feat_Refinery)

	/* Transportation-type features */
	TOKEN(feat_BusTerminal)
	TOKEN(feat_TrainTerminal)
	TOKEN(feat_SeaTerminal)
	TOKEN(feat_Dam)
	TOKEN(feat_Tramway)

	/* Tall Obstacles */
	TOKEN(feat_RadioTower)
	TOKEN(feat_Pole)
	TOKEN(feat_Crane)
	TOKEN(feat_Elevator)
	TOKEN(feat_Windmill)
	TOKEN(feat_Tank)
	TOKEN(feat_Smokestack)
	TOKEN(feat_Smokestacks)

	/* Landmarks */
	TOKEN(feat_Arch)
	TOKEN(feat_CoolingTower)
	TOKEN(feat_Monument)
	TOKEN(feat_Spire)
	TOKEN(feat_Dome)
	TOKEN(feat_Sign)

	/* Misc. features */
	TOKEN(feat_AmusementCenter)

	/* Airport features.  Note: these features are NOT put in the
	   global scenery final output, but they are modeled in XES files
	   so we can avoid having collisions and chaos! */

	TOKEN(feat_FirstAirportFurniture)
	TOKEN(feat_Windsock)
	TOKEN(feat_BeaconNDB)
	TOKEN(feat_MarkerBeacon)
	TOKEN(feat_BeaconGS)
	TOKEN(feat_BeaconLDA)
	TOKEN(feat_BeaconILS)
	TOKEN(feat_BeaconVOR)
	TOKEN(feat_RotatingBeacon)
	TOKEN(feat_RadarASR)
	TOKEN(feat_RadarARSR)
	TOKEN(feat_LastAirportFurniture)

	/* Generic features - these features are not distinguishable
	 * from other feature types...can you tell an apartment building
	 * from a dormitory? */

	/* Buildings - Residential */
	TOKEN(feat_ResidentialLowRise)
	TOKEN(feat_ResidentialMidRise)
	TOKEN(feat_ResidentialComplex)
	TOKEN(feat_ResidentialHouse)
	TOKEN(feat_ResidentialLowValueComplex)

	/* Buildings - Commercial */
	TOKEN(feat_CommercialOffice)
	TOKEN(feat_CommercialShoppingPlaza)
	TOKEN(feat_Government)

	/* Buildings - Industrial */
	TOKEN(feat_Industrial)
	TOKEN(feat_Plant)

	/* Buildings - Generic */
	TOKEN(feat_Skyscraper)
	TOKEN(feat_Building)

	/* Generic open space */
	TOKEN(feat_Park)
	TOKEN(feat_ForestPark)

/************************ TERRAIN ENUMERATIONS **************************
 * These tokens help us use the enum-based terrain definitions.
 ************************************************************************/

 /* These tokens describe our official land use code system, which comes
  * from the Olson Global Ecosystem Legend.  This is a superset of the
  * XP7 values in that a few values were missing in XP7.  Typically used
  * with dem_LandUse. */
/*
	TOKEN(lu_usgs_INTERRUPTED_AREAS)
*/	
	TOKEN(lu_usgs_URBAN_IRREGULAR)
	TOKEN(lu_usgs_URBAN_SQUARE)
/*	
	TOKEN(lu_usgs_LOW_SPARSE_GRASSLAND)
	TOKEN(lu_usgs_CONIFEROUS_FOREST)
	TOKEN(lu_usgs_DECIDUOUS_CONIFER_FOREST)
	TOKEN(lu_usgs_DECIDUOUS_BROADLEAF_FOREST)
	TOKEN(lu_usgs_EVERGREEN_BROADLEAF_FORESTS)
	TOKEN(lu_usgs_TALL_GRASSES_AND_SHRUBS)
	TOKEN(lu_usgs_BARE_DESERT)
	TOKEN(lu_usgs_UPLAND_TUNDRA)
	TOKEN(lu_usgs_IRRIGATED_GRASSLAND)
	TOKEN(lu_usgs_SEMI_DESERT)
	TOKEN(lu_usgs_GLACIER_ICE)
	TOKEN(lu_usgs_WOODED_WET_SWAMP)
*/	
	TOKEN(lu_usgs_INLAND_WATER)
	TOKEN(lu_usgs_SEA_WATER)
/*	
	TOKEN(lu_usgs_SHRUB_EVERGREEN)
	TOKEN(lu_usgs_SHRUB_DECIDUOUS)
	TOKEN(lu_usgs_MIXED_FOREST_AND_FIELD)
	TOKEN(lu_usgs_EVERGREEN_FOREST_AND_FIELDS)
	TOKEN(lu_usgs_COOL_RAIN_FOREST)
	TOKEN(lu_usgs_CONIFER_BOREAL_FOREST)
	TOKEN(lu_usgs_COOL_CONIFER_FOREST)
	TOKEN(lu_usgs_COOL_MIXED_FOREST)
	TOKEN(lu_usgs_MIXED_FOREST)
	TOKEN(lu_usgs_COOL_BROADLEAF_FOREST)
	TOKEN(lu_usgs_DECIDUOUS_BROADLEAF_FOREST2)	// There is a dupe in the OGE coding system.
	TOKEN(lu_usgs_CONIFER_FOREST)
	TOKEN(lu_usgs_MONTANE_TROPICAL_FORESTS)
	TOKEN(lu_usgs_SEASONAL_TROPICAL_FOREST)
	TOKEN(lu_usgs_COOL_CROPS_AND_TOWNS_IRREGULAR)
	TOKEN(lu_usgs_COOL_CROPS_AND_TOWNS_SQUARE)
	TOKEN(lu_usgs_CROPS_AND_TOWN_IRREGULAR)
	TOKEN(lu_usgs_CROPS_AND_TOWN_SQUARE)
	TOKEN(lu_usgs_DRY_TROPICAL_WOODS)
	TOKEN(lu_usgs_TROPICAL_RAINFOREST)
	TOKEN(lu_usgs_TROPICAL_DEGRADED_FOREST)
	TOKEN(lu_usgs_CORN_AND_BEANS_CROPLAND_IRREGULAR)
	TOKEN(lu_usgs_CORN_AND_BEANS_CROPLAND_SQUARE)
	TOKEN(lu_usgs_RICE_PADDY_AND_FIELD_IRREGULAR)
	TOKEN(lu_usgs_RICE_PADDY_AND_FIELD_SQUARE)
	TOKEN(lu_usgs_HOT_IRRIGATED_CROPLAND_IRREGULAR)
	TOKEN(lu_usgs_HOT_IRRIGATED_CROPLAND_SQUARE)
	TOKEN(lu_usgs_COOL_IRRIGATED_CROPLAND_IRREGULAR)
	TOKEN(lu_usgs_COOL_IRRIGATED_CROPLAND_SQUARE)
	TOKEN(lu_usgs_COLD_IRRIGATED_CROPLAND)
	TOKEN(lu_usgs_COOL_GRASSES_AND_SHRUBS)
	TOKEN(lu_usgs_HOT_AND_MILD_GRASSES_AND_SHRUBS)
	TOKEN(lu_usgs_COLD_GRASSLAND)
	TOKEN(lu_usgs_SAVANNA_WOODS)
	TOKEN(lu_usgs_MIRE_BOG_FEN)
	TOKEN(lu_usgs_MARSH_WETLAND)
	TOKEN(lu_usgs_MEDITERRANEAN_SCRUB)
	TOKEN(lu_usgs_DRY_WOODY_SCRUB)
	TOKEN(lu_usgs_DRY_EVERGREEN_WOODS)
	TOKEN(lu_usgs_VOLCANIC_ROCK)
	TOKEN(lu_usgs_SAND_DESERT)
	TOKEN(lu_usgs_SEMI_DESERT_SHRUBS)
	TOKEN(lu_usgs_SEMI_DESERT_SAGE)
	TOKEN(lu_usgs_BARREN_TUNDRA)
	TOKEN(lu_usgs_COOL_SOUTHERN_HEMISPHERE_MIXED_FORESTS)
	TOKEN(lu_usgs_COOL_FIELDS_AND_WOODS)
	TOKEN(lu_usgs_FOREST_AND_FIELD)
	TOKEN(lu_usgs_COOL_FOREST_AND_FIELD)
	TOKEN(lu_usgs_FIELDS_AND_WOODY_SAVANNA)
	TOKEN(lu_usgs_SUCCULENT_AND_THORN_SCRUB)
	TOKEN(lu_usgs_SMALL_LEAF_MIXED_WOODS)
	TOKEN(lu_usgs_DECIDUOUS_AND_MIXED_BOREAL_FOREST)
	TOKEN(lu_usgs_NARROW_CONIFERS)
	TOKEN(lu_usgs_WOODED_TUNDRA)
	TOKEN(lu_usgs_HEATH_SCRUB)
	TOKEN(lu_usgs_COASTAL_WETLAND__NW)
	TOKEN(lu_usgs_COASTAL_WETLAND__NE)
	TOKEN(lu_usgs_COASTAL_WETLAND__SE)
	TOKEN(lu_usgs_COASTAL_WETLAND__SW)
	TOKEN(lu_usgs_POLAR_AND_ALPINE_DESERT)
	TOKEN(lu_usgs_GLACIER_ROCK)
	TOKEN(lu_usgs_SALT_PLAYAS)
	TOKEN(lu_usgs_MANGROVE)
	TOKEN(lu_usgs_WATER_AND_ISLAND_FRINGE)
	TOKEN(lu_usgs_LAND_WATER_AND_SHORE)
	TOKEN(lu_usgs_LAND_AND_WATER_RIVERS)
	TOKEN(lu_usgs_CROP_AND_WATER_MIXTURES_IRREGULAR)
	TOKEN(lu_usgs_CROP_AND_WATER_MIXTURES_SQUARE)
	TOKEN(lu_usgs_SOUTHERN_HEMISPHERE_CONIFERS)
	TOKEN(lu_usgs_SOUTHERN_HEMISPHERE_MIXED_FOREST)
	TOKEN(lu_usgs_WET_SCLEROPHYLIC_FOREST)
	TOKEN(lu_usgs_COASTLINE_FRINGE)
	TOKEN(lu_usgs_BEACHES_AND_DUNES)
	TOKEN(lu_usgs_SPARSE_DUNES_AND_RIDGES)
	TOKEN(lu_usgs_BARE_COASTAL_DUNES)
	TOKEN(lu_usgs_RESIDUAL_DUNES_AND_BEACHES)
	TOKEN(lu_usgs_COMPOUND_COASTLINES)
	TOKEN(lu_usgs_ROCKY_CLIFFS_AND_SLOPES)
	TOKEN(lu_usgs_SANDY_GRASSLAND_AND_SHRUBS)
	TOKEN(lu_usgs_BAMBOO)
	TOKEN(lu_usgs_MOIST_EUCALYPTUS)
	TOKEN(lu_usgs_RAIN_GREEN_TROPICAL_FOREST)
	TOKEN(lu_usgs_WOODY_SAVANNA)
	TOKEN(lu_usgs_BROADLEAF_CROPS)
	TOKEN(lu_usgs_GRASS_CROPS_IRREGULAR)
	TOKEN(lu_usgs_GRASS_CROPS_SQUARE)
	TOKEN(lu_usgs_CROPS_GRASS_SHRUBS_IRREGULAR)
	TOKEN(lu_usgs_CROPS_GRASS_SHRUBS_SQUARE)
	TOKEN(lu_usgs_EVERGREEN_TREE_CROP)
	TOKEN(lu_usgs_DECIDUOUS_TREE_CROP)
	TOKEN(lu_usgs_Unused97)
	TOKEN(lu_usgs_Unused98)
	TOKEN(lu_usgs_Unused99)
	TOKEN(lu_usgs_NO_DATA)
*/	

	TOKEN(lu_globcover_BARE_CONSOLIDATED)
	TOKEN(lu_globcover_BARE_ROCKS)
	TOKEN(lu_globcover_BARE_SANDY)
	TOKEN(lu_globcover_BARE_SCREE)
	TOKEN(lu_globcover_CROP)
	TOKEN(lu_globcover_CROP_FRUIT)
	TOKEN(lu_globcover_CROP_MOSAIC)
	TOKEN(lu_globcover_CROP_ORCHARDS)
	TOKEN(lu_globcover_CROP_SQUARE)
	TOKEN(lu_globcover_FOREST_BRAODLEAVED_DENSE)
	TOKEN(lu_globcover_FOREST_BRAODLEAVED_SPARSE)
	TOKEN(lu_globcover_FOREST_CONIFER_DENSE)
	TOKEN(lu_globcover_FOREST_CONIFER_SPARSE)
	TOKEN(lu_globcover_FOREST_DECIDUOS_DENSE)
	TOKEN(lu_globcover_FOREST_DECIDUOS_SPARSE)
	TOKEN(lu_globcover_FOREST_MIXED_DENSE)
	TOKEN(lu_globcover_FOREST_MIXED_SPARSE)
	TOKEN(lu_globcover_FOREST_MOSAIC)
	TOKEN(lu_globcover_GRASSLAND)
	TOKEN(lu_globcover_INDUSTRY)
	TOKEN(lu_globcover_INDUSTRY_SQUARE)
	TOKEN(lu_globcover_NO_DATA)
	TOKEN(lu_globcover_PASTURES)
	TOKEN(lu_globcover_SHRUB)
	TOKEN(lu_globcover_SNOW_ICE)
	TOKEN(lu_globcover_SPARSE)
	TOKEN(lu_globcover_SPARSE_SCREE)
	TOKEN(lu_globcover_SPARSE_TREES)
	TOKEN(lu_globcover_URBAN_CROP_TOWN)
	TOKEN(lu_globcover_URBAN_HIGH)
	TOKEN(lu_globcover_URBAN_LOW)
	TOKEN(lu_globcover_URBAN_MEDIUM)
	TOKEN(lu_globcover_URBAN_SQUARE_CROP_TOWN)
	TOKEN(lu_globcover_URBAN_SQUARE_HIGH)
	TOKEN(lu_globcover_URBAN_SQUARE_LOW)
	TOKEN(lu_globcover_URBAN_SQUARE_MEDIUM)
	TOKEN(lu_globcover_URBAN_SQUARE_TOWN)
	TOKEN(lu_globcover_URBAN_TOWN)
	TOKEN(lu_globcover_WATER)
	TOKEN(lu_globcover_WETLAND_BROADLEAVED_OPEN)
	TOKEN(lu_globcover_WETLAND_GRASSLAND)
	TOKEN(lu_globcover_WETLAND_SHRUB_CLOSED)

	/* Natural phenomena for dem_TerrainPhenomena - these define things
	   that happen on the ground other than plants. */

#if 0
	TOKEN(phenom_Unknown)
	TOKEN(phenom_Water)
	TOKEN(phenom_FreshWater)
	TOKEN(phenom_SeaWater)
	TOKEN(phenom_Ice)

	TOKEN(phenom_Soil)							// No plant life at all
	TOKEN(phenom_Rock)							// Rock exposed
	TOKEN(phenom_Sand)							// Some form of soil or sand

	TOKEN(phenom_Urban)							// World taken over by man.

	/* Natural phenomena for dem_2dVegePhenomena - what is on ground. */

	TOKEN(phenom_Grass)
	TOKEN(phenom_TallGrass)
	TOKEN(phenom_GrassAndShrubs)
	TOKEN(phenom_TallShrubs)
	TOKEN(phenom_Swamp)
	TOKEN(phenom_Crops)							// Any kind of field crops
	TOKEN(phenom_WetCrops)

	/* Natural phenomena for dem_3dVegePhenomena - what grows up. */

	TOKEN(phenom_Forest)						// Trees
	TOKEN(phenom_DeciForest)
	TOKEN(phenom_ConiForest)
	TOKEN(phenom_MixedForest)
	TOKEN(phenom_SwampTrees)				// Trees for swamp
	TOKEN(phenom_Orchard)
	TOKEN(phenom_ConiOrchard)
	TOKEN(phenom_DeciOrchard)
#endif

	/* Climate types, for dem_Climate.  These are the different climates in
	 * which the above phenomena instantiate themselves. */

	TOKEN(climate_TropicalRainForest)	// Type Af
	TOKEN(climate_TropicalMonsoon)		// Type Am
	TOKEN(climate_TropicalDry)			// Type Aw
	TOKEN(climate_DrySteppe)			// Type BS
	TOKEN(climate_DryDesert)			// Type BW
	TOKEN(climate_TemperateAny)			// Type C
	TOKEN(climate_TemperateSummerDry)	// Type Cs
	TOKEN(climate_TemperateWinterDry)	// Type Cw
	TOKEN(climate_TemperateWet)			// Type Cf
	TOKEN(climate_ColdAny)				// Type D
	TOKEN(climate_ColdSummerDry)		// Type Ds
	TOKEN(climate_ColdWinterDry)		// Type Dw
	TOKEN(climate_PolarTundra)			// Type ET
	TOKEN(climate_PolarFrozen)			// Type EF

	/* Rock and bare earth colors */
	TOKEN(nude_Red)
	TOKEN(nude_Yellow)
	TOKEN(nude_Gray)
	TOKEN(nude_Brown)
	TOKEN(nude_Black)

	/* 2-D Vegetation colors */
	TOKEN(grass_TropicalWet)
	TOKEN(grass_TropicalMed)
	TOKEN(grass_TropicalDry)
	TOKEN(grass_DrySavana)
	TOKEN(grass_TemperateWet)
	TOKEN(grass_TemperateMed_Wet)
	TOKEN(grass_TemperateMed_Dry)
	TOKEN(grass_TemperateDry)
	TOKEN(grass_ColdWet)
	TOKEN(grass_ColdMed)
	TOKEN(grass_ColdDry)
	TOKEN(grass_PolarWet)
	TOKEN(grass_PolarDry)

	/* TERRAIN CLASSES
	 * These values can be stored in the mTerrainType field on a per-polygon
	 * basis.  terrain_Natural indicates that land uses should be used to
	 * rebuild this terrain.
	 *
	 * NOTE: These terrains are in priority order in the sim!!  Also, the 'hill'
	 * variant MUST follow the non-hill variant!
	 *
	 */


	 /*
	 	IMPORTANT: this area needs to be cleaned up after the V8 global render!  Basically:
	 	terrain_Water and all .ter enum types are FINAL terrain types - they can go to a DSF
	 	without trouble.
	 	terrain_Natural is a place-holder indicating that the mesh needs to be filled in using
	 	the natural terrain rule-system.
	 	terrain_Airport, et. al. are feature-placeholders; they indicate that we also want to
	 	use the terrain rule system, but inputting this feature into the 'terrain' column
	 	as a restriction.

	 	All other types are no longer used (but were part of zoning once upon a time.)

	 */

/*
	TOKEN(terrain_VirtualOrtho00)			// Do not use in mTerrainType
	TOKEN(terrain_VirtualOrtho01)			// Do not use in mTerrainType
	TOKEN(terrain_VirtualOrtho10)			// Do not use in mTerrainType
	TOKEN(terrain_VirtualOrtho11)			// Do not use in mTerrainType
*/
	// Natural land uses
	TOKEN(terrain_Water)
	TOKEN(terrain_VisualWater)				// This is a special token to indicate visual but not physical water.

	// Man made land uses....each of tehse must have a hill variant
/*
	// Aggricultural land uses.
	TOKEN(terrain_Marker_Artificial)
	TOKEN(terrain_MixedFarm)				// A mix of farm + natural terrain
	TOKEN(terrain_MixedFarmHill)
	TOKEN(terrain_Farm)						// Total cultivation
	TOKEN(terrain_FarmHill)
	TOKEN(terrain_FarmTown)					// Farm + buildings and town
	TOKEN(terrain_FarmTownHill)					// Farm + buildings and town

	// Man-made city-like land uses.
	TOKEN(terrain_OutlayResidential)		// Sparse housing
	TOKEN(terrain_OutlayResidentialHill)
	TOKEN(terrain_OutlayHighrise)			// An incongruous tall building out in the woods
	TOKEN(terrain_OutlayHighriseHill)
	TOKEN(terrain_Residential)				// Dense single unit housing, etc.
	TOKEN(terrain_ResidentialHill)
	TOKEN(terrain_CommercialSprawl)			// Spread out (big box) commercial
	TOKEN(terrain_CommercialSprawlHill)
	TOKEN(terrain_Urban)					// Low-rise dense buildings...can't really tell if they're commercial, office, or residential at this scale.
	TOKEN(terrain_UrbanHill)
	TOKEN(terrain_Industrial)				// Dirty development
	TOKEN(terrain_IndustrialHill)
	TOKEN(terrain_Downtown)					// High-rise and sky scrapers
	TOKEN(terrain_DowntownHill)
*/
	// Feature land uses - for specific things that are in the scenery.  No hill variants.
	TOKEN(terrain_Marker_Features)
	TOKEN(terrain_MilitaryBase)				// Feature terrains - these terrains
	TOKEN(terrain_TrailerPark)				// are used under "area features", that is,
	TOKEN(terrain_Campground)				// specifically described small areas.
	TOKEN(terrain_Marina)
	TOKEN(terrain_GolfCourse)
	TOKEN(terrain_Cemetary)
	TOKEN(terrain_Park)
	TOKEN(terrain_ForestPark)
	TOKEN(terrain_Airport)
	TOKEN(terrain_AirportOuter)				// Temporary - we use two land-uses when building airports to make sure we have inner and outer area.

	// NOTE: this terrain is at the END of the terrain enums so we can serialize
	// specific natural terrain after this!
	TOKEN(terrain_Natural)					// "landuse" types Do not use in final triangles


/************************ X-PLANE ENTITIES *******************************
 * These enums describe X-Plane entities that can be placed in X-plane.
 ************************************************************************/

	/* Rep Types - direct representations */

	TOKEN(rep_Refinery)
	TOKEN(rep_Crane)
	TOKEN(rep_ConstructionSite)
	TOKEN(rep_GasTank)
	TOKEN(rep_Smokestack)
	TOKEN(rep_Factory)
	TOKEN(rep_Warehouse)
	TOKEN(rep_SelfStorage)
	TOKEN(rep_PowerplantCoal)
	TOKEN(rep_PowerplantNuke)
	TOKEN(rep_PowerplantHydro)
	TOKEN(rep_PowerplantGas)
	TOKEN(rep_TruckCenter)
	TOKEN(rep_TrailerPark)
	TOKEN(rep_TrailerParkDense)
	TOKEN(rep_LowriseApartment)
	TOKEN(rep_MidriseApartment)
	TOKEN(rep_ApartmentComplex)
	TOKEN(rep_Duplex)
	TOKEN(rep_StarterHome)
	TOKEN(rep_SingleFamilyHouse)
	TOKEN(rep_HugeSingleFamilyHouse)
	TOKEN(rep_Motel)
	TOKEN(rep_Hotel)
	TOKEN(rep_LuxuryHotel)
	TOKEN(rep_RowHouses)
	TOKEN(rep_WalkupApartments)
	TOKEN(rep_Farmhouse)
	TOKEN(rep_GrainSilo)
	TOKEN(rep_FireTower)
	TOKEN(rep_OfficeTrailers)
	TOKEN(rep_Skyscraper)
	TOKEN(rep_LowriseOffice)
	TOKEN(rep_MidriseOffice)
	TOKEN(rep_SmallStripMall)
	TOKEN(rep_LargeStripMall)
	TOKEN(rep_SmallMall)
	TOKEN(rep_LargeVerticalMall)
	TOKEN(rep_LargeHorizontalMal)
	TOKEN(rep_BankBranchOffice)
	TOKEN(rep_MajorBank)
	TOKEN(rep_FastfoodChain)
	TOKEN(rep_ConvertedSingle)
	TOKEN(rep_ConvertedRowHouses)
	TOKEN(rep_RowShops)
	TOKEN(rep_AutoRepair)
	TOKEN(rep_Marina)
	TOKEN(rep_SeaTerminal)
	TOKEN(rep_BusStation)
	TOKEN(rep_TrainStation)
	TOKEN(rep_CargoLoader)
	TOKEN(rep_CoolingTower)
	TOKEN(rep_GasStation)
	TOKEN(rep_TruckStop)
	TOKEN(rep_RadioTower)
	TOKEN(rep_CampTent)
	TOKEN(rep_CampTents)
	TOKEN(rep_Cabin)
	TOKEN(rep_Cabins)
	TOKEN(rep_GolfHole)
	TOKEN(rep_AmusementRides)
	TOKEN(rep_Arcade)
	TOKEN(rep_Cassino)
	TOKEN(rep_StadiumFootball)
	TOKEN(rep_StadiumBaseball)
	TOKEN(rep_Arena)
	TOKEN(rep_Playground)
	TOKEN(rep_PublicPool)
	TOKEN(rep_Fountain)
	TOKEN(rep_Barracks)
	TOKEN(rep_SupplyDepot)
	TOKEN(rep_RuralHospital)
	TOKEN(rep_CityHospital)
	TOKEN(rep_CityHospitalWithHeliport)
	TOKEN(rep_MedicalComplex)
	TOKEN(rep_RuralSchool)
	TOKEN(rep_BigCitySchool)
	TOKEN(rep_CampusSchool)
	TOKEN(rep_University)
	TOKEN(rep_PostOffice)
	TOKEN(rep_Jail)
	TOKEN(rep_PoliceStation)
	TOKEN(rep_FireStation)
	TOKEN(rep_CapitalBuilding)
	TOKEN(rep_Courthouse)
	TOKEN(rep_Church)
	TOKEN(rep_Synagogue)
	TOKEN(rep_Mosque)
	TOKEN(rep_Temple)
	TOKEN(rep_Flagpole)
	TOKEN(rep_Elevator)
	TOKEN(rep_Windmill)
	TOKEN(rep_Arch)
	TOKEN(rep_Monument)
	TOKEN(rep_Statue)
	TOKEN(rep_Obelisk)
	TOKEN(rep_Dome)
	TOKEN(rep_Sign)
	TOKEN(rep_Windsock)
	TOKEN(rep_RotatingBeacon)
	TOKEN(rep_BeaconNDB)
	TOKEN(rep_BeaconVOR)
	TOKEN(rep_BeaconILS)
	TOKEN(rep_BeaconLDA)
	TOKEN(rep_BeaconGS)
	TOKEN(rep_MarkerBeacon)
	TOKEN(rep_RadarASR)
	TOKEN(rep_RadarARSR)

	/* ROAD USAGES */

	TOKEN(use_None)
	TOKEN(use_Limited)
	TOKEN(use_Street)
	TOKEN(use_Ramp)
	TOKEN(use_Rail)
	TOKEN(use_Power)

	/* X-Plane Road Types - this is the master list of all road types that
	   x-plane knows about. */

	/* Six and four-lane highways...may be together or seprated.  If separated,
	 * small, and with no trains or brigdeg, there may be no guard rails (for out in Arizona).
	 * The inner edge line is always yellow, the outer edge line is white. */
	TOKEN(net_RampCity)
	TOKEN(net_RampRural)
	TOKEN(net_4City)
	TOKEN(net_4Rural)
	TOKEN(net_6City)
	TOKEN(net_6Rural)

	/* Primary road - two lanes in each direction, double yellow lines in the center
	 * and white lines on the edge.  If there are no sidewalks, there should be a rough
	 * edge.  If there are no sidewalks, the trains should show grass (clear alpha)
	 * and not cement underneath. */
	TOKEN(net_Primary_xx)
	TOKEN(net_Primary_sx)
	TOKEN(net_Primary_Sx)
	TOKEN(net_Primary_xs)
	TOKEN(net_Primary_ss)
	TOKEN(net_Primary_Ss)
	TOKEN(net_Primary_xS)
	TOKEN(net_Primary_sS)
	TOKEN(net_Primary_SS)

	TOKEN(net_PrimaryOneway_xx)
	TOKEN(net_PrimaryOneway_sx)
	TOKEN(net_PrimaryOneway_Sx)
	TOKEN(net_PrimaryOneway_xs)
	TOKEN(net_PrimaryOneway_ss)
	TOKEN(net_PrimaryOneway_Ss)
	TOKEN(net_PrimaryOneway_xS)
	TOKEN(net_PrimaryOneway_sS)
	TOKEN(net_PrimaryOneway_SS)
	
	/* Local roads - two directions, no separators.  Only access ramps have
	 * markings.  Alleys are damaged. */
	TOKEN(net_Local_xx)
	TOKEN(net_Local_sx)
	TOKEN(net_Local_Sx)
	TOKEN(net_Local_xs)
	TOKEN(net_Local_ss)
	TOKEN(net_Local_Ss)
	TOKEN(net_Local_xS)
	TOKEN(net_Local_sS)
	TOKEN(net_Local_SS)

	TOKEN(net_LocalOneway_xx)
	TOKEN(net_LocalOneway_sx)
	TOKEN(net_LocalOneway_Sx)
	TOKEN(net_LocalOneway_xs)
	TOKEN(net_LocalOneway_ss)
	TOKEN(net_LocalOneway_Ss)
	TOKEN(net_LocalOneway_xS)
	TOKEN(net_LocalOneway_sS)
	TOKEN(net_LocalOneway_SS)

	/* Trains - one or two tracks.  Need graphics for both by themselves and
	 * crossing roads */
	TOKEN(net_TrainsOneway)

	/* Walking trail - enough said */
	TOKEN(net_Walking)

	/* Electrical */
	TOKEN(net_Powerlines)

#endif /* EXTRACT_TOKENS */

#if !EXTRACT_TOKENS

	#ifndef PARAMDEFS_H
	#define PARAMDEFS_H
	inline	int Road_IsDam		(int x){ return x == dam_Generic; }
	inline	int Road_IsPowerline(int x){ return x == powerline_Generic; }
	inline	int	Road_IsHighway (int x) { return x > road_Start_Highway   && x < road_End_Highway;   }
	inline	int	Road_IsMainDrag(int x) { return x > road_Start_MainDrag  && x < road_End_MainDrag;  }
	inline	int	Road_IsLocal   (int x) { return x > road_Start_LocalRoad && x < road_End_LocalRoad; }
	inline	int	Road_IsAccess  (int x) { return x > road_Start_Access    && x < road_End_Access;    }
	inline	int Road_IsWalkway (int x) { return x > road_Start_Walkway   && x < road_End_Walkway;   }
	inline	int	Road_IsTrain   (int x) { return x > train_Start			 && x < train_End;			}

	inline	int	Feature_IsAirportFurniture(int x) { return x > feat_FirstAirportFurniture && x < feat_LastAirportFurniture; }

	#endif

#endif /* EXTRACT_TOKENS */

#if EXTRACT_TOKENS

	/******************** MISC ENUMERATIONS ****************************
	 * This is a set of misc. enums kept in param defs for file saving.
	 *******************************************************************/


	/******************* DRAINAGE RELATED ENUMS *********************/

	TOKEN(sink_Known)		// We expect a sink - we hvae a vector lake here.
	TOKEN(sink_Invalid)		// We couldn't flood out - bad DEM point?
	TOKEN(sink_Unresolved)	// We haven't tried to fix this sink yet.
	TOKEN(sink_Lake)		// Flooding produced a lake here.
	TOKEN(drain_Dir0)		// Drainage directions - start from straight up (or straight up right)
	TOKEN(drain_Dir1)
	TOKEN(drain_Dir2)
	TOKEN(drain_Dir3)
	TOKEN(drain_Dir4)
	TOKEN(drain_Dir5)
	TOKEN(drain_Dir6)
	TOKEN(drain_Dir7)


#endif
