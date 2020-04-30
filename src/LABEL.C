#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include "asm.h"
#include "words.h"
#include "valstr.h"

#define MAX_CREF_N 8000

typedef struct {
	char         far *word;
	unsigned     val;
	unsigned     stus;
}LABEL;

/* LABEL.stus */
/*  b15 b14 b13 b12 b11 b10  b9  b8  b7  b6  b5  b4  b3  b2  b1  b0 */
/*   |   |           |---------------- 登録番号------------------|  */
/*   |   +---クロスリファレンスで使用                               */
/*   +-------DATA - 1 : ADDRESS - 0                                 */

typedef struct {
	char	*a;
	char	*b;
}DEFINE;


typedef struct {
	unsigned     line;
	unsigned     iline;
	unsigned     stus;
}CREF;


static char far *LabelBuff;		/* ラベル文字列格納部   		*/
static unsigned Labelstroff;	/* ラベル文字列格納オフセット	*/
static LABEL  *Label;			/* ラベル格納部 		*/
static LABEL **Lp;				/* ラベルポインタ配列 	*/
static int     Ln;				/* ラベル数 			*/

static char   *Defstr;			/* 定義文字列格納部				*/
static unsigned Defstroff;		/* 定義文字列格納オフセット		*/
static DEFINE *Define;			/* 定義構造体配列格納部			*/
static int     Def;				/* 定義数						*/
static unsigned Defnum[16];		/* 定義番号－配列中オフセット対照表 */

extern char	cref_file[];		/* クロスリファレンスファイル名 */
extern int	CrefFile;			/* クロスリファレンスファイル作成フラグ */
extern char	OpenCRFerrMsg[];
extern char	MallocerrMsg[];
extern char	Crefoverflow[];

static CREF far *CrefBuff;		/* クロスリファレンス情報格納部	*/
static CREF far *CrefPtr;
static int  Cref_enable;

static Cref_n;	/* バッファへの登録数 */




/* ラベルを探索したり登録したことを知らせる変数 */
static unsigned  LabelSearchedFlag;			/* 探索または登録された数 */
static unsigned  LabelSearchedStus[16];


void reset_LabelSearchedFlag(void)
{
	LabelSearchedFlag=0;
}

int get_LabelSearchedFlag(unsigned *buff)
{
	int n;
	for(n=0;n<LabelSearchedFlag;n++){
		*buff++=LabelSearchedStus[n];
	}
	return LabelSearchedFlag;
}


int  init_label(void)			/* ラベル領域の確保 	*/
{
	int i;
	LABEL *lp;
	
	
	
	if((Defstr =malloc(MAXDEFSTR))==NULL) return 0;
	Defstroff  =0;
	Def=0;
	if((Define =malloc(sizeof(DEFINE)*MAXDEF))==NULL) return 0;
	
	Defnum[0]  =0;
	for(i=1;i<16;i++) Defnum[i]=MAXDEF;
	
	if((LabelBuff=farmalloc(MAXLABELSTR))==NULL) return 0;
	Labelstroff  =0;
	
	if((Label	 =malloc(sizeof(LABEL)*MAXLABEL))==NULL) return 0;
	if((Lp   	 =malloc(sizeof(LABEL*)*MAXLABEL))==NULL)return 0;
	lp = Label;
	
	for(i=0;i<MAXLABEL;i++){	/* ラベルポインタ初期化  */
		Lp[i] = lp++;
	}
	Ln = 0;
	return 1;
}


