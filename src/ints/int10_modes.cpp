#include <string.h>

#include "dosbox.h"
#include "mem.h"
#include "inout.h"
#include "int10.h"
#include "mouse.h"

#define _HALF_CLOCK		0x0001
#define _LINE_DOUBLE	0x0002

#define SEQ_REGS 0x05
#define GFX_REGS 0x09
#define ATT_REGS 0x15

VideoModeBlock ModeList[]={
/* mode  ,type     ,sw  ,sh  ,tw ,th ,cw,ch ,pt,pstart  ,plength,htot,vtot,hde,vde ,special flags */
{ 0x000  ,M_TEXT16 ,320 ,400 ,40 ,25 ,9 ,16 ,8 ,0xB8000 ,0x0800 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	},
{ 0x001  ,M_TEXT16 ,320 ,400 ,40 ,25 ,9 ,16 ,8 ,0xB8000 ,0x0800 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	},
{ 0x002  ,M_TEXT16 ,640 ,400 ,80 ,25 ,9 ,16 ,4 ,0xB8000 ,0x1000 ,100 ,449 ,80 ,400 ,0	},
{ 0x003  ,M_TEXT16 ,640 ,400 ,80 ,25 ,9 ,16 ,4 ,0xB8000 ,0x1000 ,100 ,449 ,80 ,400 ,0	},
{ 0x004  ,M_CGA4   ,320 ,200 ,40 ,25 ,8 ,8  ,4 ,0xB8000 ,0x0800 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	|_LINE_DOUBLE	},
{ 0x005  ,M_CGA4   ,320 ,200 ,40 ,25 ,8 ,8  ,4 ,0xB8000 ,0x0800 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	|_LINE_DOUBLE	},
{ 0x006  ,M_CGA2   ,640 ,200 ,80 ,25 ,8 ,8  ,4 ,0xB8000 ,0x0800 ,100 ,449 ,80 ,400 ,_LINE_DOUBLE	},
{ 0x007  ,M_TEXT16 ,720 ,400 ,80 ,25 ,9 ,16 ,4 ,0xB8000 ,0x1000 ,100 ,449 ,80 ,400 ,0	},
/* 8,9,0xa are tandy modes */
{ 0x009  ,M_TANDY16,320 ,200 ,40 ,25 ,8 ,8  ,8 ,0xA0000 ,0x2000 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	|_LINE_DOUBLE	},


 
{ 0x00D  ,M_EGA16  ,320 ,200 ,40 ,25 ,8 ,8  ,8 ,0xA0000 ,0x2000 ,50  ,449 ,40 ,400 ,_HALF_CLOCK	|_LINE_DOUBLE	},
{ 0x00E  ,M_EGA16  ,640 ,200 ,80 ,25 ,8 ,8  ,4 ,0xA0000 ,0x4000 ,100 ,449 ,80 ,400 ,_LINE_DOUBLE },
{ 0x00F  ,M_EGA2   ,640 ,350 ,80 ,25 ,8 ,14 ,2 ,0xA0000 ,0x8000 ,100 ,449 ,80 ,400 ,0	},
{ 0x010  ,M_EGA16  ,640 ,350 ,80 ,25 ,8 ,14 ,1 ,0xA0000 ,0x8000 ,100 ,449 ,80 ,350 ,0	},
{ 0x011  ,M_EGA2   ,640 ,480 ,80 ,25 ,8 ,16 ,1 ,0xA0000 ,0xA000 ,100 ,449 ,80 ,480 ,0	},
{ 0x012  ,M_EGA16  ,640 ,480 ,80 ,25 ,8 ,16 ,1 ,0xA0000 ,0xA000 ,100 ,525 ,80 ,480 ,0	},
{ 0x013  ,M_VGA    ,320 ,200 ,40 ,25 ,8 ,8  ,1 ,0xA0000 ,0x0000 ,100 ,449 ,80 ,400 ,0	},

{ 0x100  ,M_LIN8   ,640 ,400 ,80 ,20 ,8 ,16 ,1 ,0xA0000 ,0x10000,100 ,449 ,80 ,400 ,0  },
{ 0x101  ,M_LIN8   ,640 ,480 ,80 ,25 ,8 ,16 ,1 ,0xA0000 ,0xA000 ,100 ,525 ,80 ,480 ,0	},
	

{0xFFFF  ,M_ERROR ,0   ,0   ,0  ,0  ,0 ,0  ,0 ,0x00000 ,0x0000 ,0   ,0   ,0  ,0   ,0 	},

};

