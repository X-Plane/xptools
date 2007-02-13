BEGIN TRANSACTION setup;

CREATE TABLE WED_classes (			
	id				integer		PRIMARY KEY	,					
	name			varchar		NOT NULL	,					
	parent			integer
);

INSERT INTO WED_classes VALUES(0,				"WED_Thing",			NULL);
INSERT INTO WED_classes VALUES(1,				"WED_Entity",			0	);
INSERT INTO WED_classes VALUES(2,				"WED_Group",			0	);
INSERT INTO WED_classes VALUES(3,				"WED_ObjPlacement",		1	);
INSERT INTO WED_classes VALUES(4,				"WED_Select",			0	);

CREATE TABLE WED_things(
	id				integer		PRIMARY KEY	,
	parent			integer					,
	seq				integer					,
	name			varchar		NOT NULL	,
	class_id		integer		REFERENCES WED_classes(id)
);

CREATE TABLE WED_entities(
	id				integer		primary key	,
	locked			integer		NOT NULL	,
	hidden			integer		NOT NULL	
);

CREATE TABLE WED_objects(
	id				integer		PRIMARY KEY	,
	latitude		double		NOT NULL	,
	longitude		double		NOT NULL	,
	rotation		double		NOT NULL	,
	model_id		integer		NOT NULL	REFERENCES WED_models(id)
);

CREATE TABLE WED_selection(
	id				integer		PRIMARY KEY
);

INSERT INTO WED_things VALUES(1,NULL,NULL,"Root",2);
INSERT INTO WED_things VALUES(2,1,0,"Test Obj 1",3);
INSERT INTO WED_things VALUES(3,1,1,"Test Obj 2",3);
INSERT INTO WED_things VALUES(4,1,2,"Sub group",2);
INSERT INTO WED_things VALUES(5,4,0,"Test Obj 3",3);
INSERT INTO WED_things VALUES(6,4,1,"Test Obj 4",3);
INSERT INTO WED_things VALUES(7,4,2,"Test Obj 5",3);
INSERT INTO WED_things VALUES(8,4,3,"Test Obj 6",3);
INSERT INTO WED_things VALUES(9,4,4,"Test Obj 7",3);
INSERT INTO WED_things VALUES(10,1,3,"Selection",4);

INSERT INTO WED_selection VALUES(6);
INSERT INTO WED_selection VALUES(8);


INSERT INTO WED_entities VALUES(1,0,0);
INSERT INTO WED_entities VALUES(2,0,0);
INSERT INTO WED_entities VALUES(3,0,0);
INSERT INTO WED_entities VALUES(4,0,0);
INSERT INTO WED_entities VALUES(5,0,0);
INSERT INTO WED_entities VALUES(6,0,0);
INSERT INTO WED_entities VALUES(7,0,0);
INSERT INTO WED_entities VALUES(8,0,0);
INSERT INTO WED_entities VALUES(9,0,0);


INSERT INTO WED_objects VALUES(2,42.72,-72.42,10,0);
INSERT INTO WED_objects VALUES(3,42.72,-72.42,20,0);
INSERT INTO WED_objects VALUES(5,42.72,-72.42,40,0);
INSERT INTO WED_objects VALUES(6,42.72,-72.42,50,0);
INSERT INTO WED_objects VALUES(7,42.72,-72.42,60,0);
INSERT INTO WED_objects VALUES(8,42.72,-72.42,70,0);
INSERT INTO WED_objects VALUES(9,42.72,-72.42,80,0);

COMMIT TRANSACTION setup;

