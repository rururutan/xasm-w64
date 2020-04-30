#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "lasm.h"
#include "label.h"
#include "valstr.h"
#include "pass1.h"
#include "pass2.h"
#include <windows.h>

extern char CopyrightMsg[];
extern char UsageMsg[];
extern char OptionerrMsg[];
extern char MallocerrMsg[];
extern char SrcExt[];
extern char TableFileName[];
extern char TablereaderrMsg[];
extern char	*P1ErrorMsg[];


char    srcfile[FILENAME_MAX];
char    hexfile[FILENAME_MAX];
char    lstfile[FILENAME_MAX];
char	symfile[FILENAME_MAX];
char	tmp_file[FILENAME_MAX];
char	cref_file[FILENAME_MAX];

int 	WordRev=0;
char	Str[128];				
char	Linebuffer[256];		
char    Linebuffer2[256];		


/* オプションフラグ */
int		SymbolFile=0;
int		TemporaryFile=0;
int		CrefFile=0;
int		BinFile =0;
long BinFileStart=0;

static void getprimaryname(char *dest,char *src)
{
	while(*src!=0){
		if(*src=='.'){
			switch( *(src+1)){
				case '.' :*dest++ = *src++;		/* 親ディレクトリを表す */
				          break;
				case '\\':break;				/* 現ディレクトリを表す */
				case '/':break;					/* 現ディレクトリを表す */
				default  :*dest=0; return;
							/* どうやら拡張子との区切りだったようだ */
			}
		}
		*dest++ = *src++;
	}
	*dest=0;
}



static void setpathname(char *filename)
{
	char pname[FILENAME_MAX];
	getprimaryname(pname,filename);
	
	strcpy(srcfile,filename);
	
	strcpy(hexfile,pname);
	if(BinFile)  strcat(hexfile,".bin");
	else		 strcat(hexfile,".hex");
	
	strcpy(lstfile,pname);
	strcat(lstfile,".lst");	
	
	if(TemporaryFile!=0){
		strcpy(tmp_file,pname);
		strcat(tmp_file,".$$$");
	}
	
	if(SymbolFile!=0){
		strcpy(symfile,pname);
		strcat(symfile,".sym");
	}	

	if(CrefFile!=0){
		strcpy(cref_file,pname);
		strcat(cref_file,".crf");
	}	

}



void err(char *mess)
{
	puts(mess);
	exit(1);
}

void p1_err(int ln,int iln,int n)
{
	char str[8];
	
	itoa(ln,str,10);
	fputs(str,stdout);
	if(iln){
		str[0]='-';
		itoa(iln,str+1,10);
		fputs(str,stdout);
	}
	fputs(P1ErrorMsg[n],stdout);
	putchar('\n');
}

static int read_table_file(void)
{
	FILE *fp;
	char str[256];
	int  def = 0;
	
	if((fp=fopen(TableFileName,"rt")) == NULL) {
#if 1
		// 開けない場合はexeのパスを検索
		char exepath[_MAX_PATH];
		char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		GetModuleFileName(NULL, exepath, sizeof(exepath));
		_splitpath(exepath, drive, dir, fname, ext );
		_makepath(exepath, drive, dir, TableFileName, "");
		if ((fp = fopen(exepath, "rt")) == NULL) {
			return -1;
		}
#else
		return -1;
#endif
	}

	while(fgets(str,255,fp)!=NULL){
		if(*str==';') continue;
		if(*str=='@'){
			switch(*(str+1)){
				case '\\': def=get_defn(str+2); break;
				case 'T': def=-1; break;
				case 'W': WordRev=1; break;
				case 'P': fputs(str+2,stdout); break;
				case 'E': goto quit;
				default : return -1;
			}
			continue;
		}
		if(def>=0)  entry_define(def,str);
		else        entry_table(str);
	}
quit:
	fclose(fp);
	
#ifdef DEBUG
	print_define();
#endif
	
	return 0;
}

static void setoption(int argc,char **argv)
{
	int n=1;
	unsigned nTmp;
	
	argc--;
	if(argc==0) err(UsageMsg);
	while(argv[n][0]=='/' || argv[n][0]=='-'){
		if(n==argc) err(UsageMsg);
		if(argv[n][2]==0){
			switch(argv[n][1]){
				case 's':
				case 'S':SymbolFile=1;break;
				case 't':
				case 'T':TemporaryFile=1;break;
				case 'c':
				case 'C':CrefFile=1;break;
				default :err(OptionerrMsg);
			}
		}else{
			switch(argv[n][1]){
				case 'f':
				case 'F':strcpy(TableFileName,&(argv[n][2]));break;
				case 'b':
				case 'B':if(hextoi(&argv[n][2],&nTmp)!=0) BinFile=1;
				         BinFileStart=nTmp;
						 break;
			    default :err(OptionerrMsg);
			}
		}
		n++;
	}
	setpathname(argv[n]);
}


void main(int argc,char **argv)
{
	puts(CopyrightMsg);
	setoption(argc,argv);
	if(init_label()==0)		err(MallocerrMsg);
	if(init_lasm()==0)		err(MallocerrMsg);
	if(CrefFile) if(init_cref()==0)	err(MallocerrMsg);
	
	if(read_table_file())	err(TablereaderrMsg);

	fputs("** PASS 1 **\n",stdout);

	if(pass1()){
		exit(1);
	}

	sort_label();

	
	fputs("** PASS 2 **\n",stdout);
	pass2();
	
	if(CrefFile) output_cref();
}
