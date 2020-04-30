#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "words.h"
#include "lasm.h"
#include "label.h"
#include "eval.h"
#include "valstr.h"
#include "temp.h"

extern int      SymbolFile;			/* シンボルファイル生成フラグ	*/
extern int      WordRev;			/* ワ－ドデ－タ順序フラグ		*/
extern char		Str[128];			/* テンポラリファイル読み込み用 */
extern char		Linebuffer[256];	/* ソ－スファイル読み込み用     */
extern char		Linebuffer2[256];	/* リストファイル作成用		    */
extern char    	srcfile[64];
extern char     hexfile[64];
extern char     lstfile[64];
extern char     symfile[64];
	

extern char		OpenTMPerrMsg[];
extern char		OpenSRCerrMsg[];
extern char		OpenLSTerrMsg[];
extern char		OpenHEXerrMsg[];
extern char		OpenBINerrMsg[];
extern char		OpenSYMerrMsg[];
extern char		LstOuterrMsg[];
extern char     MallocerrMsg[];
extern char		WriteHEXerrMsg[];
extern char		WriteBINerrMsg[];

extern int		CrefFile;
extern int		BinFile;
extern long     BinFileStart;


FILE *Src,*Tmp,*Lst,*Hex,*Sym;

unsigned char *Hexbuff;
long Hexmin=0xffff,Hexmax=0;


static int init_hexout(void)
{
	long i;
	unsigned char *tptr;
/*	unsigned ui;	*/
	
	if((tptr=malloc(0x10010))==NULL) return 0;
/*	printf("HEX buffer %04x %04x\n",FP_SEG(tptr),FP_OFF(tptr)); */

	Hexbuff = tptr;

	tptr=Hexbuff;
/*	printf("HEX buffer %04x %04x\n",FP_SEG(tptr),FP_OFF(tptr)); */
	for(i=0;i<0x10000;i++) *tptr++ = 0xff;
	return 1;
}

static void hexout(unsigned long adr,int n,unsigned char *binbuff)
{
	unsigned char *fptr;
	if(Hexmin>adr) Hexmin=adr;
	if(Hexmax<(adr+n-1)) Hexmax=adr+n-1;
	if(Hexmax>0xffff) Hexmax=0xffff;
	fptr=Hexbuff+adr;
	while(n){
		*fptr++ = *binbuff++;
		n--;
	}
}


	static int hexflush_line(unsigned adr,int n,unsigned char *binbuff)
	{
		int i;
		unsigned char uc;
		unsigned char *cptr;
		
		uc=0xff; cptr=binbuff;
		for(i=0;i<n;i++){
			uc &= *cptr;
			cptr++;
		}
		if(uc==0xff) return 1;
		
		cptr=Linebuffer;
		*cptr++ = ':';
		uc=n;
		itohex(cptr,n,2);
		cptr+=2;
		uc+=(adr&0xff00)>>8;
		uc+=adr&0xff;
		itohex(cptr,adr,4);
		cptr+=4;

		*cptr++ = '0';
		*cptr++ = '0';
		while(n){
			itohex(cptr,*binbuff,2);
			uc+=*binbuff;
			cptr+=2;
			binbuff++;
			n--;
		}
		uc^=0xff;
		uc++;
		itohex(cptr,uc,2);
		cptr+=2;
		*cptr++ = '\n';
		*cptr = 0;
		
		if(fputs(Linebuffer,Hex)==EOF) return 0;
		return 1;
	}

static int hexflush(void)
{
	unsigned char *fptr;
	unsigned char *nptr;
	unsigned char binbuff[40];
	int i,n;
	
	while(Hexmin<=Hexmax){
		if((Hexmin+32-1)>Hexmax) n=Hexmax-Hexmin+1; else n=32;
		fptr=Hexbuff+Hexmin;
		nptr=binbuff;
		for(i=0;i<n;i++) *nptr++ = *fptr++;
		if(hexflush_line(Hexmin,n,binbuff)==0) return 0;
		Hexmin+=32;
	}
	
	return 1;
}

static int binflush(void)
{
	unsigned char *fptr;
	unsigned char *nptr;
	unsigned char binbuff[40];
	int i,n;
	
	while(BinFileStart<=Hexmax){
		if((BinFileStart+32-1)>Hexmax) n=Hexmax-BinFileStart+1; else n=32;
		fptr=Hexbuff+BinFileStart;
		nptr=binbuff;
		for(i=0;i<n;i++) *nptr++ = *fptr++;
		if(fwrite(binbuff,n,1,Hex)==0) return 0;
		BinFileStart+=32;
	}
	
	return 1;
}





static int lstout(unsigned adr,int n,unsigned char *binbuff,char *lstbuff)
{
	int column=0;
	char buff[40];
	char *dest;
	
	
	dest=buff;
	if(n){
		itohex(dest,adr,4);
		dest+=4;
		*dest++ = ':';  *dest++ = ' ';
		column = 6;
	}
	while(n&&(column<30)){
		itohex(dest,*binbuff,2);
		dest+=2;
		*dest++ = ' ';
		column += 3;
		n--;  binbuff++;
	}
	while(column<32){
		*dest++ = ' ';
		column++;
	}
	*dest = 0;
	
	if(fputs(buff,Lst)==EOF) return 0;
	if(fputs(lstbuff,Lst)==EOF) return 0;
	return 1;
}

