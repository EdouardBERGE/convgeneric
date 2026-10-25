#undef __FILENAME__
#define __FILENAME__ "screen_sprite.c"

int processScreenAndSprite(
    struct s_parameter *parameter,
    struct s_png_info *photo,
    unsigned char *palette,
    unsigned char *cpcdata,
    unsigned char *transdata,
    struct s_sprite_info **spinfo_ptr,
    int *ispi_ptr,
    int *mspi_ptr,
    int *idata_ptr,
    int *maxdata_ptr,
    int *itrans_ptr,
    int *scradr,
    int *curline_ptr,
    int *maxscradr_ptr)
{
    struct s_sprite_info *spinfo = *spinfo_ptr;
    int ispi = *ispi_ptr;
    int mspi = *mspi_ptr;
    int idata = *idata_ptr;
    int maxdata = *maxdata_ptr;
    int itrans = *itrans_ptr;
    int curline = *curline_ptr;
    int maxscradr = *maxscradr_ptr;

    int i,j,xs,ys,zepix=0,pix1=0,pix2=0,pix3=0,pix4=0,adr=0,istep=0,tilewidth=0,tileheight=0,itile=0,tiletrans=0;
    int colorz_in_a_row[32]={0};
    int colorz_prev_row[32]={0};
    struct s_sprite_info curspi={0};
    int *tileidx=NULL;
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
								return 0;
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

							// last pixel test
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

    *spinfo_ptr = spinfo;
    *ispi_ptr = ispi;
    *mspi_ptr = mspi;
    *idata_ptr = idata;
    *maxdata_ptr = maxdata;
    *itrans_ptr = itrans;
    *curline_ptr = curline;
    *maxscradr_ptr = maxscradr;

    return 1;
}
