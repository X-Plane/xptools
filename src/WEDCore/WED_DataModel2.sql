BEGIN TRANSACTION setup2;

CREATE TABLE IF NOT EXISTS WED_taxiroute(
	id				integer		PRIMARY KEY,
	oneway			integer		NOT NULL
);

CREATE TABLE IF NOT EXISTS WED_atcflow(
	id				integer		PRIMARY KEY,
	icao			string		NOT NULL,
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

COMMIT TRANSACTION setup2;
