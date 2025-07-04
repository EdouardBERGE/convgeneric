#define PROD 1

#ifdef PROD
#include "rasmlib.h"
#include "minilib.h"
#else
#include "../tools/library.h"
#include "../tools/libgfx.h"
#endif

#define __FILENAME__ "convgeneric.c"

#include<float.h>

#ifndef OS_WIN
/* gris */
#define KNORMAL  "\x1B[0m"
/* rouge foncé */
#define KERROR   "\x1B[31m"
/* vert */
#define KAYGREEN "\x1B[32m"
/* orange */
#define KWARNING "\x1B[33m"
/* bleu foncé */
#define KBLUE    "\x1B[34m"
/* bleu ciel */
#define KVERBOSE "\x1B[36m"
/* blanc */
#define KIO      "\x1B[97m"
#else
#define KNORMAL  ""
#define KERROR   ""
#define KAYGREEN ""
#define KWARNING ""
#define KBLUE    ""
#define KVERBOSE ""
#define KIO      ""
#endif

#include<math.h>

enum e_packed_format {
E_PACKED_CPC_MODE0=0,
E_PACKED_CPC_MODE1,
E_PACKED_CPC_MODE2,
E_PACKED_HSP_RAW,
E_PACKED_HSP_2PIX_LOGICAL,
E_PACKED_HSP_2PIX_REVERSE,
E_PACKED_HSP_4PIX_REVERSE,
E_PACKED_END
};

struct s_parameter {
	/* file */
	char *filename;
	char *outputfilename;
	char *sheetfilename;
	/* lara option */
	int lara;
	/* general options */
	int mode;
	int width;
	int split;
	int grad;
	int asmdump;
	int tiles,metatiles;
	int cpccolorz;
	int noalpha;
	char *exportpalettefilename;
	char *exportpaletteBINfilename;
	char *importpalettefilename;
	/* screen options */
	int single,splitLowHigh,tilexclude;
	int scrx,scry;
	int sx,sy;
	int ox,oy;
	int mask;
	int scrmode,oldmode;
	int lineperblock;
	int nblinescreen;
	int splitraster;
	int rastaline;
	int packAlpha;
	/* hardware sprite options */
	int hsp;
	int scan,fontscan;
	int maxextract;
	int black;
	int packed; /* 0, 4, 2 */
	int forceextraction;
	int transparency_color;
	int search_transparency;
	int keep_empty;
	int metax,metay;
	int reverse;
	/* demomaking options */
	int rotoffset;
	int rotoheight;
	char *rotoffsetfilename;
	char *rotoheightfilename;
	int heightmap;
	char *heightmapfilename;
};

struct s_sprite_info {
	int x,y;
	int tadr,adr,size;
	unsigned char *bitmap;
	unsigned int ibitmap;
};


#define MAXSPLIT 6

struct s_rastblock {
int col[16];
int nbcol;
int newcol;
};

struct s_rastline {
struct s_rastblock block[48];
int col[27];
int cptcol;
int nbblock;
int freenop;
int mode;
int reg[MAXSPLIT];
};

struct s_rastecran {
struct s_rastline line[280];
int nbline;
};

struct s_score {
int score;
int ga;
};


void color_error(int r, int v, int b) {
	static char rvb[4096]={0};
	int idx;

	idx=(r>>4)+((v>>4)<<4)+((b>>4)<<8);

	if (!rvb[idx]) {
		printf(KERROR"rvb color #%02X%02X%02X not found\n"KNORMAL,r,v,b);
		rvb[idx]=1;
	}
}

int GetIDXFromPixel(unsigned char *palette, unsigned char *pixel)
{
	#undef FUNC
	#define FUNC "GetIDXFromPalette"

	int imin=0,vmin,va;
	int i;

	if (!pixel[3]) return 0; /* transparency is zero */

	for (i=0;i<16;i++) {
		if (palette[i*3+0]==pixel[0] && palette[i*3+1]==pixel[1] && palette[i*3+2]==pixel[2]) return i;
	}
	color_error(pixel[0],pixel[1],pixel[2]);
	return -1;
}

int GetIDXFromPalette(unsigned char *palette, int r, int v, int b)
{
	#undef FUNC
	#define FUNC "GetIDXFromPalette"

	int imin=0,vmin,va;
	int i;


	for (i=0;i<16;i++) {
//printf("palette[%d]=%d/%d/%d\n",i,palette[i*3+0],palette[i*3+1],palette[i*3+2]);
		if (palette[i*3+0]==r && palette[i*3+1]==v && palette[i*3+2]==b) return i;
	}

//printf("pixel = R:%X G:%X B:%X ",r,v,b);
	if (r==0x70) r=0x80;
	if (v==0x70) v=0x80;
	if (b==0x70) b=0x80;
	for (i=0;i<16;i++) {
		if (palette[i*3+0]==r && palette[i*3+1]==v && palette[i*3+2]==b) return i;
	}

	color_error(r,v,b);
	return -1;
}

int GetGAFromRGB(int r, int v, int b) {
	#undef FUNC
	#define FUNC "GetGAFromRGB"

	int ga=64,rgb;

	if (r<85) r=0; else if (r>170) r=255; else r=128;
	if (v<85) v=0; else if (v>170) v=255; else v=128;
	if (b<85) b=0; else if (b>170) b=255; else b=128;

	rgb=r*65536+v*256+b;

	switch (rgb) {
		case 0x000000:ga=64+20;break;
		case 0x000080:ga=64+4;break;
		case 0x0000FF:ga=64+21;break;
		case 0x800000:ga=64+28;break;
		case 0x800080:ga=64+24;break;
		case 0x8000FF:ga=64+29;break;
		case 0xFF0000:ga=64+12;break;
		case 0xFF0080:ga=64+5;break;
		case 0xFF00FF:ga=64+13;break;
		case 0x008000:ga=64+22;break;
		case 0x008080:ga=64+6;break;
		case 0x0080FF:ga=64+23;break;
		case 0x808000:ga=64+30;break;
		case 0x808080:ga=64+0;break;
		case 0x8080FF:ga=64+31;break;
		case 0xFF8000:ga=64+14;break;
		case 0xFF8080:ga=64+7;break;
		case 0xFF80FF:ga=64+15;break;
		case 0x00FF00:ga=64+18;break;
		case 0x00FF80:ga=64+2;break;
		case 0x00FFFF:ga=64+19;break;
		case 0x80FF00:ga=64+26;break;
		case 0x80FF80:ga=64+25;break;
		case 0x80FFFF:ga=64+27;break;
		case 0xFFFF00:ga=64+10;break;
		case 0xFFFF80:ga=64+3;break;
		case 0xFFFFFF:ga=64+11;break;
		default:printf(KERROR"unknown color for CPC\n"KNORMAL);exit(-1);
	}
	return ga;
}
void PushGA(int *GApal,int r, int v, int b) {
	int i;
	int gaval=GetGAFromRGB(r,v,b);
	for (i=0;i<32;i++) {
		if (GApal[i]==gaval) {
			return;
		} else if (!GApal[i]) {
			GApal[i]=gaval;
			return;
		}
	}
}
int GetBASICFromRGB(int r, int v, int b, int zeline) {
	#undef FUNC
	#define FUNC "GetBASICFromRGB"

	int ga=64,rgb;

	if (r<85) r=0; else if (r>170) r=255; else r=128;
	if (v<85) v=0; else if (v>170) v=255; else v=128;
	if (b<85) b=0; else if (b>170) b=255; else b=128;

	rgb=r*65536+v*256+b;

	switch (rgb) {
		case 0x000000:ga=0;break;
		case 0x000080:ga=1;break;
		case 0x0000FF:ga=2;break;
		case 0x800000:ga=3;break;
		case 0x800080:ga=4;break;
		case 0x8000FF:ga=5;break;
		case 0xFF0000:ga=6;break;
		case 0xFF0080:ga=7;break;
		case 0xFF00FF:ga=8;break;
		case 0x008000:ga=9;break;
		case 0x008080:ga=10;break;
		case 0x0080FF:ga=11;break;
		case 0x808000:ga=12;break;
		case 0x808080:ga=13;break;
		case 0x8080FF:ga=14;break;
		case 0xFF8000:ga=15;break;
		case 0xFF8080:ga=16;break;
		case 0xFF80FF:ga=17;break;
		case 0x00FF00:ga=18;break;
		case 0x00FF80:ga=19;break;
		case 0x00FFFF:ga=20;break;
		case 0x80FF00:ga=21;break;
		case 0x80FF80:ga=22;break;
		case 0x80FFFF:ga=23;break;
		case 0xFFFF00:ga=24;break;
		case 0xFFFF80:ga=25;break;
		case 0xFFFFFF:ga=26;break;
		default:printf("unknown color for CPC line %d\n",zeline);exit(-1);
	}
	return ga;
}
char *GetStringFromGA(int ga) {
	#undef FUNC
	#define FUNC "GetStringFromGA"

	switch (ga) {
		case 64+20:return "black";break;
		case 64+4:return "deep blue";break;
		case 64+21:return "blue";break;
		case 64+28:return "brown";break;
		case 64+24:return "magenta";break;
		case 64+29:return "mauve";break;
		case 64+12:return "red";break;
		case 64+5:return "purple";break;
		case 64+13:return "bright magenta";break;
		case 64+22:return "green";break;
		case 64+6:return "cyan";break;
		case 64+23:return "sky blue";break;
		case 64+30:return "kaki";break;
		case 64+0:return "grey";break;
		case 64+31:return "pastel blue";break;
		case 64+14:return "orange";break;
		case 64+7:return "pink";break;
		case 64+15:return "pastel magenta";break;
		case 64+18:return "bright green";break;
		case 64+2:return "sea green";break;
		case 64+19:return "bright cyan";break;
		case 64+26:return "lime";break;
		case 64+25:return "pastel green";break;
		case 64+27:return "pastel cyan";break;
		case 64+10:return "yellow";break;
		case 64+3:return "bright yellow";break;
		case 64+11:return "white";break;
		default:break;
	}
	return "unknown color";
}
void GetRGBFromGA(int ga, unsigned char *r, unsigned char *v, unsigned char *b, int line) {
	#undef FUNC
	#define FUNC "GetRGBFromGA"

	switch (ga) {
		case 64+20:*r=0;*v=0;*b=0;break;
		case 64+4:*r=0;*v=0;*b=128;break;
		case 64+21:*r=0;*v=0;*b=255;break;
		case 64+28:*r=128;*v=0;*b=0;break;
		case 64+24:*r=128;*v=0;*b=128;break;
		case 64+29:*r=128;*v=0;*b=255;break;
		case 64+12:*r=255;*v=0;*b=0;break;
		case 64+5:*r=255;*v=0;*b=128;break;
		case 64+13:*r=255;*v=0;*b=255;break;
		case 64+22:*r=0;*v=128;*b=0;break;
		case 64+6:*r=0;*v=128;*b=128;break;
		case 64+23:*r=0;*v=128;*b=255;break;
		case 64+30:*r=128;*v=128;*b=0;break;
		case 64+0:*r=128;*v=128;*b=128;break;
		case 64+31:*r=128;*v=128;*b=255;break;
		case 64+14:*r=255;*v=128;*b=0;break;
		case 64+7:*r=255;*v=128;*b=128;break;
		case 64+15:*r=255;*v=128;*b=255;break;
		case 64+18:*r=0;*v=255;*b=0;break;
		case 64+2:*r=0;*v=255;*b=128;break;
		case 64+19:*r=0;*v=255;*b=255;break;
		case 64+26:*r=128;*v=255;*b=0;break;
		case 64+25:*r=128;*v=255;*b=128;break;
		case 64+27:*r=128;*v=255;*b=255;break;
		case 64+10:*r=255;*v=255;*b=0;break;
		case 64+3:*r=255;*v=255;*b=128;break;
		case 64+11:*r=255;*v=255;*b=255;break;
		default:printf("unknown gate array value\n");exit(-1);
	}
}
void GetRGBFromBASIC(int ba, unsigned char *r, unsigned char *v, unsigned char *b) {
	#undef FUNC
	#define FUNC "GetRGBFromBASIC"

	switch (ba) {
		case 0:*r=0;*v=0;*b=0;break;
		case 1:*r=0;*v=0;*b=128;break;
		case 2:*r=0;*v=0;*b=255;break;
		case 3:*r=128;*v=0;*b=0;break;
		case 4:*r=128;*v=0;*b=128;break;
		case 5:*r=128;*v=0;*b=255;break;
		case 6:*r=255;*v=0;*b=0;break;
		case 7:*r=255;*v=0;*b=128;break;
		case 8:*r=255;*v=0;*b=255;break;
		case 9:*r=0;*v=128;*b=0;break;
		case 10:*r=0;*v=128;*b=128;break;
		case 11:*r=0;*v=128;*b=255;break;
		case 12:*r=128;*v=128;*b=0;break;
		case 13:*r=128;*v=128;*b=128;break;
		case 14:*r=128;*v=128;*b=255;break;
		case 15:*r=255;*v=128;*b=0;break;
		case 16:*r=255;*v=128;*b=128;break;
		case 17:*r=255;*v=128;*b=255;break;
		case 18:*r=0;*v=255;*b=0;break;
		case 19:*r=0;*v=255;*b=128;break;
		case 20:*r=0;*v=255;*b=255;break;
		case 21:*r=128;*v=255;*b=0;break;
		case 22:*r=128;*v=255;*b=128;break;
		case 23:*r=128;*v=255;*b=255;break;
		case 24:*r=255;*v=255;*b=0;break;
		case 25:*r=255;*v=255;*b=128;break;
		case 26:*r=255;*v=255;*b=255;break;
		default:printf("unknown basic color value\n");exit(-1);
	}
}

void SortPalette(unsigned char *palette, int maxcoul)
{
	#undef FUNC
	#define FUNC "SortPalette"

	unsigned r,v,b;
	int i,j,imin;
	float vmin,vcur;

	for (i=0;i<maxcoul;i++) {
		vmin=palette[i*3+0]*palette[i*3+0]+palette[i*3+1]*palette[i*3+1]+palette[i*3+2]*palette[i*3+2];
		imin=i;
		for (j=i+1;j<maxcoul;j++) {
			vcur=palette[j*3+0]*palette[j*3+0]+palette[j*3+1]*palette[j*3+1]+palette[j*3+2]*palette[j*3+2];
			if (vcur<vmin) {
				vmin=vcur;
				imin=j;
			}
		}
		if (imin!=i) {
			r=palette[i*3+0];
			v=palette[i*3+1];
			b=palette[i*3+2];
			palette[i*3+0]=palette[imin*3+0];
			palette[i*3+1]=palette[imin*3+1];
			palette[i*3+2]=palette[imin*3+2];
			palette[imin*3+0]=r;
			palette[imin*3+1]=v;
			palette[imin*3+2]=b;
		}
	}

}

