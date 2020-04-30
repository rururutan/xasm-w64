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
#include "src.h"

extern char		Str[128];
extern char		Linebuffer[256];
extern char		Linebuffer2[256];

extern char		OpenTMPerrMsg[];
extern char		OpenSRCerrMsg[];

extern int		CrefFile;


void p1_ds(unsigned pc,char *op,char *dest,unsigned  *size)
{
	unsigned s=1;
	if(!wordtoi(op,size,0)) *size=s=0;
	itohex(dest,pc,4); dest[4]=' ';
	itohex(dest+5,s,4);		/* 93/09/23 ﾊﾟｽ2に渡すDSのｻｲｽﾞを1とする */
}

/* １行あたりの定義可能サイズはテンポラリファイルのラインバッファ
　のサイズの制限（１２８バイト）を受ける
*/
int p1_db(unsigned pc,int opn,char **op,char *dest,unsigned *size)
{
	char dstr[128];
	unsigned i,j,k=0,m=0;
	
	dstr[0]=' ';
	
	/*   93/09/22迄の処理
		*size=opn;
		itohex(dest,pc,4); dest[4]=' ';
		itohex(dest+5,*size,4);
		for(i=0;i<*size;i++){
			strcpy(str+2,op[i]);
			strcat(dest,str);
		}
	*/
	/* 93/09/23 DB擬似命令で文字列が扱えるようにした */
	for(i=0;i<opn;i++){
		if(op[i][0]=='\"'){
			j=1;
			while((op[i][j]!=0)&&(op[i][j]!='\"')){
				if(k>110) return 0;
				dstr[k++]=' ';
				itohex(dstr+k,op[i][j],2);
				k+=2;
				m++;j++;
			}
		}else{
			j=0;
			if(k>100) return 0;
			dstr[k++]=' ';
			dstr[k++]='S';
			while(op[i][j]) dstr[k++]=op[i][j++];
			m++;
		}
	}
	dstr[k]=0;
	*size=m;
	itohex(dest,pc,4); dest[4]=' ';
	itohex(dest+5,*size,4);
	strcat(dest,dstr);
	return 1;
}

int p1_dw(unsigned pc,int opn,char **op,char *dest,unsigned *size)
{
	char dstr[128];
	unsigned i,j,k=0;
	
	*size = opn*2;
	
	itohex(dest,pc,4); dest[4]=' ';
	itohex(dest+5,*size,4);
	for(i=0;i<opn;i++){
		j=0;
		if(k>100) return 0;
		dstr[k++]=' ';
		dstr[k++]='W';
		while(op[i][j]) dstr[k++]=op[i][j++];
	}
	dstr[k]=0;
	strcat(dest,dstr);
	return 1;
}

void p1_cpu(unsigned pc,int i,char **match,char *dest,unsigned *size)
{
	char buff[64];
	char *str;
	char *ptr;
	char *ptr2;
	
	str = buff;
	*size = get_fmt(i,str);
	itohex(dest,pc,4); dest[4]=' ';
	itohex(dest+5,*size,4); dest[9]=' ';
	ptr=&dest[10];
	while(*str){
		if(*str=='~'){
			str++;
			ptr2=match[(*str)-'1'];
			while(*ptr2)	*ptr++ = *ptr2++;
			str++;
		}else{
			*ptr++ = *str++;
		}
	}
	*ptr = 0;
}



