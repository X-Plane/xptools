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

-- 

INSERT INTO WED_things VALUES(1,NULL,NULL,"root",0);
INSERT INTO WED_things VALUES(2,1,0,"selection",1);
INSERT INTO WED_things VALUES(3,1,1,"choices",2);


COMMIT TRANSACTION setup;