void AutoScanHSP(struct s_parameter *parameter, struct s_png_info *photo, unsigned char *palette, int *i, int *j)
{
	#undef FUNC
	#define FUNC "AutoScanHSP"

	static int match=0;
	static int xs=0,ys=0,xe=0,ye=0;
	int idx,tic=0;

	printf("autoscan HSP: i=%d j=%d ox=%d oy=%d w=%d h=%d\n",*i,*j,parameter->ox,parameter->oy,photo->width,photo->height);
	if (!match) {
		/* first run, is there an area? */
		while (GetIDXFromPixel(palette,&photo->data[((*i)+(*j)*photo->width)*4+0])==-1) {
			(*i)++;
			if (*i>=photo->width) {
				*i=parameter->ox;
				/* after a reinit AND a carriage return, we must go down the previous global boxes */
				if (ye && !tic) {
					*j=ye;
					tic=1;
				} else {
					(*j)++;
				}
				if (*j>=photo->height) break;
			}
		}
		match=1;
		xs=*i;ys=*j;
	} else {
		/* are they another sprite in the area? */
		match=0;
		//*i=(*i)+16; déjà fait par la capture du sprite...
printf("check right (%d/%d) =%d | ",*i,*j,GetIDXFromPixel(palette,&photo->data[((*i)+(*j)*photo->width)*4+0]));
		if (*i<photo->width && GetIDXFromPixel(palette,&photo->data[((*i)+(*j)*photo->width)*4+0])!=-1) {
printf("to the right | ");
			match=1;
			if (xe<*i) xe=*i; /* update global box */
		} else {
			/* rollback on i */
			*i=xs;
printf("return to xstart and check down below the whole sprite | ");
			if (*j<photo->height-16) {
				*j=(*j)+15;
printf("may be down %d/%d | ",*i,(*j)+1);
				for (idx=xs;GetIDXFromPixel(palette,&photo->data[((*i)+idx+(*j)*photo->width)*4+0])!=-1;idx++) {
					if (GetIDXFromPixel(palette,&photo->data[((*i)+idx+((*j)+1)*photo->width)*4+0])!=-1) {
						match=1;
						*j=(*j)+1;
						break;
					}
				}
				if (match) {
					ye=*j+16; /* update global box */
					while (GetIDXFromPixel(palette,&photo->data[((*i)-1+(*j)*photo->width)*4+0])!=-1) {
						(*i)--;
					}
printf("match xmin=%d! | ",*i);
					if (*i<xs) {
						xs=*i;
printf("update xs to %d | ",xs);
					}
				} else {
					/* reinit from the top right of the area */
					*i=xe+16;
					*j=ys;
printf("reinit in %d/%d | ",*i,*j);
					AutoScanHSP(parameter,photo,palette,i,j);
					return;
				}
			} else {
				*i=photo->width-1;
				*j=photo->height-1;
				printf("no moar to scan\n");
			}
		}
	}
#if 0
	/* get area size, which supposed to be squared */
	if (*i<photo->width && *j<photo->height) {
		/* looking for top right corner */
		parameter->sx=1;
		while (GetIDXFromPixel(palette,&photo->data[(parameter->sx+(*i)+(*j)*photo->width)*4+0])!=-1) parameter->sx++;
		/* looking for bottom right corner */
		parameter->sy=1;
		while (GetIDXFromPixel(palette,&photo->data[(parameter->sx+(*i)+((*j)+parameter->sy)*photo->width)*4+0])!=-1) parameter->sy++;
	}
#endif
printf("\n");
}

void FontAutoScan(struct s_parameter *parameter, struct s_png_info *photo, unsigned char *palette, int *i, int *j)
{
	#undef FUNC
	#define FUNC "FontAutoScan"

	static int ystart=-1;
	int hasdata,x,y;

	// yscan / find first line of data
	if (ystart==-1) {
		// offset start
		*i=parameter->ox;
		*j=parameter->oy;
		while (GetIDXFromPalette(palette, photo->data[(*i+*j*photo->width)*4+0],photo->data[(*i+*j*photo->width)*4+1],photo->data[(*i+*j*photo->width)*4+2])<=0) {
			(*i)++;
			if (*i>=photo->width) {
				*i=parameter->ox;
				(*j)++;
				if (*j>=photo->height) break;
			}
		}
		printf("@@DBG first data line = %d\n",*j);

		parameter->sy=0;
		hasdata=1;
		y=*j;
		while (hasdata>0) {
			for (x=parameter->ox;x<photo->width && hasdata<=0;x++) hasdata=GetIDXFromPalette(palette, photo->data[(x+y*photo->width)*4+0],photo->data[(x+y*photo->width)*4+1],photo->data[(x+y*photo->width)*4+2]);
			parameter->sy++;
			y++;
			if (y>=photo->height) break;
		}
		printf("@@DBG font height = %d\n",parameter->sy);
	}

	// xscan / find first data column
	while (hasdata<=0) {
		for (y=*j;y<parameter->sy && hasdata<=0;y++) {
			hasdata=GetIDXFromPalette(palette, photo->data[(x+y*photo->width)*4+0],photo->data[(x+y*photo->width)*4+1],photo->data[(x+y*photo->width)*4+2]);
		}
		if (hasdata<=0) (*i)++;

		if (*i>=photo->width) {
			printf("@@DBG EOL!!! (unfinished)\n");
			*j=*j+parameter->sy;
			*i=parameter->ox;

			*i=photo->width;
			*j=photo->height;
			return;

			break;
		}
	}

	// xscan / find end of data column
	parameter->sx=0;
	while (hasdata>0) {
		for (y=*j;y<parameter->sy && hasdata<=0;y++) {
			hasdata=GetIDXFromPalette(palette, photo->data[(x+y*photo->width)*4+0],photo->data[(x+y*photo->width)*4+1],photo->data[(x+y*photo->width)*4+2]);
		}
		if (hasdata>0) parameter->sx++;
	}
	
	printf("@@DBG char size = %dx%d\n",parameter->sx,parameter->sy);
	
	parameter->sx=0; // variable width @@TODO
}

void AutoScan(struct s_parameter *parameter, struct s_png_info *photo, unsigned char *palette, int *i, int *j)
{
	#undef FUNC
	#define FUNC "AutoScan"

	if (parameter->fontscan) {
		FontAutoScan(parameter,photo,palette,i,j);
	} else if (parameter->scan) {
		if (parameter->hsp) {
			AutoScanHSP(parameter,photo,palette,i,j);
		} else {
			/* is there an area? */
			while (GetIDXFromPalette(palette, photo->data[(*i+*j*photo->width)*4+0],photo->data[(*i+*j*photo->width)*4+1],photo->data[(*i+*j*photo->width)*4+2])==-1) {
				(*i)++;
				if (*i>=photo->width) {
					*i=parameter->ox;
					(*j)++;
					if (*j>=photo->height) break;
				}
			}
			/* get area size, which supposed to be squared */
			if (*i<photo->width && *j<photo->height) {
				/* looking for top right corner */
				parameter->sx=1;
				while (GetIDXFromPalette(palette, photo->data[(parameter->sx+*i+*j*photo->width)*4+0],photo->data[(parameter->sx+*i+*j*photo->width)*4+1],photo->data[(parameter->sx+*i+*j*photo->width)*4+2])!=-1) parameter->sx++;
				/* looking for bottom right corner */
				parameter->sy=1;
				while (GetIDXFromPalette(palette, photo->data[(parameter->sx+*i+(*j+parameter->sy)*photo->width)*4+0],photo->data[(parameter->sx+*i+(*j+parameter->sy)*photo->width)*4+1],photo->data[(parameter->sx+*i+(*j+parameter->sy)*photo->width)*4+2])!=-1) parameter->sy++;
			}
		}
	}
}

int cmpscore(const void *a, const void *b) {
	struct s_score ia,ib;

	ia=*(struct s_score *)a;
	ib=*(struct s_score *)b;
	/* tri inverse! */
	return ib.score-ia.score;
}
int cmpcol(const void *a, const void *b) {
	int ia,ib;

	ia=*(int*)a;
	ib=*(int*)b;
	return ia-ib;
}
int cmprow(const void *a, const void *b) {
	int ia,ib;

	ia=*(int*)a;
	ib=*(int*)b;
	return ib-ia;
}

