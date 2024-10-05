/***********************************************************
         extracts from library_v046 & libgfx_v013
************************************************************/

#define __FILENAME__ "minilib.h"
#define LIBGFX_LINK " -lpng16"

#include"png.h"

#ifndef Z_DEFAULT_COMPRESSION
#define Z_DEFAULT_COMPRESSION 7
#endif


char *stristr (char *ch1, char *ch2)
{
  char  *chN1, *chN2;
  char  *chNdx;
  char  *chRet        = NULL;
 
  chN1 = strdup (ch1);
  chN2 = strdup (ch2);
  if (chN1 && chN2)
  {
    chNdx = chN1;
    while (*chNdx)
    {
      *chNdx = (char) tolower (*chNdx);
      chNdx ++;
    }
    chNdx = chN2;
    while (*chNdx)
    {
      *chNdx = (char) tolower (*chNdx);
      chNdx ++;
    }
    chNdx = strstr (chN1, chN2);
    if (chNdx)
      chRet = ch1 + (chNdx - chN1);
  }
  free (chN1);
  free (chN2);
  return chRet;
}


/***
        MinMaxInt
*/
int MinMaxInt(int zeval, int zemin, int zemax)
{
        #undef FUNC
        #define FUNC "MinMaxInt"

        if (zeval<zemin) return zemin;
        if (zeval>zemax) return zemax;
        return zeval;
}

/***
	ImageRGBToCRB
*/
unsigned char * ImageRGBToCRB(unsigned char *pixels, int width, int height)
{
        #undef FUNC
        #define FUNC "ImageRGBToCRB"

        unsigned char *newpix;;
        int i;
        double r,g,b;

        newpix=MemMalloc(width*height*3);

        for (i=0;i<width*height*3;i+=3) {
                r=pixels[i+0];
                g=pixels[i+1];
                b=pixels[i+2];
                newpix[i+0]=MinMaxInt(0.257*r+0.504*g+0.098*b+16,0,255);
                newpix[i+1]=MinMaxInt(-0.148*r-0.291*g+0.439*b+128,0,255);
                newpix[i+2]=MinMaxInt(0.439*r-0.368*g-0.071*b+128,0,255);
        }
        return newpix;
}

unsigned char *ImageRRGGBBToRRGGBBAA(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageRRGGBBToRRGGBBAA"

	unsigned char *newdata;
	int i,ptr=0;

	if (!data) return NULL;
	if (sizex<=0 || sizey<=0) return NULL;

	newdata=MemMalloc(sizex*sizey*8);

	for (i=0;i<sizex*sizey*6;i+=6) {
		newdata[ptr++]=data[i];
		newdata[ptr++]=data[i+1];
		newdata[ptr++]=data[i+2];
		newdata[ptr++]=data[i+3];
		newdata[ptr++]=data[i+4];
		newdata[ptr++]=data[i+5];
		newdata[ptr++]=0;
		newdata[ptr++]=0;
	}
	return newdata;
}
unsigned char *ImageFreeRRGGBBToRRGGBBAA(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageFreeRRGGBBToRRGGBBAA"

	unsigned char *newdata;

	if (!data) return NULL;

	newdata=ImageRRGGBBToRRGGBBAA(data,sizex,sizey);
	MemFree(data);
	return newdata;
}

unsigned char *ImageRGBToRGBA(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageRGBToRGBA"

	unsigned char *newdata;
	int i,ptr=0;

	if (!data) return NULL;
	if (sizex<=0 || sizey<=0) return NULL;

	newdata=MemMalloc(sizex*sizey*4);

	for (i=0;i<sizex*sizey*3;i+=3) {
		newdata[ptr++]=data[i];
		newdata[ptr++]=data[i+1];
		newdata[ptr++]=data[i+2];
		newdata[ptr++]=255;
	}
	return newdata;
}
unsigned char *ImageFreeRGBToRGBA(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageRGBToRGBA"

	unsigned char *newdata;

	if (!data) return NULL;

	newdata=ImageRGBToRGBA(data,sizex,sizey);
	MemFree(data);
	return newdata;
}

unsigned char *ImageRGBAToRGB(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageRGBAToRGB"

	unsigned char *newdata;
	int i,ptr=0;

	if (!data) return NULL;
	if (sizex<=0 || sizey<=0) return NULL;

	newdata=MemMalloc(sizex*sizey*3);

	for (i=0;i<sizex*sizey*4;i+=4) {
		newdata[ptr++]=data[i];
		newdata[ptr++]=data[i+1];
		newdata[ptr++]=data[i+2];
	}
	return newdata;
}
unsigned char *ImageFreeRGBAToRGB(unsigned char *data, int sizex,int sizey)
{
	#undef FUNC
	#define FUNC "ImageRGBAToRGB"

	unsigned char *newdata;

	if (!data) return NULL;

	newdata=ImageRGBAToRGB(data,sizex,sizey);
	MemFree(data);
	return newdata;
}