static Bit8u text_palette[64][3]=
{
  {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x2a,0x00},{0x2a,0x2a,0x2a},
  {0x00,0x00,0x15},{0x00,0x00,0x3f},{0x00,0x2a,0x15},{0x00,0x2a,0x3f},{0x2a,0x00,0x15},{0x2a,0x00,0x3f},{0x2a,0x2a,0x15},{0x2a,0x2a,0x3f},
  {0x00,0x15,0x00},{0x00,0x15,0x2a},{0x00,0x3f,0x00},{0x00,0x3f,0x2a},{0x2a,0x15,0x00},{0x2a,0x15,0x2a},{0x2a,0x3f,0x00},{0x2a,0x3f,0x2a},
  {0x00,0x15,0x15},{0x00,0x15,0x3f},{0x00,0x3f,0x15},{0x00,0x3f,0x3f},{0x2a,0x15,0x15},{0x2a,0x15,0x3f},{0x2a,0x3f,0x15},{0x2a,0x3f,0x3f},
  {0x15,0x00,0x00},{0x15,0x00,0x2a},{0x15,0x2a,0x00},{0x15,0x2a,0x2a},{0x3f,0x00,0x00},{0x3f,0x00,0x2a},{0x3f,0x2a,0x00},{0x3f,0x2a,0x2a},
  {0x15,0x00,0x15},{0x15,0x00,0x3f},{0x15,0x2a,0x15},{0x15,0x2a,0x3f},{0x3f,0x00,0x15},{0x3f,0x00,0x3f},{0x3f,0x2a,0x15},{0x3f,0x2a,0x3f},
  {0x15,0x15,0x00},{0x15,0x15,0x2a},{0x15,0x3f,0x00},{0x15,0x3f,0x2a},{0x3f,0x15,0x00},{0x3f,0x15,0x2a},{0x3f,0x3f,0x00},{0x3f,0x3f,0x2a},
  {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f}
};

static Bit8u ega_palette[64][3]=
{
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f}
};


