BEGIN TRANSACTION setup;

CREATE TABLE WED_classes (			
	id				integer		PRIMARY KEY	,					
	name			varchar		NOT NULL	
);

INSERT INTO WED_classes	VALUES(0,	"WED_Root"				);
INSERT INTO WED_classes	VALUES(1,	"WED_Select"			);
INSERT INTO WED_classes	VALUES(2,	"WED_KeyObjects"		);
INSERT INTO WED_classes	VALUES(3,	"WED_AirportBeacon"		);
INSERT INTO WED_classes	VALUES(4,	"WED_ObjPlacement"		);
INSERT INTO WED_classes	VALUES(5,	"WED_Group"				);

-- THINGS

CREATE TABLE WED_things(
	id				integer		PRIMARY KEY	,
	parent			integer					,
	seq				integer					,
	name			varchar		NOT NULL	,
	class_id		integer		REFERENCES WED_classes(id)
);

CREATE TABLE WED_selection(
	id				integer		PRIMARY KEY	,
	item			integer		NOT NULL
);

CREATE TABLE WED_key_objects(
	id				integer		PRIMARY KEY	,
	key				string		NOT NULL	,
	value			integer		NOT NULL
);

-- ENTITIES AND GIS

CREATE TABLE WED_entities(
	id				integer		primary key	,
	locked			integer		NOT NULL	,
	hidden			integer		NOT NULL	
);


CREATE TABLE GIS_points(
	id				integer		PRIMARY KEY,
	latitude		double		NOT NULL,
	longitude		double		NOT NULL
);

CREATE TABLE GIS_points_bezier(
	id					integer		PRIMARY KEY,
	ctrl_latitude		double		NOT NULL,
	ctrl_longitude		double		NOT NULL
);

CREATE TABLE GIS_points_heading(
	id					integer		PRIMARY KEY,
	heading				double		NOT NULL
);

CREATE TABLE GIS_points_headingwidthlength(
	id					integer		PRIMARY KEY,
	width				double		NOT NULL,
	length				double		NOT NULL
);

CREATE TABLE GIS_lines_headings(
	id					integer		PRIMARY KEY,
	width				double		NOT NULL
);

-- Specific Types

CREATE TABLE WED_objects(
	id				integer		PRIMARY KEY	,
	model_id		integer		NOT NULL	REFERENCES WED_models(id)
);

CREATE TABLE WED_beacons(
	id				integer		PRIMARY KEY,
	type			string		NOT NULL
);

CREATE TABLE WED_windsocks(
	id				integer		PRIMARY KEY,
	lit				integer		NOT NULL
);

CREATE TABLE WED_towerviewpoint(
	id				integer		PRIMARY KEY,
	height			double		NOT NULL
);

CREATE TABLE WED_airportsign(
	id				integer		PRIMARY KEY,
	style			string		NOT NULL,
	size			string		NOT NULL
);

CREATE TABLE WED_lightfixture(
	id				integer		PRIMARY KEY,
	kind			string		NOT NULL,
	angle			double		NOT NULL
);

CREATE TABLE WED_helipad(
	id				integer		PRIMARY KEY,
	surface			string		NOT NULL,
	markings		string		NOT NULL,
	shoulder		string		NOT NULL,
	roughness		double		NOT NULL,
	lights			string		NOT NULL
);

CREATE TABLE WED_runway(
	id				integer		PRIMARY KEY,
	surface			string		NOT NULL,
	shoulder		string		NOT NULL,
	roughness		double		NOT NULL,
	center_lites	integer		NOT NULL,
	edge_lites		string		NOT NULL,
	distance_signs	integer		NOT NULL,
	id1				string		NOT NULL,
	displaced1		double		NOT NULL,
	blastpad1		double		NOT NULL,
	markings1		string		NOT NULL,
	app_lites1		string		NOT NULL,
	TDZL1			integer		NOT NULL,
	REIL1			string		NOT NULL,
	id2				string		NOT NULL,
	displaced2		double		NOT NULL,
	blastpad2		double		NOT NULL,
	markings2		string		NOT NULL,
	app_lites2		string		NOT NULL,
	TDZL2			integer		NOT NULL,
	REIL2			string		NOT NULL
);

CREATE TABLE WED_sealane(
	id				integer		PRIMARY KEY,
	buoys			integer		NOT NULL,
	id1				string		NOT NULL,
	id2				string		NOT NULL
);

CREATE TABLE WED_airportchains(
	id				integer		PRIMARY KEY,
	closed			integer		NOT NULL
);

CREATE TABLE WED_taxiway(
	id				integer		PRIMARY KEY,
	surface			string		NOT NULL,
	roughness		double		NOT NULL,
	heading			double		NOT NULL
);

CREATE TABLE WED_airport(
	id				integer		PRIMARY KEY,
	kind			string		NOT NULL,
	elevation		double		NOT NULL,
	has_atc			integer		NOT NULL,
	icao			string		NOT NULL
);

CREATE TABLE WED_airportnode(
	id				integer		PRIMARY KEY,
	attributes		string		NOT NULL
);
-- 

INSERT INTO WED_things VALUES(1,NULL,NULL,"root",0);
INSERT INTO WED_things VALUES(2,1,0,"selection",1);
INSERT INTO WED_things VALUES(3,1,1,"choices",2);
INSERT INTO WED_things VALUES(4,1,2,"world",5);
INSERT INTO WED_things VALUES(5,4,0,"test beacon",3);
INSERT INTO WED_things VALUES(6,4,0,"test 2 beacon",3);

INSERT INTO WED_entities VALUES(4, 0, 0);
INSERT INTO WED_entities VALUES(5, 0, 0);
INSERT INTO WED_entities VALUES(6, 0, 0);

INSERT INTO GIS_points VALUES(5, 42.5,-72.1);
INSERT INTO GIS_points VALUES(6, 42.6,-72.2);

INSERT INTO WED_beacons VALUES(5,"beacon_Airport");
INSERT INTO WED_beacons VALUES(6,"beacon_Airport");

COMMIT TRANSACTION setup;
