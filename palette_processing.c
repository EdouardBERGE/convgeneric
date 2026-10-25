int processPalette(
    struct s_parameter *parameter,
    struct s_png_info *photo,
    unsigned char *palette,
    int limitcolors,
    int hsptr,
    int hsptv,
    int hsptb,
    int *maxcoul_ptr,
    int *is_hsp_ptr,
    int *png_has_transparency_ptr)
{
    int maxcoul = *maxcoul_ptr;
    int is_hsp = *is_hsp_ptr;
    int png_has_transparency = *png_has_transparency_ptr;
    int i,j;
    int bgr=0,bgv=0,bgb=0,scancolor=0;
    unsigned char r,v,b;

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
			return 0;
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
			/* no transparency color chosen: default black instead of -1 */
			int tr = hsptr < 0 ? 0 : hsptr;
			int tv = hsptr < 0 ? 0 : hsptv;
			int tb = hsptr < 0 ? 0 : hsptb;
			// la couleur de la transparence
			palette[0]=tr;
			palette[1]=tv;
			palette[2]=tb;
			/* update local compare (variables are by value) */
			hsptr = tr;
			hsptv = tv;
			hsptb = tb;
		}
		maxcoul=is_hsp;
printf("palette=[%s]\n",txtpalette);
		while ((curchar=strchr(txtpalette,separator))!=NULL) {
			if (maxcoul==16) {
				printf(KERROR"\nERROR: invalid palette! too much colorz!\n"KNORMAL);
				return 0;
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
						return 0;
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
						return 0;
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

    *maxcoul_ptr = maxcoul;
    *is_hsp_ptr = is_hsp;
    *png_has_transparency_ptr = png_has_transparency;
    return 1;
}