int pass1(void)
{
	int linenum=0;
	int ilinenum=0;
	int opn;
	int i,j,n;
	int symerr;
	int equflag;
	char label[64],mnem[64];

	unsigned labelsearched_n;
	unsigned labelsearched_d[16];

	static char opbuff[128];

	static char *op[64];

	char *match[MAXTABLEOPN];

	unsigned address,pc,dc,size,val;
	int pflag;
	int erf = 0;
	
	int filelevel;
	int pfilelevel;
	
	
	n=0;
	
	for(i=0;i<MAXTABLEOPN;i++){	match[i] = &Linebuffer2[n]; n+=64;	}
	
	pc    = 0;
	dc    = 0;
	pflag = 1;
	
	
	
	init_temp_for_write();
	
	if(open_src()==0){			/* ｿ-ｽ ﾌｧｲﾙ ｵ-ﾌﾟﾝ */
		err(OpenSRCerrMsg);
	}
	
	i=0;
	pfilelevel=0;
	while((i!=-9)&&(src_gets(Linebuffer,256,&filelevel)!=NULL)){
		
		if(filelevel==0){
			ilinenum=0;
			linenum++;
		}else{
			ilinenum++;
		}
		

		reset_LabelSearchedFlag();
		
		Linebuffer[127]=0;
		if(filelevel==0) Str[0] = ':';
		else             Str[0] = ';';
		Str[1] = ' ';
		Str[2] = 0;
		size = 0;	equflag=0;
		n=get_word(Linebuffer,label,mnem,&opn,op,opbuff);
		
		if(n&4){
			i=lasm(mnem,opn,op,match);
			switch(i){
				case -1 :p1_err(linenum,ilinenum,0);	erf=1;	break;
				case -2 :if(opn!=1){						/* DS */
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
						 	if(pflag) address=pc;
						 	else      address=dc;
							p1_ds(address,op[0],Str+2,&size);
						 }
						 break;
				case -3 :if(opn==0){						/* DB */
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
						 	if(pflag) address=pc;
						 	else      address=dc;
						 	if(p1_db(address,opn,op,Str+2,&size)==0){
						 		p1_err(linenum,ilinenum,7);
						 		erf=1;
						 	}
						 }
						 break;
				case -4 :if(opn==0){						/* DW */
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
						 	if(pflag) address=pc;
						 	else      address=dc;
						 	if(p1_dw(address,opn,op,Str+2,&size)==0){
						 		p1_err(linenum,ilinenum,7);
						 		erf=1;
						 	}
						 }
						 break;
				case -5 :if((n&8==0)||(opn!=1)){			/* EQU */
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
						 	n&=0xfff7;	/* ラベルステ－タス  クリア */
						 	if(pflag) address=pc;
						 	else      address=dc;
							val=evaluation(op[0],address,0,&symerr);
							if(symerr){
									p1_err(linenum,ilinenum,6);
									erf=1;
							}else{
								if(add_label(label,val,DATA_VALUE)){
									p1_err(linenum,ilinenum,2);
									erf=1;
								}
							}
						 }
						 break;
				case -6 :if((n&8)||(opn!=1)){				/* ORG */
						 	n&=0xfff7;
							p1_err(linenum,ilinenum,1);
							erf=1;
						 } else {
							Str[2] = '$'; Str[3] = 0;
						 	if(pflag) address=pc;
						 	else      address=dc;
							address=evaluation(op[0],address,0,&symerr);
						 	if(symerr){
								p1_err(linenum,ilinenum,6);
								erf=1;
						 	}
						 	if(pflag) pc=address;
						 	else      dc=address;
						 }
						 break;
				case -7 :if((n&8)||(opn!=0)){				/* CSEG */
						 	n&=0xfff7;
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
							Str[2] = 'c'; Str[3] = 0;
							pflag=1;
						 }
						 break;
				case -8 :if((n&8)||(opn!=0)){				/* DSEG */
						 	n&=0xfff7;
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
							Str[2] = 'd'; Str[3] = 0;
						 	pflag=0;
						 }
						 break;
				case -9 :if((n&8)||(opn!=0)){				/* END */
						 	n&=0xfff7;
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
							Str[2] = '#'; Str[3] = 0;
						 }
						 break;
				
				case -10:if((n&8)||(opn!=1)||(op[0][0]!='\"')){ /* INCLUDE */
						 	n&=0xfff7;
							p1_err(linenum,ilinenum,1);
							erf=1;
						 }else{
						 	j=1;
						 	while((op[0][j]!=0)&&(op[0][j]!='\"')) j++;
						 	if(op[0][j]==0){
								p1_err(linenum,ilinenum,1);
								erf=1;
							}else{
								op[0][j]=0;
								if(open_inc(&op[0][1])==0){
									p1_err(linenum,ilinenum,8);
									erf=1;
								}
							}
						 }
						 break;	
				default :
						 if(pflag) address = pc;
						 else      address = dc;
						 p1_cpu(address,i,match,Str+2,&size);
						 break;
			}
		}
		if(n&8){
			if(pflag) address = pc;
			else      address = dc;
			if(add_label(label,address,ADDRESS_VALUE)){
				p1_err(linenum,ilinenum,2);
				erf=1;
			}
		}
		
		write_tmp_info(linenum,Str);

		if(CrefFile){
			labelsearched_n = get_LabelSearchedFlag(labelsearched_d);
			regist_cref_infomation
			    (linenum,ilinenum,labelsearched_n,labelsearched_d);
		}
		
		if(pflag)	pc+=size;
		else		dc+=size;
	}
	
	close_tmp_for_write();
	
	close_src();
	return erf;
}
