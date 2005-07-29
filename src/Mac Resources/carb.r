#include <types.r>
/*
 *  Permit this Carbon application to launch on OS X
 *
 *  Copyright © 1997-2002 Metrowerks Corporation.  All Rights Reserved.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */


/*----------------------------carb ¥ Carbon on OS X launch information --------------------------*/
type 'carb' {
};


resource 'carb'(0) {
};

resource 'MENU'(128) {
	128,	/* ID */
	textMenuProc,		/* Proc */
	allEnabled,
	enabled,
	apple,
	{
		"About Tools",
		noIcon,
		noKey,
		noMark,
		plain
	};
};

resource 'MBAR' (128) {
	{ 128 }
};

resource 'open' (128) {
	'xtls',
	{ '****' }
};
