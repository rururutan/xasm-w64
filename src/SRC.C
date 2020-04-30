#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char    	srcfile[FILENAME_MAX];

static char     inc_src[64];

static FILE *s[8];
static int filelevel=0;


int open_src(void)
{
	if((s[0]=fopen(srcfile,"rt")) == NULL) return 0;
	else return 1;
}

int open_inc(char *filename)
{
	filelevel++;
	if((s[filelevel]=fopen(filename,"rt")) == NULL) return 0;
	else return 1;
}

void close_src(void)
{
	do{
		fclose(s[filelevel]);
	}while(filelevel--);
}

char *src_gets(char *buff,int n,int *level)
{
	char *c;
	for(;;){
		c=fgets(buff,n,s[filelevel]);
		if(c==NULL){			/* s[filelevel]が空だったら・・ */
			if(filelevel==0){	/* filelevel==0ならおおもとのｿｰｽが空である */
				*level=0;
				return NULL;
			}else{						/* で、なければ・・ */
				fclose(s[filelevel]);	/* s[filelevel]をｸﾛｰｽﾞして */
				filelevel--;			/* filelevelを一つ落とす   */
			}
		}else{
			break;
		}
	}
	*level= filelevel;
	return c;
}