int print_label(int adr_only,FILE *d)
/* アドレス４桁＋スペース＋文字列１６文字＋スペース＝２２文字 */
{
	int i,j,k;
	char lstr[32];
	char far *fptr;
	k=0;
	for(i=0;i<Ln;i++){
		if((adr_only)&&(Lp[i]->stus & DATA_VALUE)) continue;
		itohex(lstr,Lp[i]->val,4);
		lstr[4]=' ';
		j=5;	fptr=Lp[i]->word;
		while(*fptr){
			lstr[j]=*fptr;
			if((lstr[j]>='a')&&(lstr[j]<='z')) lstr[j]&=0xdf;
			j++; fptr++;
		}
		while(j<(5+LABELLENGTH)){
			lstr[j++]=' ';
		}
		
		if(k%3 ==2){
			lstr[j++]='\n';
		}else{
			lstr[j++]=' ';
		}
		lstr[j]=0;
		
		fputs(lstr,d);
		k++;
	}
	if((k%3)!=0) fputc('\n',d);
	return 1;
}

	static int cmplabel(char *a,LABEL **p)
	{
		int i,n;
		char far *fptr;
		
		n=0;
		fptr=(*p)->word;
		while((*a) && (*fptr)){
			if((i=(*a) - (*fptr))!=0) return i;
			a++;fptr++;n++;
			if(n==LABELLENGTH) return 0;
		}
		return(*a) - (*fptr);
	}




int add_label(char *str,unsigned val,int stus)
{
	int j,n;
	LABEL **ptr;
	char far *fptr;
	
	
	n=strlen(str);
	if(n>LABELLENGTH) n=LABELLENGTH;
	n++;
	if((Labelstroff+n)>=MAXLABELSTR) return -6;

	if(Ln>=MAXLABEL) return -6;
	if(Ln!=0){
		ptr=Lp;
		for(j=0;j<Ln;j++) if(cmplabel(str,ptr++)==0) return -4;
	}
	
	
	fptr = Lp[Ln]->word = LabelBuff+Labelstroff;
	j=1;
	while(j<n){
		*fptr++ = *str++;
		j++;
	}
	*fptr=0;
	Labelstroff+=n;
	Lp[Ln]->val = val;
	stus|=(Ln&0xfff);
	Lp[Ln]->stus = stus;
	LabelSearchedStus[LabelSearchedFlag++]=stus|0x4000;
	Ln++;
	return 0;
}


int entry_define(int defn,char *str)
{
	static pdefn=0;
	char word[64];
	int  n;
	char *cptr;
	
	decap(str);
	if((str = get_word2(str,word,','))==NULL) return 0;
	
	if(pdefn<defn){
		Defnum[defn]=Def;
		pdefn=defn;
	}
	
	n=strlen(word)+1;
	if( (Defstroff+n)>= MAXDEFSTR) return 0;
	cptr = Defstr + Defstroff;
	Define[Def].a = cptr;
	strcpy(cptr,word);
	Defstroff+=n;
	
	get_word2(str,word,',');
	n=strlen(word)+1;
	if( (Defstroff+n)>= MAXDEFSTR) return 0;
	cptr = Defstr + Defstroff;
	Define[Def].b = cptr;
	strcpy(cptr,word);
	Defstroff+=n;
	
	Def++;
	return 1;
}

#ifdef DEBUG
	int print_define(void)
	{
		int i,j;
		j=0;i=0;
		while(Defnum[j]<128){
			printf("D%d\n",j);j++;
			while((i<Defnum[j])&&(i<Def)){
				printf("\t%s , %s\n",Define[i].a,Define[i].b);
				i++;
			}
		}
	}
#endif

int get_defn(char *str)
{
	char *ptr;
	ptr = str;
	while((*str >= '0')&&(*str <= '9')) str++;
	*str=0;
	return atoi(ptr);
}



int define_replace(int defn,char *a,char *b)	/* a->b */
{
	int i;
	i=Defnum[defn];
	while((i<Defnum[defn+1])&&(i<Def)){
		if(!strcmp(a,Define[i].a)){
			strcpy(b,Define[i].b);
			return 1;
		}
		i++;
	}
	return 0;
}


int get_label_val_ls(char *str,unsigned int *v) 
{
	LABEL **ptr;
	int n;
	
	ptr=Lp;
	for(n=0;n<Ln;n++) if(cmplabel(str,ptr++)==0) break;
	if(Ln==n) return 0;
	ptr--;
	*v = (*ptr)->val;
	LabelSearchedStus[LabelSearchedFlag++]=(*ptr)->stus;
	return 1;
}


