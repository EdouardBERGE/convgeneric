
int processHardwareSprite(
    struct s_parameter *parameter,
    struct s_png_info *photo,
    unsigned char *palette,
    unsigned char *cpcdata,
    unsigned char *transdata,
    struct s_sprite_info **spinfo,
    int *ispi,
    int *mspi,
    int *idata,
    int *maxdata,
    int hsptr,
    int hsptv,
    int hsptb)
{
		int i,j,xs,ys,zepix,ticpack,metax,metay;
		struct s_sprite_info curspi={0};

		for (j=parameter->oy;j<photo->height && *ispi<parameter->maxextract;j+=parameter->metay) {
			for (i=parameter->ox;i<photo->width && *ispi<parameter->maxextract;i+=parameter->metax) {
				/* sprites automatic search */
				//AutoScan(parameter,photo,palette,&i,&j);
				// meta management
				for (metay=0;metay<parameter->metay;metay+=16)
				for (metax=0;metax<parameter->metax;metax+=16)
				if (j+parameter->sy+metay<=photo->height && i+parameter->sx+metax<=photo->width) {
					/* prepare sprite info */
					curspi.adr=*idata;
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
									cpcdata[(*idata)++]=zepix;
									break;
								case 2:
									/* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[*idata]=zepix; ticpack=1; break;
										case 1: cpcdata[(*idata)++]|=zepix*16; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								case 4:
									/* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[*idata]=zepix*64; ticpack=1; break;
										case 1: cpcdata[*idata]|=zepix*16; ticpack=2; break;
										case 2: cpcdata[*idata]|=zepix*4; ticpack=3; break;
										case 3: cpcdata[(*idata)++]|=zepix; ticpack=0; break;
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
								case 0: cpcdata[(*idata)++]=0; break;
								case 2: /* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[*idata]=0; ticpack=1; break;
										case 1: (*idata)++; ticpack=0; break;
										default: printf("warning remover\n"); break;
									}
									break;
								case 4: /* packed reversed */
									switch (ticpack) {
										case 0: cpcdata[*idata]=0; ticpack=1; break;
										case 1: ticpack=2; break;
										case 2: ticpack=3; break;
										case 3: (*idata)++; ticpack=0; break;
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
							case 0: for (xs=0;xs<16;xs++) cpcdata[(*idata)++]=0; break;
							case 2: /* packed reversed */ for (xs=0;xs<8;xs++) cpcdata[(*idata)++]=0; break;
							case 4: /* packed reversed */ for (xs=0;xs<4;xs++) cpcdata[(*idata)++]=0; break;
							default:printf("warning remover\n");break;
						}
						ys++;
					}
					/* check for empty sprite... */
					for (ys=curspi.adr;ys<*idata;ys++) {
						if (cpcdata[ys]) break;
					}
					if (parameter->keep_empty || ys!=*idata) {
						/* update sprite info */
						curspi.size=*idata-curspi.adr;
						curspi.x=parameter->sx;
						curspi.y=parameter->sy;
						ObjectArrayAddDynamicValueConcat((void **)&*spinfo,&*ispi,&*mspi,&curspi,sizeof(struct s_sprite_info));
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
						*idata=curspi.adr;
					}
				}
			}

    }
    (void)transdata;
    return 1;
}
