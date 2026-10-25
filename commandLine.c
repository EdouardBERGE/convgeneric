#undef __FILENAME__
#define __FILENAME__ "convgeneric.c"

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
	printf("-flat                output one file for all extractions\n");
	printf("-split <size>        split output files with size ex: 4K or 4096\n");
	printf("-m <mode>            output mode (0,1,2)\n");
	printf("-g                   sort palette from darkest to brightest color\n");
	printf("-asmdump             output assembly dump\n");
	printf("-exnfo <file>        export assembly informations about extracted zones\n");
	printf("-expal <file>        export palette in a text file\n");
	printf("-exbinpal <file>     export Plus palette in a binary file\n");
	printf("-impal <file>        import palette from a text file\n");
	printf("-cpccolor            round a little colorz for CPC\n");
	printf("-noalpha             skip transparency (t01,t02,...) output\n");
	printf("-nobin               skip binary (bin,b01,...) output\n");
	printf("-getColor            export/stop after palette processing\n");
	printf("-o <file>            full output filename for binary\n");
	printf("-script <file>       process each line of <file> as a command\n");
	printf("-outputName <radix>  use this radix for all output files instead of the input filename\n");
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
	printf("-tileSheet <file> override default tileSheet.png output filename\n");
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
					parameter->forceextraction=1; parameter->hspFlag=1;
				} else {
					Usage(argv);
				}
				break;
			case 'g':
			case 'G':
				if (stricmp(argv[i],"-getColor")==0) {
					parameter->getcolor=1;
				} else {
					parameter->grad=1;
				}
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
					parameter->hsp=1; parameter->hspFlag=1;
				 } else {
					Usage(argv);
				 }
				 break;
			case 'k':
			case 'K':
				parameter->keep_empty=1; parameter->hspFlag=1;
				break;
			case 'n':
			case 'N':if (stricmp(argv[i],"-noalpha")==0) {
					parameter->noalpha=1;
				 } else if (stricmp(argv[i],"-nobin")==0) {
					parameter->nobin=1;
				 } else {
					Usage(argv);
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
					parameter->oldmode=1; parameter->screenFlag=1;
				} else if (stricmp(argv[i],"-outputName")==0) {
					parameter->outputfilename=argv[++i];
				} else {
					Usage(argv);
				}
				break;
			case 'p':
			case 'P':
				if (!argv[i][2]) {
					if (i+1<argc) {
						parameter->packed=atoi(argv[++i]); parameter->hspFlag=1;
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
					parameter->reverse=1; parameter->hspFlag=1;
				} else if (stricmp(argv[i],"-rastaline")==0) {
					parameter->rastaline=1; parameter->screenFlag=1;
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
					parameter->splitraster=1; parameter->screenFlag=1;
				} else if (stricmp(argv[i],"-script")==0) {
					parameter->scriptfilename=argv[++i];
				} else if (stricmp(argv[i],"-st")==0) {
					parameter->search_transparency=1; parameter->hspFlag=1;
				} else if (stricmp(argv[i],"-single")==0) {
					parameter->single=1;
				} else if (stricmp(argv[i],"-fontscan")==0) {
					parameter->fontscan=1;
				} else if (stricmp(argv[i],"-scan")==0) {
					parameter->scan=1; parameter->hspFlag=1;
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
					parameter->scrmode=1; parameter->screenFlag=1;
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
				} else if (stricmp(argv[i],"-tileSheet")==0) {
					if (i+1<argc) {
						parameter->tilesheetfilename=argv[++i];
					} else {
						Usage(argv);
					}
				}
				break;
			case 'L':
			case 'l':if (stricmp(argv[i],"-lara")==0) {
					parameter->lara=1;
				} else switch (argv[i][2]) {
					case 'B':
					case 'b':
						parameter->lineperblock=atoi(argv[++i]); parameter->screenFlag=1;
						break;
					case 'S':
					case 's':
						parameter->nblinescreen=atoi(argv[++i]); parameter->screenFlag=1;
						break;
					default:Usage(argv);
				}
				break;
			case 'B':
			case 'b':
				parameter->black=1; parameter->hspFlag=1;
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

	if (!parameter->filename && !parameter->scriptfilename) {
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

	if (parameter->screenFlag && parameter->hspFlag) {
		printf("ERROR: conflicting demands - screen options (-scr/-old/-lb/-ls/-w/-splitraster/-rastaline) and hardware sprite options (-hsp/-meta/-scan/-b/-st/-p/-force/-k/-reverse) cannot be used together\n");
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

/*
	GetParametersFromScript
	read a script file, each non-empty line is treated as command line options
*/
void GetParametersFromScript(char *filename, struct s_parameter **out_params, int *out_count, int *out_max)
{
	char **lines = FileReadLines(filename);
	int nparams=0, maxparams=0;
	struct s_parameter *params=NULL;

	for (int li=0; lines && lines[li]; li++) {
		char *line = lines[li];
		/* skip empty lines and comment lines */
		int onlyws=1;
		char *firstchar=line;
		while (*firstchar==' ' || *firstchar=='\t') firstchar++;
		if (*firstchar=='#' || *firstchar=='\n' || *firstchar=='\r' || *firstchar=='\0') continue;
		for (char *c=line;*c;c++) if (*c!=' ' && *c!='\t' && *c!='\n' && *c!='\r') { onlyws=0;break; }
		if (onlyws) continue;

		/* tokenize */
		char *toks[128];
		int argc=0;
		char *save=NULL;
		char *copy = strdup(line);
		for (char *tok=strtok_r(copy," \t\r\n",&save); tok && argc<127; tok=strtok_r(NULL," \t\r\n",&save)) {
			toks[argc++]=tok;
		}
		/* fabricate argv with program name */
		char *argv[129];
		argv[0]="convgeneric";
		for (int j=0;j<argc;j++) argv[j+1]=toks[j];
		int argc2=argc+1;

		struct s_parameter param={0};
		param.tilexclude=-1;
		param.lineperblock=8;
		param.maxextract=1;
		param.split=16384;
		param.metax=1;
		param.metay=1;

		GetParametersFromCommandLine(argc2,argv,&param);

		/* store */
		if (nparams>=maxparams) {
			maxparams=maxparams?maxparams*2:16;
			params=realloc(params,sizeof(struct s_parameter)*maxparams);
		}
		params[nparams++]=param;
	}
	*out_params=params;
	*out_count=nparams;
	*out_max=maxparams;
}