void DisplayPalette(int hsp, int max, unsigned char *palette)
{
	int i;

	for (i=hsp;i<max+hsp;i++) {
		printf("%s#%04X",i-hsp?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
	}
printf("\n");

}

//#define CHECK_IDATA {if ((idata>photo->width*photo->height && !parameter->scrmode) || idata>=32768) {printf("internal error: too much data idata=%d width=%d height=%d\n",idata,photo->width,photo->height);exit(-1);}}
#define CHECK_IDATA ;

void Build(struct s_parameter *parameter)
{
	#undef FUNC
	#define FUNC "Build"

	/************************************************
                c o n v e r t e r    l e g a c y
        ************************************************/
	char newname[2048];

	struct s_png_info *photo=NULL;

	int j,k,l,m,n,zepix,maxcoul;
	unsigned char r,v,b;
	unsigned char r2,v2,b2;
	int *cptpalette;
	int width,height;
	float vmin,vmax,vpal,totpal;
	int imin,imax,icol,ipal,idark1,idark2;
	float vdark;
	int ligne[192],adr,adrcode,numligne=0;

	int iref;

	unsigned char spriteline[16]={0};
	int inthechar=0;

	int ticpack=0;
	int xs,ys;

	/****************************************
                c o n v e r t e r    v 2
        ****************************************/
	unsigned char *cpcdata=NULL;        /* dynamic workspace */
	int *tileidx=NULL;
	int tilewidth,tileheight,tiletrans,tilexclude;
	int scrtilex,scrtiley;
	int itile;
	int idata=0;                        /* current output */
	int pixrequested;                   /* how many pix do we need for a byte? */
	int limitcolors=16;                 /* max number of colors regarding the options */
	int hasblack,is_hsp=0;                       /* index of black color if any, else -1 */
	int hsptr=-1,hsptv=-1,hsptb=-1;              /* transparency color */
	struct s_sprite_info *spinfo=NULL;  /* info about extracted sprites */
	struct s_sprite_info curspi={0};
	int ispi=0,mspi=0;                  /* fast counters for sprite info struct */
	int *scradr,maxscradr;             	/* CRTC line adresse decoding */
	int crtcadr,crtcb,curline;          /* CRTC mimic values */
	unsigned char palette[4096*3]={0};  /* palette for 16 colors + border for scanning */
	int cptcolor[4096];                 /* pour compter les couleurs par ligne */
	int bgr,bgv,bgb,scancolor;          /* border color for scanning */
	int i,istep;
	int pix1,pix2,pix3,pix4;            /* mode 0 & mode 1 pixels */
	int maxdata=0,byteleft;               /* byte count for file output */
	int filenumber=2;                   /* file naming */
	int png_has_transparency=0;         /* png data contains transparent pixels */
	/* split-rasters */
	int scrmode[280];                   /* required mode */
	int accmode,cptcol,oldmode;
	struct s_rastecran scr={0};
	int ib,maxblockcol,col,clash=0;
	int maxlinecol;
	int garemove[27],bareplace;
	int gacompare1[256],gacompare2[256];
	int cptchange,changeback;
	int curga,noptrou;
	int colorstat[128][128];
	int colorscore[128];
	struct s_score tabscore[27];
	int lastbackgroundcolor[256]; // 16 to check...
	int splitcolor[128];
	int primarybackgroundcolor[15];    /* couleurs de background à init hors-zone */
	int iprim=0;                       /* index max des couleurs primaires de background */
	int blockhascolor;
	int colormaysplit;
	int backgroundcolor[16];
	int lindex,found;
	int reg[MAXSPLIT]; /* register A,C,D,E,H,L */
	int woffset;
	int token[312*64];
	/***************************************************
	  L A R A   e x p o r t
	***************************************************/
	int decal,csx,theight;
	/***************************************************
	  raster line export
	***************************************************/
	int colorz_in_a_row[32]={0};
	int colorz_prev_row[32]={0};

	/*****************************
	 * export de la transparence
	 ****************************/
	unsigned char *transdata=NULL;
	int itrans=0;

	tilexclude=parameter->tilexclude;

	switch (parameter->mode) {
		default:break;
		case 1:limitcolors=4;break;
		case 2:limitcolors=2;break;
	}
	limitcolors+=parameter->scan; /* border color for scan raise the max */
	if (parameter->splitraster || parameter->rastaline) {
		printf("split/raster mode rises colors up to 27\n");
		limitcolors=27; /* no limit with rasters */
	}

        photo=PNGRead32(parameter->filename);
        if (!photo) exit(-2);

	if (parameter->heightmap) {
		unsigned char *heightmap=NULL;

        	printf(KIO"Image %dx%d will be converted to heightmap (raw data)\n",photo->width,photo->height);
		heightmap=malloc(photo->width*photo->height);
		for (i=0;i<photo->width*photo->height;i++) heightmap[i]=photo->data[i*4+1]; //Blue
		FileRemoveIfExists(parameter->heightmapfilename);
		FileWriteBinary(parameter->heightmapfilename,heightmap,photo->width*photo->height);
		FileWriteBinaryClose(parameter->heightmapfilename);
		free(heightmap);
		printf(KIO"heightmap written %d bytes\n"KNORMAL,photo->width*photo->height);
		exit(0);
	}

	if (parameter->search_transparency) {
		// on va prendre la couleur à la con qu'on retrouve partout, en particulier dans les coins...
		// à faire évoluer avec un forcage RGB...
		hsptr=photo->data[photo->width*4+0]&0xF0;
		hsptv=photo->data[photo->width*4+1]&0xF0;
		hsptb=photo->data[photo->width*4+2]&0xF0;
	}

	//*****************************************************
	//                  import de palette
	//*****************************************************

	/* but first reduce colorset to fit 4096k */
	for (i=0;i<photo->height*photo->width*4;i+=4) {
		photo->data[i+0]&=0xF0;
		photo->data[i+1]&=0xF0;
		photo->data[i+2]&=0xF0;
		/* pas de AND sur le canal ALPHA!!! */
	}

	if (parameter->importpalettefilename) {
		/* load palette from a text file */
		char separator,*curchar,*txtpalette;
		int palsize,curcoul;
		int has_reduce=0;

        printf(KIO"Image %dx%d\n",photo->width,photo->height);
		printf("import de palette GRB (not RGB!!!) [%s] => ",parameter->importpalettefilename);
		palsize=FileGetSize(parameter->importpalettefilename);
		txtpalette=MemMalloc(palsize);
		FileReadBinary(parameter->importpalettefilename,txtpalette,palsize);
		FileReadBinaryClose(parameter->importpalettefilename);

		if (strchr(txtpalette,'$')) separator='$'; else
		if (strchr(txtpalette,'#')) separator='#'; else
		if (strstr(txtpalette,"0x")) separator='x'; else
		{
			printf(KERROR"\nERROR: invalid palette!\n"KNORMAL);
			exit(-4);
		}
		if (strncmp(txtpalette,"defw ",5)) {
			printf(KWARNING"Warning, old palette style! You need to generate proper palette again!\n");
		}
		if (strstr(txtpalette," HSP")) is_hsp=1; else is_hsp=0;
		if (parameter->hsp!=is_hsp) {
			printf(KWARNING"Warning, palette [%s] mode is different from output mode!\n",is_hsp?"HSP":"legacy");
		}

		/* parse text */
		if (is_hsp) {
			// la couleur de la transparence
			palette[0]=hsptr;
			palette[1]=hsptv;
			palette[2]=hsptb;
		}
		maxcoul=is_hsp;
printf("palette=[%s]\n",txtpalette);
		while ((curchar=strchr(txtpalette,separator))!=NULL) {
			if (maxcoul==16) {
				printf(KERROR"\nERROR: invalid palette! too much colorz!\n"KNORMAL);
				exit(-4);
			}
			*curchar=' ';
			curchar++;
			curcoul=strtol(curchar,NULL,16);
			r=curcoul&0xF0;
			v=(curcoul&0xF00)>>4;
			b=(curcoul&0xF)<<4;
			if (r!=hsptr || v!=hsptv || b!=hsptb) {
				palette[maxcoul*3+0]=curcoul&0xF0;
				palette[maxcoul*3+1]=(curcoul&0xF00)>>4;
				palette[maxcoul*3+2]=(curcoul&0xF)<<4;
				maxcoul++;
			}
		}
		printf("%d couleur%s importée%s\n",maxcoul-is_hsp,maxcoul-is_hsp>1?"s":"",maxcoul-is_hsp>1?"s":"");

		png_has_transparency=0;
		for (i=0;i<photo->height*photo->width;i++) {
			if (photo->data[i*4+3]>0) { //@@TODO parfaire les choses...
			} else {
				png_has_transparency=1;
			}
		}

		//*****************************************************
		// Fusion de la palette importée avec l'image actuelle
		//*****************************************************
		for (i=0;i<photo->height*photo->width;i++) {
			// we scan only visible colors in alpha channel
			if (photo->data[i*4+3]>0) {
				r=photo->data[i*4+0];
				v=photo->data[i*4+1];
				b=photo->data[i*4+2];

				for (j=0;j<maxcoul;j++) {
					if (r==palette[j*3+0] && v==palette[j*3+1] && b==palette[j*3+2]) break;
				}
				if (j==maxcoul && parameter->cpccolorz) {
					// adjust pixel to palette
					for (j=0;j<maxcoul;j++) {
						if ((r<=0x10 && palette[j*3+0]<=0x10) || (r>=0xE0 && palette[j*3+0]>=0xE0) || (r>=0x70 && palette[j*3+0]>=0x70 && r<=0x90 && palette[j*3+0]<=0x90))
						if ((v<=0x10 && palette[j*3+1]<=0x10) || (v>=0xE0 && palette[j*3+1]>=0xE0) || (v>=0x70 && palette[j*3+1]>=0x70 && v<=0x90 && palette[j*3+1]<=0x90))
						if ((b<=0x10 && palette[j*3+2]<=0x10) || (b>=0xE0 && palette[j*3+2]>=0xE0) || (b>=0x70 && palette[j*3+2]>=0x70 && b<=0x90 && palette[j*3+2]<=0x90)) {
							photo->data[i*4+0]=palette[j*3+0];
							photo->data[i*4+1]=palette[j*3+1];
							photo->data[i*4+2]=palette[j*3+2];
							break;
						}
						       
					}
				}
				// si on n'a pas trouvé la couleur dans la palette importée
				if (j==maxcoul) {
					if (j>=limitcolors) {
						printf(KERROR"\nERROR: too much colors for this resolution! moar than %d colors\n"KNORMAL,limitcolors);
						printf("RGB=%02X.%02X.%02X\n",r,v,b);
						exit(-3);
					} else {
						palette[j*3+0]=r;
						palette[j*3+1]=v;
						palette[j*3+2]=b;
						maxcoul++;
					}
					/*
					 * @@TODO faire une option FORCE_FUSION
					 *
					// search for closest color
					int distance=256*256*256,idist,curdist;
					for (j=0;j<maxcoul;j++) {
						curdist=abs((palette[j*3+0]-r)*(palette[j*3+1]-v)*(palette[j*3+2]-b));
						if (curdist<distance) {
							distance=curdist;
							idist=j;
						}
					}
					photo->data[i*4+0]=palette[idist*3+0];
					photo->data[i*4+1]=palette[idist*3+1];
					photo->data[i*4+2]=palette[idist*3+2];
					*/
				}
			}
		}

		MemFree(txtpalette);
	} else {
		is_hsp=parameter->hsp;
		/* as the bitmap is RGBA we must scan to find all colors */
		maxcoul=0;
		png_has_transparency=0;
		for (i=0;i<photo->height*photo->width;i++) {
			if (photo->data[i*4+3]>0) {
				/* we scan only visible colors */
				r=photo->data[i*4+0];
				v=photo->data[i*4+1];
				b=photo->data[i*4+2];
				for (j=0;j<maxcoul;j++) {
					if (palette[j*3+0]==r && palette[j*3+1]==v && palette[j*3+2]==b) break;
				}

				if (j==maxcoul) {
					if (j>=limitcolors) {
						printf(KERROR"\nERROR: too much colors! moar than %d colors\n"KNORMAL,limitcolors);
						exit(-3);
					} else {
						palette[j*3+0]=r;
						palette[j*3+1]=v;
						palette[j*3+2]=b;
						maxcoul++;
					}
				}
			} else {
				png_has_transparency=1;
			}
		}
        printf(KIO"Image %dx%d with %d colors\n"KNORMAL,photo->width,photo->height,maxcoul);
	}

/*	for (i=0;i<maxcoul;i++) {
		printf("#%02X%02X%02X ",palette[i*3],palette[i*3+1],palette[i*3+2]);
	}
	printf("\n");*/

	/* il ne devrait pas y avoir plus de pixels que sur l'image d'origine */
	transdata=malloc(photo->width*photo->height*2);
	memset(transdata,0,photo->width*photo->height*2);

	if (parameter->scan) {
		bgr=palette[0];
		bgv=palette[1];
		bgb=palette[2];
	}
	if (parameter->grad) {
		SortPalette(palette,maxcoul);
	}
	if (parameter->scan) {
		/* reorg palette without border color */
		scancolor=GetIDXFromPalette(palette,bgr,bgv,bgb);
		if (scancolor!=maxcoul-1) {
			for (i=scancolor*3;i<16*3;i++) palette[i]=palette[i+3];
		}
		maxcoul--;
	}

	/* options check & management */
	if (!parameter->sx || !parameter->sy) {
		parameter->sx=photo->width;
		parameter->sy=photo->height;
	}
	if (parameter->sx>photo->width) {
		printf(KERROR"ERROR: sprite width cannot exceed image dimension\n");
		exit(-5);
	}
	if (parameter->sy>photo->height) {
		printf(KERROR"ERROR: sprite height cannot exceed image dimension\n");
		exit(-5);
	}
	if (parameter->ox>=photo->width) {
		printf(KERROR"ERROR: X offset cannot exceed image dimension\n");
		exit(-5);
	}
	if (parameter->oy>=photo->height) {
		printf(KERROR"ERROR: Y offset cannot exceed image dimension\n");
		exit(-5);
	}
	if (!parameter->nblinescreen) {
		parameter->nblinescreen=photo->height;
	} else if (parameter->nblinescreen>photo->height) {
		printf(KERROR"ERROR: split-screen line cannot exceed image dimension\n");
		exit(-5);
	}

	printf(KVERBOSE"Extraction: ");
	if (parameter->scrmode) printf("screen");
	else if (parameter->hsp) printf("hardware sprite");
	else printf("sprite");
	if (parameter->oldmode) printf("overscan bit");

	if (!parameter->hsp) {
		int checkwidth=1;
		printf(" in mode %d\n"KNORMAL,parameter->mode);
		switch (parameter->mode) {
			case 0:if (photo->width&1 && !parameter->lara) printf(KWARNING"ERROR: Mode 0 image must have width multiple of 2\n");break;
			case 1:if (photo->width&3) printf(KWARNING"ERROR: Mode 1 image must have width multiple of 1\n");break;
			case 2:if (photo->width&7) printf(KWARNING"ERROR: Mode 2 image must have width multiple of 8\n");break;
		}
	} else {
		printf("\n");
	}


	if (parameter->lara) {
		int cpt1=1,cpt2=0,color1=0,color2=-1,colortmp,barjack=0;
		printf(KVERBOSE"*** LARA MODE *** ");
		color1=*(int *)(&photo->data[(photo->height-1)*photo->width*4]);
		for (i=1;i<photo->width;i++) {
			colortmp=*(int *)(&photo->data[(photo->height-1)*photo->width*4+i*4]);
			if (colortmp==color1) {
				cpt1++; 
			} else {
				cpt2++;
				if (color2==-1) {
					color2=colortmp;
					barjack=i;
				} else if (color2!=colortmp) {
					printf(KERROR"INTERNAL ERROR -> lara mode -> too much colors on last line! X1=%d X2=%d\n",barjack,i);
					exit(1);
				}
			}
		}
		if (cpt1+cpt2!=photo->width) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> count marks\n");
			exit(1);
		}
		if (cpt2<cpt1) {
			color1=color2;
			cpt1=cpt2;
		}
		/* on a le marqueur, on contrôle la cohérence */
		decal=i=0;
		color2=*(int *)(&photo->data[(photo->height-1)*photo->width*4]+i*4);
		while (color2!=color1) {
			decal++;
			i++;
			if (i>=photo->width) {
				printf(KERROR"INTERNAL ERROR -> lara mode -> test shift pixel\n");
				exit(1);
			}
			color2=*(int *)(&photo->data[(photo->height-1)*photo->width*4]+i*4);
		}
		printf("decal=%d -> %d byte ",decal,decal>>1);
		if (decal&1) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> odd shift value\n");
			exit(1);
		}
		decal>>=1;
		csx=photo->width/cpt1;
		if (photo->width % cpt1) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> cannot compute atomic width %d %% %d = %d\n",photo->width,cpt1,photo->width % cpt1);
			exit(1);
		}
		theight=0;
		i=photo->height-1;
		while (i>0) {
			theight++;
			i-=8;
		}
		/********** override parameter ****************/
		if (cpt1==25) parameter->maxextract=24; else parameter->maxextract=cpt1;
		parameter->sx=csx;
		parameter->sy=photo->height-1;
		printf(" -> sx=%d\n"KNORMAL,csx);
	}




	if (parameter->hsp) {
		if (parameter->sx>16 || parameter->sy>16) {
			printf(KERROR"ERROR: hardware sprite size cannot exceed 16x16!\n");exit(-1);
		}

		if (parameter->forceextraction) {
			int oldheight;
			oldheight=photo->height;
			photo->height=photo->height+16;
printf(KBLUE"expand image to %d\n"KNORMAL,photo->height);
			photo->data=realloc(photo->data,photo->height*photo->width*4);
			/* init all new pixels to zero */
			memset(photo->data+oldheight*4*photo->width,0,(photo->height-oldheight)*4*photo->width);
		}


		/* hardware sprite are limited to 15 different colors */
		hasblack=GetIDXFromPalette(palette,0,0,0);
		if (hasblack>=0) {
			printf("Image has black color (%d)\n",hasblack);
		} else {
			printf(KWARNING"Image has no black color => disabling -b option\n"KNORMAL);
			parameter->black=0;
		}
		if (maxcoul>15) {
			if (hasblack>=0 && parameter->black) {
				printf("-> black color is transparency\n");
			} else {
				printf(KERROR"ERROR: 15 colors maximum for hardware sprites\n"KNORMAL);
				DisplayPalette(0, maxcoul, palette);
				exit(-5);
			}
		}
		if  (!parameter->importpalettefilename) { // on doit shifter dans tous les cas si on n'importe pas la palette car seule une palette importée sera pré-shiftée
			// sauf si on a dit que le noir était la transparence...
			if (!parameter->black) {
				printf(KVERBOSE"shift palette colors to set transparency at zero index\n"KNORMAL);
				memmove(palette+3,palette,3*15);
				palette[0]=1;
				palette[1]=2;
				palette[2]=3;
			}
		}
		if (parameter->black && hasblack!=0) {
			printf(KVERBOSE"Note: 'black is transparency' forces palette sorting (-grad option)\n"KNORMAL);
			SortPalette(palette,maxcoul);
		}
	} else {
		/* conventionnal export may require screen byte width */
		if (!parameter->width) {
			parameter->width=photo->width/2;
			switch (parameter->mode) {
				default:
				case 0:break;
				case 2:parameter->width/=2; /* no break */
				case 1:parameter->width/=2;
			}
		}
	}
	/* mimic hardware screen to put GFX data where it's supposed to be */
	if (parameter->oldmode) {
		int reste;

		crtcadr=crtcb=0;
		parameter->nblinescreen=2048/parameter->width*parameter->lineperblock;

		/* est-ce qu'on a des lignes partielles ensuite? */
		if (2048/parameter->width != 2048.0/(float)parameter->width) {
			reste=parameter->width-(16384-parameter->nblinescreen*parameter->width)/parameter->lineperblock;
			printf(KWARNING"Warning! Nouvelle bank suit première bank à partir de la ligne %d\n"KNORMAL,parameter->nblinescreen);
			parameter->nblinescreen+=parameter->lineperblock;
			
		} else {
			reste=0;
		}
		printf("Nouvelle bank en début de ligne à partir de la ligne %d\n",parameter->nblinescreen);

		maxscradr=32768/parameter->width;
		scradr=malloc(sizeof(int)*maxscradr);

		for (i=0;i<maxscradr;i++) {
			if (i==parameter->nblinescreen) {
				crtcadr=0x4000+reste;
				crtcb=0;
			}
			scradr[i]=crtcadr;
			crtcadr+=0x800;
			crtcb++;
			if (crtcb>=parameter->lineperblock) {
				crtcadr=crtcadr-0x4000+parameter->width;
				crtcb=0;
			}
		}
	} else if (parameter->scrmode) {
		scradr=malloc(sizeof(int)*280);
		maxscradr=32768;
		crtcadr=crtcb=0;
		for (i=0;i<280;i++) {
			if (i==parameter->nblinescreen) {
				crtcadr=0x4000;
				crtcb=0;
			}
			scradr[i]=crtcadr;
			crtcadr+=0x800;
			crtcb++;
			if (crtcb>=parameter->lineperblock) {
				crtcadr=crtcadr-0x4000+parameter->width;
				crtcb=0;
			}
		}
	}
	

	if (!parameter->splitraster) {
		if (parameter->black) {
			if (palette[0]!=0 || palette[1]!=0 || palette[2]!=0) {
				printf(KERROR"FATAL: there is an error with first color which is not black as expected\n"KNORMAL);
				exit(0);
			}
		}


		//if (parameter->importpalettefilename) {

		printf("paletteplus: defw ");
			for (i=parameter->hsp;i<maxcoul;i++) {
				printf("%s#%03X",i-parameter->hsp?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
			}
		printf("\n");
		printf("paletteGA:   defb ");
			for (i=parameter->hsp;i<maxcoul;i++) {
				printf("%s#%02X",i-parameter->hsp?",":"",GetGAFromRGB(palette[i*3+0],palette[i*3+1],palette[i*3+2]));
			}
		printf("\n");

		if (parameter->exportpaletteBINfilename) {
			FileRemoveIfExists(parameter->exportpaletteBINfilename);
			for (i=parameter->hsp;i<maxcoul;i++) {
				unsigned char xgreen,xBR;
				xgreen=palette[i*3+1]>>4;
				xBR=(palette[i*3+0]&0xF0)|(palette[i*3+2]>>4);
				FileWriteBinary(parameter->exportpaletteBINfilename,&xBR,1);
				FileWriteBinary(parameter->exportpaletteBINfilename,&xgreen,1);
			}

			FileWriteBinaryClose(parameter->exportpaletteBINfilename);
		}
		if (parameter->exportpalettefilename) {
			/* sortie en VRB + DEFW + info transparence */
			char exporttmpcolor[128];
			
			FileRemoveIfExists(parameter->exportpalettefilename);
			strcpy(exporttmpcolor,"defw ");
			FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			// on va jusqu'au max des couleurs peu importe...
			for (i=0;i<maxcoul;i++) {
				sprintf(exporttmpcolor,"%s#%03X",i>parameter->hsp?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
				FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			}
			// si on est en export HSP il faut conserver le décalage de la palette
			if (parameter->hsp) {
				strcpy(exporttmpcolor," ; HSP");
			}
			FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			strcpy(exporttmpcolor,"\n");
			FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			FileWriteBinaryClose(parameter->exportpalettefilename);
		}

	}
	
	cpcdata=MemMalloc(32768+photo->width*photo->height+32);
	memset(cpcdata,0,32768+photo->width*photo->height+32);

	idata=0;
	curline=0;

	/*************************************************
	         h a r d w a r e    s p r i t e s
	*************************************************/
	if (parameter->hsp) {
		int metax,metay;

		for (j=parameter->oy;j<photo->height && ispi<parameter->maxextract;j+=parameter->metay) {
			for (i=parameter->ox;i<photo->width && ispi<parameter->maxextract;i+=parameter->metax) {
				/* sprites automatic search */
				//AutoScan(parameter,photo,palette,&i,&j);
				// meta management
				for (metay=0;metay<parameter->metay;metay+=16)
				for (metax=0;metax<parameter->metax;metax+=16)
				if (j+parameter->sy+metay<=photo->height && i+parameter->sx+metax<=photo->width) {
					/* prepare sprite info */
					curspi.adr=idata;
					curspi.ibitmap=0;
					for (ys=metay;ys<16+metay;ys++) {
						ticpack=0; /* reset pixel counter */
						for (xs=metax;xs<16+metax;xs++) {
							/* transparency */
							if (parameter->search_transparency
								&& photo->data[(i+xs+(j+ys)*photo->width)*4+0]==hsptr
								&& photo->data[(i+xs+(j+ys)*photo->width)*4+1]==hsptv
								&& photo->data[(i+xs+(j+ys)*photo->width)*4+2]==hsptb ) {
								zepix=0;
							} else {
								if (photo->data[(i+xs+(j+ys)*photo->width)*4+3]>0) {
									zepix=GetIDXFromPixel(palette,&photo->data[(i+xs+(j+ys)*photo->width)*4+0]);
									// RIEN A FAIRE!!! if (!parameter->black) zepix++;
								} else {
									zepix=0;
								}
							}
							/* output format */
							switch (parameter->packed) {
								case 0:
									cpcdata[idata++]=zepix;
									break;
								case 2:
									/* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[idata]=zepix; ticpack=1; break;
										case 1: cpcdata[idata++]|=zepix*16; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								case 4:
									/* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[idata]=zepix*64; ticpack=1; break;
										case 1: cpcdata[idata]|=zepix*16; ticpack=2; break;
										case 2: cpcdata[idata]|=zepix*4; ticpack=3; break;
										case 3: cpcdata[idata++]|=zepix; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								default:printf("warning remover\n");break;
							}
							CHECK_IDATA
						}
						/* fill smaller sprites with blank */
						while (xs<16) {
							switch (parameter->packed) {
								case 0: cpcdata[idata++]=0; break;
								case 2: /* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[idata]=0; ticpack=1; break;
										case 1: idata++; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								case 4: /* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[idata]=0; ticpack=1; break;
										case 1: ticpack=2; break;
										case 2: ticpack=3; break;
										case 3: idata++; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								default:printf("warning remover\n");break;
							}
							xs++;
						}
					}
					while (ys<16) {
						switch (parameter->packed) {
							case 0: for (xs=0;xs<16;xs++) cpcdata[idata++]=0; break;
							case 2: /* packed reversed */ for (xs=0;xs<8;xs++) cpcdata[idata++]=0; break;
							case 4: /* packed reversed */ for (xs=0;xs<4;xs++) cpcdata[idata++]=0; break;
							default:printf("warning remover\n");break;
						}
						ys++;
					}
					/* check for empty sprite... */
					for (ys=curspi.adr;ys<idata;ys++) {
						if (cpcdata[ys]) break;
					}
					if (parameter->keep_empty || ys!=idata) {
						/* update sprite info */
						curspi.size=idata-curspi.adr;
						curspi.x=parameter->sx;
						curspi.y=parameter->sy;
						ObjectArrayAddDynamicValueConcat((void **)&spinfo,&ispi,&mspi,&curspi,sizeof(struct s_sprite_info));
//printf("sprite in %d/%d\n",i,j);
						/* raz area with border in case of scanning */
/*
						if (parameter->scan) {
							for (ys=0;ys<parameter->sy;ys++) {
								for (xs=0;xs<parameter->sx;xs++) {
									photo->data[(i+xs+(j+ys)*photo->width)*4+0]=bgr;
									photo->data[(i+xs+(j+ys)*photo->width)*4+1]=bgv;
									photo->data[(i+xs+(j+ys)*photo->width)*4+2]=bgb;
								}
							}
						}
*/
					} else {
						/* rollback */
						idata=curspi.adr;
					}
				}
			}
		}
	} else if (parameter->splitraster) {
	/*************************************************
	      s p l i t - r a s t e r       m o d e
	*************************************************/
		memset(reg,0,sizeof(reg));
		/*
			l'analyse pour split-raster se fait en mode 2 (pour traiter tous les cas d'un coup)
			si le mode est 1 ou 0 alors on suréchantillonne
		*/
		switch (parameter->mode) {
			case 0:
				printf("upsampling in mode 2\n");
				photo->data=MemRealloc(photo->data,photo->width*4*photo->height*4);
				for (i=photo->width*photo->height*4-4;i>=0;i-=4) {
					for (j=0;j<4;j++) {
						photo->data[i*4+j]=photo->data[i+j];
						photo->data[i*4+4+j]=photo->data[i+j];
						photo->data[i*4+8+j]=photo->data[i+j];
						photo->data[i*4+12+j]=photo->data[i+j];
					}
				}
				photo->width*=4;
				break;
			case 1:
				printf("upsampling in mode 2\n");
				photo->data=MemRealloc(photo->data,photo->width*2*photo->height*4);
				for (i=photo->width*photo->height*4-4;i>=0;i-=4) {
					for (j=0;j<4;j++) {
						photo->data[i*2+j]=photo->data[i+j];
						photo->data[i*2+4+j]=photo->data[i+j];
					}
				}
				photo->width*=2;
				break;
			case 2:break;
			default:printf("warning remover\n");break;
		}
		if (photo->width>768 || photo->height>280) {
			printf("maximum image size for splitraster is 768x280 (%d/%d)\n",photo->width,photo->height);
			exit(-1);
		}
		/* analyse du mode requis pour la ligne */
		scr.nbline=photo->height;
		oldmode=-1;
		for (i=0;i<photo->height;i++) {
			accmode=0;
			for (j=0;j<photo->width;j+=8) {
				if (memcmp(&photo->data[i*photo->width*4+j*4],&photo->data[i*photo->width*4+j*4+4],4)) accmode|=2;
				if (memcmp(&photo->data[i*photo->width*4+j*4+8],&photo->data[i*photo->width*4+j*4+12],4)) accmode|=2;
				if (memcmp(&photo->data[i*photo->width*4+j*4+16],&photo->data[i*photo->width*4+j*4+20],4)) accmode|=2;
				if (memcmp(&photo->data[i*photo->width*4+j*4+24],&photo->data[i*photo->width*4+j*4+28],4)) accmode|=2;

				if (memcmp(&photo->data[i*photo->width*4+j*4],&photo->data[i*photo->width*4+j*4+8],8)) accmode|=1;
				if (memcmp(&photo->data[i*photo->width*4+j*4+16],&photo->data[i*photo->width*4+j*4+24],8)) accmode|=1;
			}
			/* combien de couleurs sur la ligne? */
			memset(cptcolor,0,sizeof(cptcolor));
			cptcol=0;
			for (j=0;j<photo->width;j++) {
				cptcolor[GetIDXFromPalette(palette, photo->data[i*photo->width*4+j*4], photo->data[i*photo->width*4+j*4+1], photo->data[i*photo->width*4+j*4+2])]=1;
			}
			for (j=0;j<4096;j++) {
				if (cptcolor[j]) {
					scr.line[i].col[cptcol++]=GetGAFromRGB(palette[j*3+0],palette[j*3+1],palette[j*3+2]);
				}
			}

			if (accmode<oldmode && accmode==0 && cptcol<5) accmode=1;
			if (accmode<oldmode && accmode==1 && cptcol<3) accmode=2;
			scr.line[i].mode=accmode;
			scr.line[i].cptcol=cptcol;
			oldmode=accmode;
		}
		/* analyse inverse pour éviter du mode 0 sur des aplats de mode 1 ou 2 en début d'écran */
		oldmode=-1;
		for (i=photo->height-1;i>=0;i--) {
			accmode=scr.line[i].mode;
			cptcol=scr.line[i].cptcol;
			if (accmode<oldmode && accmode==0 && cptcol<5) accmode=1;
			if (accmode<oldmode && accmode==1 && cptcol<3) accmode=2;
			scr.line[i].mode=accmode;
			//printf("line %3d -> mode %d in %d color%s\n",i,accmode>=2?2:accmode,cptcol,cptcol>1?"s":"");
			oldmode=accmode;
		}
		/* contrôle des clashs si plus de 7 couleurs en mode 2, 9 couleurs en mode 1 ou 21 couleurs en mode 0 */
		for (i=0;i<photo->height;i++) {
			switch (scr.line[i].mode) {
				case 0:maxlinecol=21;break;
				case 1:maxlinecol=9;break;
				case 2:maxlinecol=7;break;
				default:printf("warning remover\n");break;
			}
			if (scr.line[i].cptcol>maxlinecol) {
				clash++;
				for (j=0;j<photo->width;j++) {
					garemove[GetBASICFromRGB(photo->data[i*photo->width*4+j*4],photo->data[i*photo->width*4+j*4+1],photo->data[i*photo->width*4+j*4+2],__LINE__)]=1;
				}
				/* supprimer arbitrairement les premières couleurs rencontrées */
				while (scr.line[i].cptcol>maxlinecol) {
					bareplace=-1;
					for (j=0;j<27;j++) {
						if (garemove[j] && bareplace!=-1) {
							bareplace=j;
						} else {
							break;
						}
					}
					garemove[j]=0;
					/* on supprime la couleur Basic d'index 'j' avec celle d'index 'bareplace' */
					for (j=0;j<photo->width;j++) {
						if (GetBASICFromRGB(photo->data[i*photo->width*4+j*4],photo->data[i*photo->width*4+j*4+1],photo->data[i*photo->width*4+j*4+2],__LINE__)==j) {
							GetRGBFromBASIC(bareplace,&photo->data[i*photo->width*4+j*4],&photo->data[i*photo->width*4+j*4+1],&photo->data[i*photo->width*4+j*4+2]);
						}
					}
					scr.line[i].cptcol--;
				}
			}
		}
		/* construction des blocs */
		for (i=0;i<photo->height;i++) {
			ib=0;
			for (j=0;j<photo->width;j+=16) {
				for (k=j;k<16;k++) {
					col=GetGAFromRGB(photo->data[i*photo->width*4+(j+k)*4],photo->data[i*photo->width*4+(j+k)*4+1],photo->data[i*photo->width*4+(j+k)*4+2]);
					l=0;
					/* brutal */
					do {
						if (scr.line[i].block[ib].col[l]==col) break;
						l++;
						if (l<scr.line[i].block[ib].nbcol) continue;
						scr.line[i].block[ib].col[scr.line[i].block[ib].nbcol++]=col;
					} while (0);
				}
				/* premier contrôle sur le nombre de couleurs maxi par bloc et "correction" du bloc */
				switch (scr.line[i].mode) {
					case 0:maxblockcol=16;break; /* useless test */
					case 1:maxblockcol=4;break;
					case 2:maxblockcol=2;break;
					default:printf("warning remover\n");break;
				}
				while (scr.line[i].block[ib].nbcol>maxblockcol) {
					scr.line[i].block[ib].nbcol--;
					clash++;
					/* use first color for replacement */
					for (k=j;k<j+16;k++) {
						col=GetGAFromRGB(photo->data[i*photo->width*4+(j+k)*4],photo->data[i*photo->width*4+(j+k)*4+1],photo->data[i*photo->width*4+(j+k)*4+2]);
						if (col==scr.line[i].block[ib].col[scr.line[i].block[ib].nbcol]) {
							GetRGBFromGA(scr.line[i].block[ib].col[0],&photo->data[i*photo->width*4+(j+k)*4],&photo->data[i*photo->width*4+(j+k)*4+1],&photo->data[i*photo->width*4+(j+k)*4+2],__LINE__);
						}
					}
				}
				/* on trie par commodité */
				qsort(scr.line[i].block[ib].col,scr.line[i].block[ib].nbcol,sizeof(int),cmpcol);
				/* bloc suivant */
				ib++;
			}

		}
		/* clash probable si plus d'une encre change entre deux blocs  */
		/* clash si changement successifs se font en moins de 4 nops */
		/* scoring pour la couleur de background
		+1	couleur présente sans trou de plus de 3 nops (hors début et fin)
		+nb	couleur ayant le plus de voisines différentes
		-nb	couleur ayant le moins de couleurs non voisines
		+1	couleur était déjà un background la ligne précédente
		*/
		for (i=0;i<16;i++) lastbackgroundcolor[i]=0;
		for (i=0;i<photo->height;i++) {
			/* il faut aussi calculer le groupe des couleurs à split */


			/* scoring pour les couleurs de background */
			memset(colorscore,0,sizeof(colorscore));
			memset(colorstat,0,sizeof(colorstat));
			/* on connait les couleurs utilisées sur chaque ligne */
			for (l=0;l<scr.line[i].cptcol;l++) {
				noptrou=0;
				colormaysplit=0;
				curga=scr.line[i].col[l];
				for (j=0;j<ib;j++) {
					/* est-ce que le bloc contient la couleur? */
					for (m=blockhascolor=0;m<scr.line[i].block[j].nbcol;m++) {
						if (curga==scr.line[i].block[j].col[m]) {
							blockhascolor=1;
							break;
						}
					}
					if (blockhascolor) {
						/* oui -> réinitialiser noptrou */
						noptrou=0;
						/* oui -> ajouter les voisines */
						for (m=blockhascolor=0;m<scr.line[i].block[j].nbcol;m++) {
							if (curga!=scr.line[i].block[j].col[m]) {
								colorstat[curga][scr.line[i].block[j].col[m]]=1;
							}
						}
					} else {
						/* non -> incrémenter noptrou */
						noptrou++;
						if (noptrou>3) {
							colormaysplit=1;
						}
					}


				}
				/* premiers résultats du scoring, si on ne splite pas alors c'est fort probablement du background! */
				colorscore[curga]=(1-colormaysplit)*5;
			}



			/* calcul des meilleures voisines */
			for (l=0;l<scr.line[i].cptcol;l++) {
				curga=scr.line[i].col[l];
				for (j=m=0;j<128;j++) {
					colorscore[curga]+=colorstat[curga][j];
				}
			}

			for (l=0;l<scr.line[i].cptcol;l++) {
				curga=scr.line[i].col[l];
				for (j=0;j<ib;j++) {
					/* est-ce que le bloc contient la couleur? */
					for (m=blockhascolor=0;m<scr.line[i].block[j].nbcol;m++) {
						if (curga==scr.line[i].block[j].col[m]) {
							blockhascolor=1;
							break;
						}
					}
					if (!blockhascolor) {
						for (m=blockhascolor=0;m<scr.line[i].block[j].nbcol;m++) {
							/* une couleur non voisine ne se retrouve jamais avec la couleur en cours */
							if (curga!=scr.line[i].block[j].col[m] && colorstat[curga][scr.line[i].block[j].col[m]]==0) {
								colorstat[curga][scr.line[i].block[j].col[m]]=2;
							}
						}
					}
				}
			}
			/* score des non voisines */
			memset(splitcolor,0,sizeof(splitcolor));
			for (l=n=0;l<scr.line[i].cptcol;l++) {
				curga=scr.line[i].col[l];
				for (j=m=0;j<128;j++) {
					if (colorstat[curga][j]==2) {
						colorscore[curga]--;
						if (splitcolor[n]!=curga) {
							splitcolor[curga]++;
						}
					}
				}
			}


			/* bonus si la couleur était un background la ligne d'avant */
			switch (scr.line[i].mode) {
				case 0:maxlinecol=15;break;
				case 1:maxlinecol=3;break;
				case 2:maxlinecol=1;break;
				default:printf("warning remover\n");break;
			}
			for (l=0;l<scr.line[i].cptcol;l++) {
				curga=scr.line[i].col[l];
				for (m=0;m<maxlinecol;m++) 
				if (lastbackgroundcolor[m]==curga) colorscore[curga]++;
			}
			/* on trie les scores pour avoir nos backgrounds */
			memset(tabscore,0,sizeof(tabscore));
			for (l=0;l<scr.line[i].cptcol;l++) {
				curga=scr.line[i].col[l];
				tabscore[l].score=colorscore[curga];
				tabscore[l].ga=curga;
			}
			qsort(tabscore,scr.line[i].cptcol,sizeof(struct s_score),cmpscore);

			/* un seul background peut changer sinon ben on fera ce qu'on pourra */
			if (!i) {
				/* attribution des backgrounds */
				printf("Première(s) couleur(s) de background:");
				for (l=0;l<maxlinecol;l++) {
					if (!tabscore[l].ga) break;
					lastbackgroundcolor[l]=tabscore[l].ga;
					primarybackgroundcolor[iprim++]=tabscore[l].ga;
					printf(" %02X",tabscore[l].ga);
				}
				printf("\n");
			} else {
				changeback=0;
				for (l=0;l<maxlinecol;l++) {
					if (!tabscore[l].ga) break;
					for (m=0;m<maxlinecol;m++) {
						/* on essaie de retrouver la couleur de background dans le "last" */
						if (lastbackgroundcolor[m]==tabscore[l].ga)
							break;
					}
					if (m==maxlinecol) {
						/* background n'était pas dans la liste précédente */
printf("ligne [%03d] background %02X pas dans la liste des background précédents [",i,tabscore[l].ga);
			for (m=0;m<maxlinecol;m++) {
				printf("%02X ",lastbackgroundcolor[m]);
			}
printf("]\n");
						changeback++;
						for (m=0;m<maxlinecol;m++) {
							if (lastbackgroundcolor[m]==0) {
								/* mais il y a un slot libre */
								printf("on ajoute aux PRIMARY background la couleur %02X et à la liste en cours\n",tabscore[l].ga);
								lastbackgroundcolor[m]=tabscore[l].ga;
								printf("iprim=%d maxlinecol=%d\n",iprim,maxlinecol);
								if (iprim<maxlinecol) {
									primarybackgroundcolor[iprim++]=tabscore[l].ga;
									/* prévoir un reboot de ligne? */
									changeback--;
								}
								break;
							}
						}
						if (changeback) {
							for (m=0;m<maxlinecol;m++) {
								for (n=0;n<l;n++) {
									if (lastbackgroundcolor[m]==tabscore[n].ga)
										break;
								}
								if (n==l) {
printf("le lastbackground %02X est remplacé par %02X\n",lastbackgroundcolor[m],tabscore[l].ga);
									lastbackgroundcolor[m]=tabscore[l].ga;
								}
							}
						}
					} else {
						/* background déjà sélectionné, on ne touche à rien */
					}
					/* enlever le background qu'on vient de choisir des stats dans tous les cas */
					for (m=0;m<128;m++) {
						colorstat[tabscore[l].ga][m]=0;
						colorstat[m][tabscore[l].ga]=0;
						colorstat[m][m]=0; /* on évite les effets de bords */
					}
					/* regarder si les couleurs restantes pourraient spliter ou non (aucune voisine par couleur restante) */
					if (l>=maxlinecol-2) {
						/* on ne teste que si il reste plus d'une couleur! */
						for (m=0;m<128;m++) for (n=0;n<128;n++) {
							if (colorstat[tabscore[n].ga][m]==1) {
								break;
							}
						}
						if (m!=128 || n!=128) {
							/* il reste des couleurs qui doivent etre distinctes en blocs! */
						} else {
							/* quitter la boucle des backgrounds: */
printf("sortie anticipée du traitement des backgrounds\n");
							lindex=l+1;
							break;
						}
					}
				}
				if (changeback) {
					printf("Couleurs de background:");
					for (l=0;l<maxlinecol;l++) {
						if (!tabscore[l].ga) break;
						printf(" %02X",tabscore[l].ga);
					}
					printf(" changeback=%d primary=",changeback);
					for (l=0;l<iprim;l++) {
						printf(" %02X",primarybackgroundcolor[l]);
					}
					printf("\n");
				}
				if (changeback>1) {
					clash++;
					printf("background clash\n");
					/* faire la resolution de la couleur */
				} else if (changeback) {
					/* trouver celle qui dégage  et attribuer + modif structure ligne pour insérer l'ordre de changement de background */
					printf("background change TODO\n");
				}
			}
			/* attribution des couleurs restantes dans les registres de split (pas d'optim d'attribution) */
			m=0;
			for (l=lindex;l<maxlinecol;l++) {
				if (!tabscore[l].ga) break;
				if (m<MAXSPLIT) {
					reg[m++]=tabscore[l].ga;
				} else {
					clash++;
					break;
				}
			}
			while (m<MAXSPLIT) reg[m++]=-1;
			/* registres pour la ligne */
			memcpy(scr.line[i].reg,reg,sizeof(reg));

			
			/* reinit? */
		}
		/* stats & infos */
		printf("primary background colors=");
		for (l=0;l<iprim;l++) {
			printf(" %s",GetStringFromGA(primarybackgroundcolor[l]));
		}
		printf("\n");

		
		/*************************************************
			A s s e m b l y      g e n e r a t i o n
		*************************************************/
			/* parcours des lignes de bloc en bloc, prémices du code sous forme de tokens */
/*
struct s_rastblock block[48];
int col[27];
int cptcol;
int nbblock;
int freenop;
int mode;
int reg[MAXSPLIT];
*/
			/* on initialise pas mal de choses pour la première ligne */

			/*
				exx : ld bc,#7F01 : out (c),0 : ld de,#0203 : ld hl,#8C8D : exx
				pushregisters(reg);
			*/
			/*
			inittoken(token);
			pushtoken(token,line,TOKEN_EXX);
			pushtokenstrict(token,line,TOKEN_EXX,timecode);
			cleantoken(token);
*/
			for (i=1;i<photo->height;i++) {
				/* on reutilise au mieux les registres d'un set de couleurs à l'autre */
				int newreg[MAXSPLIT]={0};
				m=0;
				for (j=0;scr.line[i-1].reg[j]>=0x40;j++) {
					/* chercher si la couleur est réutilisée */
					for (k=0;scr.line[i].reg[k]>=0x40;k++) {
						if (scr.line[i-1].reg[j]==scr.line[i].reg[k]) {
							/* trouve */
							newreg[j]=scr.line[i-1].reg[j];
							break;
						}
					}
				}
				for (k=0;scr.line[i].reg[k]>=0x40;k++) {
					for (j=0;j<MAXSPLIT;j++) {
						if (newreg[j]==scr.line[i].reg[k]) {
							break;
						}
					}
					if (j==MAXSPLIT) {
						/* pas trouve */
						for (j=0;j<MAXSPLIT;j++) {
							if (newreg[j]<0x40) {
								newreg[j]=scr.line[i].reg[k];
								break;
							}
						}
					}
				}
				/*** quels sont les registres qui ont changé du coup? ***/
				if (newreg[0]!=scr.line[i-1].reg[0]) {
					printf("LD A,#%02X\n",newreg[0]);
				}
				if (newreg[1]!=scr.line[i-1].reg[1]) {
					printf("LD C,#%02X\n",newreg[1]);
				}
				/* on essaie de packer DE */
				if (newreg[2]!=scr.line[i-1].reg[2] && newreg[3]!=scr.line[i-1].reg[3]) {
					printf("LD DE,#%04X\n",newreg[2]<<8+newreg[3]);
				} else if (newreg[2]!=scr.line[i-1].reg[2]) {
					printf("LD D,#%02X\n",newreg[2]);
				} else if (newreg[3]!=scr.line[i-1].reg[3]) {
					printf("LD E,#%02X\n",newreg[3]);
				}
				/* et HL */
				if (newreg[4]!=scr.line[i-1].reg[4] && newreg[5]!=scr.line[i-1].reg[5]) {
					printf("LD DE,#%04X\n",newreg[4]<<8+newreg[5]);
				} else if (newreg[4]!=scr.line[i-1].reg[4]) {
					printf("LD D,#%02X\n",newreg[4]);
				} else if (newreg[5]!=scr.line[i-1].reg[5]) {
					printf("LD E,#%02X\n",newreg[5]);
				}
			}
			
			/* tokenlist
				TOKEN_FILLER,TOKEN_NOP,TOKEN_EXX,TOKEN_OUT_0
				TOKEN_OUT_A,TOKEN_OUT_C,TOKEN_OUT_D,TOKEN_OUT_E,TOKEN_OUT_H,TOKEN_OUT_L
				TOKEN_LD_A,TOKEN_LD_C,TOKEN_LD_D,TOKEN_LD_E,TOKEN_LD_H,TOKEN_LD_L,TOKEN_LD_DE,TOKEN_LD_HL
				
			*/
			
			/* relecture des tokens pour créer le code */

			/* tempo generique pour n nops avec n>7
			
			   exx         1
			   ld b,cpt    2
			   djnz $      4*(cpt-1)+3
			   exx         1
			
			*/
		
		
		if (clash) {
			printf("***********************************************\n");
			printf(" t h e r e   i s   c o l o r   c l a s h i n g\n");
			printf("***********************************************\n");
		}
	} else {
	/*************************************************************************************************************************************
	 
	                                                  s c r e e n   &   s p r i t e     m o d e

	**************************************************************************************************************************************/
		/* step */
		if (parameter->tiles) {
			printf("[mode tiles]\n");
			tilewidth=0;
			itile=0;
			tileidx=malloc(sizeof(int)*photo->height*photo->width); // laaaaaaaaaaaaarge
		}
		if (parameter->single) {
			istep=1;
			printf("mode single: one pixel per byte output\n");
		} else {
			switch (parameter->mode) {
				case 0:istep=2;printf("mode 0: pixels grouped by 2\n");break;
				case 1:istep=4;printf("mode 1: pixels grouped by 4\n");break;
				case 2:istep=8;printf("mode 2: pixels grouped by 8\n");break;
				default:printf("warning remover\n");break;
			}
		}
		printf("extraction from %d/%d to %d/%d step %d/%d istep %d\n",parameter->ox,parameter->oy,photo->width,photo->height,parameter->sx,parameter->sy,istep);
		memset(colorz_prev_row,0,sizeof(colorz_in_a_row));
		for (j=parameter->oy;j<photo->height;j+=parameter->sy) {
			tileheight++; // on compte le nombre de tiles en hauteur
			for (i=parameter->ox;i<photo->width;i+=parameter->sx) {
				/* sprites automatic search */
				AutoScan(parameter,photo,palette,&i,&j);

				if (j+parameter->sy<=photo->height && i+parameter->sx<=photo->width) {
					/* prepare sprite info */
					curspi.adr=idata;
					curspi.tadr=itrans;
					curspi.ibitmap=0;
					curspi.bitmap=malloc(parameter->sy*parameter->sx*4); // RGBA
					for (ys=0;ys<parameter->sy;ys++) {
						/* screen mode means adressing memory like CRTC does */
						if (parameter->scrmode) {
							idata=scradr[curline++];
							if (curline>=maxscradr) {
								printf(KERROR"trop de lignes parcourues (%d) pour faire une extraction encodée en entrelacé!\n",curline);
								exit(5);
							}
						}
						if (parameter->rastaline) {
							memset(colorz_in_a_row,0,sizeof(colorz_in_a_row));
							for (xs=0;xs<parameter->sx;xs+=istep) {
								adr=((j+ys)*photo->width+i+xs)*4;
								PushGA(colorz_in_a_row,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2]);
							}
							qsort(colorz_in_a_row,32,sizeof(colorz_in_a_row),cmprow);

							// build palette for the line
							printf(".line%d defb ",ys);
							for (xs=0;colorz_in_a_row[xs];xs++) {
								printf("%s#%02X",xs?",":"",colorz_in_a_row[xs]);
								GetRGBFromGA(colorz_in_a_row[xs], &palette[xs*3+0],&palette[xs*3+1],&palette[xs*3+2],0);
							}
							printf(" ; %d colorz%s\n",xs,xs>16?"TOO MUCH COLORZ":"");
						}
						for (xs=0;xs<parameter->sx;xs+=istep) {
							adr=((j+ys)*photo->width+i+xs)*4;
							zepix=0;
							// ALPHA CHANNEL if (photo->data[(i+xs+(j+ys)*photo->width)*4+3]>0)
							if (parameter->single) {
								/* one pixel per byte to the right / same calculations for every modes */
								pix2=GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2]);
								if (pix2&1) zepix+=64;if (pix2&2) zepix+=4;if (pix2&4) zepix+=16;if (pix2&8) zepix+=1;
							} else {
								switch (parameter->mode) {
									case 0:
										/* classical mode 0 */
										if (photo->data[adr+3]) pix1=GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2]); else pix1=0;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (xs+1<parameter->sx) {
											if (photo->data[adr+7]) pix2=GetIDXFromPalette(palette,photo->data[adr+4],photo->data[adr+5],photo->data[adr+6]); else pix2=0;
											transdata[itrans++]=photo->data[adr+7]>0?1:0;
										} else {
											/* largeur impaire on force à zéro + transparence */
											pix2=0;
											transdata[itrans++]=0;
										}

										if (pix1&1) zepix+=128;if (pix1&2) zepix+=8;if (pix1&4) zepix+=32;if (pix1&8) zepix+=2;
										if (pix2&1) zepix+=64;if (pix2&2) zepix+=4;if (pix2&4) zepix+=16;if (pix2&8) zepix+=1;
										break;
									case 1:
										pix1=GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2]);
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										pix2=GetIDXFromPalette(palette,photo->data[adr+4],photo->data[adr+5],photo->data[adr+6]);
										transdata[itrans++]=photo->data[adr+7]>0?1:0;
										pix3=GetIDXFromPalette(palette,photo->data[adr+8],photo->data[adr+9],photo->data[adr+10]);
										transdata[itrans++]=photo->data[adr+11]>0?1:0;
										pix4=GetIDXFromPalette(palette,photo->data[adr+12],photo->data[adr+13],photo->data[adr+14]);

if (pix1==-1 || pix2==-1 || pix3==-1 || pix4==-1) printf("pixel en %d/%d\n",i+xs,j+ys);

										transdata[itrans++]=photo->data[adr+15]>0?1:0;
										if (pix1&1) zepix+=128;if (pix1&2) zepix+=8;
										if (pix2&1) zepix+=64;if (pix2&2) zepix+=4;
										if (pix3&1) zepix+=32;if (pix3&2) zepix+=2;
										if (pix4&1) zepix+=16;if (pix4&2) zepix+=1;
										break;
									case 2:
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=128;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=64;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=32;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=16;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=8;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=4;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=2;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										if (GetIDXFromPalette(palette,photo->data[adr+0],photo->data[adr+1],photo->data[adr+2])) zepix+=1;adr+=4;
										transdata[itrans++]=photo->data[adr+3]>0?1:0;
										break;
									default:printf("warning remover\n");break;
								}
							}
							cpcdata[idata]=zepix;
							if (parameter->scrmode && parameter->oldmode) {
								if ((idata & 0x7FF)==0x7FF) idata+=0x3800;
							}
							idata++;
							CHECK_IDATA
							if (idata>maxdata) maxdata=idata;
						}
					}

					for (xs=curspi.tadr;xs<itrans;xs++) {
						if (transdata[xs]) break;
					}
					if (xs==itrans) {
						memset(&cpcdata[curspi.adr],0,curspi.size); // full trans => enforce no data
						tiletrans=1;
					} else {
						tiletrans=0;
					}

					/* update sprite info */
					curspi.size=maxdata-curspi.adr;
					curspi.x=parameter->sx;
					curspi.y=parameter->sy;

					/* tiles mode (if selected) will check for unicity */
					if (parameter->tiles) {
						int scheck;
						
						if (j==parameter->oy) tilewidth++; // on compte la largeur sur la première ligne
										   //
						if (!tiletrans) {
							for (scheck=0;scheck<ispi;scheck++) {
								if (memcmp(cpcdata+curspi.adr,cpcdata+spinfo[scheck].adr,curspi.size)==0) {
									// we already stored this tile!
									break;
								}
							}
							tileidx[itile++]=scheck; // on stocke la tile dans la megamap
							if (scheck==ispi) {
								ObjectArrayAddDynamicValueConcat((void **)&spinfo,&ispi,&mspi,&curspi,sizeof(struct s_sprite_info)); // on ajoute si la tile n'existait pas
								
								for (ys=0;ys<parameter->sy;ys++) {
									for (xs=0;xs<parameter->sx;xs++) {
										curspi.bitmap[curspi.ibitmap++]=photo->data[(i+xs+(j+ys)*photo->width)*4];
										curspi.bitmap[curspi.ibitmap++]=photo->data[(i+xs+(j+ys)*photo->width)*4+1];
										curspi.bitmap[curspi.ibitmap++]=photo->data[(i+xs+(j+ys)*photo->width)*4+2];
										curspi.bitmap[curspi.ibitmap++]=photo->data[(i+xs+(j+ys)*photo->width)*4+3];
									}
								}
							}
						} else {
							tileidx[itile++]=-1; // transparence, maximum en négatif
						}
					} else {
//printf("extraction %d %d/%d\n",ispi+1,curspi.x,curspi.y);
						/* update sprite info */
						ObjectArrayAddDynamicValueConcat((void **)&spinfo,&ispi,&mspi,&curspi,sizeof(struct s_sprite_info));
						if (parameter->maxextract<=ispi) {printf("*break*\n");i=photo->width;j=photo->height;break;}
					}
				}
			}
		}
	}


	if (parameter->rotoheight) {
		int xcenter,ycenter,wmax,iridx=0;
		float ang;
		unsigned char *rotoffsetmap;
		// contrôle de cohérence sur les données CPC produites en amont? Minimum
		if (parameter->scrmode) {
			printf("cannot process rotomap in screen mode...\n");
			exit(-344);
		}
		// trouver le centre des données pour connaitre la hauteur maximale possible de la map
		xcenter=parameter->sx>>1;
		ycenter=parameter->sy>>1;
		if (xcenter>ycenter) wmax=xcenter; else wmax=ycenter;

		rotoffsetmap=malloc(wmax*256);
		memset(rotoffsetmap,0,wmax*256);

		// ensuite on part du centre
		printf("rotoheight central byte will be: #%02X  wmax=%d\n",photo->data[(xcenter+ycenter*parameter->sx)*4],wmax);
		// et on fait des cercles petit à petit
		for (i=1;i<wmax;i++) {
			for (j=0;j<256;j++) {
				ang=(float)j/256.0*3.1415926545*2.0; // en radian
				rotoffsetmap[iridx++]=photo->data[((int)(cos(ang)*((float)i+0.5))+xcenter+((int)(sin(ang)*((float)i+0.5))+ycenter)*parameter->sx)*4];
			}
		}

		FileRemoveIfExists(parameter->rotoheightfilename);
		FileWriteBinary(parameter->rotoheightfilename,rotoffsetmap,wmax*256);
		FileWriteBinaryClose(parameter->rotoheightfilename);
		free(rotoffsetmap);
		printf(KIO"rotoheightmap written %d bytes\n"KNORMAL,wmax*256);
	}
	if (parameter->rotoffset) {
		int xcenter,ycenter,wmax,iridx=0;
		float ang;
		unsigned char *rotoffsetmap;
		// contrôle de cohérence sur les données CPC produites en amont? Minimum
		if (parameter->scrmode) {
			printf("cannot process rotomap in screen mode...\n");
			exit(-344);
		}
		// trouver le centre des données pour connaitre la hauteur maximale possible de la map
		xcenter=parameter->sx>>1;
		ycenter=parameter->sy>>1;
		if (xcenter>ycenter) wmax=xcenter; else wmax=ycenter;

		rotoffsetmap=malloc(wmax*256);
		memset(rotoffsetmap,0,wmax*256);

		// ensuite on part du centre
		printf("rotoffset central byte will be: #%02X\n",cpcdata[xcenter+ycenter*parameter->sx]);
		// et on fait des cercles petit à petit
		for (i=1;i<wmax;i++) {
			for (j=0;j<256;j++) {
				ang=(float)j/256.0*3.1415926545*2.0; // en radian
				rotoffsetmap[iridx++]=cpcdata[(int)(cos(ang)*((float)i+0.5))+xcenter+((int)(sin(ang)*((float)i+0.5))+ycenter)*parameter->sx];
			}
		}

		FileRemoveIfExists(parameter->rotoffsetfilename);
		FileWriteBinary(parameter->rotoffsetfilename,rotoffsetmap,wmax*256);
		FileWriteBinaryClose(parameter->rotoffsetfilename);
		free(rotoffsetmap);
		printf(KIO"rotoffsetmap written %d bytes\n"KNORMAL,wmax*256);
	}


	if (parameter->tiles) {
		struct s_png_info *tilesPNG;
		int extsize,idxspi=0;
		FILE *metaFile;

		if (!parameter->metatiles) {
			if (!parameter->scrx || !parameter->scry) {
				strcpy(newname,parameter->filename);
				strcpy(newname+strlen(newname)-3,"tilemap");
				metaFile=fopen(newname,"wb");
					
				fprintf(metaFile,";*****************************\n");
				fprintf(metaFile,";       tilewidth=%d\n",tilewidth);
				fprintf(metaFile,";*****************************\n");
				if (!parameter->splitLowHigh) {
					i=j=0;
					while (i<itile) {
						if (!j) fprintf(metaFile,"%s %d",ispi>256?"defw":"defb",tileidx[i++]); else fprintf(metaFile,",%d",tileidx[i++]);
						j++;
						if (j==tilewidth) {
							fprintf(metaFile,"\n");
							j=0;
						}
					}
				} else {
					i=j=0;
					while (i<itile) {
						if (!j) fprintf(metaFile,"defb %d",tileidx[i++]&0xFF); else fprintf(metaFile,",%d",tileidx[i++]&0xFF);
						j++;
						if (j==tilewidth) {
							fprintf(metaFile,"\n");
							j=0;
						}
					}
					if (ispi>256) {
						// uniquement si on dépasse 8 bits en nombre de tiles
						fprintf(metaFile,".split\n");
						i=j=0;
						while (i<itile) {
							if (!j) fprintf(metaFile,"defb %d",tileidx[i++]>>8); else fprintf(metaFile,",%d",tileidx[i++]>>8);
							j++;
							if (j==tilewidth) {
								fprintf(metaFile,"\n");
								j=0;
							}
						}
					}
				}
				fprintf(metaFile,"\n");
			} else {
				strcpy(newname,parameter->filename);
				strcpy(newname+strlen(newname)-3,"tilescreen");
				metaFile=fopen(newname,"wb");

				scrtilex=parameter->scrx/parameter->sx;
				scrtiley=parameter->scry/parameter->sy;
				fprintf(metaFile,";*****************************\n");
				fprintf(metaFile,";   tilescreen  %d x %d\n",tilewidth/scrtilex,tileheight/scrtiley);
				fprintf(metaFile,";*****************************\n");
				for (j=0;j<tileheight;j+=scrtiley) {
					for (i=0;i<tilewidth;i+=scrtilex) {
						int tilex;
						fprintf(metaFile,".screen%dx%d",i/scrtilex,j/scrtiley);

						for (ys=tiletrans=0;ys<scrtiley && !tiletrans;ys++) {
							tilex=0;
							for (xs=0;xs<scrtilex;xs++) {
								if (tileidx[(ys+j)*tilewidth+i+xs]==-1) {
									tiletrans=1;
									break;
								}
								// one line
								if (tileidx[(ys+j)*tilewidth+i+xs]==tilexclude) {
									tilex++;
								}
							}
							if (tilex==scrtilex) tiletrans=1;
						}
						if (!tiletrans) {
							fprintf(metaFile,"\nlzx0\n");

							if (!parameter->splitLowHigh) {
								for (ys=0;ys<scrtiley;ys++) {
									if (ispi>256) fprintf(metaFile,"defw "); else fprintf(metaFile,"defb ");
									for (xs=0;xs<scrtilex;xs++) {
										if (xs) fprintf(metaFile,",");
										fprintf(metaFile,"%d",tileidx[(ys+j)*tilewidth+i+xs]);
									}
									fprintf(metaFile,"\n");
								}
							} else {
								for (ys=0;ys<scrtiley;ys++) {
									fprintf(metaFile,"defb ");
									for (xs=0;xs<scrtilex;xs++) {
										if (xs) fprintf(metaFile,",");
										fprintf(metaFile,"%d",tileidx[(ys+j)*tilewidth+i+xs]&0xFF);
									}
									fprintf(metaFile,"\n");
								}
								if (ispi>256) {
									fprintf(metaFile,"; split\n");
									// uniquement si on dépasse 8 bits
									for (ys=0;ys<scrtiley;ys++) {
										fprintf(metaFile,"defb ");
										for (xs=0;xs<scrtilex;xs++) {
											if (xs) fprintf(metaFile,",");
											fprintf(metaFile,"%d",tileidx[(ys+j)*tilewidth+i+xs]>>8);
										}
										fprintf(metaFile,"\n");
									}
								}
							}

							fprintf(metaFile,"\nlzclose\n");
							fprintf(metaFile,".screenEnd%dx%d\n",i/scrtilex,j/scrtiley);
							fprintf(metaFile,"save 'screen%dx%d.zx0',.screen%dx%d,.screenEnd%dx%d-.screen%dx%d\n",
									i/scrtilex,j/scrtiley,
									i/scrtilex,j/scrtiley,
									i/scrtilex,j/scrtiley,
									i/scrtilex,j/scrtiley);
						} else {
							fprintf(metaFile," ; empty (transparency not allowed)\n");
						}
					}
				}
				fprintf(metaFile,"\n");
				fprintf(metaFile,"\n");
			}
		} else {
			int **tlist=NULL;
			int curmt[256]; // max allowed
			int mtlist=0;
			int imt;

			// metatiles!
			printf("; metatiles theorical maximum combination is %d\n",ispi*parameter->metatiles*parameter->metatiles);

			tlist=malloc(sizeof(int*)*ispi*parameter->metatiles*parameter->metatiles); // maximum combination

			strcpy(newname,parameter->filename);
			strcpy(newname+strlen(newname)-3,"meta");
			metaFile=fopen(newname,"wb");
			fprintf(metaFile,";****************************************\n");
			fprintf(metaFile,".metatiles\n");
			fprintf(metaFile,";****************************************\n");

			// extract only full metatiles
			for (j=0;(j+parameter->metatiles)*tilewidth-1<itile;j+=parameter->metatiles) {
				for (i=0;i+parameter->metatiles-1<tilewidth;i+=parameter->metatiles) {
					for (ys=imt=0;ys<parameter->metatiles;ys++) {
						for (xs=0;xs<parameter->metatiles;xs++) {
							curmt[imt++]=tileidx[(j+ys)*tilewidth+i+xs];
						}
					}
					// look for match
					for (xs=0;xs<mtlist;xs++) {
						if (!memcmp(tlist[xs],curmt,sizeof(int)*parameter->metatiles*parameter->metatiles)) break;
					}
					if (xs==mtlist) {
						tlist[mtlist]=malloc(sizeof(int)*parameter->metatiles*parameter->metatiles);
						memcpy(tlist[mtlist],curmt,sizeof(int)*parameter->metatiles*parameter->metatiles);
						mtlist++;
					}
				}
			}
			fprintf(metaFile,"; %d metatile%s found\n",mtlist,mtlist>1?"s":"");
			for (xs=0;xs<mtlist;xs++) {
				fprintf(metaFile,".metatile%d defw ",xs); // on stocke des pointeurs, + efficient
				for (i=0;i<parameter->metatiles*parameter->metatiles;i++) {
					if (i) fprintf(metaFile,",");
					fprintf(metaFile,".tile%d",tlist[xs][i]);
				}
				fprintf(metaFile,"\n");
			}
			fprintf(metaFile,"\n");

			fprintf(metaFile,";****************************************\n");
			fprintf(metaFile,".tilemap\n");
			fprintf(metaFile,";****************************************\n");
			for (j=0;(j+parameter->metatiles)*tilewidth-1<itile;j+=parameter->metatiles) {
				fprintf(metaFile,".row%d %s ",j/parameter->metatiles,mtlist>256?"defw":"defb");
				for (i=0;i+parameter->metatiles-1<tilewidth;i+=parameter->metatiles) {
					for (ys=imt=0;ys<parameter->metatiles;ys++) {
						for (xs=0;xs<parameter->metatiles;xs++) {
							curmt[imt++]=tileidx[(j+ys)*tilewidth+i+xs];
						}
					}
					// look for match
					for (xs=0;xs<mtlist;xs++) {
						if (!memcmp(tlist[xs],curmt,sizeof(int)*parameter->metatiles*parameter->metatiles)) break;
					}
					if (i) fprintf(metaFile,",");
					fprintf(metaFile,"%d",xs);
				}
				fprintf(metaFile,"\n");
			}


			i=j=0;
			while (i<itile) {
				if (!j) printf("%s %d",ispi>256?"defw":"defb",tileidx[i++]); else printf(",%d",tileidx[i++]);
				j++;
				if (j==tilewidth) {
					printf("\n");
					j=0;
				}
			}
		}
		printf(KIO"writing tile file %s\n"KNORMAL,newname);
		fclose(metaFile);

		// export des tiles
		tilesPNG=PNGInit(NULL);
		tilesPNG->color_type=PNG_COLOR_TYPE_RGBA;
		tilesPNG->bit_depth=8;
		tilesPNG->compression_type=0;
		extsize=(int)sqrt(ispi)+1; // nbtiles in a row
		tilesPNG->width=extsize*parameter->sx;
		tilesPNG->height=extsize*parameter->sy;
		tilesPNG->data=malloc(tilesPNG->width*tilesPNG->height*4);
		memset(tilesPNG->data,0,tilesPNG->width*tilesPNG->height*4);
		printf("nbtiles=%d tilemap will be %dx%d => %dx%d\n",ispi,extsize,extsize,tilesPNG->width,tilesPNG->height);

		for (j=0;j<extsize && idxspi<ispi;j++) {
			for (i=0;i<extsize && idxspi<ispi;i++) {
				// copy tile in the tilemap
				unsigned int startOffset=(j*parameter->sy*tilesPNG->width+i*parameter->sx)*4;
				for (ys=0;ys<parameter->sy;ys++) {
					for (xs=0;xs<parameter->sx;xs++) {
						tilesPNG->data[startOffset+(xs+ys*tilesPNG->width)*4+0]=spinfo[idxspi].bitmap[(xs+ys*parameter->sx)*4+0];
						tilesPNG->data[startOffset+(xs+ys*tilesPNG->width)*4+1]=spinfo[idxspi].bitmap[(xs+ys*parameter->sx)*4+1];
						tilesPNG->data[startOffset+(xs+ys*tilesPNG->width)*4+2]=spinfo[idxspi].bitmap[(xs+ys*parameter->sx)*4+2];
						tilesPNG->data[startOffset+(xs+ys*tilesPNG->width)*4+3]=spinfo[idxspi].bitmap[(xs+ys*parameter->sx)*4+3];
					}
				}

				idxspi++;
			}
		}
		PNGWrite(tilesPNG,"tileSheet.png");
	}
	/* info */
	if (!parameter->scrmode) {
		printf(KVERBOSE"%d %ssprite%s extracted\n"KNORMAL,ispi,parameter->hsp?"hardware ":"",ispi>1?"s":"");
	}

	if (parameter->outputfilename) {
		strcpy(newname,parameter->outputfilename);
	} else {
		strcpy(newname,parameter->filename);
		if (!parameter->asmdump) {
			strcpy(newname+strlen(newname)-3,"bin");
		} else {
			strcpy(newname+strlen(newname)-3,"asm");
		}
	}

	byteleft=maxdata>idata?maxdata:idata;
	if (!parameter->split) parameter->split=byteleft;
	if (parameter->asmdump) {
		char dataline[1024];
		j=0;
		while (byteleft>0) {
			strcpy(dataline,"defb ");
			for (i=0;i<32 && byteleft-i>0;i++) {
				if (i) strcat(dataline,",");
				sprintf(dataline+strlen(dataline),"#%02X",cpcdata[j+i]);
			}
			strcat(dataline,"\n");
			j+=32;
			byteleft-=32;
			FileWriteBinary(newname,dataline,strlen(dataline));
		}
		FileWriteBinaryClose(newname);
	} else {
		/**************** écriture des fichiers binaires *************/
		if (!parameter->tiles) {
			char pakaname[2048];
			unsigned char *paka,amask;
			int paki;
			paka=malloc(byteleft*2);
			woffset=0;
			strcpy(pakaname,newname);
			strcpy(pakaname+strlen(newname)-3,"p01");

			while (byteleft) {
				FileRemoveIfExists(newname);
				if (byteleft>parameter->split) {
					if (filenumber<5) {
						printf(KIO"writing %d bytes in %s\n"KNORMAL,parameter->split,newname);				
					}
					FileWriteBinary(newname,cpcdata+woffset,parameter->split);
					FileWriteBinaryClose(newname);
					/***************** écriture des packAlpha ******************************/
					if (parameter->packAlpha) {
						for (paki=0;paki<parameter->split;paki++) {
							// mask first
							paka[paki*2+1]=cpcdata[woffset+paki];
							if (transdata[woffset*2+paki*2]) amask=0; else amask=0xAA;
							if (!transdata[woffset*2+paki*2+1]) amask|=0x55;
							paka[paki*2+0]=amask;
						}
						FileRemoveIfExists(pakaname);
						FileWriteBinary(pakaname,paka,parameter->split*2);
						FileWriteBinaryClose(pakaname);
						if (filenumber<100) sprintf(pakaname+strlen(pakaname)-2,"%02d",filenumber);
						else if (filenumber>=100) sprintf(pakaname+strlen(pakaname)-3,"%03d",filenumber);
					}
					/***********************************************************************/
					woffset+=parameter->split;
					byteleft-=parameter->split;
					if (filenumber==5 && byteleft>0) {
						printf("(...)\n");
					}
					/* set next filename */
					if (filenumber<100) sprintf(newname+strlen(newname)-2,"%02d",filenumber++);
					else if (filenumber>=100) sprintf(newname+strlen(newname)-3,"%03d",filenumber++);

				} else {
					printf(KIO"writing %d bytes in %s\n"KNORMAL,byteleft,newname);
					FileWriteBinary(newname,cpcdata+woffset,byteleft);
					FileWriteBinaryClose(newname);
					/***************** écriture des packAlpha ******************************/
					if (parameter->packAlpha) {
						for (paki=0;paki<byteleft;paki++) {
							// mask first
							paka[paki*2+1]=cpcdata[woffset+paki];
							if (transdata[woffset*2+paki*2]) amask=0; else amask=0xAA;
							if (!transdata[woffset*2+paki*2+1]) amask|=0x55;
							paka[paki*2+0]=amask;
						}
						FileRemoveIfExists(pakaname);
						FileWriteBinary(pakaname,paka,byteleft*2);
						FileWriteBinaryClose(pakaname);
					}
					/***********************************************************************/
					byteleft=0;
				}
			}
			/***********************************************************************/
			// start again in revert?
			/***********************************************************************/
			if (parameter->reverse && parameter->hsp) {
				unsigned char byteTMP,revertConv[256];
				byteleft=maxdata>idata?maxdata:idata; // arm again bytecount

				// revert data
				switch (parameter->packed) {
					case 0:
						if (byteleft&0xFF) { fprintf(stderr,"hardware sprite extraction error, could be 256 factor\n"); exit(1); }
						for (j=0;j<byteleft;j+=256) {
							for (i=j;i<j+256;i+=16) {
								for (xs=0;xs<8;xs++) {
									byteTMP=cpcdata[i+xs];
									cpcdata[i+xs]=cpcdata[i+15-xs];
									cpcdata[i+15-xs]=byteTMP;
								}
							}
						}
						break;
					case 2:
						if (byteleft&0x7F) { fprintf(stderr,"hardware sprite extraction error, could be 128 factor\n"); exit(1); }
						for (i=0;i<256;i++) revertConv[i]=((i&15)<<4)|((i>>4)&15);
						for (j=0;j<byteleft;j+=128) {
							for (i=j;i<j+128;i+=8) {
								for (xs=0;xs<4;xs++) {
									byteTMP=revertConv[cpcdata[i+xs]];
									cpcdata[i+xs]=revertConv[cpcdata[i+7-xs]];
									cpcdata[i+7-xs]=byteTMP;
								}
							}
						}
						break;
					case 4:printf("conversion @@TODO\n");
					       break;
					default:fprintf(stderr,"abnormal HSP packing error with revert\n");exit(1);
				}

			}



			free(paka);
		} else {
			FileRemoveIfExists(newname);
			byteleft=0;
			for (i=0;i<ispi;i++) {
				FileWriteBinary(newname,cpcdata+spinfo[i].adr,spinfo[i].size);
				byteleft+=spinfo[i].size;
			}
			FileWriteBinaryClose(newname);
			printf(KIO"writing %d bytes in %s\n"KNORMAL,byteleft,newname);
		}
		/***************** écriture des masques ******************************/
		parameter->split*=2;
		if (!parameter->noalpha) { // pas obligé
			for (i=0;i<itrans;i++) {
				/* si on n'a pas de transparence on n'écrit pas */
				if (transdata[i]) {
					woffset=0;
					byteleft=itrans;
					strcpy(newname+strlen(newname)-3,"t01");filenumber=2;
					while (byteleft) {
						FileRemoveIfExists(newname);
						if (byteleft>parameter->split) {
							if (filenumber<5) {
								printf(KIO"writing %d bytes in %s\n"KNORMAL,parameter->split,newname);				
							}
							FileWriteBinary(newname,transdata+woffset,parameter->split);
							woffset+=parameter->split;
							FileWriteBinaryClose(newname);
							byteleft-=parameter->split;
							if (filenumber==5 && byteleft>0) {
								printf("(...)\n");
							}
							/* set next filename */

							if (filenumber<100) sprintf(newname+strlen(newname)-2,"%02d",filenumber++);
							else if (filenumber>=100) { printf("error\n");}
						} else {
							printf(KIO"writing %d bytes in %s\n"KNORMAL,byteleft,newname);
							FileWriteBinary(newname,transdata+woffset,byteleft);
							byteleft=0;
						}
						FileWriteBinaryClose(newname);
					}
					break;
				}
			}
		}
		if (parameter->lara) {
			char geobuffer[512];
			strcpy(newname+strlen(newname)-3,"geo");
			FileRemoveIfExists(newname);
			sprintf(geobuffer,"%d\n%d\n%d\n%d\n",(parameter->sx+1)>>1,parameter->sy,ispi,decal);
			FileWriteBinary(newname,geobuffer,strlen(geobuffer)+1);
			FileWriteBinaryClose(newname);
		}
	}
	/* output sheet if any */
	if (parameter->sheetfilename) {
		char txtspinfo[1024];
		printf(KIO"Extraction info saved in %s\n"KNORMAL,parameter->sheetfilename);
		FileRemoveIfExists(parameter->sheetfilename);
		sprintf(txtspinfo,"; extraction info for %s\n",parameter->filename);
		FileWriteBinary(parameter->sheetfilename,txtspinfo,strlen(txtspinfo));
		for (i=0;i<ispi;i++) {
			strcpy(txtspinfo,"defw ");
			if (1) sprintf(txtspinfo+strlen(txtspinfo),"#%04X",spinfo[i].adr);
			if (1) sprintf(txtspinfo+strlen(txtspinfo)," : defb #%02X",spinfo[i].x);
			if (1) sprintf(txtspinfo+strlen(txtspinfo),",#%02X",spinfo[i].y);
			strcat(txtspinfo,"\n");
			FileWriteBinary(parameter->sheetfilename,txtspinfo,strlen(txtspinfo));
		}
		FileWriteBinaryClose(parameter->sheetfilename);
	}
	/* clean memory */
	PNGFree(&photo);
	MemFree(cpcdata);
	if (mspi) {
		MemFree(spinfo);
	}
	if (tileidx) MemFree(tileidx);
}

