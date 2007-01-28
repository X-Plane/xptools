BEGIN TRANSACTION setup;

CREATE TABLE WED_classes (			
	id				integer		PRIMARY KEY	,					
	name			varchar		NOT NULL	,					
	parent			integer					,							
	table_name		varchar
);

INSERT INTO WED_classes VALUES(0,				"WED_Entity",	NULL,		"WED_entities"	);
INSERT INTO WED_classes VALUES(1,				"group",		0,			NULL			);
INSERT INTO WED_classes VALUES(2,				"objects",		0,			"WED_objects"	);
INSERT INTO WED_classes VALUES(3,				"models",		NULL,		"WED_models"	);

CREATE TABLE WED_properties (	
	table_name		varchar		NOT NULL,			
	col				varchar		NOT NULL,					
	name			varchar		NOT NULL,					
	type			varchar		NOT NULL,					
	foreign_key		varchar				,
	PRIMARY KEY(table_name,col)
);

INSERT INTO WED_properties VALUES("WED_entities",	"name",		"Name",		"edit_text",		NULL);
INSERT INTO WED_properties VALUES("WED_entities",	"locked",	"Locked",	"check_box",		NULL);
INSERT INTO WED_properties VALUES("WED_entities",	"hidden",	"Hidden",	"check_box",		NULL);

INSERT INTO WED_properties VALUES("WED_objects",	"latitude",	"Latitude",	"double",			NULL);
INSERT INTO WED_properties VALUES("WED_objects",	"longitude","Longitude","double",			NULL);
INSERT INTO WED_properties VALUES("WED_objects",	"rotation",	"Rotation",	"double",			NULL);
INSERT INTO WED_properties VALUES("WED_objects",	"name",		"File Name","enum",				"WED_models.id");

INSERT INTO WED_properties VALUES("WED_models",		"name",		"File Name","edit_text",		NULL);

CREATE TABLE WED_entities(
	id				integer		PRIMARY KEY	,
	parent			integer					,
	seq				integer					,
	name			varchar		NOT NULL	,
	locked			integer		NOT NULL	,
	hidden			integer		NOT NULL	,
	class_id		integer		REFERENCES WED_classes(id)
);

INSERT INTO WED_entities VALUES(0,NULL,NULL,"Root",0,0,0);

CREATE TABLE WED_models(
	id				integer		PRIMARY key	,
	name			varchar		NOT NULL
);

CREATE TABLE WED_objects(
	id				integer		PRIMARY KEY	,
	latitude		double					,
	longitude		double					,
	rotation		double					,
	model_id		integer		REFERENCES WED_models(id)
);

COMMIT TRANSACTION setup;

