int init_temp_for_write(void);
int tmp_puts(char *str);
void write_tmp_info(int linenum,char *str);
void close_tmp_for_write(void);

int init_temp_for_read(void);
char *tmp_gets(char *str);
void close_tmp_for_read(void);