unsigned char * ImageCRBToRGB(unsigned char *crb, int width, int height)
{
        #undef FUNC
        #define FUNC "ImageCRBToRGB"

        unsigned char *newpix;;
        double y;
        int i,u,v;

        newpix=MemMalloc(width*height*3);

        for (i=0;i<width*height*3;i+=3) {
                /*
                        R = Y + 1,13983⋅V
                        G = Y − 0,39465⋅U − 0,58060⋅V
                        B = Y + 2,03211⋅U
                */
                y=(crb[i+0]-16)*1.164;
                u=crb[i+1]-128;
                v=crb[i+2]-128;
                newpix[i+0]=MinMaxInt(y+1.596*v,0,255);
                newpix[i+1]=MinMaxInt(y-0.813*v-0.391*u,0,255);
                newpix[i+2]=MinMaxInt(y+2.018*u,0,255);

        }
        return newpix;
}

unsigned char *ImageRGBToBW(unsigned char *pixels, int width, int height)
{
	#undef FUNC
	#define FUNC "ImageRGBToBW"

	unsigned char *newpix;
	int i;

	newpix=MemMalloc(width*height*3);

	for (i=0;i<width*height;i++) {
		newpix[i*3+0]=newpix[i*3+1]=newpix[i*3+2]=(pixels[i*3+0]*76+pixels[i*3+1]*153+pixels[i*3+2]*25)/256;
	}
	return newpix;
}

#define PNG_HEADER_SIZE 8

struct s_png_info {
	int width;
	int height;
	int bit_depth;
	int color_type;
	int filter_method;
	int compression_type;
	int interlace_type;

	unsigned char *data;
};

void PNGFree(struct s_png_info **png)
{
	#undef FUNC
	#define FUNC "FreePNG"

	if (!png[0]) return;
	if (png[0]->data) MemFree(png[0]->data);
	MemFree(png[0]);
	*png=NULL;
}

/***
	PNGInit

	Allocate a new PNG structure
	If a reference PNG structure is given, then copy info to the new struct
*/
struct s_png_info * PNGInit(struct s_png_info *photo)
{
	#undef FUNC
	#define FUNC "PNGInit"

	struct s_png_info *new_photo;

	new_photo=MemMalloc(sizeof(struct s_png_info));
	memset(new_photo,0,sizeof(struct s_png_info));
	if (photo) {
		*new_photo=*photo;
		new_photo->data=NULL;
	}
	return new_photo;
}
struct s_png_info * PNGInitTrueColor()
{
	#undef FUNC
	#define FUNC "PNGInitTrueColor"

	struct s_png_info pngproto={0};
	pngproto.bit_depth=8;
	pngproto.color_type=2;
	pngproto.compression_type=0;
	return PNGInit(&pngproto);
}
struct s_png_info * _internal_PNGRead(char *filename, int want8, int wantALPHA)
{
	#undef FUNC
	#define FUNC "PNGRead"

	FILE *fp;
	int i,pixel_size,j;
	unsigned char **libpng_data;
	png_structp png_ptr;
	png_infop   info_ptr;
	char png_header[PNG_HEADER_SIZE];
	struct s_png_info *png_info;
	/* alpha test */
	png_bytep trans_alpha = NULL;
	int num_trans = 0;
	png_color_16p trans_color = NULL;
	int expand16=0;
	int packedpixel;

printf("PNG opening [%s]\n",filename);
	/* we must read 8 byte of the supposed png file for libpng, cause they can't open png file (crappy coders make crappy code) */
	FileReadBinary(filename,png_header,PNG_HEADER_SIZE);
	FileReadBinaryClose(filename);

