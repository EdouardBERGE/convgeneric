
#undef __FILENAME__
#define __FILENAME__ "palette.c"

#ifndef KNORMAL
#define KNORMAL  "\x1B[0m"
#define KERROR   "\x1B[31m"
#define KWARNING "\x1B[33m"
#define KBLUE    "\x1B[34m"
#define KVERBOSE "\x1B[36m"
#define KIO      "\x1B[97m"
#endif

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
void DisplayPalette(int hsp, int max, unsigned char *palette)
{
	int i;

	for (i=hsp;i<max+hsp;i++) {
		printf("%s#%04X",i-hsp?",":"",(palette[i*3+0]&0xF0)|((palette[i*3+1]>>4)<<8)|(palette[i*3+2]>>4));
	}
printf("\n");

}

