#include <stdlib.h>
#include <stdio.h>
#include "asm.h"
#include "label.h"

extern int St;
/* extern unsigned int ProgramCounter; */

	static int hextoi1(char ch)
	{
		if((ch>='0')&&(ch<='9')) return ch-'0';
		if((ch>='a')&&(ch<='f')) return (ch-'a'+10);
		if((ch>='A')&&(ch<='F')) return (ch-'A'+10);
		return -1;
	}

char *itohex(char *str,unsigned int i,int d)
	/* ®”i ‚ğ dŒ…‚Ì16i•¶š—ñ‚É•ÏŠ· */
{
	static char hexstr[]={'0','1','2','3','4','5','6','7',
	                      '8','9','A','B','C','D','E','F'};
	str+=d;
	*str--=0;
	while(d--){
		*str--=hexstr[i&0xf];
		i>>=4;
	}
	return str++;
}


int hextoi(char *str,unsigned int *v) /* 16i”‚ğ®”‚É•ÏŠ· */
{
	int n;
	*v=0;
	while(((*str)!='h')&&((*str)!=0)){
		(*v)<<=4;
		if((n=hextoi1(*str++))<0) return 0;
		(*v)|=n;
		if((*v)&0xf000) break;
	}
	return 1;
}

	static int btoi(char *str,unsigned int *v)
	{
		*v=0;
		while((*str)!='b'){
			(*v)<<=1;
			if((*str!='0')&&(*str!='1')) return 0;
			(*v)|=(*str=='1'); str++;
			if((*v)&0x8000) break;
		}
		return 1;
	}
	
	static int atoi_1(char *str,unsigned int *v) /* 10i”‚ğ®”‚É•ÏŠ· */
	{
		*v=0;
		while(*str){
			(*v)*=10;
			if((*str>='0')&&(*str<='9')) (*v)+=(*str)-'0'; else return 0;
			str++;
		}
		return 1;
	}
	

int wordtoi(char *str,unsigned int *v,int pass)
{
	int i=0,j;
	char c;
	
	if(*str==0)  return 0;

	if(!((*str>='0')&&(*str<='9'))){
		if(pass)return(get_label_val_bs(str,v));
		else    return(get_label_val_ls(str,v));
	}
	while(str[i])i++;
	c = str[i-1];
	switch(c){
		case 'h': j=hextoi(str,v); break;
		case 'b': j=btoi(str,v);   break;
		default:  j=atoi_1(str,v);
	}
	return j;
}


/*

static char *getvalword(char *src,char *dest)
{
	while((*src=='$')||((*src>='0')&&(*src<='9'))||((*src>='a')&&(*src<='z')))
		*dest++ = *src++;
	*dest=0;
	return src;
}

int valstr(char *str,unsigned int *v,unsigned pc)
{
	char *ptr;
	char word[64];
	char c='+';
	unsigned b;
	*v=0;
	ptr=str;
	while(c){
		ptr=getvalword(ptr,word);
		if(*str=='$'){
			b=pc;
		}else{
			if(wordtoi(word,&b,1)==0) return 0;
		}
		switch(c){
			case '+':(*v)+=b; break;
			case '-':(*v)-=b; break;
			case '>':(*v)>>=b; break;
			case '<':(*v)<<=b; break;
			case '&':(*v)&=b; break;
			case '|':(*v)|=b; break;
		}
		c=*ptr++;
	}
	return 1;
}

*/