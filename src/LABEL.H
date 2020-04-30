int  init_label(void);		/* ƒ‰ƒxƒ‹—Ìˆæ‚ÌŠm•Û 	*/

#ifdef DEBUG
	int print_define(void);
#endif

void reset_LabelSearchedFlag(void);

int get_LabelSearchedFlag(unsigned *buff);

int print_label(int adr_only,FILE *d);

int add_label(char *str,unsigned int val,int stus);

int entry_define(int defn,char *str);

int define_replace(int defn,char *a,char *b);

int get_label_val_ls(char *str,unsigned int *v);

int get_label_val_bs(char *str,unsigned int *v);

void sort_label(void);

int get_defn(char *str);

int init_cref(void);

int regist_cref_infomation(int line,int iline,unsigned n,unsigned *stus);

int output_cref(void);