/***************************************
	semi-generic body of program
***************************************/


void Help(char *zecommand) {
	#undef FUNC
	#define FUNC "Help"
	
	if (zecommand[0]=='-') zecommand++; // skip dash

	if (strcmp(zecommand,"flat")==0) {
		printf("-flat option:\n");
		printf("instead of producing one file per extraction (in sprite mode)\n");
		printf("or 16K file for overscan extraction, all files are gathered\n");
		printf("into one file\n");
	} else if (strcmp(zecommand,"split")==0) {
		printf("-split option:\n");
		printf("define output split size\n");
		printf("the default value is 16K for screen output\n");
		printf("the default value depend on sprite size for HSP/sprite output\n");
		printf("\n");
		printf("example: convgeneric myfile.png -split 4K\n");
		printf("or:      convgeneric myfile.png -split 4096\n");
	} else if (strcmp(zecommand,"")==0) {
		printf("\n");
		printf("\n");
	} else if (strcmp(zecommand,"a")==0) {
		printf("\n");
		printf("\n");
	} else if (strcmp(zecommand,"b")==0) {
		printf("\n");
		printf("\n");
	} else if (strcmp(zecommand,"c")==0) {
		printf("\n");
		printf("\n");
	} else if (strcmp(zecommand,"d")==0) {
		printf("\n");
		printf("\n");
	} else if (strcmp(zecommand,"e")==0) {
		printf("\n");
		printf("\n");
	}
}

