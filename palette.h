#ifndef PALETTE_H
#define PALETTE_H

void color_error(int r, int v, int b);
int GetIDXFromPixel(unsigned char *palette, unsigned char *pixel);
int GetIDXFromPalette(unsigned char *palette, int r, int v, int b);
int GetGAFromRGB(int r, int v, int b);
void PushGA(int *GApal, int r, int v, int b);
int GetBASICFromRGB(int r, int v, int b, int zeline);
char *GetStringFromGA(int ga);
void GetRGBFromGA(int ga, unsigned char *r, unsigned char *v, unsigned char *b, int line);
void GetRGBFromBASIC(int ba, unsigned char *r, unsigned char *v, unsigned char *b);
void SortPalette(unsigned char *palette, int maxcoul);
void DisplayPalette(int hsp, int max, unsigned char *palette);

#endif
