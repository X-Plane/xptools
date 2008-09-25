
/*  A Bison parser, made from vrml.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NUMBER	257
#define	FLOAT	258
#define	STRING	259
#define	NAME	260
#define	ANCHOR	261
#define	APPEARANCE	262
#define	AUDIOCLIP	263
#define	BACKGROUND	264
#define	BILLBOARD	265
#define	BOX	266
#define	COLLISION	267
#define	COLOR	268
#define	COLOR_INTERP	269
#define	COORDINATE	270
#define	COORDINATE_INTERP	271
#define	CYLINDER_SENSOR	272
#define	NULL_STRING	273
#define	CONE	274
#define	CUBE	275
#define	CYLINDER	276
#define	DIRECTIONALLIGHT	277
#define	FONTSTYLE	278
#define	ERROR	279
#define	EXTRUSION	280
#define	ELEVATION_GRID	281
#define	FOG	282
#define	INLINE	283
#define	MOVIE_TEXTURE	284
#define	NAVIGATION_INFO	285
#define	PIXEL_TEXTURE	286
#define	GROUP	287
#define	INDEXEDFACESET	288
#define	INDEXEDLINESET	289
#define	S_INFO	290
#define	LOD	291
#define	MATERIAL	292
#define	NORMAL	293
#define	POSITION_INTERP	294
#define	PROXIMITY_SENSOR	295
#define	SCALAR_INTERP	296
#define	SCRIPT	297
#define	SHAPE	298
#define	SOUND	299
#define	SPOTLIGHT	300
#define	SPHERE_SENSOR	301
#define	TEXT	302
#define	TEXTURE_COORDINATE	303
#define	TEXTURE_TRANSFORM	304
#define	TIME_SENSOR	305
#define	SWITCH	306
#define	TOUCH_SENSOR	307
#define	VIEWPOINT	308
#define	VISIBILITY_SENSOR	309
#define	WORLD_INFO	310
#define	NORMAL_INTERP	311
#define	ORIENTATION_INTERP	312
#define	POINTLIGHT	313
#define	POINTSET	314
#define	SPHERE	315
#define	PLANE_SENSOR	316
#define	TRANSFORM	317
#define	S_CHILDREN	318
#define	S_PARAMETER	319
#define	S_URL	320
#define	S_MATERIAL	321
#define	S_TEXTURETRANSFORM	322
#define	S_TEXTURE	323
#define	S_LOOP	324
#define	S_STARTTIME	325
#define	S_STOPTIME	326
#define	S_GROUNDANGLE	327
#define	S_GROUNDCOLOR	328
#define	S_SPEED	329
#define	S_AVATAR_SIZE	330
#define	S_BACKURL	331
#define	S_BOTTOMURL	332
#define	S_FRONTURL	333
#define	S_LEFTURL	334
#define	S_RIGHTURL	335
#define	S_TOPURL	336
#define	S_SKYANGLE	337
#define	S_SKYCOLOR	338
#define	S_AXIS_OF_ROTATION	339
#define	S_COLLIDE	340
#define	S_COLLIDETIME	341
#define	S_PROXY	342
#define	S_SIDE	343
#define	S_AUTO_OFFSET	344
#define	S_DISK_ANGLE	345
#define	S_ENABLED	346
#define	S_MAX_ANGLE	347
#define	S_MIN_ANGLE	348
#define	S_OFFSET	349
#define	S_BBOXSIZE	350
#define	S_BBOXCENTER	351
#define	S_VISIBILITY_LIMIT	352
#define	S_AMBIENT_INTENSITY	353
#define	S_NORMAL	354
#define	S_TEXCOORD	355
#define	S_CCW	356
#define	S_COLOR_PER_VERTEX	357
#define	S_CREASE_ANGLE	358
#define	S_NORMAL_PER_VERTEX	359
#define	S_XDIMENSION	360
#define	S_XSPACING	361
#define	S_ZDIMENSION	362
#define	S_ZSPACING	363
#define	S_BEGIN_CAP	364
#define	S_CROSS_SECTION	365
#define	S_END_CAP	366
#define	S_SPINE	367
#define	S_FOG_TYPE	368
#define	S_VISIBILITY_RANGE	369
#define	S_HORIZONTAL	370
#define	S_JUSTIFY	371
#define	S_LANGUAGE	372
#define	S_LEFT2RIGHT	373
#define	S_TOP2BOTTOM	374
#define	IMAGE_TEXTURE	375
#define	S_SOLID	376
#define	S_KEY	377
#define	S_KEYVALUE	378
#define	S_REPEAT_S	379
#define	S_REPEAT_T	380
#define	S_CONVEX	381
#define	S_BOTTOM	382
#define	S_PICTH	383
#define	S_COORD	384
#define	S_COLOR_INDEX	385
#define	S_COORD_INDEX	386
#define	S_NORMAL_INDEX	387
#define	S_MAX_POSITION	388
#define	S_MIN_POSITION	389
#define	S_ATTENUATION	390
#define	S_APPEARANCE	391
#define	S_GEOMETRY	392
#define	S_DIRECT_OUTPUT	393
#define	S_MUST_EVALUATE	394
#define	S_MAX_BACK	395
#define	S_MIN_BACK	396
#define	S_MAX_FRONT	397
#define	S_MIN_FRONT	398
#define	S_PRIORITY	399
#define	S_SOURCE	400
#define	S_SPATIALIZE	401
#define	S_BERM_WIDTH	402
#define	S_CHOICE	403
#define	S_WHICHCHOICE	404
#define	S_FONTSTYLE	405
#define	S_LENGTH	406
#define	S_MAX_EXTENT	407
#define	S_ROTATION	408
#define	S_SCALE	409
#define	S_CYCLE_INTERVAL	410
#define	S_FIELD_OF_VIEW	411
#define	S_JUMP	412
#define	S_TITLE	413
#define	S_TEXCOORD_INDEX	414
#define	S_HEADLIGHT	415
#define	S_TOP	416
#define	S_BOTTOMRADIUS	417
#define	S_HEIGHT	418
#define	S_POINT	419
#define	S_STRING	420
#define	S_SPACING	421
#define	S_TYPE	422
#define	S_RADIUS	423
#define	S_ON	424
#define	S_INTENSITY	425
#define	S_COLOR	426
#define	S_DIRECTION	427
#define	S_SIZE	428
#define	S_FAMILY	429
#define	S_STYLE	430
#define	S_RANGE	431
#define	S_CENTER	432
#define	S_TRANSLATION	433
#define	S_LEVEL	434
#define	S_DIFFUSECOLOR	435
#define	S_SPECULARCOLOR	436
#define	S_EMISSIVECOLOR	437
#define	S_SHININESS	438
#define	S_TRANSPARENCY	439
#define	S_VECTOR	440
#define	S_POSITION	441
#define	S_ORIENTATION	442
#define	S_LOCATION	443
#define	S_CUTOFFANGLE	444
#define	S_WHICHCHILD	445
#define	S_IMAGE	446
#define	S_SCALEORIENTATION	447
#define	S_DESCRIPTION	448
#define	SFBOOL	449
#define	SFFLOAT	450
#define	SFINT32	451
#define	SFTIME	452
#define	SFROTATION	453
#define	SFNODE	454
#define	SFCOLOR	455
#define	SFIMAGE	456
#define	SFSTRING	457
#define	SFVEC2F	458
#define	SFVEC3F	459
#define	MFBOOL	460
#define	MFFLOAT	461
#define	MFINT32	462
#define	MFTIME	463
#define	MFROTATION	464
#define	MFNODE	465
#define	MFCOLOR	466
#define	MFIMAGE	467
#define	MFSTRING	468
#define	MFVEC2F	469
#define	MFVEC3F	470
#define	FIELD	471
#define	EVENTIN	472
#define	EVENTOUT	473
#define	USE	474
#define	S_VALUE_CHANGED	475


#line 11 "vrml.y"
typedef union {
int		ival;
float	fval;
char	*sval;
} YYSTYPE;
#line 63 "vrml.y"


#include <stdio.h>
#include <stdlib.h>

#ifndef __GNUC__
#define alloca	malloc
#endif

#include "SceneGraph.h"
#include "VRMLNodeType.h"
#include "VRMLSetInfo.h"

float			gColor[3];
float			gVec2f[2];
float			gVec3f[3];
float			gRotation[4];
int				gWidth;
int				gHeight;
int				gComponents;

#define	YYMAXDEPTH	(1024 * 8 * 1000)

int yyerror(char *s);
int yyparse(void);
int yylex(void);

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		1195
#define	YYFLAG		-32768
#define	YYNTBASE	227

#define YYTRANSLATE(x) ((unsigned)(x) <= 475 ? yytranslate[x] : 520)

static const short yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,   224,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
   222,     2,   223,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   225,     2,   226,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
    87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
    97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
   107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
   117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
   127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
   137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
   147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
   157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
   167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
   177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
   187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
   197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
   207,   208,   209,   210,   211,   212,   213,   214,   215,   216,
   217,   218,   219,   220,   221
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     9,    10,    12,    14,    16,    18,
    20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
    40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
    60,    62,    64,    66,    68,    70,    72,    74,    76,    78,
    80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
   100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
   120,   122,   124,   126,   130,   135,   138,   139,   143,   148,
   151,   155,   157,   161,   164,   166,   167,   169,   173,   175,
   179,   182,   184,   185,   187,   191,   193,   197,   200,   202,
   203,   205,   209,   211,   215,   218,   220,   221,   223,   227,
   229,   233,   236,   238,   239,   241,   245,   247,   251,   254,
   256,   257,   259,   263,   265,   269,   272,   274,   275,   277,
   281,   283,   285,   288,   291,   292,   294,   296,   299,   302,
   304,   307,   310,   313,   315,   317,   319,   324,   327,   328,
   331,   334,   337,   340,   343,   346,   349,   352,   355,   358,
   361,   363,   368,   371,   372,   374,   377,   380,   383,   386,
   389,   392,   394,   399,   402,   403,   405,   407,   409,   411,
   413,   415,   417,   419,   421,   423,   426,   429,   432,   435,
   438,   441,   444,   447,   450,   453,   455,   460,   463,   464,
   466,   469,   471,   473,   475,   480,   483,   484,   487,   489,
   494,   497,   498,   503,   506,   509,   510,   512,   514,   517,
   519,   521,   524,   527,   530,   532,   537,   540,   541,   544,
   546,   551,   554,   555,   557,   559,   562,   565,   567,   572,
   575,   576,   579,   582,   585,   588,   590,   595,   598,   599,
   601,   606,   609,   610,   613,   616,   618,   623,   626,   627,
   630,   633,   636,   639,   642,   644,   649,   652,   653,   656,
   659,   662,   665,   668,   671,   673,   678,   681,   682,   685,
   688,   691,   694,   697,   699,   704,   707,   708,   710,   713,
   716,   719,   722,   725,   728,   731,   734,   737,   740,   743,
   746,   749,   752,   755,   758,   761,   764,   767,   769,   774,
   777,   778,   780,   782,   784,   786,   789,   792,   795,   798,
   801,   804,   807,   810,   813,   816,   818,   823,   826,   827,
   830,   833,   836,   838,   843,   846,   847,   849,   852,   855,
   858,   861,   864,   867,   870,   873,   876,   879,   883,   886,
   887,   889,   891,   893,   895,   900,   903,   904,   906,   909,
   912,   915,   917,   922,   925,   926,   928,   930,   932,   934,
   937,   940,   943,   946,   949,   952,   955,   958,   961,   964,
   967,   970,   973,   976,   979,   982,   985,   988,   991,   994,
   997,  1000,  1002,  1007,  1010,  1011,  1014,  1017,  1020,  1023,
  1026,  1029,  1032,  1035,  1038,  1041,  1045,  1048,  1049,  1051,
  1054,  1056,  1058,  1060,  1065,  1068,  1069,  1071,  1073,  1076,
  1079,  1082,  1087,  1089,  1094,  1097,  1098,  1101,  1104,  1107,
  1110,  1113,  1116,  1118,  1123,  1126,  1127,  1129,  1132,  1135,
  1138,  1141,  1144,  1147,  1150,  1152,  1157,  1160,  1161,  1163,
  1165,  1168,  1171,  1174,  1177,  1180,  1182,  1187,  1190,  1191,
  1194,  1196,  1201,  1204,  1205,  1208,  1211,  1214,  1216,  1221,
  1224,  1225,  1228,  1231,  1234,  1236,  1241,  1244,  1245,  1247,
  1252,  1255,  1258,  1260,  1265,  1268,  1269,  1272,  1275,  1278,
  1281,  1284,  1286,  1291,  1294,  1295,  1298,  1301,  1304,  1307,
  1310,  1313,  1316,  1318,  1323,  1326,  1327,  1330,  1333,  1336,
  1339,  1342,  1345,  1347,  1352,  1355,  1356,  1359,  1362,  1365,
  1367,  1372,  1375,  1376,  1379,  1382,  1385,  1387,  1392,  1395,
  1396,  1399,  1402,  1405,  1407,  1412,  1415,  1416,  1418,  1421,
  1424,  1427,  1431,  1435,  1439,  1443,  1447,  1451,  1455,  1459,
  1463,  1467,  1471,  1475,  1479,  1483,  1487,  1491,  1495,  1499,
  1503,  1507,  1511,  1515,  1519,  1523,  1527,  1531,  1535,  1539,
  1543,  1547,  1551,  1555,  1559,  1563,  1567,  1571,  1576,  1581,
  1586,  1591,  1596,  1601,  1607,  1612,  1617,  1622,  1627,  1629,
  1634,  1637,  1638,  1641,  1644,  1647,  1650,  1653,  1656,  1658,
  1663,  1666,  1667,  1670,  1673,  1676,  1679,  1682,  1685,  1688,
  1691,  1694,  1697,  1700,  1703,  1706,  1708,  1713,  1716,  1717,
  1720,  1722,  1727,  1730,  1731,  1734,  1737,  1740,  1742,  1747,
  1750,  1751,  1754,  1757,  1760,  1763,  1766,  1769,  1772,  1775,
  1778,  1781,  1783,  1788,  1791,  1792,  1794,  1797,  1802,  1805,
  1807,  1812,  1815,  1816,  1818,  1820,  1823,  1826,  1829,  1832,
  1835,  1838,  1840,  1845,  1848,  1849,  1852,  1854,  1859,  1862,
  1863,  1866,  1869,  1872,  1875,  1877,  1882,  1885,  1886,  1889,
  1892,  1895,  1898,  1901,  1903,  1908,  1911,  1912,  1915,  1917,
  1922,  1925,  1926,  1928,  1931,  1934,  1937,  1940,  1943,  1945,
  1947,  1949,  1954,  1957,  1958,  1961,  1964,  1967,  1970,  1973,
  1975,  1980,  1983,  1984,  1987,  1990,  1993,  1995,  2000,  2003,
  2004,  2006,  2009,  2012,  2014
};

static const short yyrhs[] = {   228,
     0,     1,     0,    25,     0,   236,   228,     0,     0,   270,
     0,   297,     0,   308,     0,   367,     0,   389,     0,   395,
     0,   481,     0,   507,     0,   318,     0,   329,     0,   418,
     0,   422,     0,   443,     0,   451,     0,   337,     0,   431,
     0,   472,     0,   447,     0,   499,     0,   503,     0,   513,
     0,   301,     0,   322,     0,   333,     0,   346,     0,   354,
     0,   380,     0,   384,     0,   439,     0,   468,     0,   487,
     0,   341,     0,   476,     0,   435,     0,   279,     0,   233,
     0,   456,     0,   460,     0,   464,     0,   519,     0,   293,
     0,   358,     0,   410,     0,   511,     0,   234,     0,   235,
     0,   363,     0,   230,     0,   231,     0,   229,     0,   220,
     0,     3,     0,     3,     0,     5,     0,     4,     0,     3,
     0,     4,     0,     3,     0,   240,   240,   240,     0,   240,
   240,   240,   240,     0,   237,   244,     0,     0,     3,     3,
     3,     0,   222,     0,   244,   223,     0,   240,   240,     0,
   240,   240,   240,     0,   242,     0,   242,   224,   247,     0,
   242,   247,     0,   224,     0,     0,   242,     0,   222,   247,
   223,     0,   237,     0,   237,   224,   249,     0,   237,   249,
     0,   224,     0,     0,   237,     0,   222,   249,   223,     0,
   240,     0,   240,   224,   251,     0,   240,   251,     0,   224,
     0,     0,   240,     0,   222,   251,   223,     0,   239,     0,
   239,   224,   253,     0,   239,   253,     0,   224,     0,     0,
   239,     0,   222,   253,   223,     0,   245,     0,   245,   224,
   255,     0,   245,   255,     0,   224,     0,     0,   245,     0,
   222,   255,   223,     0,   246,     0,   246,   224,   257,     0,
   246,   257,     0,   224,     0,     0,   246,     0,   222,   257,
   223,     0,   243,     0,   243,   224,   259,     0,   243,   259,
     0,   224,     0,     0,   243,     0,   222,   259,   223,     0,
   225,     0,   226,     0,   226,   224,     0,   268,   263,     0,
     0,    65,     0,    66,     0,    97,   246,     0,    96,   246,
     0,   303,     0,   194,   239,     0,   264,   254,     0,   265,
   254,     0,   266,     0,   267,     0,     7,     0,   269,   261,
   263,   262,     0,   272,   271,     0,     0,    67,    19,     0,
    67,   399,     0,    67,   220,     0,    69,    19,     0,    69,
   372,     0,    69,   404,     0,    69,   427,     0,    69,   220,
     0,    68,    19,     0,    68,   495,     0,    68,   220,     0,
     8,     0,   273,   261,   271,   262,     0,   277,   275,     0,
     0,    66,     0,   194,   239,     0,    70,   238,     0,   129,
   240,     0,    71,   241,     0,    72,   241,     0,   276,   254,
     0,     9,     0,   278,   261,   275,   262,     0,   291,   280,
     0,     0,    77,     0,    78,     0,    79,     0,    80,     0,
    81,     0,    82,     0,    73,     0,    74,     0,    83,     0,
    84,     0,   287,   252,     0,   288,   248,     0,   281,   254,
     0,   282,   254,     0,   283,   254,     0,   284,   254,     0,
   285,   254,     0,   286,   254,     0,   289,   252,     0,   290,
   248,     0,    10,     0,   292,   261,   280,   262,     0,   295,
   294,     0,     0,   303,     0,    85,   246,     0,   266,     0,
   267,     0,    11,     0,   296,   261,   294,   262,     0,   299,
   298,     0,     0,   174,   246,     0,    12,     0,   300,   261,
   298,   262,     0,   236,   302,     0,     0,    64,   222,   302,
   223,     0,    64,   236,     0,   306,   304,     0,     0,    88,
     0,   303,     0,    86,   238,     0,   266,     0,   267,     0,
    88,   220,     0,    88,    19,     0,   305,   236,     0,    13,
     0,   307,   261,   304,   262,     0,   310,   309,     0,     0,
   172,   248,     0,    14,     0,   311,   261,   309,   262,     0,
   316,   313,     0,     0,   123,     0,   124,     0,   314,   252,
     0,   315,   248,     0,    15,     0,   317,   261,   313,   262,
     0,   320,   319,     0,     0,    89,   238,     0,   128,   238,
     0,   163,   240,     0,   164,   240,     0,    20,     0,   321,
   261,   319,   262,     0,   165,   258,     0,     0,    16,     0,
   324,   261,   323,   262,     0,   327,   326,     0,     0,   314,
   252,     0,   315,   258,     0,    17,     0,   328,   261,   326,
   262,     0,   331,   330,     0,     0,    89,   238,     0,   128,
   238,     0,   162,   238,     0,   169,   240,     0,   164,   240,
     0,    22,     0,   332,   261,   330,   262,     0,   335,   334,
     0,     0,    90,   238,     0,    91,   240,     0,    92,   238,
     0,    93,   240,     0,    94,   240,     0,    95,   240,     0,
    18,     0,   336,   261,   334,   262,     0,   339,   338,     0,
     0,   170,   238,     0,   171,   240,     0,   172,   242,     0,
   173,   246,     0,    99,   240,     0,    23,     0,   340,   261,
   338,   262,     0,   344,   342,     0,     0,   164,     0,   172,
    19,     0,   172,   312,     0,   172,   220,     0,   100,    19,
     0,   100,   414,     0,   100,   220,     0,   101,    19,     0,
   101,   491,     0,   101,   220,     0,   343,   252,     0,   102,
   238,     0,   104,   240,     0,   122,   238,     0,   103,   238,
     0,   105,   238,     0,   106,   237,     0,   107,   240,     0,
   108,   237,     0,   109,   240,     0,    27,     0,   345,   261,
   342,   262,     0,   352,   347,     0,     0,   111,     0,   188,
     0,   155,     0,   113,     0,   110,   238,     0,   102,   238,
     0,   127,   238,     0,   104,   240,     0,   122,   238,     0,
   348,   256,     0,   112,   238,     0,   349,   260,     0,   350,
   256,     0,   351,   258,     0,    26,     0,   353,   261,   347,
   262,     0,   356,   355,     0,     0,   172,   242,     0,   114,
   239,     0,   115,   240,     0,    28,     0,   357,   261,   355,
   262,     0,   361,   359,     0,     0,   117,     0,   175,   239,
     0,   116,   238,     0,   360,   254,     0,   118,   239,     0,
   119,   238,     0,   174,   240,     0,   167,   240,     0,   176,
   239,     0,   120,   238,     0,    24,   261,     0,   362,   359,
   262,     0,   365,   364,     0,     0,   303,     0,   266,     0,
   267,     0,    33,     0,   366,   261,   364,   262,     0,   370,
   368,     0,     0,    66,     0,   369,   254,     0,   125,   238,
     0,   126,   238,     0,   121,     0,   371,   261,   368,   262,
     0,   378,   373,     0,     0,   131,     0,   132,     0,   133,
     0,   160,     0,   172,    19,     0,   172,   312,     0,   172,
   220,     0,   130,    19,     0,   130,   325,     0,   130,   220,
     0,   100,    19,     0,   100,   414,     0,   100,   220,     0,
   101,    19,     0,   101,   491,     0,   101,   220,     0,   102,
   238,     0,   127,   238,     0,   122,   238,     0,   104,   240,
     0,   374,   250,     0,   103,   238,     0,   375,   250,     0,
   376,   250,     0,   377,   250,     0,   105,   238,     0,    34,
     0,   379,   261,   373,   262,     0,   382,   381,     0,     0,
   172,    19,     0,   172,   312,     0,   172,   220,     0,   130,
    19,     0,   130,   325,     0,   130,   220,     0,   103,   238,
     0,   374,   250,     0,   375,   250,     0,    35,   261,     0,
   383,   381,   262,     0,   387,   385,     0,     0,    66,     0,
   386,   254,     0,   266,     0,   267,     0,    29,     0,   388,
   261,   385,   262,     0,   393,   390,     0,     0,   177,     0,
   180,     0,   391,   252,     0,   178,   246,     0,   392,   236,
     0,   392,   222,   228,   223,     0,    37,     0,   394,   261,
   390,   262,     0,   397,   396,     0,     0,    99,   240,     0,
   181,   242,     0,   183,   242,     0,   184,   240,     0,   182,
   242,     0,   185,   240,     0,    38,     0,   398,   261,   396,
   262,     0,   402,   400,     0,     0,    66,     0,    70,   238,
     0,    75,   240,     0,    71,   241,     0,    72,   241,     0,
   401,   254,     0,   125,   238,     0,   126,   238,     0,    30,
     0,   403,   261,   400,   262,     0,   408,   405,     0,     0,
    76,     0,   168,     0,   406,   252,     0,   161,   238,     0,
    75,   240,     0,   407,   254,     0,    98,   240,     0,    31,
     0,   409,   261,   405,   262,     0,   412,   411,     0,     0,
   186,   258,     0,    39,     0,   413,   261,   411,   262,     0,
   416,   415,     0,     0,   314,   252,     0,   315,   258,     0,
   221,   246,     0,    57,     0,   417,   261,   415,   262,     0,
   420,   419,     0,     0,   314,   252,     0,   315,   260,     0,
   221,   243,     0,    58,     0,   421,   261,   419,   262,     0,
   425,   423,     0,     0,   192,     0,   424,   222,   244,   223,
     0,   125,   238,     0,   126,   238,     0,    32,     0,   426,
   261,   423,   262,     0,   429,   428,     0,     0,    90,   238,
     0,    92,   238,     0,   134,   245,     0,   135,   245,     0,
    95,   246,     0,    62,     0,   430,   261,   428,   262,     0,
   433,   432,     0,     0,    99,   240,     0,   136,   246,     0,
   172,   242,     0,   171,   240,     0,   189,   246,     0,   170,
   238,     0,   169,   240,     0,    59,     0,   434,   261,   432,
   262,     0,   437,   436,     0,     0,   172,    19,     0,   172,
   312,     0,   172,   220,     0,   130,    19,     0,   130,   325,
     0,   130,   220,     0,    60,     0,   438,   261,   436,   262,
     0,   441,   440,     0,     0,   314,   252,     0,   315,   258,
     0,   221,   246,     0,    40,     0,   442,   261,   440,   262,
     0,   445,   444,     0,     0,   178,   246,     0,   174,   246,
     0,    92,   238,     0,    41,     0,   446,   261,   444,   262,
     0,   449,   448,     0,     0,   314,   252,     0,   315,   252,
     0,   221,   245,     0,    42,     0,   450,   261,   448,   262,
     0,   454,   452,     0,     0,    66,     0,   453,   254,     0,
   139,   238,     0,   140,   238,     0,   218,   195,     6,     0,
   218,   196,     6,     0,   218,   197,     6,     0,   218,   198,
     6,     0,   218,   199,     6,     0,   218,   201,     6,     0,
   218,   202,     6,     0,   218,   203,     6,     0,   218,   204,
     6,     0,   218,   205,     6,     0,   218,   207,     6,     0,
   218,   208,     6,     0,   218,   209,     6,     0,   218,   210,
     6,     0,   218,   212,     6,     0,   218,   214,     6,     0,
   218,   215,     6,     0,   218,   216,     6,     0,   219,   195,
     6,     0,   219,   196,     6,     0,   219,   197,     6,     0,
   219,   198,     6,     0,   219,   199,     6,     0,   219,   201,
     6,     0,   219,   202,     6,     0,   219,   203,     6,     0,
   219,   204,     6,     0,   219,   205,     6,     0,   219,   207,
     6,     0,   219,   208,     6,     0,   219,   209,     6,     0,
   219,   210,     6,     0,   219,   212,     6,     0,   219,   214,
     6,     0,   219,   215,     6,     0,   219,   216,     6,     0,
   217,   195,     6,   238,     0,   217,   196,     6,   240,     0,
   217,   197,     6,   237,     0,   217,   198,     6,   241,     0,
   217,   199,     6,   243,     0,   217,   200,     6,    19,     0,
   217,   200,     6,   220,     6,     0,   217,   201,     6,   242,
     0,   217,   203,     6,   239,     0,   217,   204,     6,   245,
     0,   217,   205,     6,   246,     0,    43,     0,   455,   261,
   452,   262,     0,   458,   457,     0,     0,   137,    19,     0,
   137,   274,     0,   137,   220,     0,   138,    19,     0,   138,
   232,     0,   138,   220,     0,    44,     0,   459,   261,   457,
   262,     0,   462,   461,     0,     0,   173,   246,     0,   171,
   240,     0,   189,   246,     0,   141,   240,     0,   143,   240,
     0,   142,   240,     0,   144,   240,     0,   145,   240,     0,
   146,    19,     0,   146,   279,     0,   146,   404,     0,   146,
   220,     0,   147,   238,     0,    45,     0,   463,   261,   461,
   262,     0,   466,   465,     0,     0,   169,   240,     0,    61,
     0,   467,   261,   465,   262,     0,   470,   469,     0,     0,
    90,   238,     0,    92,   238,     0,    95,   243,     0,    47,
     0,   471,   261,   469,   262,     0,   474,   473,     0,     0,
    99,   240,     0,   136,   246,     0,   148,   240,     0,   172,
   242,     0,   190,   240,     0,   173,   246,     0,   171,   240,
     0,   189,   246,     0,   170,   238,     0,   169,   240,     0,
    46,     0,   475,   261,   473,   262,     0,   479,   477,     0,
     0,   149,     0,   478,   236,     0,   478,   222,   228,   223,
     0,   150,   237,     0,    52,     0,   480,   261,   477,   262,
     0,   485,   482,     0,     0,   166,     0,   152,     0,   483,
   254,     0,   151,    19,     0,   151,   363,     0,   151,   220,
     0,   484,   252,     0,   153,   240,     0,    48,     0,   486,
   261,   482,   262,     0,   489,   488,     0,     0,   165,   256,
     0,    49,     0,   490,   261,   488,   262,     0,   493,   492,
     0,     0,   178,   245,     0,   154,   240,     0,   155,   245,
     0,   179,   245,     0,    50,     0,   494,   261,   492,   262,
     0,   497,   496,     0,     0,   156,   241,     0,    92,   238,
     0,    70,   238,     0,    71,   241,     0,    72,   241,     0,
    51,     0,   498,   261,   496,   262,     0,   501,   500,     0,
     0,    92,   238,     0,    53,     0,   502,   261,   500,   262,
     0,   505,   504,     0,     0,   303,     0,   178,   246,     0,
   154,   243,     0,   155,   246,     0,   193,   243,     0,   179,
   246,     0,   266,     0,   267,     0,    63,     0,   506,   261,
   504,   262,     0,   509,   508,     0,     0,   157,   240,     0,
   158,   238,     0,   188,   243,     0,   187,   246,     0,   194,
   239,     0,    54,     0,   510,   261,   508,   262,     0,   513,
   512,     0,     0,   178,   246,     0,    92,   238,     0,   174,
   246,     0,    55,     0,   514,   261,   512,   262,     0,   517,
   515,     0,     0,    36,     0,   516,   254,     0,   159,   239,
     0,    56,     0,   518,   261,   515,   262,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    95,    96,    97,   101,   102,   106,   107,   108,   109,   110,
   111,   112,   113,   117,   118,   119,   120,   121,   122,   126,
   127,   128,   129,   130,   131,   132,   136,   137,   138,   139,
   140,   141,   142,   143,   144,   145,   149,   150,   151,   155,
   156,   157,   158,   159,   160,   164,   165,   166,   167,   171,
   172,   173,   174,   175,   176,   177,   181,   188,   192,   199,
   203,   211,   212,   216,   226,   237,   238,   243,   252,   256,
   265,   275,   276,   277,   278,   279,   283,   284,   288,   289,
   290,   291,   292,   296,   297,   302,   303,   304,   305,   306,
   310,   311,   315,   316,   317,   318,   319,   323,   324,   328,
   329,   330,   331,   332,   336,   337,   341,   342,   343,   344,
   345,   349,   350,   354,   355,   356,   357,   358,   362,   363,
   367,   371,   372,   382,   383,   387,   394,   401,   408,   415,
   416,   421,   425,   429,   430,   434,   444,   459,   460,   464,
   465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
   478,   488,   503,   504,   508,   515,   519,   523,   527,   531,
   535,   542,   551,   566,   567,   571,   578,   585,   592,   599,
   606,   613,   620,   627,   634,   641,   645,   649,   653,   657,
   661,   665,   669,   673,   677,   684,   694,   709,   710,   714,
   715,   719,   720,   724,   734,   749,   750,   754,   761,   771,
   786,   787,   791,   792,   802,   803,   807,   814,   815,   819,
   820,   821,   822,   823,   830,   840,   855,   856,   860,   864,
   874,   889,   890,   894,   901,   908,   912,   919,   929,   944,
   945,   949,   953,   957,   961,   968,   978,   993,   994,   998,
  1008,  1023,  1024,  1028,  1032,  1039,  1049,  1064,  1065,  1069,
  1073,  1077,  1081,  1085,  1092,  1102,  1117,  1118,  1122,  1126,
  1130,  1134,  1138,  1142,  1150,  1160,  1175,  1176,  1180,  1184,
  1188,  1192,  1196,  1203,  1213,  1228,  1229,  1233,  1241,  1242,
  1243,  1244,  1245,  1246,  1247,  1248,  1249,  1250,  1254,  1258,
  1262,  1266,  1270,  1274,  1278,  1282,  1286,  1293,  1303,  1318,
  1319,  1323,  1330,  1337,  1344,  1351,  1355,  1359,  1363,  1367,
  1371,  1375,  1379,  1383,  1387,  1394,  1404,  1419,  1420,  1424,
  1428,  1432,  1439,  1449,  1464,  1465,  1469,  1476,  1480,  1484,
  1488,  1492,  1496,  1500,  1504,  1508,  1515,  1525,  1540,  1541,
  1545,  1546,  1547,  1551,  1561,  1576,  1577,  1581,  1588,  1592,
  1596,  1603,  1613,  1628,  1629,  1633,  1640,  1647,  1654,  1661,
  1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,
  1672,  1673,  1677,  1681,  1685,  1689,  1693,  1697,  1701,  1705,
  1709,  1716,  1726,  1741,  1742,  1746,  1747,  1748,  1749,  1750,
  1751,  1752,  1756,  1760,  1767,  1777,  1792,  1793,  1797,  1804,
  1808,  1809,  1813,  1823,  1838,  1839,  1843,  1851,  1858,  1862,
  1866,  1870,  1877,  1887,  1902,  1903,  1907,  1911,  1915,  1919,
  1923,  1927,  1933,  1943,  1958,  1959,  1963,  1970,  1974,  1978,
  1982,  1986,  1990,  1994,  2001,  2011,  2026,  2027,  2031,  2038,
  2045,  2049,  2053,  2057,  2061,  2068,  2078,  2093,  2094,  2098,
  2102,  2112,  2127,  2128,  2132,  2136,  2140,  2146,  2156,  2171,
  2172,  2176,  2180,  2184,  2190,  2200,  2215,  2216,  2220,  2227,
  2231,  2235,  2242,  2252,  2267,  2268,  2272,  2276,  2280,  2284,
  2288,  2295,  2305,  2321,  2322,  2326,  2330,  2334,  2338,  2342,
  2346,  2350,  2357,  2367,  2382,  2383,  2387,  2388,  2389,  2390,
  2391,  2392,  2397,  2407,  2421,  2422,  2426,  2430,  2434,  2440,
  2450,  2465,  2466,  2470,  2474,  2478,  2485,  2495,  2510,  2511,
  2515,  2519,  2523,  2529,  2539,  2554,  2555,  2559,  2566,  2570,
  2574,  2583,  2589,  2595,  2601,  2607,  2621,  2627,  2633,  2639,
  2645,  2656,  2662,  2668,  2674,  2688,  2694,  2700,  2706,  2717,
  2723,  2729,  2735,  2741,  2755,  2761,  2767,  2773,  2779,  2790,
  2796,  2802,  2808,  2822,  2828,  2834,  2840,  2851,  2857,  2863,
  2869,  2875,  2882,  2889,  2897,  2911,  2917,  2923,  2933,  2943,
  2959,  2960,  2964,  2965,  2966,  2967,  2968,  2969,  2973,  2983,
  2998,  2999,  3003,  3007,  3011,  3015,  3019,  3023,  3027,  3031,
  3035,  3036,  3037,  3038,  3039,  3046,  3056,  3071,  3072,  3076,
  3083,  3093,  3108,  3109,  3113,  3117,  3121,  3128,  3138,  3153,
  3154,  3158,  3162,  3166,  3170,  3174,  3178,  3182,  3186,  3190,
  3194,  3201,  3211,  3226,  3227,  3231,  3238,  3242,  3246,  3254,
  3264,  3279,  3280,  3284,  3291,  3298,  3302,  3303,  3304,  3305,
  3309,  3317,  3327,  3342,  3343,  3347,  3352,  3362,  3377,  3378,
  3382,  3386,  3390,  3394,  3402,  3412,  3427,  3428,  3432,  3436,
  3440,  3444,  3448,  3456,  3466,  3481,  3482,  3486,  3493,  3503,
  3518,  3519,  3523,  3524,  3528,  3532,  3536,  3540,  3544,  3545,
  3549,  3559,  3574,  3575,  3579,  3583,  3587,  3591,  3595,  3602,
  3612,  3627,  3628,  3632,  3636,  3640,  3647,  3657,  3672,  3673,
  3677,  3684,  3688,  3695,  3705
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NUMBER",
"FLOAT","STRING","NAME","ANCHOR","APPEARANCE","AUDIOCLIP","BACKGROUND","BILLBOARD",
"BOX","COLLISION","COLOR","COLOR_INTERP","COORDINATE","COORDINATE_INTERP","CYLINDER_SENSOR",
"NULL_STRING","CONE","CUBE","CYLINDER","DIRECTIONALLIGHT","FONTSTYLE","ERROR",
"EXTRUSION","ELEVATION_GRID","FOG","INLINE","MOVIE_TEXTURE","NAVIGATION_INFO",
"PIXEL_TEXTURE","GROUP","INDEXEDFACESET","INDEXEDLINESET","S_INFO","LOD","MATERIAL",
"NORMAL","POSITION_INTERP","PROXIMITY_SENSOR","SCALAR_INTERP","SCRIPT","SHAPE",
"SOUND","SPOTLIGHT","SPHERE_SENSOR","TEXT","TEXTURE_COORDINATE","TEXTURE_TRANSFORM",
"TIME_SENSOR","SWITCH","TOUCH_SENSOR","VIEWPOINT","VISIBILITY_SENSOR","WORLD_INFO",
"NORMAL_INTERP","ORIENTATION_INTERP","POINTLIGHT","POINTSET","SPHERE","PLANE_SENSOR",
"TRANSFORM","S_CHILDREN","S_PARAMETER","S_URL","S_MATERIAL","S_TEXTURETRANSFORM",
"S_TEXTURE","S_LOOP","S_STARTTIME","S_STOPTIME","S_GROUNDANGLE","S_GROUNDCOLOR",
"S_SPEED","S_AVATAR_SIZE","S_BACKURL","S_BOTTOMURL","S_FRONTURL","S_LEFTURL",
"S_RIGHTURL","S_TOPURL","S_SKYANGLE","S_SKYCOLOR","S_AXIS_OF_ROTATION","S_COLLIDE",
"S_COLLIDETIME","S_PROXY","S_SIDE","S_AUTO_OFFSET","S_DISK_ANGLE","S_ENABLED",
"S_MAX_ANGLE","S_MIN_ANGLE","S_OFFSET","S_BBOXSIZE","S_BBOXCENTER","S_VISIBILITY_LIMIT",
"S_AMBIENT_INTENSITY","S_NORMAL","S_TEXCOORD","S_CCW","S_COLOR_PER_VERTEX","S_CREASE_ANGLE",
"S_NORMAL_PER_VERTEX","S_XDIMENSION","S_XSPACING","S_ZDIMENSION","S_ZSPACING",
"S_BEGIN_CAP","S_CROSS_SECTION","S_END_CAP","S_SPINE","S_FOG_TYPE","S_VISIBILITY_RANGE",
"S_HORIZONTAL","S_JUSTIFY","S_LANGUAGE","S_LEFT2RIGHT","S_TOP2BOTTOM","IMAGE_TEXTURE",
"S_SOLID","S_KEY","S_KEYVALUE","S_REPEAT_S","S_REPEAT_T","S_CONVEX","S_BOTTOM",
"S_PICTH","S_COORD","S_COLOR_INDEX","S_COORD_INDEX","S_NORMAL_INDEX","S_MAX_POSITION",
"S_MIN_POSITION","S_ATTENUATION","S_APPEARANCE","S_GEOMETRY","S_DIRECT_OUTPUT",
"S_MUST_EVALUATE","S_MAX_BACK","S_MIN_BACK","S_MAX_FRONT","S_MIN_FRONT","S_PRIORITY",
"S_SOURCE","S_SPATIALIZE","S_BERM_WIDTH","S_CHOICE","S_WHICHCHOICE","S_FONTSTYLE",
"S_LENGTH","S_MAX_EXTENT","S_ROTATION","S_SCALE","S_CYCLE_INTERVAL","S_FIELD_OF_VIEW",
"S_JUMP","S_TITLE","S_TEXCOORD_INDEX","S_HEADLIGHT","S_TOP","S_BOTTOMRADIUS",
"S_HEIGHT","S_POINT","S_STRING","S_SPACING","S_TYPE","S_RADIUS","S_ON","S_INTENSITY",
"S_COLOR","S_DIRECTION","S_SIZE","S_FAMILY","S_STYLE","S_RANGE","S_CENTER","S_TRANSLATION",
"S_LEVEL","S_DIFFUSECOLOR","S_SPECULARCOLOR","S_EMISSIVECOLOR","S_SHININESS",
"S_TRANSPARENCY","S_VECTOR","S_POSITION","S_ORIENTATION","S_LOCATION","S_CUTOFFANGLE",
"S_WHICHCHILD","S_IMAGE","S_SCALEORIENTATION","S_DESCRIPTION","SFBOOL","SFFLOAT",
"SFINT32","SFTIME","SFROTATION","SFNODE","SFCOLOR","SFIMAGE","SFSTRING","SFVEC2F",
"SFVEC3F","MFBOOL","MFFLOAT","MFINT32","MFTIME","MFROTATION","MFNODE","MFCOLOR",
"MFIMAGE","MFSTRING","MFVEC2F","MFVEC3F","FIELD","EVENTIN","EVENTOUT","USE",
"S_VALUE_CHANGED","'['","']'","','","'{'","'}'","Vrml","VrmlNodes","GroupingNode",
"InterpolatorNode","SensorNode","GeometryNode","LightNode","CommonNode","BindableNode",
"SFNode","SFInt32","SFBool","SFString","SFFloat","SFTime","SFColor","SFRotation",
"SFImageList","SFVec2f","SFVec3f","SFColorList","MFColor","SFInt32List","MFInt32",
"SFFloatList","MFFloat","SFStringList","MFString","SFVec2fList","MFVec2f","SFVec3fList",
"MFVec3f","SFRotationList","MFRotation","NodeBegin","NodeEnd","AnchorElements",
"AnchorElementParameterBegin","AnchorElementURLBegin","bboxCenter","bboxSize",
"AnchorElement","AnchorBegin","Anchor","AppearanceNodes","AppearanceNode","AppearanceBegin",
"Appearance","AudioClipElements","AudioClipURL","AudioClipElement","AudioClipBegin",
"AudioClip","BackGroundElements","BackGroundBackURL","BackGroundBottomURL","BackGroundFrontURL",
"BackGroundLeftURL","BackGroundRightURL","BackGroundTopURL","BackGroundGroundAngle",
"BackGroundGroundColor","BackGroundSkyAngle","BackGroundSkyColor","BackGroundElement",
"BackgroundBegin","Background","BillboardElements","BillboardElement","BillboardBegin",
"Billboard","BoxElements","BoxElement","BoxBegin","Box","childrenElements","children",
"CollisionElements","CollisionElementProxyBegin","CollisionElement","CollisionBegin",
"Collision","ColorElements","ColorElement","ColorBegin","Color","ColorInterpElements",
"InterpolateKey","InterporlateKeyValue","ColorInterpElement","ColorInterpBegin",
"ColorInterp","ConeElements","ConeElement","ConeBegin","Cone","CoordinateElements",
"CoordinateBegin","Coordinate","CoordinateInterpElements","CoordinateInterpElement",
"CoordinateInterpBegin","CoordinateInterp","CylinderElements","CylinderElement",
"CylinderBegin","Cylinder","CylinderSensorElements","CylinderSensorElement",
"CylinderSensorBegin","CylinderSensor","DirLightElements","DirLightElement",
"DirLightBegin","DirLight","ElevationGridElements","ElevationGridHeight","ElevationGridElement",
"ElevationGridBegin","ElevationGrid","ExtrusionElements","ExtrusionCrossSection",
"ExtrusionOrientation","ExtrusionScale","ExtrusionSpine","ExtrusionElement",
"ExtrusionBegin","Extrusion","FogElements","FogElement","FogBegin","Fog","FontStyleElements",
"FontStyleJustify","FontStyleElement","FontStyleBegin","FontStyle","GroupElements",
"GroupElement","GroupBegin","Group","ImgTexElements","ImgTexURL","ImgTexElement",
"ImageTextureBegin","ImageTexture","IdxFacesetElements","ColorIndex","CoordIndex",
"NormalIndex","TextureIndex","IdxFacesetElement","IdxFacesetBegin","IdxFaceset",
"IdxLinesetElements","IdxLinesetElement","IdxLinesetBegin","IdxLineset","InlineElements",
"InlineURL","InlineElement","InlineBegin","Inline","LodElements","LodRange",
"LodLevel","LodElement","LodBegin","Lod","MaterialElements","MaterialElement",
"MaterialBegin","Material","MovieTextureElements","MovieTextureURL","MovieTextureElement",
"MovieTextureBegin","MovieTexture","NavigationInfoElements","NavigationInfoAvatarSize",
"NavigationInfoType","NavigationInfoElement","NavigationInfoBegin","NavigationInfo",
"NormalElements","NormalElement","NormalBegin","Normal","NormalInterpElements",
"NormalInterpElement","NormalInterpBegin","NormalInterp","OrientationInterpElements",
"OrientationInterpElement","OrientationInterpBegin","OrientationInterp","PixelTextureElements",
"PixelTextureImage","PixelTextureElement","PixelTextureBegin","PixelTexture",
"PlaneSensorElements","PlaneSensorElement","PlaneSensorBegin","PlaneSensor",
"PointLightNodes","PointLightNode","PointLightBegin","PointLight","PointsetElements",
"PointsetElement","PointsetBegin","Pointset","PositionInterpElements","PositionInterpElement",
"PositionInterpBegin","PositionInterp","ProximitySensorElements","ProximitySensorElement",
"ProximitySensorBegin","ProximitySensor","ScalarInterpElements","ScalarInterpElement",
"ScalarInterpBegin","ScalarInterp","ScriptElements","ScriptURL","ScriptElement",
"ScriptBegin","Script","SharpElements","SharpElement","ShapeBegin","Shape","SoundElements",
"SoundElement","SoundBegin","Sound","SphereElements","SphereElement","SphereBegin",
"Sphere","SphereSensorElements","SphereSensorElement","SphereSensorBegin","SphereSensor",
"SpotLightElements","SpotLightElement","SpotLightBegin","SpotLight","SwitchElements",
"SwitchChoice","SwitchElement","SwitchBegin","Switch","TextElements","TextString",
"TextLength","TextElement","TextBegin","Text","TexCoordElements","TexCoordElement",
"TexCoordBegin","TexCoordinate","TextureTransformElements","TextureTransformElement",
"TexTransformBegin","TexTransform","TimeSensorElements","TimeSensorElement",
"TimeSensorBegin","TimeSensor","TouchSensorElements","TouchSensorElement","TouchSensorBegin",
"TouchSensor","TransformElements","TransformElement","TransformBegin","Transform",
"ViewpointElements","ViewpointElement","ViewpointBegin","Viewpoint","VisibilitySensors",
"VisibilitySensor","VisibilitySensorBegine","WorldInfoElements","WorldInfoInfo",
"WorldInfoElement","WorldInfoBegin","WorldInfo", NULL
};
#endif

static const short yyr1[] = {     0,
   227,   227,   227,   228,   228,   229,   229,   229,   229,   229,
   229,   229,   229,   230,   230,   230,   230,   230,   230,   231,
   231,   231,   231,   231,   231,   231,   232,   232,   232,   232,
   232,   232,   232,   232,   232,   232,   233,   233,   233,   234,
   234,   234,   234,   234,   234,   235,   235,   235,   235,   236,
   236,   236,   236,   236,   236,   236,   237,   238,   239,   240,
   240,   241,   241,   242,   243,   244,   244,    -1,    -1,   245,
   246,   247,   247,   247,   247,   247,   248,   248,   249,   249,
   249,   249,   249,   250,   250,   251,   251,   251,   251,   251,
   252,   252,   253,   253,   253,   253,   253,   254,   254,   255,
   255,   255,   255,   255,   256,   256,   257,   257,   257,   257,
   257,   258,   258,   259,   259,   259,   259,   259,   260,   260,
   261,   262,   262,   263,   263,   264,   265,   266,   267,   268,
   268,   268,   268,   268,   268,   269,   270,   271,   271,   272,
   272,   272,   272,   272,   272,   272,   272,   272,   272,   272,
   273,   274,   275,   275,   276,   277,   277,   277,   277,   277,
   277,   278,   279,   280,   280,   281,   282,   283,   284,   285,
   286,   287,   288,   289,   290,   291,   291,   291,   291,   291,
   291,   291,   291,   291,   291,   292,   293,   294,   294,   295,
   295,   295,   295,   296,   297,   298,   298,   299,   300,   301,
   302,   302,   303,   303,   304,   304,   305,   306,   306,   306,
   306,   306,   306,   306,   307,   308,   309,   309,   310,   311,
   312,   313,   313,   314,   315,   316,   316,   317,   318,   319,
   319,   320,   320,   320,   320,   321,   322,   323,   323,   324,
   325,   326,   326,   327,   327,   328,   329,   330,   330,   331,
   331,   331,   331,   331,   332,   333,   334,   334,   335,   335,
   335,   335,   335,   335,   336,   337,   338,   338,   339,   339,
   339,   339,   339,   340,   341,   342,   342,   343,   344,   344,
   344,   344,   344,   344,   344,   344,   344,   344,   344,   344,
   344,   344,   344,   344,   344,   344,   344,   345,   346,   347,
   347,   348,   349,   350,   351,   352,   352,   352,   352,   352,
   352,   352,   352,   352,   352,   353,   354,   355,   355,   356,
   356,   356,   357,   358,   359,   359,   360,   361,   361,   361,
   361,   361,   361,   361,   361,   361,   362,   363,   364,   364,
   365,   365,   365,   366,   367,   368,   368,   369,   370,   370,
   370,   371,   372,   373,   373,   374,   375,   376,   377,   378,
   378,   378,   378,   378,   378,   378,   378,   378,   378,   378,
   378,   378,   378,   378,   378,   378,   378,   378,   378,   378,
   378,   379,   380,   381,   381,   382,   382,   382,   382,   382,
   382,   382,   382,   382,   383,   384,   385,   385,   386,   387,
   387,   387,   388,   389,   390,   390,   391,   392,   393,   393,
   393,   393,   394,   395,   396,   396,   397,   397,   397,   397,
   397,   397,   398,   399,   400,   400,   401,   402,   402,   402,
   402,   402,   402,   402,   403,   404,   405,   405,   406,   407,
   408,   408,   408,   408,   408,   409,   410,   411,   411,   412,
   413,   414,   415,   415,   416,   416,   416,   417,   418,   419,
   419,   420,   420,   420,   421,   422,   423,   423,   424,   425,
   425,   425,   426,   427,   428,   428,   429,   429,   429,   429,
   429,   430,   431,   432,   432,   433,   433,   433,   433,   433,
   433,   433,   434,   435,   436,   436,   437,   437,   437,   437,
   437,   437,   438,   439,   440,   440,   441,   441,   441,   442,
   443,   444,   444,   445,   445,   445,   446,   447,   448,   448,
   449,   449,   449,   450,   451,   452,   452,   453,   454,   454,
   454,   454,   454,   454,   454,   454,   454,   454,   454,   454,
   454,   454,   454,   454,   454,   454,   454,   454,   454,   454,
   454,   454,   454,   454,   454,   454,   454,   454,   454,   454,
   454,   454,   454,   454,   454,   454,   454,   454,   454,   454,
   454,   454,   454,   454,   454,   454,   454,   454,   455,   456,
   457,   457,   458,   458,   458,   458,   458,   458,   459,   460,
   461,   461,   462,   462,   462,   462,   462,   462,   462,   462,
   462,   462,   462,   462,   462,   463,   464,   465,   465,   466,
   467,   468,   469,   469,   470,   470,   470,   471,   472,   473,
   473,   474,   474,   474,   474,   474,   474,   474,   474,   474,
   474,   475,   476,   477,   477,   478,   479,   479,   479,   480,
   481,   482,   482,   483,   484,   485,   485,   485,   485,   485,
   485,   486,   487,   488,   488,   489,   490,   491,   492,   492,
   493,   493,   493,   493,   494,   495,   496,   496,   497,   497,
   497,   497,   497,   498,   499,   500,   500,   501,   502,   503,
   504,   504,   505,   505,   505,   505,   505,   505,   505,   505,
   506,   507,   508,   508,   509,   509,   509,   509,   509,   510,
   511,   512,   512,   513,   513,   513,   514,   513,   515,   515,
   516,   517,   517,   518,   519
};

static const short yyr2[] = {     0,
     1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     3,     4,     2,     0,     3,     4,     2,
     3,     1,     3,     2,     1,     0,     1,     3,     1,     3,
     2,     1,     0,     1,     3,     1,     3,     2,     1,     0,
     1,     3,     1,     3,     2,     1,     0,     1,     3,     1,
     3,     2,     1,     0,     1,     3,     1,     3,     2,     1,
     0,     1,     3,     1,     3,     2,     1,     0,     1,     3,
     1,     1,     2,     2,     0,     1,     1,     2,     2,     1,
     2,     2,     2,     1,     1,     1,     4,     2,     0,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     1,     4,     2,     0,     1,     2,     2,     2,     2,     2,
     2,     1,     4,     2,     0,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     4,     2,     0,     1,
     2,     1,     1,     1,     4,     2,     0,     2,     1,     4,
     2,     0,     4,     2,     2,     0,     1,     1,     2,     1,
     1,     2,     2,     2,     1,     4,     2,     0,     2,     1,
     4,     2,     0,     1,     1,     2,     2,     1,     4,     2,
     0,     2,     2,     2,     2,     1,     4,     2,     0,     1,
     4,     2,     0,     2,     2,     1,     4,     2,     0,     2,
     2,     2,     2,     2,     1,     4,     2,     0,     2,     2,
     2,     2,     2,     2,     1,     4,     2,     0,     2,     2,
     2,     2,     2,     1,     4,     2,     0,     1,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     1,     4,     2,
     0,     1,     1,     1,     1,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     4,     2,     0,     2,
     2,     2,     1,     4,     2,     0,     1,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     3,     2,     0,
     1,     1,     1,     1,     4,     2,     0,     1,     2,     2,
     2,     1,     4,     2,     0,     1,     1,     1,     1,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     1,     4,     2,     0,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     3,     2,     0,     1,     2,
     1,     1,     1,     4,     2,     0,     1,     1,     2,     2,
     2,     4,     1,     4,     2,     0,     2,     2,     2,     2,
     2,     2,     1,     4,     2,     0,     1,     2,     2,     2,
     2,     2,     2,     2,     1,     4,     2,     0,     1,     1,
     2,     2,     2,     2,     2,     1,     4,     2,     0,     2,
     1,     4,     2,     0,     2,     2,     2,     1,     4,     2,
     0,     2,     2,     2,     1,     4,     2,     0,     1,     4,
     2,     2,     1,     4,     2,     0,     2,     2,     2,     2,
     2,     1,     4,     2,     0,     2,     2,     2,     2,     2,
     2,     2,     1,     4,     2,     0,     2,     2,     2,     2,
     2,     2,     1,     4,     2,     0,     2,     2,     2,     1,
     4,     2,     0,     2,     2,     2,     1,     4,     2,     0,
     2,     2,     2,     1,     4,     2,     0,     1,     2,     2,
     2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     4,     4,     4,
     4,     4,     4,     5,     4,     4,     4,     4,     1,     4,
     2,     0,     2,     2,     2,     2,     2,     2,     1,     4,
     2,     0,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     4,     2,     0,     2,
     1,     4,     2,     0,     2,     2,     2,     1,     4,     2,
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     1,     4,     2,     0,     1,     2,     4,     2,     1,
     4,     2,     0,     1,     1,     2,     2,     2,     2,     2,
     2,     1,     4,     2,     0,     2,     1,     4,     2,     0,
     2,     2,     2,     2,     1,     4,     2,     0,     2,     2,
     2,     2,     2,     1,     4,     2,     0,     2,     1,     4,
     2,     0,     1,     2,     2,     2,     2,     2,     1,     1,
     1,     4,     2,     0,     2,     2,     2,     2,     2,     1,
     4,     2,     0,     2,     2,     2,     1,     4,     2,     0,
     1,     2,     2,     1,     4
};

static const short yydefact[] = {     0,
     2,   136,   162,   186,   194,   215,   228,   246,   265,   274,
     0,     3,   323,   403,   446,   344,   413,   510,   517,   524,
   579,   589,   606,   632,   618,   674,   640,   679,   700,   707,
   714,   458,   465,   493,   482,   691,     0,     0,     0,    56,
     1,    55,    53,    54,    41,    50,    51,     5,     0,     6,
     0,    40,     0,    46,     0,     7,     0,     8,     0,    14,
     0,    15,     0,    20,     0,    37,     0,    47,   326,    52,
     0,     9,     0,    10,     0,    11,     0,    48,     0,    16,
     0,    17,     0,    21,     0,    39,     0,    18,     0,    23,
     0,    19,     0,    42,     0,    43,     0,    44,     0,    22,
     0,    38,     0,    12,     0,    24,     0,    25,     0,    13,
     0,    49,    26,     0,     0,    45,   121,   337,    58,   705,
    61,    60,     0,   706,   704,     4,   125,   154,   165,   189,
   206,   223,   243,   258,   268,   319,     0,   327,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   326,   340,   398,
   406,   438,   454,   461,   476,   485,   506,   513,   520,   527,
   582,   592,   614,   621,   635,   668,   677,   682,   694,   703,
   710,     0,     0,   126,   127,     0,     0,     0,     0,     0,
     0,   134,   135,   125,   130,   155,     0,     0,     0,     0,
     0,     0,     0,   154,   172,   173,   166,   167,   168,   169,
   170,   171,   174,   175,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   165,     0,   192,   193,     0,
   189,   190,     0,   207,   210,   211,   208,     0,     0,   206,
   224,   225,     0,     0,     0,   223,     0,     0,     0,   243,
     0,     0,     0,     0,     0,     0,     0,   258,     0,     0,
     0,     0,     0,     0,   268,     0,     0,     0,     0,   319,
   329,    59,   331,   332,   336,   334,   333,   328,   335,   122,
   338,    97,    98,   330,   325,   342,   343,   341,     0,   340,
   399,   401,   402,     0,     0,   398,   407,     0,   408,     0,
     0,     0,   406,     0,   439,     0,     0,   440,     0,     0,
     0,   438,     0,     0,     0,     0,   454,     0,     0,     0,
     0,   461,     0,     0,     0,     0,     0,     0,   476,     0,
     0,     0,     0,     0,     0,     0,     0,   485,     0,     0,
     0,     0,   506,     0,     0,     0,     0,   513,     0,     0,
     0,     0,   520,   528,     0,     0,     0,     0,     0,     0,
     0,   527,     0,     0,     0,   582,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   592,     0,     0,
     0,     0,   614,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,   621,   636,     0,     0,     0,   635,
     0,     0,     0,     0,     0,     0,   668,     0,     0,   677,
     0,     0,     0,     0,     0,   689,   690,   683,     0,   682,
     0,     0,     0,     0,     0,     0,   694,     0,   703,   711,
     0,     0,     0,   710,    71,   202,   204,   129,   128,   131,
   137,   132,   133,   124,   157,    63,    62,   159,   160,   158,
   156,   163,   161,   153,   187,   178,   179,   180,   181,   182,
   183,    90,    91,   176,    76,     0,    77,   177,   184,   185,
   164,   191,   195,   188,   209,   213,   212,   216,   214,   205,
   229,   226,   227,   222,   244,   111,   112,   245,   247,   242,
   259,   260,   261,   262,   263,   264,   266,   257,   273,   269,
   270,   271,   272,   275,   267,   321,   322,   320,   324,   318,
   123,    96,    93,     0,   345,   339,   404,   400,   397,   410,
   414,   409,     5,   411,   405,   443,   445,   442,   447,   441,
   444,   437,   457,   455,   456,   459,   453,     0,   464,   462,
   118,   119,   463,   466,   460,   477,   478,   481,     0,   479,
   480,   483,   475,   486,   487,   492,   491,   489,   488,   490,
   494,   484,   509,   507,   508,   511,   505,   516,   515,   514,
   518,   512,   523,   521,   522,   525,   519,   530,   531,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   580,   529,   526,   151,   583,
   585,     0,   584,   199,   586,   236,   255,   316,   298,   382,
     0,   652,   503,   611,   588,   587,     0,    27,     0,    28,
     0,    29,     0,    30,     0,    31,     0,    32,   385,    33,
     0,    34,     0,    35,     0,    36,   590,   581,   596,   598,
   597,   599,   600,   601,   435,   604,   602,     0,   603,   605,
   594,   593,   595,   607,   591,   615,   616,   617,   619,   613,
   622,   623,   624,   631,   630,   628,   625,   627,   629,   626,
   633,   620,    57,   639,   641,     5,   637,   634,   671,   672,
   673,   670,   669,   675,   667,   678,   680,   676,   685,   686,
   684,   688,   687,   692,   681,   695,   696,   698,   697,   699,
   701,   693,   708,   702,   713,   715,   712,   709,   202,     0,
    89,    86,     0,    75,    72,     0,     0,   110,   107,     0,
    96,    95,    99,     0,     0,   117,   114,     0,    70,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   532,
   533,   534,   535,   536,   537,   538,   539,   540,   541,   542,
   543,   544,   545,   546,   547,   548,   549,   550,   551,   552,
   553,   554,   555,   556,   557,   558,   559,   560,   561,   562,
   563,   564,   565,   566,   567,   139,   395,   197,   231,   249,
   277,   301,   355,     0,     0,   356,   357,     0,     0,     0,
     0,   385,   496,   609,   643,   426,     0,   201,   203,    89,
    88,    92,    75,    74,    78,    64,   110,   109,   113,    94,
   412,     0,   117,   116,   120,   568,   569,   570,   571,   572,
   573,     0,   575,   576,   577,   578,     0,     0,     0,     0,
   139,     0,     0,   197,     0,     0,     0,     0,     0,   231,
     0,     0,     0,     0,     0,     0,   249,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   278,     0,
     0,     0,   277,     0,     0,     0,   302,     0,   305,     0,
     0,   304,   303,     0,     0,     0,     0,     0,   301,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   358,   359,
     0,     0,     0,     0,     0,     0,   355,   392,   240,   389,
   391,     0,   390,   220,   386,   388,     0,   387,    83,    84,
   393,   394,   396,   384,     0,     0,     0,   496,     0,     0,
   609,     0,   645,     0,   644,     0,     0,     0,   643,   427,
     0,     0,     0,     0,     0,     0,     0,     0,   426,   638,
    87,    73,   108,    65,   115,   574,   140,   423,   142,     0,
   141,   148,   665,   150,     0,   149,   143,   473,   352,   147,
     0,   144,   145,     0,   146,   152,   138,   198,   200,   196,
   232,   233,   234,   235,   237,   230,   250,   251,   252,   254,
   253,   256,   248,   282,   451,   284,     0,   283,   285,   657,
   287,     0,   286,   289,   292,   290,   293,   294,   295,   296,
   297,   291,   279,   281,   280,   299,   288,   276,   307,   309,
   306,   312,   310,   308,   317,   104,   105,   311,   313,   314,
   315,   300,   366,   368,   367,   369,   371,   370,   372,   377,
   375,   381,   374,   373,   363,   365,   364,   360,   362,   361,
   383,   376,   378,   379,   380,   354,   239,   218,    82,    79,
     0,   500,   502,   501,   497,   499,   498,   504,   495,   610,
   612,   608,   647,   649,   648,   651,   653,   646,   650,   642,
   428,   430,   431,   429,   433,   434,   436,   432,   425,   416,
   660,   347,   468,   449,   655,   103,   100,     0,     0,     0,
     0,     0,   218,    82,    81,    85,     0,     0,     0,     0,
     0,     0,     0,   416,     0,     0,     0,     0,     0,   660,
   348,     0,     0,     0,     0,   347,     0,     0,   469,     0,
     0,   468,     0,     0,   449,     0,     0,   655,   103,   102,
   106,   238,   241,   219,   221,   217,    80,   417,   418,   421,
   419,   420,   422,   424,   415,   662,   663,   661,   664,   666,
   659,   350,   351,   353,   349,   346,   471,   472,   474,    67,
   467,   450,   452,   448,   656,   658,   654,   101,    67,     0,
    66,   470,     0,     0,     0
};

static const short yydefgoto[] = {  1193,
    41,    42,    43,    44,   636,    45,    46,    47,    48,   930,
   120,   273,   123,   438,   457,   747,  1190,  1037,   477,   736,
   458,  1071,   931,   733,   454,   504,   274,  1108,  1038,   740,
   478,   748,   533,   118,   271,   179,   180,   181,   182,   183,
   184,    49,    50,   850,   851,   622,   623,   192,   193,   194,
    51,    52,   205,   206,   207,   208,   209,   210,   211,   212,
   213,   214,   215,   216,    53,    54,   220,   221,    55,    56,
   853,   854,   637,   638,   730,   185,   228,   229,   230,    57,
    58,  1112,  1113,   927,   928,   233,   234,   235,   236,    59,
    60,   859,   860,   639,   640,  1110,   922,   923,   239,   240,
    61,    62,   866,   867,   641,   642,   247,   248,    63,    64,
   254,   255,    65,    66,   881,   882,   883,   643,   644,   894,
   895,   896,   897,   898,   899,   645,   646,   259,   260,    67,
    68,   146,   147,   148,    69,    70,   279,   280,    71,    72,
  1134,  1135,  1136,   981,   982,   912,   809,   810,   915,   916,
   917,   647,   648,   811,   812,   649,   650,   284,   285,   286,
    73,    74,   290,   291,   292,   293,    75,    76,  1123,  1124,
   970,   971,   957,   958,   959,   668,   669,   299,   300,   301,
   302,    77,    78,  1144,  1145,  1007,  1008,   306,   307,    79,
    80,   311,   312,    81,    82,  1140,  1141,  1142,   984,   985,
   318,   319,    83,    84,   327,   328,    85,    86,   937,   938,
   651,   652,   332,   333,    87,    88,   337,   338,    89,    90,
   342,   343,    91,    92,   350,   351,   352,    93,    94,   355,
   356,    95,    96,   367,   368,    97,    98,   940,   941,   653,
   654,   372,   373,    99,   100,   384,   385,   101,   102,   388,
   389,   390,   103,   104,   946,   947,   948,   949,   655,   656,
  1147,  1148,  1012,  1013,  1129,  1130,   975,   976,   396,   397,
   105,   106,   399,   400,   107,   108,   409,   410,   109,   110,
   416,   417,   111,   112,   418,   113,   114,   422,   423,   424,
   115,   116
};

static const short yypact[] = {  1315,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -197,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    36,   316,   316,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1658,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,   648,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,-32768,  -197,  -197,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   316,-32768,-32768,-32768,   246,   371,   892,   399,
   511,   232,   232,   716,   383,   283,    36,-32768,   101,    36,
    36,   316,   316,   101,   101,  -172,     9,   648,   131,   521,
   391,   376,   166,   204,   471,   362,   228,   252,   258,   229,
   350,   574,   503,   431,   377,   433,    40,   335,   603,   282,
   -21,   316,  1372,-32768,-32768,   316,   316,   101,  -172,     9,
     9,-32768,-32768,   246,-32768,-32768,    36,   532,   532,   316,
   101,  -172,     9,   371,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,  -172,     9,     9,     9,     9,     9,
     9,    37,    80,    37,    80,   892,   316,-32768,-32768,  -172,
   399,-32768,    36,    10,-32768,-32768,-32768,  -172,  1658,   511,
-32768,-32768,  -172,    37,    80,   232,    37,    83,  -172,   232,
    36,   316,    36,   316,   316,   316,  -172,   716,   316,    36,
   316,   316,   316,  -172,   383,   101,   316,   316,  -172,   283,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -59,
-32768,    16,-32768,-32768,-32768,-32768,-32768,-32768,  -172,   131,
-32768,-32768,-32768,  -172,     9,   521,-32768,   316,-32768,  -172,
    37,  1429,   391,   316,-32768,   316,    36,-32768,  -172,    37,
     9,   376,   316,    37,    83,  -172,   166,   316,    37,    91,
  -172,   204,    36,    36,   316,   316,   316,  -172,   471,   316,
   316,   316,    36,   316,   316,   316,  -172,   362,   316,    37,
    83,  -172,   228,    36,   316,   316,  -172,   252,   316,    37,
    37,  -172,   258,-32768,    36,    36,   814,   748,   976,  -172,
     9,   229,    94,   358,  -172,   350,   316,   316,   316,   316,
   316,   128,    36,   316,   316,   316,  -172,   574,    36,    36,
   316,  -172,   503,   316,   316,   316,   316,    36,   316,   316,
   316,   316,   316,  -172,   431,-32768,   176,  -172,  1601,   377,
    36,   532,   532,    36,   532,  -172,   433,    36,  -172,    40,
   316,   316,   316,   316,   316,-32768,-32768,-32768,  -172,   335,
   316,    36,   316,   316,   101,  -172,   603,  -172,   282,-32768,
   101,  -172,     9,   -21,-32768,  1658,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    22,-32768,-32768,    34,   316,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    39,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    61,   -32,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,  1658,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   316,-32768,-32768,
    41,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   316,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   219,
   223,   260,   269,   278,   280,   288,   291,   294,   315,   318,
   330,   339,   361,   370,   380,   382,   397,   403,   406,   408,
   417,   438,   444,   448,   453,   465,   469,   475,   488,   496,
   502,   504,   509,   533,   534,   536,   541,   543,   553,   554,
   566,   570,   571,   575,   576,-32768,-32768,-32768,-32768,-32768,
-32768,  -197,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -197,-32768,-32768,-32768,-32768,-32768,  -197,-32768,  -197,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,  -197,-32768,   454,-32768,
  -197,-32768,  -197,-32768,  -197,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -197,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,  1658,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  1658,    20,
-32768,    44,   108,-32768,    49,   160,   316,-32768,    55,   216,
    16,-32768,-32768,   365,   316,-32768,    57,   368,-32768,    36,
   316,   176,   532,   316,   141,   316,   101,   316,   316,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   582,-32768,   436,   237,   381,
   758,   601,   746,    36,   114,-32768,-32768,    86,    65,    65,
  -172,   454,   -73,   442,   325,   661,   390,-32768,-32768,    22,
-32768,-32768,    34,-32768,-32768,-32768,    39,-32768,-32768,-32768,
-32768,   316,    41,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   609,-32768,-32768,-32768,-32768,   126,   134,    88,  -172,
   582,   316,  -172,   436,    36,    36,   316,   316,  -172,   237,
    36,    36,    36,   316,   316,  -172,   381,    97,   105,    36,
    36,   316,    36,   176,   316,   176,   316,    36,-32768,    96,
  -172,    37,   758,    36,   316,    36,-32768,    36,-32768,    36,
    36,-32768,-32768,  -172,    93,    91,    93,    83,   601,   171,
   195,    36,    36,   316,    36,    36,    36,   153,-32768,-32768,
   130,  -172,    65,    65,    65,    65,   746,-32768,-32768,-32768,
-32768,  -197,-32768,-32768,-32768,-32768,  -197,-32768,     8,-32768,
-32768,-32768,-32768,-32768,   207,   137,  -172,   -73,   316,  -172,
   442,    79,-32768,   316,-32768,  -172,     9,    37,   325,-32768,
    36,   532,   532,   316,    36,    36,  -172,     9,   661,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -197,
-32768,-32768,-32768,-32768,  -197,-32768,-32768,-32768,-32768,-32768,
  -197,-32768,-32768,  -197,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,  -197,-32768,-32768,-32768,
-32768,  -197,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    69,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   451,   447,-32768,    15,
   400,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   337,
   256,   146,    78,   439,   462,-32768,    77,   414,    83,  -172,
    80,  -172,   447,     8,-32768,-32768,   316,   316,   316,   316,
   316,   316,  -172,   337,   316,   316,   316,   316,  -172,   256,
-32768,    36,    36,  -172,     9,   146,    36,    36,-32768,  -172,
   407,    78,    83,  -172,   439,    93,  -172,   462,    69,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   176,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   176,   416,
-32768,-32768,   641,   643,-32768
};

static const short yypgoto[] = {-32768,
   -41,-32768,-32768,-32768,-32768,-32768,-32768,-32768,  -165,  -377,
    26,   -74,  -123,  -173,  -249,  -231,  -544,  -293,   763,  -627,
  -213,  -796,  -447,  -556,  -152,  -491,  -176,  -827,  -891,  -531,
  -304,  -588,  -243,   581,   174,   463,-32768,-32768,   -75,    12,
-32768,-32768,-32768,  -194,-32768,-32768,-32768,   467,-32768,-32768,
-32768,   297,   455,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   452,-32768,-32768,-32768,
  -191,-32768,-32768,-32768,   -60,   -80,   445,-32768,-32768,-32768,
-32768,  -436,-32768,-32768,  -659,   443,   150,   205,-32768,-32768,
-32768,  -179,-32768,-32768,-32768,-32768,-32768,  -872,   449,-32768,
-32768,-32768,  -182,-32768,-32768,-32768,   446,-32768,-32768,-32768,
   432,-32768,-32768,-32768,  -192,-32768,-32768,-32768,-32768,  -201,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   441,-32768,-32768,
-32768,   551,-32768,-32768,-32768,  -240,   424,-32768,-32768,-32768,
  -430,-32768,-32768,-32768,-32768,  -210,  -725,  -702,-32768,-32768,
-32768,-32768,-32768,  -104,-32768,-32768,-32768,   440,-32768,-32768,
-32768,-32768,   429,-32768,-32768,-32768,-32768,-32768,  -400,-32768,
-32768,-32768,  -234,-32768,-32768,-32768,  -120,   437,-32768,-32768,
-32768,-32768,-32768,  -407,-32768,-32768,  -160,   450,-32768,-32768,
-32768,   434,-32768,-32768,-32768,  -398,-32768,-32768,-32768,-32768,
   456,-32768,-32768,-32768,   420,-32768,-32768,-32768,  -188,-32768,
-32768,-32768,   418,-32768,-32768,-32768,   415,-32768,-32768,-32768,
   412,-32768,-32768,-32768,   425,-32768,-32768,-32768,-32768,   402,
-32768,-32768,-32768,   401,-32768,-32768,-32768,  -171,-32768,-32768,
-32768,   405,-32768,-32768,-32768,   398,-32768,-32768,-32768,   394,
-32768,-32768,-32768,-32768,  -164,-32768,-32768,-32768,-32768,-32768,
  -360,-32768,-32768,  -109,  -337,-32768,-32768,-32768,   421,-32768,
-32768,-32768,   395,-32768,-32768,-32768,   384,-32768,-32768,-32768,
   387,-32768,-32768,-32768,   379,  -157,-32768,   375,-32768,-32768,
-32768,-32768
};


#define	YYLAST		1878


static const short yytable[] = {   172,
   525,   460,   492,   432,   433,  1040,   126,   427,   498,   694,
   693,   742,   419,   262,   420,   439,   443,   693,   266,   267,
   262,   473,   540,   541,   121,   122,   555,   117,   466,   446,
   447,   448,   449,   450,   451,  1057,   121,   122,   119,   121,
   122,   121,   122,   121,   122,   563,   121,   122,   425,   222,
   227,   121,   122,   270,   218,   225,   935,   121,   122,   121,
   122,   459,  1074,   469,   263,   262,   440,   693,   278,   268,
   269,   121,   122,   276,   282,   549,   529,   913,   532,   121,
   122,   472,   121,   122,   475,   121,   122,   408,   453,   456,
   453,   456,   406,   121,   122,   121,   122,  1083,   936,   924,
   914,   619,    11,   430,   925,   262,   977,   824,   508,   924,
   453,   456,   620,   453,  1023,  1004,   441,   665,   482,   978,
   484,   485,   486,  1009,   521,   489,   514,   491,   456,   919,
   687,   398,   920,   497,   456,  1005,     3,   421,   512,   678,
   222,   219,   226,   924,   967,   218,   664,   520,  1058,   227,
   924,   524,   972,  1010,   225,  1075,   530,   665,   834,   841,
   277,   283,   261,   968,   501,   264,   265,   453,   919,   709,
   516,  1055,   517,   713,   617,   821,   453,   554,   693,   407,
   453,   496,   719,   973,   528,   453,   528,   564,   565,  1043,
   743,   913,   539,   539,   173,   962,   544,   503,   546,   278,
   548,   456,  1137,  1138,   276,   735,   453,   828,   979,  1005,
   282,  1131,   435,  1046,   914,   539,   453,   453,   700,   701,
  1025,   703,   919,   697,   750,  1072,   176,   177,   751,   467,
   272,  1069,   219,   659,   660,   661,   662,   663,  1114,   502,
   671,   226,   819,  1010,   965,   731,   727,   528,   465,   830,
   681,  1060,   683,   684,  1185,   686,   456,   734,   452,   690,
   729,   419,   738,   961,   746,   752,   481,   820,   483,  1139,
  1132,  1133,   823,  1115,   753,   490,  1077,   528,   827,  1150,
   833,   528,   237,   754,   741,   755,   929,   716,   231,   232,
   528,   277,  1106,   756,   344,   963,   757,   283,  1084,   758,
  1149,   455,   304,   309,   476,   926,   330,   980,   340,   173,
   174,   175,   531,   621,  1036,  1024,  1006,  1157,   121,   122,
   759,  1188,   518,   760,  1011,   855,   231,   232,   732,   408,
   822,   456,   737,   921,   406,   761,    30,   238,   536,   537,
   720,   176,   177,   334,   762,   969,   725,   666,   547,  1059,
   231,   232,   431,   974,   231,   232,  1076,   305,   310,   558,
   842,   331,   932,   341,   856,   442,   763,   345,   346,   624,
   568,   569,  1056,    37,   838,   764,   625,   626,   445,   627,
   231,   232,   825,   628,   629,   765,   303,   766,   670,   237,
  1044,   630,   631,   463,   676,   677,   256,   257,   173,   857,
   858,   468,   767,   685,   745,   632,   471,   528,   768,  1125,
  1126,   769,   479,   770,  1047,   749,   699,   633,   634,   702,
   487,   407,   771,   706,   308,   335,  1073,   494,   503,   336,
   176,   177,   499,  1127,  1128,  1117,   186,   717,   829,   178,
   187,   188,   189,   772,   238,   347,   348,   349,   329,   773,
   294,   295,   505,   774,   258,    38,   304,   507,   775,    39,
   320,   309,   173,   511,   845,  1062,  1063,  1064,  1065,   861,
   776,   744,   519,   296,   777,   942,   943,   944,   339,   526,
   778,   249,   330,   217,   534,   735,   353,   354,   401,   402,
   945,   542,   340,   779,   176,   177,  1018,   321,  1020,   190,
   551,   780,   391,   392,   393,   556,   843,   781,   862,   782,
   561,   305,   403,   404,   783,   566,   310,  1118,  1119,  1120,
  1121,  1122,   840,   616,   394,   386,   387,   405,   657,   374,
   322,   323,   324,   325,   436,   437,   297,   331,   784,   785,
   674,   786,   863,   298,   864,   679,   787,   341,   788,   865,
   326,  1070,   250,   251,   252,   253,   804,   691,   789,   790,
   313,   695,   314,   729,   191,   315,   375,   287,   288,   704,
   289,   791,   707,   735,   173,   792,   793,   635,   376,   839,
   794,   795,   714,   805,   806,   807,   281,   831,   395,   721,
   835,   723,   369,  1041,   370,   726,   223,   371,   224,   377,
   378,   379,   380,   381,   316,   317,   176,   177,   732,   852,
   939,   456,   960,   826,   966,  1109,   176,   177,  1111,   382,
   383,   832,  1116,   528,  1143,   808,  1146,   837,  1180,   127,
   528,   128,   456,   129,   539,   130,  1151,   131,  1192,   132,
  1194,   133,  1195,   134,  1191,   135,   434,   136,   847,   848,
   849,   149,  1039,   150,   817,   151,   987,   152,   667,   153,
   444,   154,   990,   155,   532,   156,   503,   157,   818,   158,
   461,   159,   464,   160,   470,   161,  1156,   162,   474,   163,
   996,   164,   844,   165,  1003,   166,   495,   167,   480,   168,
  1028,   169,  1070,   488,   170,   171,   732,  1042,   275,   456,
   500,  1085,   884,   506,   885,  1176,  1066,   934,   964,   528,
   886,   887,   888,   889,   357,   358,   359,   360,   361,   362,
   363,   515,   890,  1165,  1099,   509,   950,   891,   983,  1027,
   951,   952,   953,   993,   994,   954,  1070,  1184,   522,  1045,
  1000,  1001,  1107,  1181,   364,   535,   365,   552,  1016,  1079,
   557,  1019,   562,  1021,   567,   892,   527,   658,   453,   411,
   412,  1030,   366,   137,   138,   139,   140,   141,   675,  1082,
  1088,   539,   528,   539,   543,   836,   618,   680,  1092,  1093,
  1051,  1098,   692,   698,  1090,   955,   956,  1187,   893,   413,
   414,  1048,  1171,   715,   708,  1089,   415,   724,   728,     0,
   124,   125,  1189,   722,  1152,   241,   242,   243,   244,   245,
   246,  1189,     0,  1107,   142,  1080,     0,   705,     0,     0,
  1086,   143,   144,   145,   453,     0,     0,     0,     0,   918,
  1094,     0,  1167,  1168,  1169,     0,     0,     0,  1182,     0,
     0,     0,     0,     0,     0,   900,   901,   902,   903,   904,
   905,     0,     0,     0,     0,  1107,     0,   868,   869,   870,
   871,   872,   873,   874,   875,   876,   877,   906,  1159,  1160,
  1161,     0,   907,     0,     0,   908,   806,   807,   909,   878,
   991,   992,     0,     0,     0,     0,   997,   998,   999,     0,
     0,     0,     0,     0,     0,  1014,  1015,  1154,  1017,     0,
     0,     0,     0,  1022,     0,   910,     0,     0,     0,  1029,
     0,  1031,   539,  1032,     0,  1033,  1034,   911,     0,     0,
     0,   879,     0,     0,     0,     0,     0,  1049,  1050,   880,
  1052,  1053,  1054,     0,     0,     0,     0,     0,   428,   429,
     0,     0,   580,   581,   582,   583,   584,     0,   585,   586,
   587,   588,   589,     0,   590,   591,   592,   593,  1175,   594,
     0,   595,   596,   597,   195,   196,     0,     0,   197,   198,
   199,   200,   201,   202,   203,   204,  1091,     0,     0,   462,
  1095,  1096,     0,   539,   933,     0,     0,   456,     0,     0,
     0,     0,     0,  1158,   456,   456,   456,  1162,  1163,     0,
     0,  1166,   539,   539,   539,     0,     0,     0,   570,   571,
   572,   573,   574,   575,   576,   493,   577,   578,   579,     0,
     0,     0,   539,   986,     0,   539,   989,     0,     0,     0,
     0,     0,   995,     0,     0,     0,     0,     0,     0,  1002,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
   510,     0,     0,     0,  1026,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   523,     0,  1035,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   538,     0,     0,
     0,     0,     0,   545,     0,  1061,     0,     0,   550,     0,
     0,   553,     0,     0,     0,     0,     0,   559,   560,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
  1078,     0,     0,  1081,     0,     0,     0,     0,     0,  1087,
     0,     0,     0,     0,     0,     0,     0,   672,   673,     0,
  1097,     0,     0,     0,     0,     0,     0,   682,     0,     0,
     0,     0,     0,   688,   689,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,  1172,  1173,     0,
     0,     0,  1177,  1178,   710,   711,   712,     0,     0,     0,
   598,   599,   600,   601,   602,   718,   603,   604,   605,   606,
   607,     0,   608,   609,   610,   611,     0,   612,     0,   613,
   614,   615,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   796,     0,     0,     0,     0,     0,     0,     0,
     0,   797,     0,     0,     0,     0,     0,   798,     0,   799,
     0,   800,     0,   801,     0,   802,     0,   803,     0,     0,
     0,   813,     0,   814,     0,   815,     0,     0,   739,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   816,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,  1153,     0,  1155,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,  1164,     0,     0,     0,
     0,     0,  1170,     0,     0,     0,     0,  1174,     0,     0,
     0,     0,     0,  1179,    -5,     1,     0,  1183,     0,     0,
  1186,     2,     0,     3,     4,     5,     0,     6,     0,     7,
     0,     8,     9,     0,     0,     0,     0,    10,    11,    12,
     0,     0,    13,    14,     0,    15,     0,    16,     0,     0,
     0,    17,     0,     0,    18,    19,    20,    21,    22,    23,
    24,    25,     0,     0,     0,    26,    27,    28,    29,    30,
    31,    32,    33,    34,     0,     0,    35,    36,     2,     0,
     3,     4,     5,     0,     6,     0,     7,     0,     8,     9,
     0,     0,     0,     0,    10,    11,     0,     0,     0,    13,
    14,     0,    15,     0,    16,     0,    37,     0,    17,     0,
     0,    18,    19,    20,    21,    22,    23,    24,    25,     0,
     0,     0,    26,    27,    28,    29,    30,    31,    32,    33,
    34,     0,     0,    35,    36,     2,     0,     3,     4,     5,
     0,     6,     0,     7,     0,     8,     9,     0,     0,     0,
     0,    10,    11,     0,     0,     0,    13,    14,     0,    15,
     0,    16,     0,    37,     0,    17,     0,     0,    18,    19,
    20,    21,    22,    23,    24,    25,     0,     0,     0,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    38,     0,
    35,    36,    39,     0,     0,     0,     0,     0,     0,     0,
     0,   739,  1067,     0,     0,     0,     0,  1068,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    37,   846,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    40,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    38,     0,     0,     0,    39,
  1100,     0,     0,     0,     0,  1101,     0,     0,     0,     0,
     0,  1102,     0,     0,  1103,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,  1104,     0,   739,
     0,    40,  1105,   426,     0,     0,     0,     0,     0,     0,
     0,     0,    38,     0,     0,     0,    39,     2,     0,     3,
     4,     5,     0,     6,   988,     7,     0,     8,     9,     0,
     0,     0,     0,    10,    11,     0,     0,     0,    13,    14,
     0,    15,     0,    16,     0,     0,     0,    17,     0,     0,
    18,    19,    20,    21,    22,    23,    24,    25,    40,     0,
   513,    26,    27,    28,    29,    30,    31,    32,    33,    34,
     0,     0,    35,    36,     2,     0,     3,     4,     5,     0,
     6,     0,     7,     0,     8,     9,     0,     0,     0,     0,
    10,    11,     0,     0,     0,    13,    14,     0,    15,     0,
    16,     0,    37,     0,    17,     0,     0,    18,    19,    20,
    21,    22,    23,    24,    25,     0,     0,     0,    26,    27,
    28,    29,    30,    31,    32,    33,    34,     0,     0,    35,
    36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    37,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    38,     0,     0,     0,    39,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    40,     0,   696,     0,     0,     0,     0,     0,     0,     0,
     0,    38,     0,     0,     0,    39,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    40
};

static const short yycheck[] = {   123,
   305,   215,   252,   180,   181,   897,    48,   173,   258,   387,
     3,   503,   170,     5,    36,   189,   193,     3,   142,   143,
     5,   235,   316,   317,     3,     4,   331,   225,    19,   206,
   207,   208,   209,   210,   211,   908,     3,     4,     3,     3,
     4,     3,     4,     3,     4,   339,     3,     4,   172,   130,
   131,     3,     4,   226,   130,   131,   130,     3,     4,     3,
     4,   214,   935,   229,   139,     5,   190,     3,   149,   144,
   145,     3,     4,   149,   150,   325,   308,   803,   310,     3,
     4,   234,     3,     4,   237,     3,     4,   168,   212,   213,
   214,   215,   168,     3,     4,     3,     4,    19,   172,    14,
   803,     8,    24,   178,    19,     5,    19,   735,   285,    14,
   234,   235,    19,   237,    19,    19,   191,    30,   242,    32,
   244,   245,   246,    19,   301,   249,   292,   251,   252,    16,
   380,    92,    19,   257,   258,    39,     9,   159,   291,   371,
   221,   130,   131,    14,    19,   221,    19,   300,    19,   230,
    14,   304,    19,    49,   230,    19,   309,    30,   747,    19,
   149,   150,   137,    38,   224,   140,   141,   291,    16,   401,
   294,    19,   296,   405,   351,   732,   300,   330,     3,   168,
   304,   256,   414,    50,   308,   309,   310,   340,   341,    19,
   223,   917,   316,   317,    64,   823,   320,   272,   322,   280,
   324,   325,   125,   126,   280,   455,   330,   739,   121,    39,
   286,    66,   187,    19,   917,   339,   340,   341,   392,   393,
   880,   395,    16,   389,     6,    19,    96,    97,     6,   220,
   222,   224,   221,   357,   358,   359,   360,   361,   224,   224,
   364,   230,   223,    49,   833,   224,   423,   371,   223,   741,
   374,   911,   376,   377,  1146,   379,   380,   224,   222,   383,
   426,   419,   224,   820,   224,     6,   241,   224,   243,   192,
   125,   126,   224,  1070,     6,   250,   936,   401,   224,  1107,
   224,   405,   133,     6,   224,     6,   222,   411,   123,   124,
   414,   280,   224,     6,    66,   827,     6,   286,   220,     6,
   224,   222,   153,   154,   222,   220,   157,   220,   159,    64,
    65,    66,   222,   220,   222,   220,   220,  1114,     3,     4,
     6,  1149,   297,     6,   220,    89,   123,   124,   452,   410,
   223,   455,   456,   220,   410,     6,    55,   133,   313,   314,
   415,    96,    97,    92,     6,   220,   421,   220,   323,   220,
   123,   124,   179,   220,   123,   124,   220,   153,   154,   334,
   220,   157,   810,   159,   128,   192,     6,   139,   140,    12,
   345,   346,   220,    92,   752,     6,    19,    20,   205,    22,
   123,   124,   223,    26,    27,     6,   221,     6,   363,   240,
   220,    34,    35,   220,   369,   370,   114,   115,    64,   163,
   164,   228,     6,   378,   528,    48,   233,   531,     6,   154,
   155,     6,   239,     6,   220,   539,   391,    60,    61,   394,
   247,   410,     6,   398,   221,   174,   220,   254,   503,   178,
    96,    97,   259,   178,   179,    99,    66,   412,   223,   194,
    70,    71,    72,     6,   240,   217,   218,   219,   221,     6,
    75,    76,   279,     6,   172,   174,   307,   284,     6,   178,
    99,   312,    64,   290,   758,   913,   914,   915,   916,    89,
     6,   513,   299,    98,     6,   151,   152,   153,   221,   306,
     6,    99,   333,    85,   311,   735,   137,   138,   154,   155,
   166,   318,   343,     6,    96,    97,   874,   136,   876,   129,
   327,     6,    70,    71,    72,   332,   756,     6,   128,     6,
   337,   307,   178,   179,     6,   342,   312,   181,   182,   183,
   184,   185,   754,   350,    92,   149,   150,   193,   355,    99,
   169,   170,   171,   172,     3,     4,   161,   333,     6,     6,
   367,     6,   162,   168,   164,   372,     6,   343,     6,   169,
   189,   929,   170,   171,   172,   173,   103,   384,     6,     6,
    90,   388,    92,   729,   194,    95,   136,   177,   178,   396,
   180,     6,   399,   823,    64,     6,     6,   220,   148,   753,
     6,     6,   409,   130,   131,   132,    66,   223,   156,   416,
   223,   418,    90,   898,    92,   422,    86,    95,    88,   169,
   170,   171,   172,   173,   134,   135,    96,    97,   732,   174,
   169,   735,   223,   737,     6,   165,    96,    97,   172,   189,
   190,   745,   223,   747,   186,   172,   165,   751,   222,    49,
   754,    51,   756,    53,   758,    55,   223,    57,   223,    59,
     0,    61,     0,    63,  1189,    65,   184,    67,    67,    68,
    69,    71,   896,    73,   696,    75,   851,    77,   362,    79,
   194,    81,   854,    83,   896,    85,   741,    87,   729,    89,
   216,    91,   221,    93,   230,    95,  1113,    97,   236,    99,
   860,   101,   757,   103,   867,   105,   255,   107,   240,   109,
   883,   111,  1070,   248,   114,   115,   820,   899,   148,   823,
   260,   942,   102,   280,   104,  1136,   917,   812,   832,   833,
   110,   111,   112,   113,   141,   142,   143,   144,   145,   146,
   147,   293,   122,  1124,   959,   286,    66,   127,   849,   882,
    70,    71,    72,   857,   858,    75,  1114,  1145,   302,   900,
   864,   865,  1036,  1142,   171,   312,   173,   328,   872,   938,
   333,   875,   338,   877,   343,   155,   307,   356,   882,   157,
   158,   885,   189,   116,   117,   118,   119,   120,   368,   941,
   947,   895,   896,   897,   319,   750,   352,   373,   952,   953,
   904,   958,   385,   390,   949,   125,   126,  1148,   188,   187,
   188,   901,  1130,   410,   400,   948,   194,   419,   424,    -1,
    38,    39,  1180,   417,  1109,    90,    91,    92,    93,    94,
    95,  1189,    -1,  1107,   167,   939,    -1,   397,    -1,    -1,
   944,   174,   175,   176,   948,    -1,    -1,    -1,    -1,   804,
   954,    -1,  1126,  1127,  1128,    -1,    -1,    -1,  1143,    -1,
    -1,    -1,    -1,    -1,    -1,   100,   101,   102,   103,   104,
   105,    -1,    -1,    -1,    -1,  1149,    -1,   100,   101,   102,
   103,   104,   105,   106,   107,   108,   109,   122,  1118,  1119,
  1120,    -1,   127,    -1,    -1,   130,   131,   132,   133,   122,
   855,   856,    -1,    -1,    -1,    -1,   861,   862,   863,    -1,
    -1,    -1,    -1,    -1,    -1,   870,   871,  1111,   873,    -1,
    -1,    -1,    -1,   878,    -1,   160,    -1,    -1,    -1,   884,
    -1,   886,  1036,   888,    -1,   890,   891,   172,    -1,    -1,
    -1,   164,    -1,    -1,    -1,    -1,    -1,   902,   903,   172,
   905,   906,   907,    -1,    -1,    -1,    -1,    -1,   176,   177,
    -1,    -1,   195,   196,   197,   198,   199,    -1,   201,   202,
   203,   204,   205,    -1,   207,   208,   209,   210,  1135,   212,
    -1,   214,   215,   216,    73,    74,    -1,    -1,    77,    78,
    79,    80,    81,    82,    83,    84,   951,    -1,    -1,   217,
   955,   956,    -1,  1107,   811,    -1,    -1,  1111,    -1,    -1,
    -1,    -1,    -1,  1117,  1118,  1119,  1120,  1121,  1122,    -1,
    -1,  1125,  1126,  1127,  1128,    -1,    -1,    -1,   195,   196,
   197,   198,   199,   200,   201,   253,   203,   204,   205,    -1,
    -1,    -1,  1146,   850,    -1,  1149,   853,    -1,    -1,    -1,
    -1,    -1,   859,    -1,    -1,    -1,    -1,    -1,    -1,   866,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   288,    -1,    -1,    -1,   881,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   303,    -1,   894,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   315,    -1,    -1,
    -1,    -1,    -1,   321,    -1,   912,    -1,    -1,   326,    -1,
    -1,   329,    -1,    -1,    -1,    -1,    -1,   335,   336,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   937,    -1,    -1,   940,    -1,    -1,    -1,    -1,    -1,   946,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   365,   366,    -1,
   957,    -1,    -1,    -1,    -1,    -1,    -1,   375,    -1,    -1,
    -1,    -1,    -1,   381,   382,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1132,  1133,    -1,
    -1,    -1,  1137,  1138,   402,   403,   404,    -1,    -1,    -1,
   195,   196,   197,   198,   199,   413,   201,   202,   203,   204,
   205,    -1,   207,   208,   209,   210,    -1,   212,    -1,   214,
   215,   216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   622,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   631,    -1,    -1,    -1,    -1,    -1,   637,    -1,   639,
    -1,   641,    -1,   643,    -1,   645,    -1,   647,    -1,    -1,
    -1,   651,    -1,   653,    -1,   655,    -1,    -1,   476,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   668,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,  1110,    -1,  1112,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,  1123,    -1,    -1,    -1,
    -1,    -1,  1129,    -1,    -1,    -1,    -1,  1134,    -1,    -1,
    -1,    -1,    -1,  1140,     0,     1,    -1,  1144,    -1,    -1,
  1147,     7,    -1,     9,    10,    11,    -1,    13,    -1,    15,
    -1,    17,    18,    -1,    -1,    -1,    -1,    23,    24,    25,
    -1,    -1,    28,    29,    -1,    31,    -1,    33,    -1,    -1,
    -1,    37,    -1,    -1,    40,    41,    42,    43,    44,    45,
    46,    47,    -1,    -1,    -1,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    -1,    -1,    62,    63,     7,    -1,
     9,    10,    11,    -1,    13,    -1,    15,    -1,    17,    18,
    -1,    -1,    -1,    -1,    23,    24,    -1,    -1,    -1,    28,
    29,    -1,    31,    -1,    33,    -1,    92,    -1,    37,    -1,
    -1,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
    -1,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
    59,    -1,    -1,    62,    63,     7,    -1,     9,    10,    11,
    -1,    13,    -1,    15,    -1,    17,    18,    -1,    -1,    -1,
    -1,    23,    24,    -1,    -1,    -1,    28,    29,    -1,    31,
    -1,    33,    -1,    92,    -1,    37,    -1,    -1,    40,    41,
    42,    43,    44,    45,    46,    47,    -1,    -1,    -1,    51,
    52,    53,    54,    55,    56,    57,    58,    59,   174,    -1,
    62,    63,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   739,   922,    -1,    -1,    -1,    -1,   927,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    92,   759,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   220,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,   178,
   970,    -1,    -1,    -1,    -1,   975,    -1,    -1,    -1,    -1,
    -1,   981,    -1,    -1,   984,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1007,    -1,   827,
    -1,   220,  1012,   222,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,   174,    -1,    -1,    -1,   178,     7,    -1,     9,
    10,    11,    -1,    13,   852,    15,    -1,    17,    18,    -1,
    -1,    -1,    -1,    23,    24,    -1,    -1,    -1,    28,    29,
    -1,    31,    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,
    40,    41,    42,    43,    44,    45,    46,    47,   220,    -1,
   222,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    -1,    -1,    62,    63,     7,    -1,     9,    10,    11,    -1,
    13,    -1,    15,    -1,    17,    18,    -1,    -1,    -1,    -1,
    23,    24,    -1,    -1,    -1,    28,    29,    -1,    31,    -1,
    33,    -1,    92,    -1,    37,    -1,    -1,    40,    41,    42,
    43,    44,    45,    46,    47,    -1,    -1,    -1,    51,    52,
    53,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    92,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,   174,    -1,    -1,    -1,   178,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
   220,    -1,   222,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,   174,    -1,    -1,    -1,   178,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,   220
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 96 "vrml.y"
{YYABORT;;
    break;}
case 3:
#line 97 "vrml.y"
{YYABORT;;
    break;}
case 57:
#line 182 "vrml.y"
{
			AddSFInt32(yyvsp[0].ival);
		;
    break;}
case 59:
#line 193 "vrml.y"
{
			AddSFString(yyvsp[0].sval);
		;
    break;}
case 60:
#line 200 "vrml.y"
{
			AddSFFloat(yyvsp[0].fval);
		;
    break;}
case 61:
#line 204 "vrml.y"
{
			yyval.fval = (float)yyvsp[0].ival;
			AddSFFloat((float)yyvsp[0].ival);
		;
    break;}
case 63:
#line 212 "vrml.y"
{yyval.fval = (float)yyvsp[0].ival;;
    break;}
case 64:
#line 217 "vrml.y"
{
			gColor[0] = yyvsp[-2].fval;
			gColor[1] = yyvsp[-1].fval;
			gColor[2] = yyvsp[0].fval;
			AddSFColor(gColor);
	    ;
    break;}
case 65:
#line 227 "vrml.y"
{
			gRotation[0] = yyvsp[-3].fval;
			gRotation[1] = yyvsp[-2].fval;
			gRotation[2] = yyvsp[-1].fval;
			gRotation[3] = yyvsp[0].fval;
			AddSFRotation(gRotation);
		;
    break;}
case 66:
#line 237 "vrml.y"
{;
    break;}
case 68:
#line 244 "vrml.y"
{
			gWidth = yyvsp[-2].ival;
			gHeight = yyvsp[-1].ival;
			gComponents = yyvsp[0].ival;
	    ;
    break;}
case 70:
#line 257 "vrml.y"
{
			gVec2f[0] = yyvsp[-1].fval;
			gVec2f[1] = yyvsp[0].fval;
			AddSFVec2f(gVec2f);
		;
    break;}
case 71:
#line 266 "vrml.y"
{
			gVec3f[0] = yyvsp[-2].fval;
			gVec3f[1] = yyvsp[-1].fval;
			gVec3f[2] = yyvsp[0].fval;
			AddSFVec3f(gVec3f);
		;
    break;}
case 79:
#line 288 "vrml.y"
{;
    break;}
case 80:
#line 289 "vrml.y"
{;
    break;}
case 81:
#line 290 "vrml.y"
{;
    break;}
case 84:
#line 296 "vrml.y"
{;
    break;}
case 85:
#line 297 "vrml.y"
{;
    break;}
case 86:
#line 302 "vrml.y"
{;
    break;}
case 87:
#line 303 "vrml.y"
{;
    break;}
case 88:
#line 304 "vrml.y"
{;
    break;}
case 91:
#line 310 "vrml.y"
{;
    break;}
case 92:
#line 311 "vrml.y"
{;
    break;}
case 93:
#line 315 "vrml.y"
{;
    break;}
case 94:
#line 316 "vrml.y"
{;
    break;}
case 95:
#line 317 "vrml.y"
{;
    break;}
case 98:
#line 323 "vrml.y"
{;
    break;}
case 99:
#line 324 "vrml.y"
{;
    break;}
case 126:
#line 388 "vrml.y"
{
			PushNode(VRML_NODETYPE_ANCHOR_PARAMETER, GetCurrentNodeObject());
		;
    break;}
case 127:
#line 395 "vrml.y"
{
			PushNode(VRML_NODETYPE_ANCHOR_URL, GetCurrentNodeObject());
		;
    break;}
case 128:
#line 402 "vrml.y"
{
			((GroupingNode *)GetCurrentNodeObject())->setBoundingBoxCenter(gVec3f);
		;
    break;}
case 129:
#line 409 "vrml.y"
{
			((GroupingNode *)GetCurrentNodeObject())->setBoundingBoxSize(gVec3f);
		;
    break;}
case 131:
#line 417 "vrml.y"
{
			((AnchorNode *)GetCurrentNodeObject())->setDescription(yyvsp[0].sval);
		;
    break;}
case 132:
#line 422 "vrml.y"
{
			PopNode();
		;
    break;}
case 133:
#line 426 "vrml.y"
{
			PopNode();
		;
    break;}
case 136:
#line 435 "vrml.y"
{
			AnchorNode	*anchor = new AnchorNode();
			anchor->setName(GetDEFName());
			AddNode(anchor);
			PushNode(VRML_NODETYPE_ANCHOR, anchor);
		;
    break;}
case 137:
#line 445 "vrml.y"
{
			AnchorNode *anchor = (AnchorNode *)GetCurrentNodeObject();
			anchor->initialize();
			PopNode();
		;
    break;}
case 151:
#line 479 "vrml.y"
{
			AppearanceNode	*appearance = new AppearanceNode();
			appearance->setName(GetDEFName());
			AddNode(appearance);
			PushNode(VRML_NODETYPE_APPEARANCE, appearance);
		;
    break;}
case 152:
#line 489 "vrml.y"
{
			AppearanceNode	*appearance = (AppearanceNode *)GetCurrentNodeObject();
			appearance->initialize();
			PopNode();
		;
    break;}
case 155:
#line 509 "vrml.y"
{
			PushNode(VRML_NODETYPE_AUDIOCLIP_URL, GetCurrentNodeObject());
		;
    break;}
case 156:
#line 516 "vrml.y"
{
			((AudioClipNode *)GetCurrentNodeObject())->setDescription(yyvsp[0].sval);
		;
    break;}
case 157:
#line 520 "vrml.y"
{
			((AudioClipNode *)GetCurrentNodeObject())->setLoop(yyvsp[0].ival);
		;
    break;}
case 158:
#line 524 "vrml.y"
{
			((AudioClipNode *)GetCurrentNodeObject())->setPitch(yyvsp[0].fval);
		;
    break;}
case 159:
#line 528 "vrml.y"
{
			((AudioClipNode *)GetCurrentNodeObject())->setStartTime(yyvsp[0].fval);
		;
    break;}
case 160:
#line 532 "vrml.y"
{
			((AudioClipNode *)GetCurrentNodeObject())->setStopTime(yyvsp[0].fval);
		;
    break;}
case 161:
#line 536 "vrml.y"
{
			PopNode();
		;
    break;}
case 162:
#line 543 "vrml.y"
{
			AudioClipNode	*audioClip = new AudioClipNode();
			audioClip->setName(GetDEFName());
			AddNode(audioClip);
			PushNode(VRML_NODETYPE_AUDIOCLIP, audioClip);
		;
    break;}
case 163:
#line 552 "vrml.y"
{
			AudioClipNode *audioClip = (AudioClipNode *)GetCurrentNodeObject();
			audioClip->initialize();
			PopNode();
		;
    break;}
case 166:
#line 572 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_BACKURL, GetCurrentNodeObject());
		;
    break;}
case 167:
#line 579 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_BOTTOMURL, GetCurrentNodeObject());
		;
    break;}
case 168:
#line 586 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_FRONTURL, GetCurrentNodeObject());
		;
    break;}
case 169:
#line 593 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_LEFTURL, GetCurrentNodeObject());
		;
    break;}
case 170:
#line 600 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_RIGHTURL, GetCurrentNodeObject());
		;
    break;}
case 171:
#line 607 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_TOPURL, GetCurrentNodeObject());
		;
    break;}
case 172:
#line 614 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_GROUNDANGLE, GetCurrentNodeObject());
		;
    break;}
case 173:
#line 621 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_GROUNDCOLOR, GetCurrentNodeObject());
		;
    break;}
case 174:
#line 628 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_SKYANGLE, GetCurrentNodeObject());
		;
    break;}
case 175:
#line 635 "vrml.y"
{
			PushNode(VRML_NODETYPE_BACKGROUND_SKYCOLOR, GetCurrentNodeObject());
		;
    break;}
case 176:
#line 642 "vrml.y"
{
			PopNode();
		;
    break;}
case 177:
#line 646 "vrml.y"
{
			PopNode();
		;
    break;}
case 178:
#line 650 "vrml.y"
{
			PopNode();
		;
    break;}
case 179:
#line 654 "vrml.y"
{
			PopNode();
		;
    break;}
case 180:
#line 658 "vrml.y"
{
			PopNode();
		;
    break;}
case 181:
#line 662 "vrml.y"
{
			PopNode();
		;
    break;}
case 182:
#line 666 "vrml.y"
{
			PopNode();
		;
    break;}
case 183:
#line 670 "vrml.y"
{
			PopNode();
		;
    break;}
case 184:
#line 674 "vrml.y"
{
			PopNode();
		;
    break;}
case 185:
#line 678 "vrml.y"
{
			PopNode();
		;
    break;}
case 186:
#line 685 "vrml.y"
{
			BackgroundNode *bg = new BackgroundNode();
			bg->setName(GetDEFName());
			AddNode(bg);
			PushNode(VRML_NODETYPE_BACKGROUND, bg);
		;
    break;}
case 187:
#line 695 "vrml.y"
{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->initialize();
			PopNode();
		;
    break;}
case 191:
#line 716 "vrml.y"
{
			((BillboardNode *)GetCurrentNodeObject())->setAxisOfRotation(gVec3f);
		;
    break;}
case 194:
#line 725 "vrml.y"
{
			BillboardNode *billboard = new BillboardNode();
			billboard->setName(GetDEFName());
			AddNode(billboard);
			PushNode(VRML_NODETYPE_BILLBOARD, billboard);
		;
    break;}
case 195:
#line 735 "vrml.y"
{
			BillboardNode *billboard = (BillboardNode *)GetCurrentNodeObject();
			billboard->initialize();
			PopNode();
		;
    break;}
case 198:
#line 755 "vrml.y"
{
			((BoxNode *)GetCurrentNodeObject())->setSize(gVec3f);
		;
    break;}
case 199:
#line 762 "vrml.y"
{
			BoxNode *box = new BoxNode();
			box->setName(GetDEFName());
			AddNode(box);
			PushNode(VRML_NODETYPE_BOX, box);
		;
    break;}
case 200:
#line 772 "vrml.y"
{
			BoxNode *box = (BoxNode *)GetCurrentNodeObject();
			box->initialize();
			PopNode();
		;
    break;}
case 207:
#line 808 "vrml.y"
{
			PushNode(VRML_NODETYPE_COLLISION_PROXY, GetCurrentNodeObject());
		;
    break;}
case 209:
#line 816 "vrml.y"
{
			((CollisionNode *)GetCurrentNodeObject())->setCollide(yyvsp[0].ival);
		;
    break;}
case 214:
#line 824 "vrml.y"
{
			PopNode();
		;
    break;}
case 215:
#line 831 "vrml.y"
{
			CollisionNode *collision = new CollisionNode();
			collision->setName(GetDEFName());
			AddNode(collision);
			PushNode(VRML_NODETYPE_BOX, collision);
		;
    break;}
case 216:
#line 841 "vrml.y"
{
			CollisionNode *collision = (CollisionNode *)GetCurrentNodeObject();
			collision->initialize();
			PopNode();
		;
    break;}
case 220:
#line 865 "vrml.y"
{
			ColorNode *color = new ColorNode();
			color->setName(GetDEFName());
			AddNode(color);
			PushNode(VRML_NODETYPE_COLOR, color);
		;
    break;}
case 221:
#line 875 "vrml.y"
{
			ColorNode *color = (ColorNode *)GetCurrentNodeObject();
			color->initialize();
			PopNode();
		;
    break;}
case 224:
#line 895 "vrml.y"
{
			PushNode(VRML_NODETYPE_INTERPOLATOR_KEY, GetCurrentNodeObject());
		;
    break;}
case 225:
#line 902 "vrml.y"
{
			PushNode(VRML_NODETYPE_INTERPOLATOR_KEYVALUE, GetCurrentNodeObject());
		;
    break;}
case 226:
#line 909 "vrml.y"
{
			PopNode();
		;
    break;}
case 227:
#line 913 "vrml.y"
{
			PopNode();
		;
    break;}
case 228:
#line 920 "vrml.y"
{
			ColorInterpolatorNode *colInterp = new ColorInterpolatorNode();
			colInterp->setName(GetDEFName());
			AddNode(colInterp);
			PushNode(VRML_NODETYPE_COLORINTERPOLATOR, colInterp);
		;
    break;}
case 229:
#line 930 "vrml.y"
{
			ColorInterpolatorNode *colInterp = (ColorInterpolatorNode *)GetCurrentNodeObject();
			colInterp->initialize();
			PopNode();
		;
    break;}
case 232:
#line 950 "vrml.y"
{
			((ConeNode *)GetCurrentNodeObject())->setSide(yyvsp[0].ival);
		;
    break;}
case 233:
#line 954 "vrml.y"
{
			((ConeNode *)GetCurrentNodeObject())->setBottom(yyvsp[0].ival);
		;
    break;}
case 234:
#line 958 "vrml.y"
{
			((ConeNode *)GetCurrentNodeObject())->setBottomRadius(yyvsp[0].fval);
		;
    break;}
case 235:
#line 962 "vrml.y"
{
			((ConeNode *)GetCurrentNodeObject())->setHeight(yyvsp[0].fval);
		;
    break;}
case 236:
#line 969 "vrml.y"
{
			ConeNode *cone = new ConeNode();
			cone->setName(GetDEFName());
			AddNode(cone);
			PushNode(VRML_NODETYPE_CONE, cone);
		;
    break;}
case 237:
#line 979 "vrml.y"
{
			ConeNode *cone = (ConeNode *)GetCurrentNodeObject();
			cone->initialize();
			PopNode();
		;
    break;}
case 240:
#line 999 "vrml.y"
{
			CoordinateNode *coord = new CoordinateNode();
			coord->setName(GetDEFName());
			AddNode(coord);
			PushNode(VRML_NODETYPE_COORDINATE, coord);
		;
    break;}
case 241:
#line 1009 "vrml.y"
{
			CoordinateNode *coord = (CoordinateNode *)GetCurrentNodeObject();
			coord->initialize();
			PopNode();
		;
    break;}
case 244:
#line 1029 "vrml.y"
{
			PopNode();
		;
    break;}
case 245:
#line 1033 "vrml.y"
{
			PopNode();
		;
    break;}
case 246:
#line 1040 "vrml.y"
{
			CoordinateInterpolatorNode *coordInterp = new CoordinateInterpolatorNode();
			coordInterp->setName(GetDEFName());
			AddNode(coordInterp);
			PushNode(VRML_NODETYPE_COORDINATEINTERPOLATOR, coordInterp);
		;
    break;}
case 247:
#line 1050 "vrml.y"
{
			CoordinateInterpolatorNode *coordInterp = (CoordinateInterpolatorNode *)GetCurrentNodeObject();
			coordInterp->initialize();
			PopNode();
		;
    break;}
case 250:
#line 1070 "vrml.y"
{
			((CylinderNode *)GetCurrentNodeObject())->setSide(yyvsp[0].ival);
		;
    break;}
case 251:
#line 1074 "vrml.y"
{
			((CylinderNode *)GetCurrentNodeObject())->setBottom(yyvsp[0].ival);
		;
    break;}
case 252:
#line 1078 "vrml.y"
{
			((CylinderNode *)GetCurrentNodeObject())->setTop(yyvsp[0].ival);
		;
    break;}
case 253:
#line 1082 "vrml.y"
{
			((CylinderNode *)GetCurrentNodeObject())->setRadius(yyvsp[0].fval);
		;
    break;}
case 254:
#line 1086 "vrml.y"
{
			((CylinderNode *)GetCurrentNodeObject())->setHeight(yyvsp[0].fval);
		;
    break;}
case 255:
#line 1093 "vrml.y"
{
			CylinderNode *cylinder = new CylinderNode();
			cylinder->setName(GetDEFName());
			AddNode(cylinder);
			PushNode(VRML_NODETYPE_CYLINDER, cylinder);
		;
    break;}
case 256:
#line 1103 "vrml.y"
{
			CylinderNode *cylinder = (CylinderNode *)GetCurrentNodeObject();
			cylinder->initialize();
			PopNode();
		;
    break;}
case 259:
#line 1123 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setAutoOffset(yyvsp[0].ival);
		;
    break;}
case 260:
#line 1127 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setDiskAngle(yyvsp[0].fval);
		;
    break;}
case 261:
#line 1131 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 262:
#line 1135 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setMaxAngle(yyvsp[0].fval);
		;
    break;}
case 263:
#line 1139 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setMinAngle(yyvsp[0].fval);
		;
    break;}
case 264:
#line 1143 "vrml.y"
{
			((CylinderSensorNode *)GetCurrentNodeObject())->setOffset(yyvsp[0].fval);
		;
    break;}
case 265:
#line 1151 "vrml.y"
{
			CylinderSensorNode *cysensor = new CylinderSensorNode();
			cysensor->setName(GetDEFName());
			AddNode(cysensor);
			PushNode(VRML_NODETYPE_CYLINDERSENSOR, cysensor);
		;
    break;}
case 266:
#line 1161 "vrml.y"
{
			CylinderSensorNode *cysensor = (CylinderSensorNode *)GetCurrentNodeObject();
			cysensor->initialize();
			PopNode();
		;
    break;}
case 269:
#line 1181 "vrml.y"
{
			((DirectionalLightNode *)GetCurrentNodeObject())->setOn(yyvsp[0].ival);
		;
    break;}
case 270:
#line 1185 "vrml.y"
{
			((DirectionalLightNode *)GetCurrentNodeObject())->setIntensity(yyvsp[0].fval);
		;
    break;}
case 271:
#line 1189 "vrml.y"
{
			((DirectionalLightNode *)GetCurrentNodeObject())->setColor(gColor);
		;
    break;}
case 272:
#line 1193 "vrml.y"
{
			((DirectionalLightNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		;
    break;}
case 273:
#line 1197 "vrml.y"
{
			((DirectionalLightNode *)GetCurrentNodeObject())->setAmbientIntensity(yyvsp[0].fval);
		;
    break;}
case 274:
#line 1204 "vrml.y"
{
			DirectionalLightNode *dirLight = new DirectionalLightNode();
			dirLight->setName(GetDEFName());
			AddNode(dirLight);
			PushNode(VRML_NODETYPE_DIRECTIONALLIGHT, dirLight);
		;
    break;}
case 275:
#line 1214 "vrml.y"
{
			DirectionalLightNode *dirLight = (DirectionalLightNode *)GetCurrentNodeObject();
			dirLight->initialize();
			PopNode();
		;
    break;}
case 278:
#line 1234 "vrml.y"
{
			PushNode(VRML_NODETYPE_ELEVATIONGRID_HEIGHT, GetCurrentNodeObject());
		;
    break;}
case 288:
#line 1251 "vrml.y"
{
			PopNode();
		;
    break;}
case 289:
#line 1255 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setCCW(yyvsp[0].ival);
		;
    break;}
case 290:
#line 1259 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setCreaseAngle(yyvsp[0].fval);
		;
    break;}
case 291:
#line 1263 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setSolid(yyvsp[0].ival);
		;
    break;}
case 292:
#line 1267 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setColorPerVertex(yyvsp[0].ival);
		;
    break;}
case 293:
#line 1271 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setNormalPerVertex(yyvsp[0].ival);
		;
    break;}
case 294:
#line 1275 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setXDimension(yyvsp[0].ival);
		;
    break;}
case 295:
#line 1279 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setXSpacing(yyvsp[0].fval);
		;
    break;}
case 296:
#line 1283 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setZDimension(yyvsp[0].ival);
		;
    break;}
case 297:
#line 1287 "vrml.y"
{
			((ElevationGridNode *)GetCurrentNodeObject())->setZSpacing(yyvsp[0].fval);
		;
    break;}
case 298:
#line 1294 "vrml.y"
{
			ElevationGridNode *elev = new ElevationGridNode();
			elev->setName(GetDEFName());
			AddNode(elev);
			PushNode(VRML_NODETYPE_ELEVATIONGRID, elev);
		;
    break;}
case 299:
#line 1304 "vrml.y"
{
			ElevationGridNode *elev = (ElevationGridNode *)GetCurrentNodeObject();
			elev->initialize();
			PopNode();
		;
    break;}
case 302:
#line 1324 "vrml.y"
{
			PushNode(VRML_NODETYPE_EXTRUSION_CROSSSECTION, GetCurrentNodeObject());
		;
    break;}
case 303:
#line 1331 "vrml.y"
{
			PushNode(VRML_NODETYPE_EXTRUSION_ORIENTATION, GetCurrentNodeObject());
		;
    break;}
case 304:
#line 1338 "vrml.y"
{
			PushNode(VRML_NODETYPE_EXTRUSION_SCALE, GetCurrentNodeObject());
		;
    break;}
case 305:
#line 1345 "vrml.y"
{
			PushNode(VRML_NODETYPE_EXTRUSION_SPINE, GetCurrentNodeObject());
		;
    break;}
case 306:
#line 1352 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setBeginCap(yyvsp[0].ival);
		;
    break;}
case 307:
#line 1356 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setCCW(yyvsp[0].ival);
		;
    break;}
case 308:
#line 1360 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setConvex(yyvsp[0].ival);
		;
    break;}
case 309:
#line 1364 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setCreaseAngle(yyvsp[0].fval);
		;
    break;}
case 310:
#line 1368 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setSolid(yyvsp[0].ival);
		;
    break;}
case 311:
#line 1372 "vrml.y"
{
			PopNode();
		;
    break;}
case 312:
#line 1376 "vrml.y"
{
			((ExtrusionNode *)GetCurrentNodeObject())->setEndCap(yyvsp[0].ival);
		;
    break;}
case 313:
#line 1380 "vrml.y"
{
			PopNode();
		;
    break;}
case 314:
#line 1384 "vrml.y"
{
			PopNode();
		;
    break;}
case 315:
#line 1388 "vrml.y"
{
			PopNode();
		;
    break;}
case 316:
#line 1395 "vrml.y"
{
			ExtrusionNode *ex = new ExtrusionNode();
			ex->setName(GetDEFName());
			AddNode(ex);
			PushNode(VRML_NODETYPE_EXTRUSION, ex);
		;
    break;}
case 317:
#line 1405 "vrml.y"
{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->initialize();
			PopNode();
		;
    break;}
case 320:
#line 1425 "vrml.y"
{
			((FogNode *)GetCurrentNodeObject())->setColor(gColor);
		;
    break;}
case 321:
#line 1429 "vrml.y"
{
			((FogNode *)GetCurrentNodeObject())->setFogType(yyvsp[0].sval);
		;
    break;}
case 322:
#line 1433 "vrml.y"
{
			((FogNode *)GetCurrentNodeObject())->setVisibilityRange(yyvsp[0].fval);
		;
    break;}
case 323:
#line 1440 "vrml.y"
{
			FogNode *fog= new FogNode();
			fog->setName(GetDEFName());
			AddNode(fog);
			PushNode(VRML_NODETYPE_FOG, fog);
		;
    break;}
case 324:
#line 1450 "vrml.y"
{
			FogNode *fog= (FogNode *)GetCurrentNodeObject();
			fog->initialize();
			PopNode();
		;
    break;}
case 327:
#line 1470 "vrml.y"
{
			PushNode(VRML_NODETYPE_FONTSTYLE_JUSTIFY, GetCurrentNodeObject());
		;
    break;}
case 328:
#line 1477 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setFamily(yyvsp[0].sval);
		;
    break;}
case 329:
#line 1481 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setHorizontal(yyvsp[0].ival);
		;
    break;}
case 330:
#line 1485 "vrml.y"
{
			PopNode();
		;
    break;}
case 331:
#line 1489 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setLanguage(yyvsp[0].sval);
		;
    break;}
case 332:
#line 1493 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setLeftToRight(yyvsp[0].ival);
		;
    break;}
case 333:
#line 1497 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setSize(yyvsp[0].fval);
		;
    break;}
case 334:
#line 1501 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setSpacing(yyvsp[0].fval);
		;
    break;}
case 335:
#line 1505 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setStyle(yyvsp[0].sval);
		;
    break;}
case 336:
#line 1509 "vrml.y"
{
			((FontStyleNode *)GetCurrentNodeObject())->setTopToBottom(yyvsp[0].ival);
		;
    break;}
case 337:
#line 1516 "vrml.y"
{
			FontStyleNode *fs = new FontStyleNode();
			fs->setName(GetDEFName());
			AddNode(fs);
			PushNode(VRML_NODETYPE_FONTSTYLE, fs);
		;
    break;}
case 338:
#line 1526 "vrml.y"
{
			FontStyleNode *fs = (FontStyleNode *)GetCurrentNodeObject();
			fs->initialize();
			PopNode();
		;
    break;}
case 344:
#line 1552 "vrml.y"
{
			GroupNode *group = new GroupNode();
			group->setName(GetDEFName());
			AddNode(group);
			PushNode(VRML_NODETYPE_GROUP, group);
		;
    break;}
case 345:
#line 1562 "vrml.y"
{
			GroupNode *group = (GroupNode *)GetCurrentNodeObject();
			group->initialize();
			PopNode();
		;
    break;}
case 348:
#line 1582 "vrml.y"
{
			PushNode(VRML_NODETYPE_IMAGETEXTURE_URL, GetCurrentNodeObject());
		;
    break;}
case 349:
#line 1589 "vrml.y"
{
			PopNode();
		;
    break;}
case 350:
#line 1593 "vrml.y"
{
			((ImageTextureNode *)GetCurrentNodeObject())->setRepeatS(yyvsp[0].ival);
		;
    break;}
case 351:
#line 1597 "vrml.y"
{
			((ImageTextureNode *)GetCurrentNodeObject())->setRepeatT(yyvsp[0].ival);
		;
    break;}
case 352:
#line 1604 "vrml.y"
{
			ImageTextureNode *imgTexture = new ImageTextureNode();
			imgTexture->setName(GetDEFName());
			AddNode(imgTexture);
			PushNode(VRML_NODETYPE_IMAGETEXTURE, imgTexture);
		;
    break;}
case 353:
#line 1614 "vrml.y"
{
			ImageTextureNode *imgTexture = (ImageTextureNode *)GetCurrentNodeObject();
			imgTexture->initialize();
			PopNode();
		;
    break;}
case 356:
#line 1634 "vrml.y"
{
			PushNode(VRML_NODETYPE_COLOR_INDEX, GetCurrentNodeObject());
		;
    break;}
case 357:
#line 1641 "vrml.y"
{
			PushNode(VRML_NODETYPE_COORDINATE_INDEX, GetCurrentNodeObject());
		;
    break;}
case 358:
#line 1648 "vrml.y"
{
			PushNode(VRML_NODETYPE_NORMAL_INDEX, GetCurrentNodeObject());
		;
    break;}
case 359:
#line 1655 "vrml.y"
{
			PushNode(VRML_NODETYPE_TEXTURECOODINATE_INDEX, GetCurrentNodeObject());
		;
    break;}
case 372:
#line 1674 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setCCW(yyvsp[0].ival);
		;
    break;}
case 373:
#line 1678 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setConvex(yyvsp[0].ival);
		;
    break;}
case 374:
#line 1682 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setSolid(yyvsp[0].ival);
		;
    break;}
case 375:
#line 1686 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setCreaseAngle(yyvsp[0].fval);
		;
    break;}
case 376:
#line 1690 "vrml.y"
{
			PopNode();
		;
    break;}
case 377:
#line 1694 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setColorPerVertex(yyvsp[0].ival);
		;
    break;}
case 378:
#line 1698 "vrml.y"
{
			PopNode();
		;
    break;}
case 379:
#line 1702 "vrml.y"
{
			PopNode();
		;
    break;}
case 380:
#line 1706 "vrml.y"
{
			PopNode();
		;
    break;}
case 381:
#line 1710 "vrml.y"
{
			((IndexedFaceSetNode *)GetCurrentNodeObject())->setNormalPerVertex(yyvsp[0].ival);
		;
    break;}
case 382:
#line 1717 "vrml.y"
{
			IndexedFaceSetNode	*idxFaceset = new IndexedFaceSetNode();
			idxFaceset->setName(GetDEFName());
			AddNode(idxFaceset);
			PushNode(VRML_NODETYPE_INDEXEDFACESET, idxFaceset);
		;
    break;}
case 383:
#line 1727 "vrml.y"
{
			IndexedFaceSetNode *idxFaceset = (IndexedFaceSetNode *)GetCurrentNodeObject();
			idxFaceset->initialize();
			PopNode();
		;
    break;}
case 392:
#line 1753 "vrml.y"
{
			((IndexedLineSetNode *)GetCurrentNodeObject())->setColorPerVertex(yyvsp[0].ival);
		;
    break;}
case 393:
#line 1757 "vrml.y"
{
			PopNode();
		;
    break;}
case 394:
#line 1761 "vrml.y"
{
			PopNode();
		;
    break;}
case 395:
#line 1768 "vrml.y"
{
			IndexedLineSetNode	*idxLineset = new IndexedLineSetNode();
			idxLineset->setName(GetDEFName());
			AddNode(idxLineset);
			PushNode(VRML_NODETYPE_INDEXEDLINESET, idxLineset);
		;
    break;}
case 396:
#line 1778 "vrml.y"
{
			IndexedLineSetNode *idxLineset = (IndexedLineSetNode *)GetCurrentNodeObject();
			idxLineset->initialize();
			PopNode();
		;
    break;}
case 399:
#line 1798 "vrml.y"
{
			PushNode(VRML_NODETYPE_INLINE_URL, GetCurrentNodeObject());
		;
    break;}
case 400:
#line 1805 "vrml.y"
{
			PopNode();
		;
    break;}
case 403:
#line 1814 "vrml.y"
{
			InlineNode *inlineNode = new InlineNode();
			inlineNode->setName(GetDEFName());
			AddNode(inlineNode);
			PushNode(VRML_NODETYPE_INLINE, inlineNode);
		;
    break;}
case 404:
#line 1824 "vrml.y"
{
			InlineNode *inlineNode = (InlineNode *)GetCurrentNodeObject();
			//inlineNode->initialize();
			PopNode();
		;
    break;}
case 407:
#line 1844 "vrml.y"
{
			PushNode(VRML_NODETYPE_LOD_RANGE, GetCurrentNodeObject());
		;
    break;}
case 408:
#line 1852 "vrml.y"
{
			PushNode(VRML_NODETYPE_LOD_LEVEL, GetCurrentNodeObject());
		;
    break;}
case 409:
#line 1859 "vrml.y"
{
			PopNode();
		;
    break;}
case 410:
#line 1863 "vrml.y"
{
			((LodNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		;
    break;}
case 411:
#line 1867 "vrml.y"
{
			PopNode();
		;
    break;}
case 412:
#line 1871 "vrml.y"
{
			PopNode();
		;
    break;}
case 413:
#line 1878 "vrml.y"
{
			LodNode	*lod = new LodNode();
			lod->setName(GetDEFName());
			AddNode(lod);
			PushNode(VRML_NODETYPE_INLINE, lod);
		;
    break;}
case 414:
#line 1888 "vrml.y"
{
			LodNode	*lod = (LodNode *)GetCurrentNodeObject();
			lod->initialize();
			PopNode();
		;
    break;}
case 417:
#line 1908 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setAmbientIntensity(yyvsp[0].fval);
		;
    break;}
case 418:
#line 1912 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setDiffuseColor(gColor);
		;
    break;}
case 419:
#line 1916 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setEmissiveColor(gColor);
		;
    break;}
case 420:
#line 1920 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setShininess(yyvsp[0].fval);
		;
    break;}
case 421:
#line 1924 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setSpecularColor(gColor);
		;
    break;}
case 422:
#line 1928 "vrml.y"
{
			((MaterialNode *)GetCurrentNodeObject())->setTransparency(yyvsp[0].fval);
		;
    break;}
case 423:
#line 1934 "vrml.y"
{
			MaterialNode *material = new MaterialNode();
			material->setName(GetDEFName());
			AddNode(material);
			PushNode(VRML_NODETYPE_MATERIAL, material);
		;
    break;}
case 424:
#line 1944 "vrml.y"
{
			MaterialNode *material = (MaterialNode *)GetCurrentNodeObject();
			material->initialize();
			PopNode();
		;
    break;}
case 427:
#line 1964 "vrml.y"
{
			PushNode(VRML_NODETYPE_MOVIETEXTURE_URL, GetCurrentNodeObject());
		;
    break;}
case 428:
#line 1971 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setLoop(yyvsp[0].ival);
		;
    break;}
case 429:
#line 1975 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setSpeed(yyvsp[0].fval);
		;
    break;}
case 430:
#line 1979 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setStartTime(yyvsp[0].fval);
		;
    break;}
case 431:
#line 1983 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setStopTime(yyvsp[0].fval);
		;
    break;}
case 432:
#line 1987 "vrml.y"
{
			PopNode();
		;
    break;}
case 433:
#line 1991 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setRepeatS(yyvsp[0].ival);
		;
    break;}
case 434:
#line 1995 "vrml.y"
{
			((MovieTextureNode *)GetCurrentNodeObject())->setRepeatT(yyvsp[0].ival);
		;
    break;}
case 435:
#line 2002 "vrml.y"
{
			MovieTextureNode *movieTexture = new MovieTextureNode();
			movieTexture->setName(GetDEFName());
			AddNode(movieTexture);
			PushNode(VRML_NODETYPE_MOVIETEXTURE, movieTexture);
		;
    break;}
case 436:
#line 2012 "vrml.y"
{
			MovieTextureNode *movieTexture = (MovieTextureNode *)GetCurrentNodeObject();
			movieTexture->initialize();
			PopNode();
		;
    break;}
case 439:
#line 2032 "vrml.y"
{
			PushNode(VRML_NODETYPE_NAVIGATIONINFO_AVATARSIZE, GetCurrentNodeObject());
		;
    break;}
case 440:
#line 2039 "vrml.y"
{
			PushNode(VRML_NODETYPE_NAVIGATIONINFO_TYPE, GetCurrentNodeObject());
		;
    break;}
case 441:
#line 2046 "vrml.y"
{
			PopNode();
		;
    break;}
case 442:
#line 2050 "vrml.y"
{
			((NavigationInfoNode *)GetCurrentNodeObject())->setHeadlight(yyvsp[0].ival);
		;
    break;}
case 443:
#line 2054 "vrml.y"
{
			((NavigationInfoNode *)GetCurrentNodeObject())->setSpeed(yyvsp[0].fval);
		;
    break;}
case 444:
#line 2058 "vrml.y"
{
			PopNode();
		;
    break;}
case 445:
#line 2062 "vrml.y"
{
			((NavigationInfoNode *)GetCurrentNodeObject())->setVisibilityLimit(yyvsp[0].fval);
		;
    break;}
case 446:
#line 2069 "vrml.y"
{
			NavigationInfoNode *navInfo = new NavigationInfoNode();
			navInfo->setName(GetDEFName());
			AddNode(navInfo);
			PushNode(VRML_NODETYPE_NAVIGATIONINFO, navInfo);
		;
    break;}
case 447:
#line 2079 "vrml.y"
{
			NavigationInfoNode *navInfo = (NavigationInfoNode *)GetCurrentNodeObject();
			navInfo->initialize();
			PopNode();
		;
    break;}
case 451:
#line 2103 "vrml.y"
{
			NormalNode *normal = new NormalNode();
			normal->setName(GetDEFName());
			AddNode(normal);
			PushNode(VRML_NODETYPE_NORMAL, normal);
		;
    break;}
case 452:
#line 2113 "vrml.y"
{
			NormalNode *normal = (NormalNode *)GetCurrentNodeObject();
			normal->initialize();
			PopNode();
		;
    break;}
case 455:
#line 2133 "vrml.y"
{
			PopNode();
		;
    break;}
case 456:
#line 2137 "vrml.y"
{
			PopNode();
		;
    break;}
case 457:
#line 2141 "vrml.y"
{
		;
    break;}
case 458:
#line 2147 "vrml.y"
{
			NormalInterpolatorNode *normInterp = new NormalInterpolatorNode();
			normInterp->setName(GetDEFName());
			AddNode(normInterp);
			PushNode(VRML_NODETYPE_NORMALINTERPOLATOR, normInterp);
		;
    break;}
case 459:
#line 2157 "vrml.y"
{
			NormalInterpolatorNode *normInterp = (NormalInterpolatorNode *)GetCurrentNodeObject();
			normInterp->initialize();
			PopNode();
		;
    break;}
case 462:
#line 2177 "vrml.y"
{
			PopNode();
		;
    break;}
case 463:
#line 2181 "vrml.y"
{
			PopNode();
		;
    break;}
case 464:
#line 2185 "vrml.y"
{
		;
    break;}
case 465:
#line 2191 "vrml.y"
{
			OrientationInterpolatorNode *oriInterp = new OrientationInterpolatorNode();
			oriInterp->setName(GetDEFName());
			AddNode(oriInterp);
			PushNode(VRML_NODETYPE_ORIENTATIONINTERPOLATOR, oriInterp);
		;
    break;}
case 466:
#line 2201 "vrml.y"
{
			OrientationInterpolatorNode *oriInterp = (OrientationInterpolatorNode *)GetCurrentNodeObject();
			oriInterp->initialize();
			PopNode();
		;
    break;}
case 469:
#line 2221 "vrml.y"
{
			PushNode(VRML_NODETYPE_PIXELTEXTURE_IMAGE, GetCurrentNodeObject());
		;
    break;}
case 470:
#line 2228 "vrml.y"
{
			PopNode();
		;
    break;}
case 471:
#line 2232 "vrml.y"
{
			((PixelTextureNode *)GetCurrentNodeObject())->setRepeatS(yyvsp[0].ival);
		;
    break;}
case 472:
#line 2236 "vrml.y"
{
			((PixelTextureNode *)GetCurrentNodeObject())->setRepeatT(yyvsp[0].ival);
		;
    break;}
case 473:
#line 2243 "vrml.y"
{
			PixelTextureNode *pixTexture = new PixelTextureNode();
			pixTexture->setName(GetDEFName());
			AddNode(pixTexture);
			PushNode(VRML_NODETYPE_PIXELTEXTURE, pixTexture);
		;
    break;}
case 474:
#line 2253 "vrml.y"
{
			PixelTextureNode *pixTexture = (PixelTextureNode *)GetCurrentNodeObject();
			pixTexture->initialize();
			PopNode();
		;
    break;}
case 477:
#line 2273 "vrml.y"
{
			((PlaneSensorNode *)GetCurrentNodeObject())->setAutoOffset(yyvsp[0].ival);
		;
    break;}
case 478:
#line 2277 "vrml.y"
{
			((PlaneSensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 479:
#line 2281 "vrml.y"
{
			((PlaneSensorNode *)GetCurrentNodeObject())->setMaxPosition(gVec2f);
		;
    break;}
case 480:
#line 2285 "vrml.y"
{
			((PlaneSensorNode *)GetCurrentNodeObject())->setMinPosition(gVec2f);
		;
    break;}
case 481:
#line 2289 "vrml.y"
{
			((PlaneSensorNode *)GetCurrentNodeObject())->setOffset(gVec3f);
		;
    break;}
case 482:
#line 2296 "vrml.y"
{
			PlaneSensorNode *psensor = new PlaneSensorNode();
			psensor->setName(GetDEFName());
			AddNode(psensor);
			PushNode(VRML_NODETYPE_PLANESENSOR, psensor);
		;
    break;}
case 483:
#line 2306 "vrml.y"
{
			PlaneSensorNode *psensor = (PlaneSensorNode *)GetCurrentNodeObject();
			psensor->initialize();
			PopNode();
		;
    break;}
case 486:
#line 2327 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setAmbientIntensity(yyvsp[0].fval);
		;
    break;}
case 487:
#line 2331 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setAttenuation(gVec3f);
		;
    break;}
case 488:
#line 2335 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setColor(gColor);
		;
    break;}
case 489:
#line 2339 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setIntensity(yyvsp[0].fval);
		;
    break;}
case 490:
#line 2343 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		;
    break;}
case 491:
#line 2347 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setOn(yyvsp[0].ival);
		;
    break;}
case 492:
#line 2351 "vrml.y"
{
			((PointLightNode *)GetCurrentNodeObject())->setRadius(yyvsp[0].fval);
		;
    break;}
case 493:
#line 2358 "vrml.y"
{
			PointLightNode *pointLight = new PointLightNode();
			pointLight->setName(GetDEFName());
			AddNode(pointLight);
			PushNode(VRML_NODETYPE_POINTLIGHT, pointLight);
		;
    break;}
case 494:
#line 2368 "vrml.y"
{
			PointLightNode *pointLight = (PointLightNode *)GetCurrentNodeObject();
			pointLight->initialize();
			PopNode();
		;
    break;}
case 503:
#line 2398 "vrml.y"
{
			PointSetNode *pset = new PointSetNode();
			pset->setName(GetDEFName());
			AddNode(pset);
			PushNode(VRML_NODETYPE_POINTSET, pset);
		;
    break;}
case 504:
#line 2408 "vrml.y"
{
			PointSetNode *pset = (PointSetNode *)GetCurrentNodeObject();
			pset->initialize();
			PopNode();
		;
    break;}
case 507:
#line 2427 "vrml.y"
{
			PopNode();
		;
    break;}
case 508:
#line 2431 "vrml.y"
{
			PopNode();
		;
    break;}
case 509:
#line 2435 "vrml.y"
{
		;
    break;}
case 510:
#line 2441 "vrml.y"
{
			PositionInterpolatorNode *posInterp = new PositionInterpolatorNode();
			posInterp->setName(GetDEFName());
			AddNode(posInterp);
			PushNode(VRML_NODETYPE_POSITIONINTERPOLATOR, posInterp);
		;
    break;}
case 511:
#line 2451 "vrml.y"
{
			PositionInterpolatorNode *posInterp = (PositionInterpolatorNode *)GetCurrentNodeObject();
			posInterp->initialize();
			PopNode();
		;
    break;}
case 514:
#line 2471 "vrml.y"
{
			((ProximitySensorNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		;
    break;}
case 515:
#line 2475 "vrml.y"
{
			((ProximitySensorNode *)GetCurrentNodeObject())->setSize(gVec3f);
		;
    break;}
case 516:
#line 2479 "vrml.y"
{
			((ProximitySensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 517:
#line 2486 "vrml.y"
{
			ProximitySensorNode *psensor = new ProximitySensorNode();
			psensor->setName(GetDEFName());
			AddNode(psensor);
			PushNode(VRML_NODETYPE_PROXIMITYSENSOR, psensor);
		;
    break;}
case 518:
#line 2496 "vrml.y"
{
			ProximitySensorNode *psensor = (ProximitySensorNode *)GetCurrentNodeObject();
			psensor->initialize();
			PopNode();
		;
    break;}
case 521:
#line 2516 "vrml.y"
{
			PopNode();
		;
    break;}
case 522:
#line 2520 "vrml.y"
{
			PopNode();
		;
    break;}
case 523:
#line 2524 "vrml.y"
{
		;
    break;}
case 524:
#line 2530 "vrml.y"
{
			ScalarInterpolatorNode *scalarInterp = new ScalarInterpolatorNode();
			scalarInterp->setName(GetDEFName());
			AddNode(scalarInterp);
			PushNode(VRML_NODETYPE_SCALARINTERPOLATOR, scalarInterp);
		;
    break;}
case 525:
#line 2540 "vrml.y"
{
			ScalarInterpolatorNode *scalarInterp = (ScalarInterpolatorNode *)GetCurrentNodeObject();
			scalarInterp->initialize();
			PopNode();
		;
    break;}
case 528:
#line 2560 "vrml.y"
{
			PushNode(VRML_NODETYPE_SCRIPT_URL, GetCurrentNodeObject());
		;
    break;}
case 529:
#line 2567 "vrml.y"
{
			PopNode();
		;
    break;}
case 530:
#line 2571 "vrml.y"
{
			((ScriptNode *)GetCurrentNodeObject())->setDirectOutput(yyvsp[0].ival);
		;
    break;}
case 531:
#line 2575 "vrml.y"
{
			((ScriptNode *)GetCurrentNodeObject())->setMustEvaluate(yyvsp[0].ival);
		;
    break;}
case 532:
#line 2584 "vrml.y"
{
			SFBool *value = new SFBool();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 533:
#line 2590 "vrml.y"
{
			SFFloat *value = new SFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 534:
#line 2596 "vrml.y"
{
			SFInt32 *value = new SFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 535:
#line 2602 "vrml.y"
{
			SFTime *value = new SFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 536:
#line 2608 "vrml.y"
{
			SFRotation *value = new SFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 537:
#line 2622 "vrml.y"
{
			SFColor *value = new SFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 538:
#line 2628 "vrml.y"
{
			SFImage *value = new SFImage();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 539:
#line 2634 "vrml.y"
{
			SFString *value = new SFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 540:
#line 2640 "vrml.y"
{
			SFVec2f *value = new SFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 541:
#line 2646 "vrml.y"
{
			SFVec3f *value = new SFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 542:
#line 2657 "vrml.y"
{
			MFFloat *value = new MFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 543:
#line 2663 "vrml.y"
{
			MFInt32 *value = new MFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 544:
#line 2669 "vrml.y"
{
			MFTime *value = new MFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 545:
#line 2675 "vrml.y"
{
			MFRotation *value = new MFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 546:
#line 2689 "vrml.y"
{
			MFColor *value = new MFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 547:
#line 2695 "vrml.y"
{
			MFString *value = new MFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 548:
#line 2701 "vrml.y"
{
			MFVec2f *value = new MFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 549:
#line 2707 "vrml.y"
{
			MFVec3f *value = new MFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventIn(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 550:
#line 2718 "vrml.y"
{
			SFBool *value = new SFBool();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 551:
#line 2724 "vrml.y"
{
			SFFloat *value = new SFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 552:
#line 2730 "vrml.y"
{
			SFInt32 *value = new SFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 553:
#line 2736 "vrml.y"
{
			SFTime *value = new SFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 554:
#line 2742 "vrml.y"
{
			SFRotation *value = new SFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 555:
#line 2756 "vrml.y"
{
			SFColor *value = new SFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 556:
#line 2762 "vrml.y"
{
			SFImage *value = new SFImage();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 557:
#line 2768 "vrml.y"
{
			SFString *value = new SFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 558:
#line 2774 "vrml.y"
{
			SFVec2f *value = new SFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 559:
#line 2780 "vrml.y"
{
			SFVec3f *value = new SFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 560:
#line 2791 "vrml.y"
{
			MFFloat *value = new MFFloat();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 561:
#line 2797 "vrml.y"
{
			MFInt32 *value = new MFInt32();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 562:
#line 2803 "vrml.y"
{
			MFTime *value = new MFTime();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 563:
#line 2809 "vrml.y"
{
			MFRotation *value = new MFRotation();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 564:
#line 2823 "vrml.y"
{
			MFColor *value = new MFColor();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 565:
#line 2829 "vrml.y"
{
			MFString *value = new MFString();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 566:
#line 2835 "vrml.y"
{
			MFVec2f *value = new MFVec2f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 567:
#line 2841 "vrml.y"
{
			MFVec3f *value = new MFVec3f();
			((ScriptNode *)GetCurrentNodeObject())->addEventOut(yyvsp[0].sval, value);
			delete[] yyvsp[0].sval;
		;
    break;}
case 568:
#line 2852 "vrml.y"
{
			SFBool *value = new SFBool(yyvsp[0].ival);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 569:
#line 2858 "vrml.y"
{
			SFFloat *value = new SFFloat(yyvsp[0].fval);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 570:
#line 2864 "vrml.y"
{
			SFInt32 *value = new SFInt32(yyvsp[0].ival);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 571:
#line 2870 "vrml.y"
{
			SFTime *value = new SFTime(yyvsp[0].fval);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 572:
#line 2876 "vrml.y"
{
			SFRotation *value = new SFRotation(gRotation);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 573:
#line 2883 "vrml.y"
{
			SFNode *value = new SFNode();
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 574:
#line 2890 "vrml.y"
{
			Node *node = GetParserObject()->findNodeByName(yyvsp[0].sval);
			SFNode *value = new SFNode(node);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-2].sval, value);
			delete[] yyvsp[-2].sval; delete[] yyvsp[0].sval;
		;
    break;}
case 575:
#line 2898 "vrml.y"
{
			SFColor *value = new SFColor(gColor);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 576:
#line 2912 "vrml.y"
{
			SFString *value = new SFString(yyvsp[0].sval);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 577:
#line 2918 "vrml.y"
{
			SFVec2f *value = new SFVec2f(gVec2f);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 578:
#line 2924 "vrml.y"
{
			SFVec3f *value = new SFVec3f(gVec3f);
			((ScriptNode *)GetCurrentNodeObject())->addField(yyvsp[-1].sval, value);
			delete[] yyvsp[-1].sval;
		;
    break;}
case 579:
#line 2934 "vrml.y"
{
			ScriptNode *script = new ScriptNode();
			script->setName(GetDEFName());
			AddNode(script);
			PushNode(VRML_NODETYPE_SCRIPT, script);
		;
    break;}
case 580:
#line 2944 "vrml.y"
{
			ScriptNode *script = (ScriptNode *)GetCurrentNodeObject();
			script->initialize();
			PopNode();
		;
    break;}
case 589:
#line 2974 "vrml.y"
{
			ShapeNode *shape = new ShapeNode();
			shape->setName(GetDEFName());
			AddNode(shape);
			PushNode(VRML_NODETYPE_SHAPE, shape);
		;
    break;}
case 590:
#line 2984 "vrml.y"
{
			ShapeNode *shape = (ShapeNode *)GetCurrentNodeObject();
			shape->initialize();
			PopNode();
		;
    break;}
case 593:
#line 3004 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		;
    break;}
case 594:
#line 3008 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setIntensity(yyvsp[0].fval);
		;
    break;}
case 595:
#line 3012 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		;
    break;}
case 596:
#line 3016 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setMinBack(yyvsp[0].fval);
		;
    break;}
case 597:
#line 3020 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setMaxFront(yyvsp[0].fval);
		;
    break;}
case 598:
#line 3024 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setMinBack(yyvsp[0].fval);
		;
    break;}
case 599:
#line 3028 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setMinFront(yyvsp[0].fval);
		;
    break;}
case 600:
#line 3032 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setPriority(yyvsp[0].fval);
		;
    break;}
case 605:
#line 3040 "vrml.y"
{
			((SoundNode *)GetCurrentNodeObject())->setSpatialize(yyvsp[0].ival);
		;
    break;}
case 606:
#line 3047 "vrml.y"
{
			SoundNode *sound = new SoundNode();
			sound->setName(GetDEFName());
			AddNode(sound);
			PushNode(VRML_NODETYPE_SOUND, sound);
		;
    break;}
case 607:
#line 3057 "vrml.y"
{
			SoundNode *sound = (SoundNode *)GetCurrentNodeObject();
			sound->initialize();
			PopNode();
		;
    break;}
case 610:
#line 3077 "vrml.y"
{
			((SphereNode *)GetCurrentNodeObject())->setRadius(yyvsp[0].fval);
		;
    break;}
case 611:
#line 3084 "vrml.y"
{
			SphereNode *sphere = new SphereNode();
			sphere->setName(GetDEFName());
			AddNode(sphere);
			PushNode(VRML_NODETYPE_SPHERE, sphere);
		;
    break;}
case 612:
#line 3094 "vrml.y"
{
			SphereNode *sphere = (SphereNode *)GetCurrentNodeObject();
			sphere->initialize();
			PopNode();
		;
    break;}
case 615:
#line 3114 "vrml.y"
{
			((SphereSensorNode *)GetCurrentNodeObject())->setAutoOffset(yyvsp[0].ival);
		;
    break;}
case 616:
#line 3118 "vrml.y"
{
			((SphereSensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 617:
#line 3122 "vrml.y"
{
			((SphereSensorNode *)GetCurrentNodeObject())->setOffset(gRotation);
		;
    break;}
case 618:
#line 3129 "vrml.y"
{
			SphereSensorNode *spsensor = new SphereSensorNode();
			spsensor->setName(GetDEFName());
			AddNode(spsensor);
			PushNode(VRML_NODETYPE_SPHERESENSOR, spsensor);
		;
    break;}
case 619:
#line 3139 "vrml.y"
{
			SphereSensorNode *spsensor = (SphereSensorNode *)GetCurrentNodeObject();
			spsensor->initialize();
			PopNode();
		;
    break;}
case 622:
#line 3159 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setAmbientIntensity(yyvsp[0].fval);
		;
    break;}
case 623:
#line 3163 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setAttenuation(gVec3f);
		;
    break;}
case 624:
#line 3167 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setBeamWidth(yyvsp[0].fval);
		;
    break;}
case 625:
#line 3171 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setColor(gColor);
		;
    break;}
case 626:
#line 3175 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setCutOffAngle(yyvsp[0].fval);
		;
    break;}
case 627:
#line 3179 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setDirection(gVec3f);
		;
    break;}
case 628:
#line 3183 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setIntensity(yyvsp[0].fval);
		;
    break;}
case 629:
#line 3187 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setLocation(gVec3f);
		;
    break;}
case 630:
#line 3191 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setOn(yyvsp[0].ival);
		;
    break;}
case 631:
#line 3195 "vrml.y"
{
			((SpotLightNode *)GetCurrentNodeObject())->setRadius(yyvsp[0].fval);
		;
    break;}
case 632:
#line 3202 "vrml.y"
{
			SpotLightNode *spotLight = new SpotLightNode();
			spotLight->setName(GetDEFName());
			AddNode(spotLight);
			PushNode(VRML_NODETYPE_SPOTLIGHT, spotLight);
		;
    break;}
case 633:
#line 3212 "vrml.y"
{
			SpotLightNode *spotLight = (SpotLightNode *)GetCurrentNodeObject();
			spotLight->initialize();
			PopNode();
		;
    break;}
case 636:
#line 3232 "vrml.y"
{
			PushNode(VRML_NODETYPE_SWITCH_CHOICE, GetCurrentNodeObject());
		;
    break;}
case 637:
#line 3239 "vrml.y"
{
			PopNode();
		;
    break;}
case 638:
#line 3243 "vrml.y"
{
			PopNode();
		;
    break;}
case 639:
#line 3247 "vrml.y"
{
			((SwitchNode *)GetCurrentNodeObject())->setWhichChoice(yyvsp[0].ival);
		;
    break;}
case 640:
#line 3255 "vrml.y"
{
			SwitchNode *switchNode = new SwitchNode();
			switchNode->setName(GetDEFName());
			AddNode(switchNode);
			PushNode(VRML_NODETYPE_SWITCH, switchNode);
		;
    break;}
case 641:
#line 3265 "vrml.y"
{
			SwitchNode *switchNode = (SwitchNode *)GetCurrentNodeObject();
			switchNode->initialize();
			PopNode();
		;
    break;}
case 644:
#line 3285 "vrml.y"
{
			PushNode(VRML_NODETYPE_TEXT_STRING, GetCurrentNodeObject());
		;
    break;}
case 645:
#line 3292 "vrml.y"
{
			PushNode(VRML_NODETYPE_TEXT_LENGTH, GetCurrentNodeObject());
		;
    break;}
case 646:
#line 3299 "vrml.y"
{
			PopNode();
		;
    break;}
case 650:
#line 3306 "vrml.y"
{
			PopNode();
		;
    break;}
case 651:
#line 3310 "vrml.y"
{
			((TextNode *)GetCurrentNodeObject())->setMaxExtent(yyvsp[0].fval);
		;
    break;}
case 652:
#line 3318 "vrml.y"
{
			TextNode *text = new TextNode();
			text->setName(GetDEFName());
			AddNode(text);
			PushNode(VRML_NODETYPE_TEXT, text);
		;
    break;}
case 653:
#line 3328 "vrml.y"
{
			TextNode *text = (TextNode *)GetCurrentNodeObject();
			text->initialize();
			PopNode();
		;
    break;}
case 657:
#line 3353 "vrml.y"
{
			TextureCoordinateNode *texCoord = new TextureCoordinateNode();
			texCoord->setName(GetDEFName());
			AddNode(texCoord);
			PushNode(VRML_NODETYPE_TEXTURECOODINATE, texCoord);
		;
    break;}
case 658:
#line 3363 "vrml.y"
{
			TextureCoordinateNode *texCoord = (TextureCoordinateNode *)GetCurrentNodeObject();
			texCoord->initialize();
			PopNode();
		;
    break;}
case 661:
#line 3383 "vrml.y"
{
			((TextureTransformNode *)GetCurrentNodeObject())->setCenter(gVec2f);
		;
    break;}
case 662:
#line 3387 "vrml.y"
{
			((TextureTransformNode *)GetCurrentNodeObject())->setRotation(yyvsp[0].fval);
		;
    break;}
case 663:
#line 3391 "vrml.y"
{
			((TextureTransformNode *)GetCurrentNodeObject())->setScale(gVec2f);
		;
    break;}
case 664:
#line 3395 "vrml.y"
{
			((TextureTransformNode *)GetCurrentNodeObject())->setTranslation(gVec2f);
		;
    break;}
case 665:
#line 3403 "vrml.y"
{
			TextureTransformNode *textureTransform = new TextureTransformNode();
			textureTransform->setName(GetDEFName());
			AddNode(textureTransform);
			PushNode(VRML_NODETYPE_TEXTURETRANSFORM, textureTransform);
		;
    break;}
case 666:
#line 3413 "vrml.y"
{
			TextureTransformNode *textureTransform = (TextureTransformNode *)GetCurrentNodeObject();
			textureTransform->initialize();
			PopNode();
		;
    break;}
case 669:
#line 3433 "vrml.y"
{
			((TimeSensorNode *)GetCurrentNodeObject())->setCycleInterval(yyvsp[0].fval);
		;
    break;}
case 670:
#line 3437 "vrml.y"
{
			((TimeSensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 671:
#line 3441 "vrml.y"
{
			((TimeSensorNode *)GetCurrentNodeObject())->setLoop(yyvsp[0].ival);
		;
    break;}
case 672:
#line 3445 "vrml.y"
{
			((TimeSensorNode *)GetCurrentNodeObject())->setStartTime(yyvsp[0].fval);
		;
    break;}
case 673:
#line 3449 "vrml.y"
{
			((TimeSensorNode *)GetCurrentNodeObject())->setStopTime(yyvsp[0].fval);
		;
    break;}
case 674:
#line 3457 "vrml.y"
{
			TimeSensorNode *tsensor = new TimeSensorNode();
			tsensor->setName(GetDEFName());
			AddNode(tsensor);
			PushNode(VRML_NODETYPE_TIMESENSOR, tsensor);
		;
    break;}
case 675:
#line 3467 "vrml.y"
{
			TimeSensorNode *tsensor = (TimeSensorNode *)GetCurrentNodeObject();
			tsensor->initialize();
			PopNode();
		;
    break;}
case 678:
#line 3487 "vrml.y"
{
			((TouchSensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 679:
#line 3494 "vrml.y"
{
			TouchSensorNode *touchSensor = new TouchSensorNode();
			touchSensor->setName(GetDEFName());
			AddNode(touchSensor);
			PushNode(VRML_NODETYPE_TOUCHSENSOR, touchSensor);
		;
    break;}
case 680:
#line 3504 "vrml.y"
{
			TouchSensorNode *touchSensor = (TouchSensorNode *)GetCurrentNodeObject();
			touchSensor->initialize();
			PopNode();
		;
    break;}
case 684:
#line 3525 "vrml.y"
{
			((TransformNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		;
    break;}
case 685:
#line 3529 "vrml.y"
{
			((TransformNode *)GetCurrentNodeObject())->setRotation(gRotation);
		;
    break;}
case 686:
#line 3533 "vrml.y"
{
			((TransformNode *)GetCurrentNodeObject())->setScale(gVec3f);
		;
    break;}
case 687:
#line 3537 "vrml.y"
{
			((TransformNode *)GetCurrentNodeObject())->setScaleOrientation(gRotation);
		;
    break;}
case 688:
#line 3541 "vrml.y"
{
			((TransformNode *)GetCurrentNodeObject())->setTranslation(gVec3f);
		;
    break;}
case 691:
#line 3550 "vrml.y"
{
			TransformNode *transform = new TransformNode();
			transform->setName(GetDEFName());
			AddNode(transform);
			PushNode(VRML_NODETYPE_TRANSFORM, transform);
		;
    break;}
case 692:
#line 3560 "vrml.y"
{
			TransformNode *transform = (TransformNode *)GetCurrentNodeObject();
			transform->initialize();
			PopNode();
		;
    break;}
case 695:
#line 3580 "vrml.y"
{
			((ViewpointNode *)GetCurrentNodeObject())->setFieldOfView(yyvsp[0].fval);
		;
    break;}
case 696:
#line 3584 "vrml.y"
{
			((ViewpointNode *)GetCurrentNodeObject())->setJump(yyvsp[0].ival);
		;
    break;}
case 697:
#line 3588 "vrml.y"
{
			((ViewpointNode *)GetCurrentNodeObject())->setOrientation(gRotation);
		;
    break;}
case 698:
#line 3592 "vrml.y"
{
			((ViewpointNode *)GetCurrentNodeObject())->setPosition(gVec3f);
		;
    break;}
case 699:
#line 3596 "vrml.y"
{
			((ViewpointNode *)GetCurrentNodeObject())->setDescription(yyvsp[0].sval);
		;
    break;}
case 700:
#line 3603 "vrml.y"
{
			ViewpointNode *viewpoint = new ViewpointNode();
			viewpoint->setName(GetDEFName());
			AddNode(viewpoint);
			PushNode(VRML_NODETYPE_VIEWPOINT, viewpoint);
		;
    break;}
case 701:
#line 3613 "vrml.y"
{
			ViewpointNode *viewpoint = (ViewpointNode *)GetCurrentNodeObject();
			viewpoint->initialize();
			PopNode();
		;
    break;}
case 704:
#line 3633 "vrml.y"
{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setCenter(gVec3f);
		;
    break;}
case 705:
#line 3637 "vrml.y"
{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setEnabled(yyvsp[0].ival);
		;
    break;}
case 706:
#line 3641 "vrml.y"
{
			((VisibilitySensorNode *)GetCurrentNodeObject())->setSize(gVec3f);
		;
    break;}
case 707:
#line 3648 "vrml.y"
{
			VisibilitySensorNode *vsensor = new VisibilitySensorNode();
			vsensor->setName(GetDEFName());
			AddNode(vsensor);
			PushNode(VRML_NODETYPE_VISIBILITYSENSOR, vsensor);
		;
    break;}
case 708:
#line 3658 "vrml.y"
{
			VisibilitySensorNode *vsensor = (VisibilitySensorNode *)GetCurrentNodeObject();
			vsensor->initialize();
			PopNode();
		;
    break;}
case 711:
#line 3678 "vrml.y"
{
			PushNode(VRML_NODETYPE_WORLDINFO_INFO, GetCurrentNodeObject());
		;
    break;}
case 712:
#line 3685 "vrml.y"
{
			PopNode();
		;
    break;}
case 713:
#line 3689 "vrml.y"
{
			((WorldInfoNode *)GetCurrentNodeObject())->setTitle(yyvsp[0].sval);
		;
    break;}
case 714:
#line 3696 "vrml.y"
{
			WorldInfoNode *worldInfo = new WorldInfoNode();
			worldInfo->setName(GetDEFName());
			AddNode(worldInfo);
			PushNode(VRML_NODETYPE_WORLDINFO, worldInfo);
		;
    break;}
case 715:
#line 3706 "vrml.y"
{
			WorldInfoNode *worldInfo = (WorldInfoNode *)GetCurrentNodeObject();
			worldInfo->initialize();
			PopNode();
		;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 3713 "vrml.y"