	/* legacy libpng code */
	if (png_sig_cmp(png_header,0,PNG_HEADER_SIZE)) {
		logerr("This file is not a PNG picture");
		exit(INTERNAL_ERROR);
	}
	if ((png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL))==NULL) {
		png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL);
		logerr("PngLib error: cannot create read struct");
		exit(INTERNAL_ERROR);
	}
	if ((info_ptr=png_create_info_struct(png_ptr))==NULL) {
		png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
		logerr("PngLib error: cannot create info struct");
		exit(INTERNAL_ERROR);
	}
	
	/* libpng cannot open png file by itself, so we do (crappy coders...) */
	fp=fopen(filename,"rb");
	png_init_io(png_ptr,fp);
	png_read_info(png_ptr,info_ptr);
	png_info=MemMalloc(sizeof(struct s_png_info));
	png_info->width=png_get_image_width(png_ptr,info_ptr);
	png_info->height=png_get_image_height(png_ptr,info_ptr);
	png_info->bit_depth=png_get_bit_depth(png_ptr,info_ptr);
	png_info->color_type=png_get_color_type(png_ptr,info_ptr);
	switch (png_info->color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
			if (trans_alpha != NULL) {
				png_info->color_type=PNG_COLOR_TYPE_RGBA;
				//loginfo("PNG palette -> RGBA");
				pixel_size=4;
			} else {
				png_info->color_type=PNG_COLOR_TYPE_RGB;
				//loginfo("PNG palette -> RGB");
				pixel_size=3;
			}
			png_set_palette_to_rgb(png_ptr);
			break;
		case PNG_COLOR_TYPE_GRAY:
				pixel_size=3;
				png_set_gray_to_rgb(png_ptr);
				break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:png_set_gray_to_rgb(png_ptr);
				pixel_size=4;
				break;
		case PNG_COLOR_TYPE_RGB:
				pixel_size=3;
				break;
		case PNG_COLOR_TYPE_RGBA:
				pixel_size=4;
				break;
		default:
			logerr("unsupported PNG format -> color_type=%d",png_info->color_type);
			break;
	}
	//loginfo("Loading PNG [%dx%d] bt=%d  %s",png_info->width,png_info->height,png_info->bit_depth,filename);

	//png_set_invert_alpha(png_ptr); /* alpha is transparency 0:opaque 255/65535:fully transparent */

	switch (wantALPHA) {
		case 0:
			/* desactive le canal ALPHA si on n'en veut pas */
			if (png_info->color_type & PNG_COLOR_MASK_ALPHA) {
				png_set_strip_alpha(png_ptr);
				//loginfo("PNG RGBA -> RGB");
				pixel_size=3;
			}
			break;
		default:
			/* si la transparence n'est pas ALPHA on en fait un masque ALPHA */
			if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)) {
				png_set_tRNS_to_alpha(png_ptr);
				//loginfo("PNG make Alpha channel");
				pixel_size=4;
			}
	}

	if (png_info->bit_depth==16) {
		if (want8) {
			//loginfo("PNG 16 bits -> 8 bits");
			png_set_strip_16(png_ptr);
			png_info->bit_depth=8;
		}
		else {
			png_set_swap(png_ptr); /* little-endian requested */
			pixel_size*=2;
		}
	} else if (png_info->bit_depth==8) {
		if (!want8) {
			pixel_size*=2;
			expand16=1;
		}
	} else if (png_info->bit_depth<8) {
		loginfo("png_set_packing\n");
		png_set_packing(png_ptr);
	}

	/* libpng does not handle linear memory... */
	libpng_data=MemMalloc(sizeof(char*)*png_info->height);
	for (i=0;i<png_info->height;i++) {
		libpng_data[i]=MemMalloc(pixel_size*png_info->width);
	}
	png_read_image(png_ptr,(png_bytepp)libpng_data);
printf("PNG read OK\n");
	switch (png_info->color_type) {
		case PNG_COLOR_TYPE_PALETTE:loginfo("RGB ou RGBA du coup?");break;
		default:break;
	}

	/* copy each rows to linear memory */
	png_info->data=MemMalloc(pixel_size*png_info->width*png_info->height);
	for (i=0;i<png_info->height;i++) {
		memcpy(png_info->data+i*pixel_size*png_info->width,libpng_data[i],pixel_size*png_info->width);
	}
	/* free crappy png row memory */
	for (i=0;i<png_info->height;i++) {
		MemFree(libpng_data[i]);
	}
	MemFree(libpng_data);

	/* cause libpng cannot close file by itself... */
	fclose(fp);
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);

	switch (wantALPHA) {
		case 0:png_info->color_type=PNG_COLOR_TYPE_RGB;break;
		case 1:
			if (png_info->color_type!=PNG_COLOR_TYPE_RGBA) {
				if (png_info->bit_depth==8 && png_info->color_type==PNG_COLOR_TYPE_RGB) {
					loginfo("PNG make empty Alpha channel");
					png_info->color_type=PNG_COLOR_TYPE_RGBA;
					png_info->data=ImageFreeRGBToRGBA(png_info->data,png_info->width,png_info->height);
				} else if (png_info->bit_depth==16 && png_info->color_type==PNG_COLOR_TYPE_RGB) {
					loginfo("Make empty 16bits Alpha channel");
					png_info->color_type=PNG_COLOR_TYPE_RGBA;
					png_info->data=ImageFreeRRGGBBToRRGGBBAA(png_info->data,png_info->width,png_info->height);
				} else {
					logerr("INTERNAL ERROR - cannot process PNG BD:%d CT:%d",png_info->bit_depth,png_info->color_type);
					exit(INTERNAL_ERROR);
				}
			}
		default:break;
	}
	if (expand16) {
		png_info->bit_depth=16;
		if (PNG_COLOR_TYPE_RGB) {
			packedpixel=6;
		} else {
			packedpixel=8;
		}
		j=png_info->height*png_info->width*packedpixel-packedpixel;
		i=png_info->height*png_info->width*packedpixel/2-packedpixel/2;
		loginfo("Expand 8bits PNG %sto 16bits",packedpixel==8?"(+Apha) ":"");
		while (i>=0) {
			png_info->data[j+0]=0;
			png_info->data[j+1]=png_info->data[i+0];
			png_info->data[j+2]=0;
			png_info->data[j+3]=png_info->data[i+1];
			png_info->data[j+4]=0;
			png_info->data[j+5]=png_info->data[i+2];
			if (packedpixel==8) {
				png_info->data[j+6]=0;
				png_info->data[j+7]=png_info->data[i+3];
				i-=4;
				j-=8;
			} else {
				i-=3;
				j-=6;
			}
		}
	}

	return png_info;
}

