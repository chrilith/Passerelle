#pragma once

// See _gd2GetHeader() in gd_gd2.c for details
#define GD_HEADER_SIZE	24
#define GD_MAGIC		'gd2\0'

#define GD2H_MAGIC(p)		(p + 0)
#define GD2H_VER(p)			(p + 4)
#define GD2H_WIDTH(p)		(p + 6)
#define GD2H_HEIGHT(p)		(p + 8)
#define GD2H_CHUNK(p)		(p + 10)
#define GD2H_FORMAT(p)		(p + 12)
#define GD2H_NCX(p)			(p + 14)
#define GD2H_NCY(p)			(p + 16)
#define GD2H_TRUECOLOR(p)	(p + 18)
#define GD2H_TRANS(p)		(p + 19)
#define GD2H_BITS(p)		(p + GD_HEADER_SIZE)
