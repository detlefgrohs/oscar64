#ifndef C64_VIC_H
#define C64_VIC_H

#include "types.h"

#define VIC_CTRL1_RSEL	0x08
#define VIC_CTRL1_DEN	0x10
#define VIC_CTRL1_BMM	0x20
#define VIC_CTRL1_ECM	0x40
#define VIC_CTRL1_RST8	0x80

#define VIC_CTRL2_CSEL	0x08
#define VIC_CTRL2_MCM	0x10
#define VIC_CTRL2_RES	0x20

#define VIC_INTR_RST	0x01
#define VIC_INTR_MBC	0x02
#define VIC_INTR_MMC	0x04
#define VIC_INTR_ILP	0x08
#define VIC_INTR_IRQ	0x80

enum VICColors
{
	VCOL_BLACK,
	VCOL_WHITE,
	VCOL_RED,
	VCOL_CYAN,
	VCOL_PURPLE,
	VCOL_GREEN,
	VCOL_BLUE,
	VCOL_YELLOW,

	VCOL_ORANGE,
	VCOL_BROWN,
	VCOL_LT_RED,
	VCOL_DARK_GREY,
	VCOL_MED_GREY,
	VCOL_LT_GREEN,
	VCOL_LT_BLUE,
	VCOL_LT_GREY
};

struct VIC
{
	struct XY
	{
		byte	x, y;
	}	spr_pos[8];
	byte	spr_msbx;

	volatile byte	ctrl1;
	volatile byte	raster;
	byte	lpx, lpy;
	byte	spr_enable;
	byte	ctrl2;
	byte	spr_expand_y;
	byte	memptr;
	byte	intr_ctrl;
	byte	intr_enable;
	byte	spr_priority;
	byte	spr_multi;
	byte	spr_expand_x;
	byte	spr_sprcol;
	byte	spr_backcol;
	byte	color_border;
	byte	color_back;
	byte	color_back1;
	byte	color_back2;
	byte	color_back3;
	byte	spr_mcolor0;
	byte	spr_mcolor1;
	byte	spr_color[8];

};

// set the 16k Bank for the vic
// 0 : 0x0000..0x3fff
// 1 : 0x4000..0x7fff
// 2 : 0x8000..0xbfff
// 3 : 0xc000..0xffff
void vic_setbank(char bank);

enum VicMode
{
	VICM_TEXT,
	VICM_TEXT_MC,
	VICM_TEXT_ECM,
	VICM_HIRES,
	VICM_HIRES_MC
};

// set the display mode and base address. This will also
// adapt the bank.
void vic_setmode(VicMode mode, char * text, char * font);

// put a sprite at the given x/y location, taking care of the
// x MSB
inline void vic_sprxy(byte s, int x, int y);

// wait for the beam to reach the bottom of the visual area
inline void vic_waitBottom(void);

// wait for the beam to reach the top of the frame
inline void vic_waitTop(void);

// wait for the top of the frame and then for the bottom of the visual area
inline void vic_waitFrame(void);

// wait for a specific raster line
void vic_waitLine(int line);

// reference to the VIC chip
#define vic	(*((struct VIC *)0xd000))

#pragma compile("vic.c")

#endif
