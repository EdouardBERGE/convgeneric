#define PROD 1

#ifdef PROD
#include "rasmlib.h"
#include "minilib.h"
#include "palette.c"
#else
#include "../tools/library.h"
#include "../tools/libgfx.h"
#endif

#undef __FILENAME__
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

#ifdef OS_WIN
#include<errno.h>
#include<io.h>
#include<ctype.h>
#include<direct.h>
#include<windows.h>
#include<setjmp.h>
#include<intrin.h>
#include<sys/timeb.h>

#include"winpthread.h"
#endif

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
	char *scriptfilename;
	char *outputfilename;
	char *sheetfilename;
	/* lara option */
	int lara;
	/* general options */
	int mode;
	int split;
	int grad;
	int asmdump;
	int tiles,metatiles;
	int cpccolorz;
	int noalpha;
	int getcolor;
	char *exportpalettefilename;
	char *exportpaletteBINfilename;
	char *importpalettefilename;
	/* screen options */
	int screenFlag,hspFlag;
	int single,splitLowHigh,tilexclude;
	int scrmode,oldmode;
	int scrx,scry;
	int sx,sy;
	int width,height;
	int ox,oy;
	int mask;
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
int col[27];
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


//#define CHECK_IDATA {if ((idata>photo->width*photo->height && !parameter->scrmode) || idata>=32768) {printf("internal error: too much data idata=%d width=%d height=%d\n",idata,photo->width,photo->height);exit(-1);}}
#define CHECK_IDATA ;

#include "hardware_sprite.c"
#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"
#include "split_raster.c"
#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"
#include "screen_sprite.c"
#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"
#include "palette_processing.c"
#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"
#define FREEMEM_AND_EXIT(code) do { \
    if (photo) { PNGFree(&photo); photo=NULL; } \
    if (cpcdata) { MemFree(cpcdata); cpcdata=NULL; } \
    if (tileidx) { MemFree(tileidx); tileidx=NULL; } \
    if (spinfo && mspi) { MemFree(spinfo); spinfo=NULL; } \
    if (scradr) { MemFree(scradr); scradr=NULL; } \
    if (transdata) { MemFree(transdata); transdata=NULL; } \
    return (code); \
} while (0)