/*
	Usage
	display the mandatory parameters
*/
void Usage(char **argv)
{
	#undef FUNC
	#define FUNC "Usage"
	
	printf("usage: %.*s.exe <pngfile> <options>\n",(int)(sizeof(__FILENAME__)-3),__FILENAME__);
	printf("\n");
	printf("integrated help: convgeneric help <command>\n");
	printf("\n");
	printf("general options:\n");
	printf("-o <file>        full output filename\n");
	printf("-flat            output one file for all extractions\n");
	printf("-split <size>    split output files with size ex: 4K or 4096\n");
	printf("-m <mode>        output mode (0,1,2)\n");
	printf("-g               sort palette from darkest to brightest color\n");
	printf("-asmdump         output assembly dump\n");
	printf("-exnfo <file>    export assembly informations about extracted zones\n");
	printf("-expal <file>    export palette in a text file\n");
	printf("-exbinpal <file> export Plus palette in a binary file\n");
	printf("-impal <file>    import palette from a text file\n");
	printf("-cpccolor        round a little colorz for CPC\n");
	printf("-noalpha         do not extract transparency map\n");
	printf("\n");
	printf("demomaking options:\n");
	printf("-rotoffset  <file> export rototexture for rotoffset\n");
	printf("-rotoheight <file> export rotoheightmap for rotoffset\n");
	printf("-heightmap  <file> export heightmap\n");
	printf("\n");
	printf("sprite options: (default progressive output)\n");
	printf("-scan             scan sprites inside border\n");
	printf("-fontscan         font extraction\n");
	printf("-single           only one pixel per byte to the right side\n");
	printf("-size <geometry>  set sprite dimensions in pixels. Ex: -size 16x16 \n");
	printf("-scrz <geometry>  set screen dimensions in pixels. Ex: -scrz 320x200 \n");
	printf("-splitLowHigh     export low bytes then high bytes for tilemap\n");
	printf("-tilexclude <n>   exclude tile number n for tilemap\n");
	printf("-offset <pos>     set start offset from top/left ex: 20,2\n");
	printf("-mask             add mask info to data (see doc)\n");
	printf("-c <maxsprite>    maximum number of sprites to extract\n");
	printf("-meta <geometry>  gather sprites when scanning. Ex: -meta 3x2\n");
	printf("-tiles            extract uniques sprites + map\n");
	printf("-metatiles <size> create metatiles translation table\n");
	printf("-packAlpha        create data+transparency mask datafile (.p01)\n");

	printf("\n");
	printf("screen options:\n");
	printf("-scr             enable screen output (interlaced data)\n");
	printf("-old             enable overscan screen output (interlaced data)\n");
	printf("-lb <nblines>    number of lines per block for screen output (for CPC+ split)\n");
	printf("-ls <nblines>    number of lines of 1st screen in extended screen output\n");
	printf("-w <width>       force output screen width in bytes\n");
	printf("-splitraster     split-raster analysis (unfinished)\n");
	printf("-rastaline       allow more color in global image and generate raster info\n");
	printf("\n");
	printf("hardware sprite options:\n");
	printf("-hsp             enable hardware sprite mode\n");
	printf("-meta <nxn>      extraction of meta sprites\n");
	printf("-scan            scan sprites inside border\n");
	printf("-b               black is transparency (keep real transparency)\n");
	printf("-st              search transparency color\n");
	printf("-p 2             store data 4bits+4bits in reverse order into a single byte\n");
	printf("-p 4             store data 2+2+2+2bits in logical order into a single byte\n");
	printf("-force           force extraction of incomplete sprites\n");
	printf("-k               keep empty sprites\n");
	printf("-reverse         create additional sprites flipped left/right\n");

	printf("\n");

	if (argv) {
		printf("\n");
		printf("Error parsing word: %s\n",argv[0]);
		printf("\n");
	}
	
	exit(ABORT_ERROR);
}