static char* get_word_p2(char *src,char *dest)
{
	while(*src==' ') src++;
	if((*src=='\n')||(*src==0)) return NULL;
	while(  (*src!=' ')&&(*src!=0)&&(*src!='\n')  )
		*dest++ = *src++;
	*dest=0;
	return src;
}

static int str2bin(char *src,unsigned *adr,int *n,unsigned char *destbin)
{
	char word[64];
	unsigned dat;
	int i;
	long ld;
	int symerr;
	long ld1;
	
	src+=8;
	if((src=get_word_p2(src,word))==NULL) 	return 1;
	if(hextoi(word,adr)==0) 				return 1;
	if((src=get_word_p2(src,word))==NULL) 	return 1;
	if(hextoi(word,n)==0)	 				return 1;
	
	i=0;
	while(1){
		if((src=get_word_p2(src,word))==NULL)break;
		symerr=0;
		switch(word[0]){
			case 'S':	*destbin++ = evaluation(word+1,*adr,1,&symerr)&0xff;
						i++;
						break;
			case 'W':	ld= evaluation(word+1,*adr,1,&symerr);
						if(WordRev){
							*destbin++ = (ld>>8)&0xff;
							*destbin++ = ld &0xff;
						}else{
							*destbin++ = ld &0xff;
							*destbin++ = (ld>>8)&0xff;
						}
						i+=2;
						break;
			case 'T':	if(ld1=evaluation(word+1,*adr,1,&symerr)){
						/*	printf("****%lx****\n",ld1); */
							return -1;
						}else{
						/*	printf("****%lx****\n",ld1); */
							break;
						}
						
			default:	hextoi(word,&dat);
						*destbin++ = dat&0xff;
						i++;
						break;
		}
		if(symerr) return (-1)-symerr;  /* 1996/08/15 V1.75 */
		                                /* もともと symerr に 1 が返ってきた
		                                   ときに -2 を返したが，symerr が
		                                   1か2を返すようになったので変更 */
	}
	
	while(i<*n){
		*destbin++=0xff;
		i++;
	}
	return 0;
}


void pass2(void)
{
	int codeflag=1,endflag=0,lstflag;
	unsigned adr=0;
	int n,ln,sln,isln=0;
	static unsigned char binbuff[128];

	unsigned labelsearched_n;
	unsigned labelsearched_d[16];


	
	if(init_hexout()==0){
		puts(MallocerrMsg);
		exit(1);
	}
	
	
	init_temp_for_read();
	
	if((Src=fopen(srcfile,"rt")) == NULL){			/* ソース ファイル オープン */
		err(OpenSRCerrMsg);
	}

	if((Lst=fopen(lstfile,"wt")) == NULL){			/* リスト ファイル オープン */
		err(OpenLSTerrMsg);
	}

	sln=ln=0;
	while((!endflag) && (tmp_gets(Str)!=NULL)){
		ln++;

		reset_LabelSearchedFlag();
		if((Str[6]!=':')&&(Str[6]!=';'))break;
		
		if(Str[6]==':'){
			lstflag=1; 
			sln++;
			isln=0;
		}else{
			isln++;
			lstflag=0;
		}
		n=0;
		if( (Str[8]>='0')&&(Str[8]<='9') ||
		    (Str[8]>='A')&&(Str[8]<='F')  ){
			switch(str2bin(Str,&adr,&n,binbuff)){
				case  1:	p1_err(sln,isln,4); exit(1);
				case -1:	p1_err(sln,isln,5); break;
				case -2:	p1_err(sln,isln,6); break;
				case -3:	p1_err(sln,isln,9); break;
                
			}
			
		/*	printf("%04X  ",adr);
			for(i=0;i<n;i++){
				printf("%02X ",binbuff[i]);
			}
			printf("\n");
		*/
		}else{
			switch(Str[8]){
				case 'c': codeflag=1;break;
				case 'd': codeflag=0;break;
				case '#': endflag=1; break;
			}
		}
		if(lstflag){
			if(fgets(Linebuffer,256,Src)==NULL) break;
			if(lstout(adr,n,binbuff,Linebuffer)==0){
				err(LstOuterrMsg);
			}
		}
		if(codeflag&&(n>0)) hexout(adr,n,binbuff);
		
		if(CrefFile){
			labelsearched_n = get_LabelSearchedFlag(labelsearched_d);
			regist_cref_infomation(sln,isln,labelsearched_n,labelsearched_d);
		}
	}
	
	close_tmp_for_read();
	fclose(Lst);
	fclose(Src);
	
	if(SymbolFile!=0){
		if((Sym=fopen(symfile,"wt")) == NULL){	
			err(OpenSYMerrMsg);
		}
		print_label(1,Sym);
		fclose(Sym);
	}
	
	if(BinFile){
		if((Hex=fopen(hexfile,"wb")) == NULL){	
			err(OpenHEXerrMsg);
		}
		if(binflush()==0){
			err(WriteBINerrMsg);
		}
		fclose(Hex);
	}else{
		if((Hex=fopen(hexfile,"wt")) == NULL){	
			err(OpenHEXerrMsg);
		}
		if(hexflush()==0){
			err(WriteHEXerrMsg);
		}
		fputs(":00000001FF\n",Hex);
		fclose(Hex);
	}
}

