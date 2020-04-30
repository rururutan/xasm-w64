#include <stdio.h>
#include <stdlib.h>
#include "valstr.h"
#define  STACKMAX 32




#define isoperator(c) (	((c)=='+')||((c)=='-')||((c)=='*')||((c)=='/')|| \
						((c)=='<')||((c)=='>')||((c)=='&')||((c)=='|')|| \
						((c)=='(')||((c)==')')||((c)=='=')||((c)=='!') )


#define isoperand(c)  ( (((c)>='0')&&((c)<='9'))||(((c)>='a')&&((c)<='z'))|| \
						((c)=='_'))

struct EvalData{
	int	 prio;
	long val;
};

/* ｐｒｉｏ
	(		0xf0
	)		0x00
	- (uni)	0xe0
	+ (uni)	0x01
	*		0xc0
	/		0xc1
	+		0xb0
	-		0xb1
	<<		0xa0
	>>		0xa1
	<		0x90
	>		0x91
	<=		0x92
	>=		0x93
	=		0x80
	<>		0x81
	&		0x70
	|		0x50
	&&		0x40
	||		0x30
*/
	
static struct EvalData evaldata[STACKMAX];
static struct EvalData *Stack[STACKMAX];
static int  StackPtr;
static struct EvalData *Rpn[STACKMAX];
static int  RpnPtr;




static char* evalgetdata(unsigned char *src,struct EvalData *dest,int uflag,\
						unsigned pc,int pass,int *symbolerr)
{
	char str[32];
	int i;
	unsigned val;
	
	*symbolerr=0;

	if(*src==0) return NULL;

	if(*src=='\"'){
		if( (*(src+1)==0)||(*(src+2)!='\"') ) return NULL;
		dest->val=*(src+1);
		dest->prio=0x100;
		return src+3;
	}

	if(	 isoperand(*src) ){
		i=0;
		while(  isoperand(*src) )  str[i++]=*src++;
		str[i]=0;
		if((wordtoi(str,&val,pass)==0)&&(pass==1)){
			*symbolerr=1;
			dest->val=0;
		}else{
			dest->val=val;
		}
		dest->prio=0x100;
		return src;
	}else{
		if(*src=='$'){
			dest->val=pc;
			dest->prio=0x100;
			return src+1;
		}else{
			dest->val  = 0;
			switch(*src){
				case '(': dest->prio = 0xf0; break;
				case ')': dest->prio = 0x00; break;
				case '*': dest->prio = 0xc0; break;
				case '/': dest->prio = 0xc1; break;
				case '+': if(uflag) dest->prio = 0x01;
						  else		dest->prio = 0xb0;
						  break;
				case '-': if(uflag)	dest->prio = 0xe0;
						  else	    dest->prio = 0xb1;
						  break;
				case '>': switch(*(src+1)){
							case '>':dest->prio = 0xa1; src++; break;
							case '=':dest->prio = 0x93; src++; break;
						    default :dest->prio = 0x91;        break;
						  }
						  break;
				case '<': switch(*(src+1)){
							case '<':dest->prio = 0xa0; src++; break;
							case '>':dest->prio = 0x81; src++; break;
							case '=':dest->prio = 0x92; src++; break;
						    default :dest->prio = 0x90;        break;
						  }
						  break;
				case '&': switch(*(src+1)){
							case '&':dest->prio = 0x40; src++; break;
						    default :dest->prio = 0x70;        break;
						  }
						  break;
				case '|': switch(*(src+1)){
							case '|':dest->prio = 0x30; src++; break;
						    default :dest->prio = 0x50;        break;
						  }
						  break;
				case '=': dest->prio = 0x80; break;
				default : *symbolerr=2;
			}
			return src+1;
		}
	}
}




static int str2rpn(char *src,unsigned pc,int ps,int *symbolerr)
{
	int uflag=1;
	int n,i;
	int serr;

/*		スペースの削除はwords.cで
	char *ptr1,*ptr2;
	
	ptr1=ptr2=src;
	
	while(*ptr1){
		if(*ptr1!=' ')*ptr2++=*ptr1;
		ptr1++;
	}
	*ptr2=0;
*/
	
	*symbolerr=0;
	evaldata[0].prio=0xf0;
	Stack[0]=evaldata+0; StackPtr=1;
	RpnPtr=0;
	
	n=1;
	while(	(n<STACKMAX)&&
			((src=evalgetdata(src,evaldata+n,uflag,pc,ps,&serr))!=NULL)){
		if((evaldata[n].prio!=0x100)&&(evaldata[n].prio!=0x00))
			uflag=1;
		else
			uflag=0;
		n++;
		if(serr) *symbolerr=serr;
	}
	
	for(i=1;i<n;i++){
		while(
			((Stack[StackPtr-1]->prio & 0xff0) >= (evaldata[i].prio & 0xff0))
		  &&(Stack[StackPtr-1]->prio != 0xf0)
		){
			StackPtr--;
			Rpn[RpnPtr++]=Stack[StackPtr];
		}
		if(Stack[StackPtr-1]->prio == 0xf0){
			if(evaldata[i].prio==0x00){
				if(StackPtr==1) return 1;		/* error */
				StackPtr--;
				continue;
			}
		}
		if((evaldata[i].prio & 0xff0)==0) continue;
		Stack[StackPtr++]=evaldata+i;
	}
	while(StackPtr>1){
		StackPtr--;
		if(Stack[StackPtr]->prio==0xf0) return 1;	/* error */
		Rpn[RpnPtr++]=Stack[StackPtr];
	}
	return 0;
}



static long eval_rpn(void)
{
	long la,lb;
	int i;
	
	
	StackPtr=0;
	
	for(i=0;i<RpnPtr;i++){
		if(Rpn[i]->prio==0x100){
			Stack[StackPtr++]=Rpn[i];
		}else{
			if(Rpn[i]->prio==0xe0){
				if(StackPtr==0)   return 0;
				Stack[StackPtr-1]->val*=(-1);
				continue;
			}else{
				if(StackPtr<2)    return 0;
				la=Stack[StackPtr-2]->val;
				lb=Stack[StackPtr-1]->val;
				switch(Rpn[i]->prio){
					case 0xc0: la=la*lb; break;
					case 0xc1: la=la/lb; break;
					case 0xb0: la=la+lb; break;
					case 0xb1: la=la-lb; break;
					case 0xa0: la=la<<lb; break;
					case 0xa1: la=la>>lb; break;
					case 0x90: if(la<lb) la=1; else la=0; break;
					case 0x91: if(la>lb) la=1; else la=0; break;
					case 0x92: if(la<=lb) la=1; else la=0; break;
					case 0x93: if(la>=lb) la=1; else la=0; break;
					case 0x80: if(la==lb) la=1; else la=0; break;
					case 0x81: if(la!=lb) la=1; else la=0; break;
					case 0x70: la=la&lb; break;
					case 0x50: la=la|lb; break;
					case 0x40: if(la&&lb) la=1; else la=0; break;
					case 0x30: if(la||lb) la=1; else la=0; break;
				}
				Stack[StackPtr-2]->val = la;
				StackPtr--;
			}
		}
	}
	return Stack[0]->val;
}

long evaluation(char *str,unsigned pc,int pass,int *symbolerr)
{
	if(str2rpn(str,pc,pass,symbolerr)) return 0;
	return eval_rpn();
}
