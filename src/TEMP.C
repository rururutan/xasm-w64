#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "asm.h"
#include "words.h"
#include "label.h"


extern char	tmp_file[];
extern int	TemporaryFile;
extern char	OpenTMPerrMsg[];
extern char	MallocerrMsg[];
extern char	Tempoverflow[];

static FILE *Tmp;
static char far *TmpBuff;
static unsigned TmpBuffPtr;
static int  Eof;


int init_temp_for_write(void)
{
	if(TemporaryFile){
		if((Tmp=fopen(tmp_file,"wt")) == NULL){	/* ������ ̧�� �-��� */
			err(OpenTMPerrMsg);
			return 0;
		}
		return 1;
	}else{
		if((TmpBuff=farmalloc(64000))==NULL){		/* ������ �ޯ̧ �m�� */
			err(MallocerrMsg);
			return 0;
		}
		TmpBuffPtr=0;
		return 1;
	}
}

int tmp_puts(char *str)
{
	char far *fptr;
	
	if(TemporaryFile){
		return fputs(str,Tmp);
	}else{
		fptr=TmpBuff+TmpBuffPtr;
		while(*str){
			*fptr++=*str++;
			TmpBuffPtr++;
			if(TmpBuffPtr>=64000) err(Tempoverflow);
		}
		return 1;
	}
}

int write_tmp_info(int linenum,char *str)
{
	char str1[16],str2[16];
	if(TemporaryFile){
		itoa(linenum,str1,10);
		setright(6,str2,str1);
		tmp_puts(str2);
	}
	tmp_puts(str);
	tmp_puts("\n");
}

void close_tmp_for_write(void)
{
	if(TemporaryFile){
		print_label(0,Tmp);
		fclose(Tmp);
	}else{
		*(TmpBuff+TmpBuffPtr)=EOF;
	}
}



int init_temp_for_read(void)
{
	if(TemporaryFile){
		if((Tmp=fopen(tmp_file,"rt")) == NULL){	/* ������ ̧�� �-��� */
			err(OpenTMPerrMsg);
			return 0;
		}
		return 1;
	}else{
		TmpBuffPtr=0;
		Eof=0;
		return 1;
	}
}



char *tmp_gets(char *str)
{
	char *s;
	char far *fptr;
	int  i;
	s=str;
	
	
	if(TemporaryFile){
		return fgets(str,256,Tmp);	/* �P�s���Q�T�U�������z���邱�Ƃ͂Ȃ� */
	}else{
		if(Eof) return NULL;			/* ����؂ł́A����ُ��͏������܂��*/
		for(i=0;i<6;i++) *str++=' ';	/* ���Ȃ� */
		fptr=TmpBuff+TmpBuffPtr;
		while((*fptr!='\n')&&(*fptr!=EOF)){
			*str++ = *fptr++;
			TmpBuffPtr++;
		}
		if(*fptr==EOF){
			Eof=1;
		}
		*str++='\n';
		TmpBuffPtr++;
		*str=0;
		return s;
	}
}


void close_tmp_for_read(void)
{
	if(TemporaryFile) fclose(Tmp);
}
