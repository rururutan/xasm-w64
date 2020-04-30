#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "asm.h"
#include "valstr.h"
#include "words.h"
#include "label.h"

struct lasmtable {
	char *mnem;
	int  opn;
	char *op[MAXTABLEOPN];
	int	 size;
	char *fmt;
};

static struct lasmtable *LaTbl;
static char   *TableBuffer;

static int     Table;
static char   *TableEnter;
static int     Mnem;			/* ƒjƒ‚ƒjƒbƒN‚Ìí—Ş */
static int    *MnemTable;		/* ƒjƒ‚ƒjƒbƒN‚Ì•Ï‚í‚è–Ú‚ªTable‚Ì‰½”Ô–Ú‚© */


int init_lasm(void)
{
	Table=0;
	Mnem=0;
	if((LaTbl=malloc(sizeof(struct lasmtable)*MAXTABLE))==NULL) return 0;
	if((TableBuffer=malloc(16384))==NULL) return 0;
	if((MnemTable=malloc(sizeof(int *)*MAXMNEM))==NULL) return 0;
	TableEnter = TableBuffer;
	return 1;
}


int entry_table(char *str)
{
	int i,n,m,dum;
	char word[64];
	char *ptr;
	char buff[64];
	static char pmnem[16]="";

	detab(str);
	if((str = get_word1(str,word,&dum))==NULL) return 1;		/* ƒjƒ‚ƒjƒbƒN */
	if(strcmp(pmnem,word)){
		MnemTable[Mnem]=Table;
		strcpy(pmnem,word);
		Mnem++;
	}
	LaTbl[Table].mnem = TableEnter;
	m=0;
	while(word[m]) *TableEnter++ = word[m++];
	*TableEnter++ = 0;
		
	if((str = get_word1(str,word,&dum))==NULL) return 1; /* ƒIƒyƒ‰ƒ“ƒh‚Ì” */
	n=atoi(word);
	LaTbl[Table].opn = n;
	for(i=0;i<n;i++){
		if((str = get_word1(str,word,&dum))==NULL) return 1; /* ƒIƒyƒ‰ƒ“ƒh */
		LaTbl[Table].op[i] = TableEnter;
		m=0;
		while(word[m]) *TableEnter++ = word[m++];
		*TableEnter++ = 0;
	}
	if((str = get_word1(str,word,&dum))==NULL) return 1;	/* ƒR|ƒhƒTƒCƒY */
	n=atoi(word);
	LaTbl[Table].size = n;

	ptr = buff;								/* ƒR|ƒhƒtƒH|ƒ}ƒbƒg */
	while(1){
		if((str = get_word1(str,word,&dum))==NULL) break;
		m=0;
		while(word[m]) *ptr++ = word[m++];
		*ptr++ = ' ';
	}
	*(ptr-1)=0;
	ptr = buff;
	LaTbl[Table].fmt = TableEnter;
	while(*ptr) *TableEnter++ = *ptr++;
	*TableEnter++ = 0;
	Table++;
	return 0;
}

#ifdef DEB
	void print_table(void)
	{
		int i,j;
		for(i=0;i<Table;i++){
			printf("%8s %2d ",LaTbl[i].mnem,LaTbl[i].opn);
			for(j=0;j<2;j++){
				if(j<LaTbl[i].opn) 	printf("%5s ",LaTbl[i].op[j]);
				else    			printf("      ");
			}
			printf("%d %s\n",LaTbl[i].size,LaTbl[i].fmt);
		}
		for(i=0;i<Mnem;i++){
			printf("%3d %s\n",MnemTable[i],LaTbl[MnemTable[i]].mnem);
		}
	}
#endif

static int pseudo_inst(char *word)
/*--------------------------------------
 ‚Ç‚Ì‹[—–½—ß‚©‚ğ‚©‚¦‚·
 -1    FŠY“–‚È‚µ
 -2    F‚c‚r
 -3	   F‚c‚a
 -4	   F‚c‚v
 -5    F‚d‚p‚t
 -6    F‚n‚q‚f
 -7    F‚b‚r‚d‚f
 -8    F‚c‚r‚d‚f
 -9    F‚d‚m‚c
 -10   F‚h‚m‚b‚k‚t‚c‚d
 --------------------------------------*/
{
	int i;
 	static char *tbl[] = {"ds","db","dw","equ","org","cseg","dseg","end",
 						  "include","defs","defb","defw",NULL};
 	
 	i=0;
 	while(1){
 		if(strcmp(word,tbl[i])==0) break;
 		i++;
 		if(tbl[i]==NULL) return -1;
 	}
 	if((i>=9)&&(i<=11)) i-=9;
 	
 	return -2-i;
}

static int search_mnem(char *word)
{
	int t,b,m,r;
	
	t=0;b=Mnem-1;
	while(b>=t){
		m=(t+b)/2;
		if((r=strcmp(word,LaTbl[MnemTable[m]].mnem))==0) return m;
		if(r>0) t=m+1; else b=m-1;
	}
	return -1;
}


static void get_table_limit(int mnem_num,int *top,int *bot)
{
	*top = MnemTable[mnem_num];
	if(mnem_num==(Mnem-1)) *bot = Table-1;
	else *bot = MnemTable[mnem_num+1]-1;
}

static char tail(char *str)
{
	while(*str) str++;
	str--;
	return *str;
}

static void deltail(char *str)
{
	while(*str) str++;
	str--;
	*str=0;
}



static int op_match(char *table_op,char *s_word,char *match_word)
{
	int paflag=0;
	int n;
	char buff[32];
	char *word;
	
	
	strcpy(buff,s_word);
	word=buff;
	*match_word = 0;
	if(*table_op=='(') paflag=1;
	while(*table_op && *word){
		if(*table_op == '\\'){
			if(paflag){
				if(tail(word)==')')deltail(word); else return -1;
			}
			table_op++;
			n=get_defn(table_op);
			if(!define_replace(n,word,match_word)){
				return 0;
			}
			return 2;
		}
		if(*table_op == '~'){
			if(paflag){
				if(tail(word)==')')deltail(word); else return -1;
			}
			while(*word) *match_word++ = *word++;
			*match_word = 0;
			return 2;
		}
		if(*table_op != *word) return 0;
		table_op++; word++;
	}
	if(*table_op != *word) return 0;
	else return 1;
}


int lasm(char *mnem,int opn,char *op[],char *match[])
/* ˆê’v‚·‚éƒCƒ“ƒXƒgƒ‰ƒNƒVƒ‡ƒ“‚ª‚İ‚Â‚©‚ç‚È‚¯‚ê‚ÎC|‚P‚ğ•Ô‚· */
{
	int r,i,flg;
	int t,b;
	
	r = pseudo_inst(mnem);
	if(r != -1) return r;
	
	r = search_mnem(mnem);
	if(r == -1){
		return -1;
	}
	get_table_limit(r,&t,&b);
	while(t<=b){
		if( LaTbl[t].opn != opn){
			goto Cont;
		}
		for(i=0;i<opn;i++){
			flg=op_match(LaTbl[t].op[i],op[i],match[i]);
			if(flg<0){
				return -1;
			}

			else if(flg==0) goto Cont;
		}
		return t;
	Cont:
		t++;
	}
	return -1;
	
}

int get_fmt(int table_num,char *fmt)
{
	strcpy(fmt,LaTbl[table_num].fmt);
	return  LaTbl[table_num].size;
}