/*
	ParseOptions
	
	used to parse command line and configuration file
*/
int ParseOptions(char **argv,int argc, struct s_parameter *parameter)
{
	#undef FUNC
	#define FUNC "ParseOptions"
	char *scissor;
	int i=0;

	if (argv[i][0]=='-')
	{
		switch(argv[i][1])
		{
			case 'a':
			case 'A':if (stricmp(argv[i],"-asmdump")==0) {
					parameter->asmdump=1;
				} else {
					Usage(argv);
				}
				break;
			case 'C':
			case 'c':
				if (stricmp(argv[i],"-cpccolor")==0 || stricmp(argv[i],"-cpccolorz")==0) {
					parameter->cpccolorz=1;
				} else if (stricmp(argv[i],"-c")==0) {
					parameter->maxextract=atoi(argv[++i]);
				}
				break;
			case 'W':
			case 'w':
				parameter->width=atoi(argv[++i]);
				break;
			case 'e':
			case 'E':if (stricmp(argv[i],"-expal")==0) {
					parameter->exportpalettefilename=argv[++i];
				} else if (stricmp(argv[i],"-exbinpal")==0) {
					parameter->exportpaletteBINfilename=argv[++i];
				} else if (stricmp(argv[i],"-exnfo")==0) {
					parameter->sheetfilename=argv[++i];
				} else {
					Usage(argv);
				}
				break;
			case 'i':
			case 'I':if (stricmp(argv[i],"-impal")==0) {
					parameter->importpalettefilename=argv[++i];
				} else {
					Usage(argv);
				}
				break;
			case 'F':
			case 'f':if (stricmp(argv[i],"-flat")==0) {
					parameter->split=0;
				} else if (stricmp(argv[i],"-force")==0) {
					parameter->forceextraction=1;
				} else {
					Usage(argv);
				}
				break;
			case 'g':
			case 'G':
				parameter->grad=1;
				break;
			case 'h':
			case 'H':if (stricmp(argv[i],"-heightmap")==0) {
					 if (i+1<argc) {
						parameter->heightmapfilename=argv[++i];
						parameter->heightmap=1;
						parameter->single=1; // force single output!
					 } else {
						 Usage(argv);
					 }
				 } else if (stricmp(argv[i],"-hsp")==0) {
					parameter->hsp=1;
				 } else {
					Usage(argv);
				 }
				 break;
			case 'k':
			case 'K':
				parameter->keep_empty=1;
				break;
			case 'n':
			case 'N':if (stricmp(argv[i],"-noalpha")==0) {
					parameter->noalpha=1;
				 }
				 break;
			case 'm':
			case 'M':if (stricmp(argv[i],"-mask")==0) {
					parameter->mask=1;
				} else if (stricmp(argv[i],"-metatiles")==0) {
					if (i+1<argc) {
						i++;
						parameter->metatiles=atoi(argv[i]); // squared metatiles
						if (parameter->metatiles<1 || parameter->metatiles>16) {
							printf("Invalid metatiles value (2-16)\n");
							Usage(argv);
						}
					} else {
						Usage(argv);
					}
				} else if (stricmp(argv[i],"-meta")==0) {
					i++;
					if ((scissor=strchr(argv[i],'x'))!=NULL) {
						parameter->metax=atoi(argv[i]);
						parameter->metay=atoi(scissor+1);
						if (parameter->metax>0 && parameter->metay>0) {
						} else {
							Usage(argv);
						}
					} else {
						Usage(argv);
					}
				} else {
					if (!argv[i][2]) {
						parameter->mode=atoi(argv[++i]);
					} else {
						Usage(argv);
					}
				}
				break;
			case 'o':
			case 'O': if (!argv[i][2]) {
					parameter->outputfilename=argv[++i];
				} else if (stricmp(argv[i],"-offset")==0) {
					i++;
		printf("parsing offset [%s]\n",argv[i]);
					if ((scissor=stristr(argv[i],","))!=NULL) {
						*scissor=0;
						parameter->ox=atoi(argv[i]);
						parameter->oy=atoi(scissor+1);
						if (parameter->ox>=0 && parameter->oy>=0) {
						} else {
							Usage(argv);
						}
					} else {
						Usage(argv);
					}
				} else if (stricmp(argv[i],"-old")==0) {
					parameter->oldmode=1;
				} else {
					Usage(argv);
				}
				break;
			case 'p':
			case 'P':
				if (!argv[i][2]) {
					if (i+1<argc) {
						parameter->packed=atoi(argv[++i]);
						switch (parameter->packed) {
							case 2:
							case 4:break;
							default:Usage(argv);
						}
					} else Usage(argv);
				} else if (strcmp(argv[i],"-packAlpha")==0) {
					parameter->packAlpha=1;
				} else {
					Usage(argv);
				}
				break;
			case 'r':
			case 'R':if (stricmp(argv[i],"-rotoffset")==0) {
					 if (i+1<argc) {
						parameter->rotoffsetfilename=argv[++i];
						parameter->rotoffset=1;
						parameter->single=1; // force single output!
					 } else {
						 Usage(argv);
					 }
				} else if (stricmp(argv[i],"-rotoheight")==0) {
					 if (i+1<argc) {
						parameter->rotoheightfilename=argv[++i];
						parameter->rotoheight=1;
						parameter->single=1; // force single output!
					 } else {
						 Usage(argv);
					 }
				} else if (stricmp(argv[i],"-reverse")==0) {
					parameter->reverse=1;
				} else if (stricmp(argv[i],"-rastaline")==0) {
					parameter->rastaline=1;
				} else Usage(argv);
				 break;
			case 's':
			case 'S':if (stricmp(argv[i],"-split")==0) {
					i++;
					if (strcmp(argv[i],"auto")==0 || strcmp(argv[i],"AUTO")==0) {
						parameter->split=parameter->sx*parameter->sy;
						switch (parameter->mode) {
							case 2:parameter->split/=8;break;
							case 1:parameter->split/=4;break;
							case 0:parameter->split/=2;break;
							default:Usage(argv);
						}
					} else {
						parameter->split=atoi(argv[i]);
						if (toupper(argv[i][strlen(argv[i])-1])=='K') parameter->split*=1024;
					}
				} else if (stricmp(argv[i],"-splitraster")==0) {
					parameter->splitraster=1;
				} else if (stricmp(argv[i],"-st")==0) {
					parameter->search_transparency=1;
				} else if (stricmp(argv[i],"-single")==0) {
					parameter->single=1;
				} else if (stricmp(argv[i],"-fontscan")==0) {
					parameter->fontscan=1;
				} else if (stricmp(argv[i],"-scan")==0) {
					parameter->scan=1;
				} else if (stricmp(argv[i],"-splitLowHigh")==0) {
					parameter->splitLowHigh=1;
				} else if (stricmp(argv[i],"-scrz")==0) {
					i++;
					if ((scissor=strchr(argv[i],'x'))!=NULL) {
						parameter->scrx=atoi(argv[i]);
						parameter->scry=atoi(scissor+1);
						if (parameter->scrx>0 && parameter->scry>0) {
						} else {
							Usage(argv);
						}
					} else {
						Usage(argv);
					}
				} else if (stricmp(argv[i],"-size")==0) {
					i++;
					if ((scissor=strchr(argv[i],'x'))!=NULL) {
						parameter->sx=atoi(argv[i]);
						parameter->sy=atoi(scissor+1);
						if (parameter->sx>0 && parameter->sy>0) {
						} else {
							Usage(argv);
						}
					} else {
						Usage(argv);
					}
				} else if (stricmp(argv[i],"-scr")==0) {
					parameter->scrmode=1;
				} else {
					Usage(argv);
				}
				break;
			case 't':
			case 'T':
				if (stricmp(argv[i],"-tilexclude")==0) {
					if (i+1<argc) {
						parameter->tilexclude=atoi(argv[++i]);
					} else {
						Usage(argv);
					}
				} else if (stricmp(argv[i],"-tiles")==0) {
					parameter->tiles=1;
				}
				break;
			case 'L':
			case 'l':if (stricmp(argv[i],"-lara")==0) {
					parameter->lara=1;
				} else switch (argv[i][2]) {
					case 'B':
					case 'b':
						parameter->lineperblock=atoi(argv[++i]);
						break;
					case 'S':
					case 's':
						parameter->nblinescreen=atoi(argv[++i]);
						break;
					default:Usage(argv);
				}
				break;
			case 'B':
			case 'b':
				parameter->black=1;
				break;
			default:
				Usage(argv);		
		}
	} else {
		if (!parameter->filename) {
			parameter->filename=argv[i];
		} else {
			Usage(argv);
		}
	}
	return i;
}