int Build(struct s_parameter *parameter)
{
	#undef FUNC
	#define FUNC "Build"

	/************************************************
                c o n v e r t e r    l e g a c y
        ************************************************/
	char newname[2048];

	struct s_png_info *photo=NULL;

	int j,maxcoul;



	int xs,ys;

	/****************************************
                c o n v e r t e r    v 2
        ****************************************/
	unsigned char *cpcdata=NULL;        /* dynamic workspace */
	int *tileidx=NULL;
	int tilewidth=0,tileheight=0,tiletrans=0,tilexclude=0;
	int scrtilex,scrtiley;
	int itile=0;
	int idata=0;                        /* current output */
	int limitcolors=16;                 /* max number of colors regarding the options */
	int hasblack,is_hsp=0;                       /* index of black color if any, else -1 */
	int hsptr=-1,hsptv=-1,hsptb=-1;              /* transparency color */
	struct s_sprite_info *spinfo=NULL;  /* info about extracted sprites */
	int ispi=0,mspi=0;                  /* fast counters for sprite info struct */
	int *scradr=NULL,maxscradr=0;             	/* CRTC line adresse decoding */
	int crtcadr,crtcb,curline;          /* CRTC mimic values */
	unsigned char palette[4096*3]={0};  /* palette for 16 colors + border for scanning */
	int i=0;
	int maxdata=0,byteleft;               /* byte count for file output */
	int filenumber=2;                   /* file naming */
	int png_has_transparency=0;         /* png data contains transparent pixels */
	/* split-rasters */
	struct s_rastecran scr={0};
	int clash=0;
	int woffset;
	/***************************************************
	  L A R A   e x p o r t
	***************************************************/
	int decal=0,csx=0,theight=0;
	/***************************************************
	  raster line export
	***************************************************/

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
        if (!photo) FREEMEM_AND_EXIT(0);

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
		FREEMEM_AND_EXIT(1);
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
	if (!processPalette(parameter, photo, palette, limitcolors, hsptr, hsptv, hsptb, &maxcoul, &is_hsp, &png_has_transparency)) FREEMEM_AND_EXIT(0);

	/* il ne devrait pas y avoir plus de pixels que sur l'image d'origine */
	transdata=malloc(photo->width*photo->height*2);
	memset(transdata,0,photo->width*photo->height*2);

	/* options check & management */
	if (!parameter->sx || !parameter->sy) {
		parameter->sx=photo->width;
		parameter->sy=photo->height;
	}
	if (parameter->sx>photo->width) {
		printf(KERROR"ERROR: sprite width cannot exceed image dimension\n");
		FREEMEM_AND_EXIT(0);
	}
	if (parameter->sy>photo->height) {
		printf(KERROR"ERROR: sprite height cannot exceed image dimension\n");
		FREEMEM_AND_EXIT(0);
	}
	if (parameter->ox>=photo->width) {
		printf(KERROR"ERROR: X offset cannot exceed image dimension\n");
		FREEMEM_AND_EXIT(0);
	}
	if (parameter->oy>=photo->height) {
		printf(KERROR"ERROR: Y offset cannot exceed image dimension\n");
		FREEMEM_AND_EXIT(0);
	}
	if (!parameter->nblinescreen) {
		parameter->nblinescreen=photo->height;
	} else if (parameter->nblinescreen>photo->height) {
		printf(KERROR"ERROR: split-screen line cannot exceed image dimension\n");
		FREEMEM_AND_EXIT(0);
	}

	printf(KVERBOSE"Extraction: ");
	if (parameter->scrmode) printf("screen");
	else if (parameter->hsp) printf("hardware sprite");
	else printf("sprite");
	if (parameter->oldmode) printf("overscan bit");

	if (!parameter->hsp) {
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
					FREEMEM_AND_EXIT(0);
				}
			}
		}
		if (cpt1+cpt2!=photo->width) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> count marks\n");
			FREEMEM_AND_EXIT(0);
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
				FREEMEM_AND_EXIT(0);
			}
			color2=*(int *)(&photo->data[(photo->height-1)*photo->width*4]+i*4);
		}
		printf("decal=%d -> %d byte ",decal,decal>>1);
		if (decal&1) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> odd shift value\n");
			FREEMEM_AND_EXIT(0);
		}
		decal>>=1;
		csx=photo->width/cpt1;
		if (photo->width % cpt1) {
			printf(KERROR"INTERNAL ERROR -> lara mode -> cannot compute atomic width %d %% %d = %d\n",photo->width,cpt1,photo->width % cpt1);
			FREEMEM_AND_EXIT(0);
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
			printf(KERROR"ERROR: hardware sprite size cannot exceed 16x16!\n");FREEMEM_AND_EXIT(0);
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
				if (palette[0+14*3]==palette[0+15*3])
				if (palette[1+14*3]==palette[1+15*3])
				if (palette[2+14*3]==palette[2+15*3]) {
					maxcoul--;
				} else {
					printf(KERROR"ERROR: 15 colors maximum for hardware sprites\n"KNORMAL);
					DisplayPalette(0, maxcoul, palette);
					FREEMEM_AND_EXIT(0);
				}
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
	

	// Palette output and export
	if (!parameter->splitraster) {
		if (parameter->black) {
			if (palette[0]!=0 || palette[1]!=0 || palette[2]!=0) {
				printf(KERROR"FATAL: there is an error with first color which is not black as expected\n"KNORMAL);
				FREEMEM_AND_EXIT(0);
			}
		}


		//if (parameter->importpalettefilename) {

		printf("paletteplus: defw ");
		switch (parameter->mode) {
			default:
			case 0: if (maxcoul>16) maxcoul=16;break;
			case 1: if (maxcoul>4) maxcoul=4;break;
			case 2: if (maxcoul>2) maxcoul=2;break;
		}


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
			if (parameter->importpalettefilename && parameter->hsp) {
				for (i=0;i<maxcoul;i++) {
					sprintf(exporttmpcolor,"%s#%03X",i>0?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
					FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
				}
			} else {
				for (i=parameter->hsp;i<maxcoul+parameter->hsp;i++) {
					sprintf(exporttmpcolor,"%s#%03X",i>parameter->hsp?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
					FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
				}
			}
			// si on est en export HSP il faut conserver le décalage de la palette
			if (parameter->hsp) {
				strcpy(exporttmpcolor," ; HSP");
			} else {
				exporttmpcolor[0]='\0';
			}
			FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			strcpy(exporttmpcolor,"\n");
			FileWriteBinary(parameter->exportpalettefilename,exporttmpcolor,strlen(exporttmpcolor));
			FileWriteBinaryClose(parameter->exportpalettefilename);
		}

		// exit right now if we only want to get colors in a file (or stdout)
		if (parameter->getcolor) {
			FREEMEM_AND_EXIT(1);
		}
	}
	
	cpcdata=MemMalloc(32768+photo->width*photo->height+32);
	memset(cpcdata,0,32768+photo->width*photo->height+32);

	idata=0;
	curline=0;

	/*************************************************
	         g f x    p r o c e s s i n g    
	*************************************************/
	if (parameter->hsp) {
		if (!processHardwareSprite(parameter, photo, palette, cpcdata, transdata, &spinfo, &ispi, &mspi, &idata, &maxdata, hsptr, hsptv, hsptb)) FREEMEM_AND_EXIT(0);
	} else if (parameter->splitraster) {
		if (!processSplitRaster(parameter, photo, palette, &scr, &clash)) FREEMEM_AND_EXIT(0);
	} else {
		if (!processScreenAndSprite(parameter, photo, palette, cpcdata, transdata, &spinfo, &ispi, &mspi, &idata, &maxdata, &itrans, scradr, &curline, &maxscradr)) FREEMEM_AND_EXIT(0);
	}


	if (parameter->rotoheight) {
		int xcenter,ycenter,wmax,iridx=0;
		float ang;
		unsigned char *rotoffsetmap=NULL;
		// contrôle de cohérence sur les données CPC produites en amont? Minimum
		if (parameter->scrmode) {
			printf("cannot process rotomap in screen mode...\n");
			FREEMEM_AND_EXIT(0);
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
		unsigned char *rotoffsetmap=NULL;
		// contrôle de cohérence sur les données CPC produites en amont? Minimum
		if (parameter->scrmode) {
			printf("cannot process rotomap in screen mode...\n");
			FREEMEM_AND_EXIT(0);
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
				strcpy(newname,parameter->outputfilename?parameter->outputfilename:parameter->filename);
				if (parameter->outputfilename) strcat(newname,".tilemap"); else strcpy(newname+strlen(newname)-3,"tilemap");
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
				strcpy(newname,parameter->outputfilename?parameter->outputfilename:parameter->filename);
				if (parameter->outputfilename) strcat(newname,".tilescreen"); else strcpy(newname+strlen(newname)-3,"tilescreen");
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
			int curmt[256] = {0}; // max allowed
			int mtlist=0;
			int imt;

			// metatiles!
			printf("; metatiles theorical maximum combination is %d\n",ispi*parameter->metatiles*parameter->metatiles);

			tlist=malloc(sizeof(int*)*ispi*parameter->metatiles*parameter->metatiles); // maximum combination

			strcpy(newname,parameter->outputfilename?parameter->outputfilename:parameter->filename);
			if (parameter->outputfilename) strcat(newname,".meta"); else strcpy(newname+strlen(newname)-3,"meta");
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
		if (!parameter->asmdump) {
			strcat(newname,".bin");
		} else {
			strcat(newname,".asm");
		}
	} else {
		strcpy(newname,parameter->filename);
		if (!parameter->asmdump) {
			strcpy(newname+strlen(newname)-3,"bin");
		} else {
			strcpy(newname+strlen(newname)-3,"asm");
		}
	}

	//if (parameter->scrmode && maxdata > parameter->width * parameter->nblinescreen) {
	//	maxdata = parameter->width * parameter->nblinescreen;
	//	idata = parameter->width * parameter->nblinescreen;
	//}
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
				int hspodd,hspidx1,hspidx2,hsp1ref,hsp2ref;
				int hspsize,hspmid,hspmax,hspmask;
				byteleft=maxdata>idata?maxdata:idata; // arm again bytecount

				// get back to unit (only metax is relevant)
				parameter->metax>>=4;

				// if we are odd, we will reverse inside last HSP lines
				if (parameter->metax&1) hspodd=parameter->metax>>1; else hspodd=-1;
				// set first even HSP indexes
				hspidx1=0; hspidx2=parameter->metax-1;

				// revert data
				switch (parameter->packed) {
					case 0: hspsize=256;hspmid=8;hspmax=15;hspmask=0xFF;
					       	for (i=0;i<256;i++) revertConv[i]=i;
						break;
					case 2: hspsize=128;hspmid=4;hspmax=7;hspmask=0x7F;
					       	for (i=0;i<256;i++) revertConv[i]=((i&15)<<4)|((i>>4)&15);
						break;
					case 4: hspsize=64;hspmid=2;hspmax=3;hspmask=0x3F;
					       	for (i=0;i<256;i++) revertConv[i]=((i&3)<<6)|((i>>6)&3)|(((i>>4)&3)<<2)|(((i>>2)&3)<<4);
						break;
					default:fprintf(stderr,"abnormal HSP packing error with revert\n");FREEMEM_AND_EXIT(0);
				}

				if ((byteleft/parameter->metax)&hspmask) { fprintf(stderr,"hardware sprite extraction error, could be %dxwidth factor\n",hspmask+1); FREEMEM_AND_EXIT(0); }
				j=0;
				hsp1ref=hspidx1;
				hsp2ref=hspidx2;
				while (j<byteleft) {
					printf("j (offset) = %d\n",j);
					// invert even HSP (if any)
					hspidx1=hsp1ref;
					hspidx2=hsp2ref;
					while (hspidx1<hspidx2) {
						printf("invert %d and %d on the line\n",hspidx1,hspidx2);
						for (i=0;i<hspsize;i+=hspmax+1) {
							for (xs=0;xs<hspmax+1;xs++) {
								byteTMP=revertConv[cpcdata[j+hspidx1*hspsize+i+xs]];
								cpcdata[j+hspidx1*hspsize+i+xs]=revertConv[cpcdata[j+hspidx2*hspsize+i+hspmax-xs]];
								cpcdata[j+hspidx2*hspsize+i+hspmax-xs]=byteTMP;
							}
						}
						hspidx1++;
						hspidx2--;
					}

					// invert odd HSP (if any)
					if (hspodd>=0) {
						printf("invert central hsp on the line\n");
						for (i=0;i<hspsize;i+=hspmax+1) {
							for (xs=0;xs<hspmid;xs++) {
								byteTMP=revertConv[cpcdata[j+hspodd*hspsize+i+xs]];
								cpcdata[j+hspodd*hspsize+i+xs]=revertConv[cpcdata[j+hspodd*hspsize+i+hspmax-xs]];
								cpcdata[j+hspodd*hspsize+i+hspmax-xs]=byteTMP;
							}
						}
					}
					j+=parameter->metax*hspsize; // next bunch of sprite(s)
				}

				// reverse name...
				filenumber=2;
				woffset=0;
				if (parameter->outputfilename) strcat(newname,".rin"); else strcpy(newname+strlen(newname)-3,"rin");
				while (byteleft) {
					FileRemoveIfExists(newname);
					if (byteleft>parameter->split) {
						if (filenumber<5) {
							printf(KIO"writing %d bytes in %s\n"KNORMAL,parameter->split,newname);				
						}
						FileWriteBinary(newname,cpcdata+woffset,parameter->split);
						FileWriteBinaryClose(newname);
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
						byteleft=0;
					}
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
					if (parameter->outputfilename) { strcpy(newname,parameter->outputfilename); strcat(newname,".t01"); } else strcpy(newname+strlen(newname)-3,"t01"); filenumber=2;
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
			if (parameter->outputfilename) strcat(newname,".geo"); else strcpy(newname+strlen(newname)-3,"geo");
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
	PNGFree(&photo); photo=NULL;
	MemFree(cpcdata); cpcdata=NULL;
	if (mspi) {
		MemFree(spinfo); spinfo=NULL; mspi=0;
	}
	if (tileidx) { MemFree(tileidx); tileidx=NULL; }
	if (scradr) { MemFree(scradr); scradr=NULL; }
	if (transdata) { MemFree(transdata); transdata=NULL; }
	FREEMEM_AND_EXIT(1);
}

/***************************************
	semi-generic body of program
***************************************/


#include "commandLine.c"
#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"
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

	if (parameter.scriptfilename) {
		struct s_parameter *script_params=NULL;
		int nscript=0, maxscript=0;
		int counter=0;
		GetParametersFromScript(parameter.scriptfilename,&script_params,&nscript,&maxscript);
		for (int si=0; si<nscript; si++) {
			script_params[si].metax*=16;
			script_params[si].metay*=16;
			counter += Build(&script_params[si]);
		}
		if (counter != nscript) exit(1);
		exit(0);
	}

	parameter.metax*=16;
	parameter.metay*=16;

	{
		int counter=0;
		counter += Build(&parameter);
#ifndef PROD
		CloseLibrary();
#endif
		if (counter != 1) exit(1);
		exit(0);
	}
}



