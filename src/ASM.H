#define LABELLENGTH	16		/* 有効ラベル長 */
#define MAXLABEL 1500		/* 最大ラベル数 */
#define MAXLABELSTR 0x8000	/* ラベル文字列テーブル */
#define MAXTABLE 512		/* テ－ブルの最大数 */
#define MAXTABLEOPN 3	/* テ－ブルのオペランドの最大数 */
#define MAXMNEM  256		/* ニモニックの最大数 */

#define MAXDEFSTR 10000
#define MAXDEF    512

#define DATA_VALUE		0x8000
#define ADDRESS_VALUE	0x0000

void err(char *mess);
void p1_err(int ln,int iln,int n);
