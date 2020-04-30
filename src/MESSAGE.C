char CopyrightMsg[]
	= "XASM Version 1.75 Copyright (c) 1993-96 H.Okada";
char UsageMsg[]			= ">XASM [/s /bxxxx /c /t /fNAME] filename";
char OptionerrMsg[]		= "Invalid command-line option.";
char MallocerrMsg[]		= "Memory allocation error.";
char WriteHEXerrMsg[]	= "Write hex file error.";
char WriteBINerrMsg[]	= "Write binary file error.";
char OpenTMPerrMsg[]	= "Can't open temporary file.";
char OpenCRFerrMsg[]	= "Can't open cross reference file.";
char OpenSRCerrMsg[]	= "Can't open source file.";
char OpenHEXerrMsg[]	= "Can't open hex file.";
char OpenBINerrMsg[]	= "Can't open binary file.";
char OpenSYMerrMsg[]	= "Can't open sym file.";
char OpenLSTerrMsg[]	= "Can't open list file.";
char Tempoverflow[]     = "Temporary buffer overflow. Use '/t' option.";
char Crefoverflow[]     = "Cref buffer overflow.";
char TableFileName[64]	= "XASM.TBL";
char TablereaderrMsg[]	= "Read error : table file.";
char *P1ErrorMsg[]		={" : Instruction or operand illegal.",	/*  0 */
						  " : Syntax error.",					/*  1 */
						  " : Lable registration error.",		/*  2 */
						  " : Illegal symbol or number.",		/*  3 */
						  " : Read error : temporary file.",	/*  4 */
						  " : Warning. Out of range.",			/*  5 */
						  " : Warning. Label mismatch.",		/*  6 */
						  " : Too much data.",					/*  7 */
						  " : Can't open include file.",		/*  8 */
						  " : Illegal operator."     			/*  9 */
						 };
char LstOuterrMsg[]		= ".LST file write error.";