/*
	GetParametersFromCommandLine	
	retrieve parameters from command line and fill pointers to file names
*/
void GetParametersFromCommandLine(int argc, char **argv, struct s_parameter *parameter)
{
	#undef FUNC
	#define FUNC "GetParametersFromCommandLine"
	int i;
	
	for (i=1;i<argc;i++)
		i+=ParseOptions(&argv[i],argc-i,parameter);

	if (!parameter->filename) {
		Usage(NULL);
	}

	if (parameter->hsp && (!parameter->sx || !parameter->sy)) {
		printf("using default size for hardware sprites -> 16x16\n");
		parameter->sx=16;
		parameter->sy=16;
	}

	if (parameter->packAlpha && parameter->hsp) {
		printf("packAlpha cannot be used with hardware sprite extraction (non-sense)\n");
		exit(2);
	}

printf("param OK file=[%s] split=%d scr=%d\nsplitraster=%d hsp=%d max=%d black is transparency=%d",parameter->filename,parameter->split,parameter->scrmode,parameter->splitraster,parameter->hsp,parameter->maxextract,parameter->black);
if (parameter->metax>1 || parameter->metay>1) printf(" meta=%dx%d",parameter->metax,parameter->metay);
printf("\n");
}

/*
	main
	
	check parameters
	execute the main processing
*/
void main(int argc, char **argv)
{
	#undef FUNC
	#define FUNC "main"

	struct s_parameter parameter={0};
	/* default */
	parameter.tilexclude=-1;
	parameter.lineperblock=8;
	parameter.maxextract=1;
	parameter.split=16384;
	parameter.metax=1;
	parameter.metay=1;

	printf(KVERBOSE"%.*s.exe v2.0 / Edouard BERGE 2016-2022",(int)(sizeof(__FILENAME__)-3),__FILENAME__);
	printf(" / powered by zlib & libpng\n"KNORMAL);

	GetParametersFromCommandLine(argc,argv,&parameter);

	parameter.metax*=16;
	parameter.metay*=16;

	Build(&parameter);
#ifndef PROD
	CloseLibrary();
#endif
	exit(0);
}