int get_label_val_bs(char *str,unsigned int *v) 
{
	LABEL **ptr;
	
	ptr=bsearch(str,Lp,Ln,sizeof(LABEL *),
	(int(*)(const void *,const void *))cmplabel);
	if(ptr==NULL) return 0;
	*v = (*ptr)->val;
	LabelSearchedStus[LabelSearchedFlag++]=(*ptr)->stus;
	return 1;
}

	static int cmplabel2(LABEL **a,LABEL **b)
	{
		int i;
		char far *fptr_a,far *fptr_b;
		
		fptr_a=(*a)->word;
		fptr_b=(*b)->word;
		
		while((*fptr_a) && (*fptr_b)){
			if((i= (*fptr_a) - (*fptr_b))!=0) return i;
			fptr_a++; fptr_b++;
		}
		return *fptr_a - *fptr_b;
	}

void sort_label(void)
{
	qsort(Lp,Ln,sizeof(LABEL *),
	(int(*)(const void *,const void *))cmplabel2);
}



/**************************/
/* クロスリファレンス関連 */
/**************************/
int init_cref(void)
{
	/* CREF ﾊﾞｯﾌｧ 確保 */
	if((CrefBuff=farmalloc(sizeof(CREF)*MAX_CREF_N))==NULL){
		err(MallocerrMsg);
		return 0;
	}
	CrefPtr=CrefBuff;
	Cref_n=0;
	Cref_enable=1;
}

int regist_cref_infomation(int line,int iline,unsigned n,unsigned *stus)
{
	while(n){
		if(!Cref_enable)return 0;
		
		CrefPtr->line=line;
		CrefPtr->iline=iline;
		CrefPtr->stus=*stus++;
		CrefPtr++;
		Cref_n++;
		if(Cref_n==MAX_CREF_N){
			Cref_enable=0;
			puts(Crefoverflow);
			return 0;
		}
		n--;
	}
	return 1;
}


int output_cref(void)
{
	FILE *Cref_f;

	int i,j,k,m,line,iline;
	char lstr[32];
	char nstr[16];
	char nstr2[16];
	unsigned stus,stus2;
	int crlfflag=0;
	
	CREF far *fcrefptr;
	char far *fcharptr;

	if((Cref_f=fopen(cref_file,"wt")) == NULL){
		err(OpenCRFerrMsg);
		return 0;
	}
	
	for(k=0;k<18;k++) lstr[k]=' '; lstr[k]=0;
	for(i=0;i<Ln;i++){
		fcharptr=Lp[i]->word;
		stus=Lp[i]->stus;
		k=2;
		while(*fcharptr){
			lstr[k]=*fcharptr++;
			if((lstr[k]>='a')&&(lstr[k]<='z')) lstr[k]&=0xdf;
			k++;
		}
		if(stus&0x8000) lstr[0]='*';
		stus&=0xfff;
		fputs(lstr,Cref_f);
		for(k=0;k<18;k++) lstr[k]=' '; lstr[k]=0;
		fcrefptr=CrefBuff;
		m=0;
		crlfflag=0;
		for(j=0;j<Cref_n;j++){
			stus2=fcrefptr->stus;
			line=fcrefptr->line;
			iline=fcrefptr->iline;
			fcrefptr++;
			if(stus!=(stus2&0xfff)) continue;

			if(crlfflag) fputs(lstr,Cref_f);
			crlfflag=0;
		/*	if(stus2 & 0x4000) nstr[0]='#';else nstr[0]=' '; */
			itoa(line & 0xfff,nstr,10);
			if(iline!=0){
				nstr2[0]='-';
				itoa(iline & 0xfff,nstr2+1,10);
				strcat(nstr,nstr2);
			}
			setright(10,nstr2,nstr);
			fputs(nstr2,Cref_f);
			m++;
			if(m==6){
				m=0;
				fputc('\n',Cref_f);
				crlfflag=1;
			}
		}
		if(!crlfflag) fputc('\n',Cref_f);
	}
	fclose(Cref_f);
	return 1;
}