static Bit8u vga_palette[256][3]=
{
  {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
  {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f},
  {0x00,0x00,0x00},{0x05,0x05,0x05},{0x08,0x08,0x08},{0x0b,0x0b,0x0b},{0x0e,0x0e,0x0e},{0x11,0x11,0x11},{0x14,0x14,0x14},{0x18,0x18,0x18},
  {0x1c,0x1c,0x1c},{0x20,0x20,0x20},{0x24,0x24,0x24},{0x28,0x28,0x28},{0x2d,0x2d,0x2d},{0x32,0x32,0x32},{0x38,0x38,0x38},{0x3f,0x3f,0x3f},
  {0x00,0x00,0x3f},{0x10,0x00,0x3f},{0x1f,0x00,0x3f},{0x2f,0x00,0x3f},{0x3f,0x00,0x3f},{0x3f,0x00,0x2f},{0x3f,0x00,0x1f},{0x3f,0x00,0x10},
  {0x3f,0x00,0x00},{0x3f,0x10,0x00},{0x3f,0x1f,0x00},{0x3f,0x2f,0x00},{0x3f,0x3f,0x00},{0x2f,0x3f,0x00},{0x1f,0x3f,0x00},{0x10,0x3f,0x00},
  {0x00,0x3f,0x00},{0x00,0x3f,0x10},{0x00,0x3f,0x1f},{0x00,0x3f,0x2f},{0x00,0x3f,0x3f},{0x00,0x2f,0x3f},{0x00,0x1f,0x3f},{0x00,0x10,0x3f},
  {0x1f,0x1f,0x3f},{0x27,0x1f,0x3f},{0x2f,0x1f,0x3f},{0x37,0x1f,0x3f},{0x3f,0x1f,0x3f},{0x3f,0x1f,0x37},{0x3f,0x1f,0x2f},{0x3f,0x1f,0x27},

  {0x3f,0x1f,0x1f},{0x3f,0x27,0x1f},{0x3f,0x2f,0x1f},{0x3f,0x37,0x1f},{0x3f,0x3f,0x1f},{0x37,0x3f,0x1f},{0x2f,0x3f,0x1f},{0x27,0x3f,0x1f},
  {0x1f,0x3f,0x1f},{0x1f,0x3f,0x27},{0x1f,0x3f,0x2f},{0x1f,0x3f,0x37},{0x1f,0x3f,0x3f},{0x1f,0x37,0x3f},{0x1f,0x2f,0x3f},{0x1f,0x27,0x3f},
  {0x2d,0x2d,0x3f},{0x31,0x2d,0x3f},{0x36,0x2d,0x3f},{0x3a,0x2d,0x3f},{0x3f,0x2d,0x3f},{0x3f,0x2d,0x3a},{0x3f,0x2d,0x36},{0x3f,0x2d,0x31},
  {0x3f,0x2d,0x2d},{0x3f,0x31,0x2d},{0x3f,0x36,0x2d},{0x3f,0x3a,0x2d},{0x3f,0x3f,0x2d},{0x3a,0x3f,0x2d},{0x36,0x3f,0x2d},{0x31,0x3f,0x2d},
  {0x2d,0x3f,0x2d},{0x2d,0x3f,0x31},{0x2d,0x3f,0x36},{0x2d,0x3f,0x3a},{0x2d,0x3f,0x3f},{0x2d,0x3a,0x3f},{0x2d,0x36,0x3f},{0x2d,0x31,0x3f},
  {0x00,0x00,0x1c},{0x07,0x00,0x1c},{0x0e,0x00,0x1c},{0x15,0x00,0x1c},{0x1c,0x00,0x1c},{0x1c,0x00,0x15},{0x1c,0x00,0x0e},{0x1c,0x00,0x07},
  {0x1c,0x00,0x00},{0x1c,0x07,0x00},{0x1c,0x0e,0x00},{0x1c,0x15,0x00},{0x1c,0x1c,0x00},{0x15,0x1c,0x00},{0x0e,0x1c,0x00},{0x07,0x1c,0x00},
  {0x00,0x1c,0x00},{0x00,0x1c,0x07},{0x00,0x1c,0x0e},{0x00,0x1c,0x15},{0x00,0x1c,0x1c},{0x00,0x15,0x1c},{0x00,0x0e,0x1c},{0x00,0x07,0x1c},

  {0x0e,0x0e,0x1c},{0x11,0x0e,0x1c},{0x15,0x0e,0x1c},{0x18,0x0e,0x1c},{0x1c,0x0e,0x1c},{0x1c,0x0e,0x18},{0x1c,0x0e,0x15},{0x1c,0x0e,0x11},
  {0x1c,0x0e,0x0e},{0x1c,0x11,0x0e},{0x1c,0x15,0x0e},{0x1c,0x18,0x0e},{0x1c,0x1c,0x0e},{0x18,0x1c,0x0e},{0x15,0x1c,0x0e},{0x11,0x1c,0x0e},
  {0x0e,0x1c,0x0e},{0x0e,0x1c,0x11},{0x0e,0x1c,0x15},{0x0e,0x1c,0x18},{0x0e,0x1c,0x1c},{0x0e,0x18,0x1c},{0x0e,0x15,0x1c},{0x0e,0x11,0x1c},
  {0x14,0x14,0x1c},{0x16,0x14,0x1c},{0x18,0x14,0x1c},{0x1a,0x14,0x1c},{0x1c,0x14,0x1c},{0x1c,0x14,0x1a},{0x1c,0x14,0x18},{0x1c,0x14,0x16},
  {0x1c,0x14,0x14},{0x1c,0x16,0x14},{0x1c,0x18,0x14},{0x1c,0x1a,0x14},{0x1c,0x1c,0x14},{0x1a,0x1c,0x14},{0x18,0x1c,0x14},{0x16,0x1c,0x14},
  {0x14,0x1c,0x14},{0x14,0x1c,0x16},{0x14,0x1c,0x18},{0x14,0x1c,0x1a},{0x14,0x1c,0x1c},{0x14,0x1a,0x1c},{0x14,0x18,0x1c},{0x14,0x16,0x1c},
  {0x00,0x00,0x10},{0x04,0x00,0x10},{0x08,0x00,0x10},{0x0c,0x00,0x10},{0x10,0x00,0x10},{0x10,0x00,0x0c},{0x10,0x00,0x08},{0x10,0x00,0x04},
  {0x10,0x00,0x00},{0x10,0x04,0x00},{0x10,0x08,0x00},{0x10,0x0c,0x00},{0x10,0x10,0x00},{0x0c,0x10,0x00},{0x08,0x10,0x00},{0x04,0x10,0x00},

  {0x00,0x10,0x00},{0x00,0x10,0x04},{0x00,0x10,0x08},{0x00,0x10,0x0c},{0x00,0x10,0x10},{0x00,0x0c,0x10},{0x00,0x08,0x10},{0x00,0x04,0x10},
  {0x08,0x08,0x10},{0x0a,0x08,0x10},{0x0c,0x08,0x10},{0x0e,0x08,0x10},{0x10,0x08,0x10},{0x10,0x08,0x0e},{0x10,0x08,0x0c},{0x10,0x08,0x0a},
  {0x10,0x08,0x08},{0x10,0x0a,0x08},{0x10,0x0c,0x08},{0x10,0x0e,0x08},{0x10,0x10,0x08},{0x0e,0x10,0x08},{0x0c,0x10,0x08},{0x0a,0x10,0x08},
  {0x08,0x10,0x08},{0x08,0x10,0x0a},{0x08,0x10,0x0c},{0x08,0x10,0x0e},{0x08,0x10,0x10},{0x08,0x0e,0x10},{0x08,0x0c,0x10},{0x08,0x0a,0x10},
  {0x0b,0x0b,0x10},{0x0c,0x0b,0x10},{0x0d,0x0b,0x10},{0x0f,0x0b,0x10},{0x10,0x0b,0x10},{0x10,0x0b,0x0f},{0x10,0x0b,0x0d},{0x10,0x0b,0x0c},
  {0x10,0x0b,0x0b},{0x10,0x0c,0x0b},{0x10,0x0d,0x0b},{0x10,0x0f,0x0b},{0x10,0x10,0x0b},{0x0f,0x10,0x0b},{0x0d,0x10,0x0b},{0x0c,0x10,0x0b},
  {0x0b,0x10,0x0b},{0x0b,0x10,0x0c},{0x0b,0x10,0x0d},{0x0b,0x10,0x0f},{0x0b,0x10,0x10},{0x0b,0x0f,0x10},{0x0b,0x0d,0x10},{0x0b,0x0c,0x10},
  {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00}
};

VideoModeBlock * CurMode;

bool INT10_SetVideoMode(Bitu mode) {


	bool clearmem=true;
	Bit8u modeset_ctl,video_ctl,vga_switches;
	Bit16u crtc_addr;

	if (mode<256) {
		if (mode & 128) {
			clearmem=false;
			mode-=128;
		}
	} else {
		/* Check for special vesa mode bits */
		mode&=0xfff;
	}
	LOG(LOG_INT10,LOG_NORMAL)("Set Video Mode %X",mode);
	Bitu i=0;
	while (ModeList[i].mode!=0xffff) {
		if (ModeList[i].mode==mode) goto foundmode;
		i++;
	}
	LOG(LOG_INT10,LOG_ERROR)("Trying to set illegal mode %X",mode);
	return false;
foundmode:
	CurMode=&ModeList[i];

	/* First read mode setup settings from bios area */
	video_ctl=real_readb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL);
	vga_switches=real_readb(BIOSMEM_SEG,BIOSMEM_SWITCHES);
	modeset_ctl=real_readb(BIOSMEM_SEG,BIOSMEM_MODESET_CTL);

	/* Setup the VGA to the correct mode */
	VGA_SetMode(CurMode->type);

	/* Setup MISC Output Register */
	Bit8u misc_output=0x3;				//Color and cpu memory access
	/* Program Sequencer */
	Bit8u seq_data[SEQ_REGS];
	memset(seq_data,0,SEQ_REGS);
	seq_data[1]|=1;		//8 dot fonts by default
	seq_data[1]|=		//Check for half clock
		(CurMode->special & _HALF_CLOCK) ? 0x08 : 0x00;
	seq_data[4]|=0x02;	//More than 64kb
	switch (CurMode->type) {
	case M_TEXT16:
		seq_data[2]|=0x3;				//Enable plane 0 and 1
		seq_data[4]|=0x05;				//Alpanumeric and odd/even enabled
		break;
	case M_EGA16:
		seq_data[2]|=0xf;				//Enable all planes for writing
		break;
	case M_LIN8:						//Seems to have the same reg layout from testing
	case M_VGA:
		seq_data[2]|=0xf;				//Enable all planes for writing
		seq_data[4]|=0xc;				//Graphics - odd/even - Chained
		break;
	}
	for (i=0;i<SEQ_REGS;i++) {
		IO_Write(0x3c4,i);
		IO_Write(0x3c5,seq_data[i]);
	}
	/* Program CRTC */
	/* First disable write protection */
	IO_Write(0x3d4,0x11);
	IO_Write(0x3d5,IO_Read(0x3d5)&0x7f);
	/* Clear all the regs */
	for (i=0x0;i<=0x18;i++) {
		IO_Write(0x3d4,i);IO_Write(0x3d5,0);
	}
	Bit8u overflow=0;Bit8u max_scanline=0;
	Bit8u ver_overflow=0;Bit8u hor_overflow=0;
	/* Horizontal Total */
	IO_Write(0x3d4,0x00);IO_Write(0x3d5,CurMode->htotal-5);
	hor_overflow|=((CurMode->htotal-5) & 0x100) >> 8;
	/* Horizontal Display End */
	IO_Write(0x3d4,0x01);IO_Write(0x3d5,CurMode->hdispend-1);
	hor_overflow|=((CurMode->hdispend-1) & 0x100) >> 7;
	/* Start horizontal Blanking */
	IO_Write(0x3d4,0x02);IO_Write(0x3d5,CurMode->hdispend);
	hor_overflow|=((CurMode->hdispend) & 0x100) >> 6;
	/* End horizontal Blanking */
	Bitu blank_end;
	if (CurMode->special & _HALF_CLOCK) {
		blank_end = (CurMode->htotal-1) & 0x7f;
	} else {
		blank_end = (CurMode->htotal-2) & 0x7f;
	}
	IO_Write(0x3d4,0x03);IO_Write(0x3d5,0x80|(blank_end & 0x1f));
//	hor_overflow|=(blank_end & 0x40) >> 3;

	/* Start Horizontal Retrace */
	Bitu ret_start;
	if (CurMode->special & _HALF_CLOCK) {
		ret_start = (CurMode->hdispend+2);
	} else {
		ret_start = (CurMode->hdispend+4);
	}
	IO_Write(0x3d4,0x04);IO_Write(0x3d5,ret_start);
	hor_overflow|=(ret_start & 0x100) >> 4;
	/* End Horizontal Retrace */
	Bitu ret_end;
	if (CurMode->special & _HALF_CLOCK) {
		ret_end = (CurMode->htotal-2) & 0x3f;
	} else {
		ret_end = (CurMode->htotal-4) & 0x3f;
	}
	IO_Write(0x3d4,0x05);IO_Write(0x3d5,(ret_end & 0x1f) | (blank_end & 0x20) << 2);
//	hor_overflow|=(ret_end & 0x20);
//TODO Be sure about these ending values in extended overflow of s3

	/* Vertical Total */
	IO_Write(0x3d4,0x06);IO_Write(0x3d5,(CurMode->vtotal-2));
	overflow|=((CurMode->vtotal-2) & 0x100) >> 8;
	overflow|=((CurMode->vtotal-2) & 0x200) >> 4;
	ver_overflow|=((CurMode->vtotal-2) & 0x400) >> 10;
/*
	These aren't exactly accurate i think, 
	Should be more like a certain percentage based on vertical total
	So you get same sized borders, but okay :)
 */
	/* Vertical Retrace Start */
	IO_Write(0x3d4,0x10);IO_Write(0x3d5,(CurMode->vdispend+12));
	overflow|=((CurMode->vdispend+12) & 0x100) >> 6;
	overflow|=((CurMode->vdispend+12) & 0x200) >> 2;
	ver_overflow|=((CurMode->vdispend+12) & 0x400) >> 6;
	/* Vertical Retrace End */
	IO_Write(0x3d4,0x11);IO_Write(0x3d5,(CurMode->vdispend+14) & 0xF);

	/* Vertical Display End */
	IO_Write(0x3d4,0x12);IO_Write(0x3d5,(CurMode->vdispend-1));
	overflow|=((CurMode->vdispend-1) & 0x100) >> 7;
	overflow|=((CurMode->vdispend-1) & 0x200) >> 3;
	ver_overflow|=((CurMode->vdispend-1) & 0x400) >> 9;
	
	/* Vertical Blank Start */
	IO_Write(0x3d4,0x15);IO_Write(0x3d5,(CurMode->vdispend+8));
	overflow|=((CurMode->vdispend+8) & 0x100) >> 5;
	max_scanline|=((CurMode->vdispend+8) & 0x200) >> 3;
	ver_overflow|=((CurMode->vdispend+8) & 0x400) >> 8;
	/* Vertical Retrace End */
	IO_Write(0x3d4,0x16);IO_Write(0x3d5,(CurMode->vtotal-8));
	/* Line Compare */
	Bitu line_compare=CurMode->vtotal+1;		//Out of range
	IO_Write(0x3d4,0x18);IO_Write(0x3d5,line_compare&0xff);
	overflow|=(line_compare & 0x100) >> 4;
	max_scanline|=(line_compare & 0x200) >> 3;
	ver_overflow|=(line_compare & 0x400) >> 4;
	Bit8u underline=0;
	/* Maximum scanline / Underline Location */
	if (CurMode->special & _LINE_DOUBLE) max_scanline|=0x80;
	switch (CurMode->type) {
	case M_TEXT16:
		max_scanline|=CurMode->theight-1;
		underline=0x1f;
		break;
	case M_VGA:
		underline=0x40;
		max_scanline|=1;		//Vga doesn't use double line but this
		break;
	case M_LIN8:
		underline=0x60;			//Seems to enable the every 4th clock on my s3
		break;
	}
	IO_Write(0x3d4,0x09);IO_Write(0x3d5,max_scanline);
	IO_Write(0x3d4,0x14);IO_Write(0x3d5,underline);

	/* OverFlow */
	IO_Write(0x3d4,0x07);IO_Write(0x3d5,overflow);
	/* Extended Horizontal Overflow */
	IO_Write(0x3d4,0x5d);IO_Write(0x3d5,hor_overflow);
	/* Extended Vertical Overflow */
	IO_Write(0x3d4,0x5e);IO_Write(0x3d5,ver_overflow);
	/* Offset Register */
	IO_Write(0x3d4,0x13);
	switch (CurMode->type) {
	case M_LIN8:
		IO_Write(0x3d5,CurMode->swidth/8);
		break;
	default:
		IO_Write(0x3d5,CurMode->hdispend/2);
	}
	/* Mode Control */
	Bit8u mode_control=0;
	switch (CurMode->type) {
	case M_CGA4:
	case M_CGA2:
		mode_control=0xa2;
		break;
	case M_EGA16:
		mode_control=0xe3;
		break;
	case M_TEXT16:
	case M_VGA:
		mode_control=0xa3;
		break;
	case M_LIN8:
		mode_control=0xab;
		break;

	}
	IO_Write(0x3d4,0x17);IO_Write(0x3d5,mode_control);
	/* Renable write protection */
	IO_Write(0x3d4,0x11);
	IO_Write(0x3d5,IO_Read(0x3d5)|0x80);
	/* Setup the correct clock */
	if (CurMode->mode<0x100) {
		//Stick to 25mhz clock for now
	} else {
		misc_output|=0xef;		//Select clock 3 
		Bitu clock=CurMode->vtotal*8*CurMode->htotal*70;
		VGA_SetClock(3,clock/1000);
	}
	/* Write Misc Output */
	IO_Write(0x3c2,misc_output);

	/* Program Graphics controller */
	Bit8u gfx_data[GFX_REGS];
	memset(gfx_data,0,GFX_REGS);
	gfx_data[0x7]=0xf;				/* Color don't care */
	gfx_data[0x8]=0xff;				/* BitMask */
	switch (CurMode->type) {
	case M_TEXT16:
		gfx_data[0x5]|=0x10;		//Odd-Even Mode
		gfx_data[0x6]|=0x0e;		//alphanumeric mode at 0xb800=0xbfff
		break;
	case M_LIN8:
	case M_VGA:
		gfx_data[0x5]|=0x40;		//256 color mode
		gfx_data[0x6]|=0x05;		//graphics mode at 0xa000-affff
		break;
	case M_EGA16:
		gfx_data[0x6]|=0x05;		//graphics mode at 0xa000-affff
		break;
	case M_CGA2:
	case M_CGA4:
	case M_TANDY16:
		gfx_data[0x5]|=0x20;		//CGA mode
		gfx_data[0x6]|=0x0f;		//graphics mode at at 0xb800=0xbfff
		break;
	}
	for (i=0;i<GFX_REGS;i++) {
		IO_Write(0x3ce,i);
		IO_Write(0x3cf,gfx_data[i]);
	}
	Bit8u att_data[ATT_REGS];
	memset(att_data,0,ATT_REGS);
	att_data[0x12]=0xf;				//Always have all color planes enabled
	/* Porgram Attribute Controller */
	switch (CurMode->type) {
	case M_EGA16:
		if (CurMode->mode>0xe) goto att_text16;
	case M_TANDY16:
		att_data[0x10]=0x01;		//Color Graphics
		for (i=0;i<8;i++) {
			att_data[i]=i;
			att_data[i+8]=i+0x10;
		}
		break;
	case M_TEXT16:
		att_data[0x13]=0x08;		//Pel panning on 8, although we don't have 9 dot text mode
		att_data[0x10]=0x0C;		//Color Text with blinking
att_text16:
		for (i=0;i<8;i++) {
			att_data[i]=i;
			att_data[i+8]=i+0x38;
		}
		break;
	case M_CGA2:
	case M_CGA4:
		IO_Write(0x3d9,0x20);		//Setup using CGA color select register
		goto skipatt;
	case M_VGA:
	case M_LIN8:
		for (i=0;i<16;i++) {
			att_data[i]=i;
		}
		att_data[0x10]=0x41;		//Color Graphics 8-bit
		break;
	}

	IO_Read(0x3da);
	for (i=0;i<ATT_REGS;i++) {
		IO_Write(0x3c0,i);
		IO_Write(0x3c0,att_data[i]);
	}
skipatt:
	/* Setup the DAC */
	IO_Write(0x3c8,0);
	switch (CurMode->type) {
	case M_EGA16:
		if (CurMode->mode>0xe) goto dac_text16;
	case M_CGA2:
	case M_CGA4:
	case M_TANDY16:
		for (i=0;i<64;i++) {
			IO_Write(0x3c9,ega_palette[i][0]);
			IO_Write(0x3c9,ega_palette[i][1]);
			IO_Write(0x3c9,ega_palette[i][2]);
		}
		break;
	case M_TEXT16:
dac_text16:
		for (i=0;i<64;i++) {
			IO_Write(0x3c9,text_palette[i][0]);
			IO_Write(0x3c9,text_palette[i][1]);
			IO_Write(0x3c9,text_palette[i][2]);
		}
		break;
	case M_VGA:
	case M_LIN8:
		for (i=0;i<256;i++) {
			IO_Write(0x3c9,vga_palette[i][0]);
			IO_Write(0x3c9,vga_palette[i][1]);
			IO_Write(0x3c9,vga_palette[i][2]);
		}
		break;
	}
	/* Setup registers for special video modes */
	switch (CurMode->type) {
	case M_TANDY16:
		IO_Write(0x3df,0x80);		//Enter 32k mode and banks on 0
		break;
	}
	/* Setup the CPU Window */
	IO_Write(0x3d4,0x6a);
	IO_Write(0x3d5,0);
	
	/* Setup some remaining S3 registers */
	IO_Write(0x3d4,0x31);IO_Write(0x3d5,0x9);	//Enable banked memory and 256k+ access
	IO_Write(0x3d4,0x58);IO_Write(0x3d5,0x3);	//Enable 8 mb of linear addressing
	IO_Write(0x3d4,0x38);IO_Write(0x3d5,0x48);	//Register lock 1
	IO_Write(0x3d4,0x39);IO_Write(0x3d5,0xa5);	//Register lock 2
	
	/* Load text mode font */
	if (CurMode->type==M_TEXT16) {
		INT10_LoadFont(Real2Phys(int10.rom.font_16),true,256,0,0,16);
	}
	/* Clear video memory if needs be */
	if (clearmem) {
		switch (CurMode->type) {
		case M_CGA4:
		case M_CGA2:
			for (i=0;i<16*1024;i++) {
				real_writew(0xb800,i*2,0x0000);
			}
			break;
		case M_TEXT16:
			for (i=0;i<16*1024;i++) {
				real_writew(0xb800,i*2,0x0700);
			}
			break;
		case M_EGA16:
		case M_VGA:
			for (i=0;i<64*1024;i++) {
				real_writeb(0xa000,i,0x00);
			}
			break;
		}
	}
	/* Setup the CRTC Address */

	crtc_addr=0x3d4;

	/* Setup the BIOS */
	if (mode<128) real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE,mode);
	else real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE,mode-0x98);	//Looks like the s3 bios
	real_writew(BIOSMEM_SEG,BIOSMEM_NB_COLS,CurMode->twidth);
	real_writew(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE,CurMode->plength);
	real_writew(BIOSMEM_SEG,BIOSMEM_CRTC_ADDRESS,crtc_addr);
	real_writeb(BIOSMEM_SEG,BIOSMEM_NB_ROWS,CurMode->theight-1);
	real_writew(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT,CurMode->cheight);
	real_writeb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL,(0x60|(clearmem << 7)));
	real_writeb(BIOSMEM_SEG,BIOSMEM_SWITCHES,0x09);
	real_writeb(BIOSMEM_SEG,BIOSMEM_MODESET_CTL,real_readb(BIOSMEM_SEG,BIOSMEM_MODESET_CTL)&0x7f);

	// FIXME We nearly have the good tables. to be reworked
	real_writeb(BIOSMEM_SEG,BIOSMEM_DCC_INDEX,0x08);    // 8 is VGA should be ok for now
	real_writew(BIOSMEM_SEG,BIOSMEM_VS_POINTER,0x00);
	real_writew(BIOSMEM_SEG,BIOSMEM_VS_POINTER+2,0x00);

	// FIXME
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x00); // Unavailable on vanilla vga, but...
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAL,0x00); // Unavailable on vanilla vga, but...
	
	// Set cursor shape
	if(CurMode->type==M_TEXT16) {
		INT10_SetCursorShape(0x06,07);
	}
	// Set cursor pos for page 0..7
	for(i=0;i<8;i++) INT10_SetCursorPos(0,0,(Bit8u)i);
	// Set active page 0
	INT10_SetActivePage(0);
	/* Set some interrupt vectors */
	switch (CurMode->cheight) {
	case 8:RealSetVec(0x43,int10.rom.font_8_first);break;
	case 14:RealSetVec(0x43,int10.rom.font_14);break;
	case 16:RealSetVec(0x43,int10.rom.font_16);break;
	}
	/* Tell mouse resolution change */
	Mouse_NewVideoMode();
	return true;
}



