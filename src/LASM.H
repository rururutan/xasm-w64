int entry_table(char *str);
int init_lasm(void);
#ifdef DEB
	void print_table(void);
#endif
int pseudo_inst(char *word);
int lasm(char *mnem,int opn,char *op[],char *match[]);
int get_fmt(int table_num,char *fmt);


