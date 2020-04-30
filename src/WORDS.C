#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"

#define MAXC 63

void  decap(char *ptr)		/* 大文字-小文字変換 ＴＡＢ-スペ－ス変換 */
{
	while((*ptr!=0)&&(*ptr!='\n')&&(*ptr!=';')){
		if(*ptr=='\"'){
			do{
				ptr++;
			}while((*ptr!='\"')&&(*ptr!=0)&&(*ptr!='\n')&&(*ptr!=';'));
			if(*ptr=='\"') ptr++;
		}else{
			if((*ptr >= 'A')&&(*ptr <= 'Z')) (*ptr)|=0x20;
			else if(*ptr == '\t') (*ptr)=' ';
			ptr++;
		}
	}
	*ptr=0;
}


void  detab(char *ptr)		/* ＴＡＢ-スペ－ス変換 */
{
	while((*ptr!='\n')&&(*ptr)){
		if(*ptr == '\t') (*ptr)=' ';
		ptr++;
	}
	*ptr=0;
}


/* 半角ｽﾍﾟｰｽとｶﾝﾏをﾃﾞﾘﾐﾀとした語取り出し */
char* get_word1(char *src,char *word,int *labelflag)
{
	while((*src==' ')||(*src==',')) src++;
	if(*src==0){ *word=0; return NULL; }
	
	while(  (*src!=' ')&&(*src!=',')&&(*src!=0)&&(*src!=':')  ){
		*word++ = *src++;
	}
	*word=0;
	if(*src==':') *labelflag=1; else *labelflag=0;
	if(*src!=0){
		src++;
	}
	return src;
}

/* 指定文字をﾃﾞﾘﾐﾀとした語取り出し */
/* 文字定数を除き、半角ｽﾍﾟｰｽを削除 */
char* get_word2(char *str,char *word,char dlm)
{
	*word=0;
	while((*str==' ')) str++;	/* 先頭のスペースの削除 			*/
	if(*str==0) return NULL;	/* 文字列が無かった 				*/
	while((*str!=dlm)&&(*str!=0)){
		if(*str=='\"'){
			if( (*(str+1)==' ')&&(*(str+2)=='\"') ){
				strcpy(word,"20h");
				word+=3;
				str+=3;
			}else{
 				*word++=*str++;
				while((*str!=0)&&(*str!='\"')) *word++=*str++;
				if(*str=='\"') *word++=*str++;
			}
			continue;
		}
		if(*str!=' ') *word++ = *str++;
		else str++;
	}
	*word=0;
	if(*str==dlm)str++;
	return str;
}


int get_word(char *src,char *label,char *mnem,int *opn,char *op[],char *opbuff)
/*---------------------------------------------------------------
　ｱｾﾝﾌﾞﾘ言語のための行解析
  行をラベル、ニモニック、オペランドにわける
  返値　bit3 bit2 bit1 bit0
         |    |            
         |    +--------------ニモニック
         +-------------------ラベル
-----------------------------------------------------------------*/
{
	int  labelflag,retv=0;
	int  equflag=0;
	char *ptr;
	static char word[128];
	int i;
	
	*label=0;
	*mnem =0;
	*opn=0;
	
	/* 大文字を小文字に変換 */
	decap(src);
	
	/* まず2番目の語を取り出してequ擬似命令行かどうか判断 */
	ptr=src;
	while((*ptr!=0)&&(*ptr==' '))ptr++;
	while((*ptr!=0)&&(*ptr!=' '))ptr++;
	while((*ptr!=0)&&(*ptr==' '))ptr++;
	if((*(ptr)=='e')&&(*(ptr+1)=='q')&&(*(ptr+2)=='u')&&(*(ptr+3)==' '))
		equflag=1; 
	
	ptr=src;
	if((ptr=get_word1(ptr,word,&labelflag))==NULL) return 0;
	if((labelflag)||(equflag)){
		strcpy(label,word);
		retv |= 0x8;
		if((ptr=get_word1(ptr,word,&labelflag))==NULL) return retv;
	}
	strcpy(mnem,word);
	retv |= 0x4;
	while(ptr=get_word2(ptr,word,',')){
		op[*opn]=opbuff;
		i=0;
		while(word[i]){
			*opbuff++ = word[i++];
		}
		*opbuff++ = 0;
		(*opn)++;
	}
	return retv;
}


void setright(int n,char *str,char *word)
/* 語を文字列に右詰めセット　規定桁までスペースで埋める */
{
	int i;
	i=strlen(word);
	while(i<n){
		*str++ = ' ';
		i++;
	}
	while(*word) *str++=*word++;
	*str=0;
}
