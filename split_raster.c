#undef __FILENAME__
#define __FILENAME__ "split_raster.c"

int processSplitRaster(
    struct s_parameter *parameter,
    struct s_png_info *photo,
    unsigned char *palette,
    struct s_rastecran *scr_param,
    int *clash_param)
{
    struct s_rastecran scr = *scr_param;
    int clash = *clash_param;
    int i,j,k,l,m,n,accmode,cptcol,oldmode,ib,col,maxblockcol=0,maxlinecol=0,bareplace=0,noptrou=0,colormaysplit=0,lindex=0,iprim=0,changeback;
    int reg[MAXSPLIT];
    int garemove[27];
    int lastbackgroundcolor[16];
    int primarybackgroundcolor[15];
    int colorscore[128];
    int colorstat[128][128];
    int splitcolor[128];
    struct s_score tabscore[27];
    int cptcolor[4096];
    int curga,blockhascolor;
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
			return 0;
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

    *scr_param = scr;
    *clash_param = clash;
    return 1;
}