#define PNGRead(filename) _internal_PNGRead(filename,-1,-1)
#define PNGRead64(filename) _internal_PNGRead(filename,0,1)
#define PNGRead48(filename) _internal_PNGRead(filename,0,0)
#define PNGRead32(filename) _internal_PNGRead(filename,1,1)
#define PNGRead24(filename) _internal_PNGRead(filename,1,0)

void PNGWriteCallBack(png_structp png_ptr, png_uint_32 row, int pass)
{
	#undef FUNC
	#define FUNC "PNGWriteCallBack"
	printf("row: %d (%d)\r",(int)row,pass);
}

void PNGWrite(struct s_png_info *photo, char *filename)
{
	#undef FUNC
	#define FUNC "PNGWrite"

	int i;
	FILE *fp;
	unsigned char **libpng_data;
	png_structp png_ptr;
	png_infop   info_ptr;
	png_text    text_ptr={0};

	if ((png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL))==NULL) {
		logerr("PngLib error: cannot create write struct");
		exit(INTERNAL_ERROR);
	}
	if ((info_ptr=png_create_info_struct(png_ptr))==NULL) {
		png_destroy_write_struct(&png_ptr,NULL);
		logerr("PngLib error: cannot create info struct");
		exit(INTERNAL_ERROR);
	}
	fp=fopen(filename,"wb");
	if (!fp) {
		logerr("Cannot create [%s] for writing",filename);
		exit(INTERNAL_ERROR);
	}
	png_init_io(png_ptr,fp);
	png_set_write_status_fn(png_ptr,PNGWriteCallBack);
	png_set_compression_level(png_ptr,Z_DEFAULT_COMPRESSION); /* best compromise between speed and compression ratio */
	png_set_IHDR(png_ptr,info_ptr,photo->width,photo->height,photo->bit_depth,photo->color_type,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
	
	/*
	text_ptr.compression=PNG_TEXT_COMPRESSION_NONE;
	strcpy(text_ptr.key,"Copyright");
	text_ptr.text=TxtStrDup("image created with %s using LibPNG dot org",LIBRARY_LABEL);
	strcpy(text_ptr.lang,"EN");
	png_set_text(png_ptr,info_ptr,text_ptr,1);
	MemFree(text_ptr.text);
	*/

	/* libpng does not handle linear memory... */
	libpng_data=MemMalloc(sizeof(char*)*photo->height);
       /* copy each rows from linear memory */
        switch (photo->color_type) {
                default:
                case PNG_COLOR_TYPE_RGB:
			for (i=0;i<photo->height;i++) {
				libpng_data[i]=MemMalloc(3*photo->width);
			}
                        for (i=0;i<photo->height;i++) {
                                memcpy(libpng_data[i],photo->data+i*3*photo->width,3*photo->width);
                        }
                        break;
                case PNG_COLOR_TYPE_RGBA:
			for (i=0;i<photo->height;i++) {
				libpng_data[i]=MemMalloc(4*photo->width);
			}
                        for (i=0;i<photo->height;i++) {
                                memcpy(libpng_data[i],photo->data+i*4*photo->width,4*photo->width);
                        }
                        break;
        }

	/* then assign them to png struct */
	png_set_rows(png_ptr,info_ptr,libpng_data);
	png_write_png(png_ptr,info_ptr,PNG_TRANSFORM_IDENTITY,NULL);

	/* free crappy png row memory */
	for (i=0;i<photo->height;i++) {
		MemFree(libpng_data[i]);
	}
	MemFree(libpng_data);

        /* cause libpng cannot close file by itself... */
        fclose(fp);
        png_destroy_write_struct(&png_ptr,&info_ptr);

	loginfo("Writing PNG [%dx%d] %s",photo->width,photo->height,filename);
}

#undef __FILENAME__

/************************************
	end of mini-library
********************************EOF*/
