typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;

void  decap(char *ptr);		/* ‘å•¶š-¬•¶š•ÏŠ· ‚s‚`‚a-ƒXƒy|ƒX•ÏŠ· */
void  detab(char *ptr);		/* ‚s‚`‚a-ƒXƒy|ƒX•ÏŠ· */


char* get_word1(char *src,char *word,int *labelflag);
char* get_word2(char *str,char *word,char dlm);
/* char* get_word4(char *src,int n,char *buff); */

int get_word(char *src,char *label,char *mnem,int *opn,char *op[],char *buff);
/*---------------------------------------------------------------
 devide line_string to Label,Mnemonic,1st operand,2nd operand
  return bit3 bit2 
         |    |    
         |    +-------------- mnemonic
         +------------------- label
-----------------------------------------------------------------*/

void setright(int n,char *str,char *word);
/* Right_Set word to str */
