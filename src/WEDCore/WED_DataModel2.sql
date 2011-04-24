BEGIN TRANSACTION setup2;

CREATE TABLE IF NOT EXISTS WED_taxiroute(
	id				integer		PRIMARY KEY,
	oneway			integer		NOT NULL,
	runway			integer		NOT NULL
);

CREATE TABLE IF NOT EXISTS WED_taxiroute_depart(
	id				integer		NOT NULL,
	departures		varchar		NOT NULL,
	PRIMARY KEY(id,departures)
);		

CREATE TABLE IF NOT EXISTS WED_taxiroute_arrive(
	id				integer		NOT NULL,
	arrivals		varchar		NOT NULL,
	PRIMARY KEY(id,arrivals)
);		

CREATE TABLE IF NOT EXISTS WED_taxiroute_ils(
	id				integer		NOT NULL,
	ils		varchar		NOT NULL,
	PRIMARY KEY(id,ils)
);		
	
CREATE TABLE IF NOT EXISTS WED_atcflow(
	id				integer		PRIMARY KEY,
	icao			varchar		NOT NULL,
	cld_min			integer		NOT NULL,
	vis_min			integer		NOT NULL,
	wnd_spd_max		integer		NOT NULL,
	wnd_dir_min		integer		NOT NULL,
	wnd_dir_max		integer		NOT NULL,
	time_min		integer		NOT NULL,
	time_max		integer		NOT NULL,
	pattern_side	integer		NOT NULL,
	pattern_rwy		integer		NOT NULL
);	

CREATE TABLE IF NOT EXISTS WED_runwayuse(
	id				integer		PRIMARY KEY,
	rwy				integer		NOT NULL,
	dep_frq			double		NOT NULL,
	traffic			int			NOT NULL,
	operations		int			NOT NULL,
	dep_min			int			NOT NULL,
	dep_max			int			NOT NULL,
	ini_min			int			NOT NULL,
	ini_max			int			NOT NULL
);	

CREATE TABLE IF NOT EXISTS WED_roadedge(
	id				integer		PRIMARY KEY,
	layer			integer		NOT NULL,
	subtype			integer		NOT NULL
);

CREATE TABLE IF NOT EXISTS WED_facadenode(
	id				integer		PRIMARY KEY,
	wall			integer		NOT NULL
);

COMMIT TRANSACTION setup2;
