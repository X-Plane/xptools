BEGIN TRANSACTION setup;

CREATE TABLE WED_classes (			
	id				integer		PRIMARY KEY	,					
	name			varchar		NOT NULL	,					
	parent			integer					,							
	table_name		varchar
);

INSERT INTO WED_classes VALUES(0,				"WED_Thing",			NULL,		"WED_things"	);
INSERT INTO WED_classes VALUES(1,				"WED_Entity",			0,			"WED_entities"	);
INSERT INTO WED_classes VALUES(2,				"WED_Group",			0,			 NULL			);
INSERT INTO WED_classes VALUES(3,				"WED_ObjPlacement",		1,			"WED_objects"	);

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

INSERT INTO WED_things VALUES(0,NULL,NULL,"Root Group",2);
INSERT INTO WED_things VALUES(1,0,0,"Test Obj 1",3);
INSERT INTO WED_things VALUES(2,0,1,"Test Obj 2",3);
INSERT INTO WED_things VALUES(3,0,2,"Sub group",2);
INSERT INTO WED_things VALUES(4,3,0,"Test Obj 3",3);

INSERT INTO WED_entities VALUES(0,0,0);
INSERT INTO WED_entities VALUES(1,0,0);
INSERT INTO WED_entities VALUES(2,0,0);
INSERT INTO WED_entities VALUES(3,0,0);
INSERT INTO WED_entities VALUES(4,0,0);


INSERT INTO WED_objects VALUES(1,42.72,-72.42,10,0);
INSERT INTO WED_objects VALUES(2,42.72,-72.42,20,0);
INSERT INTO WED_objects VALUES(4,42.72,-72.42,30,0);

COMMIT TRANSACTION setup;

