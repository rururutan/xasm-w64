#include <stdio.h>
const char CopyrightMsg[]
	= "XASM Version 1.75/W32\n Win32 porting 2019 Ru^3\n Original author 1993-96 H.Okada";
const char UsageMsg[]		= ">XASM [/s /bxxxx /c /t /fNAME] filename";
const char OptionerrMsg[]	= "Invalid command-line option.";
const char MallocerrMsg[]	= "Memory allocation error.";
const char WriteHEXerrMsg[]	= "Write hex file error.";
const char WriteBINerrMsg[]	= "Write binary file error.";
const char OpenTMPerrMsg[]	= "Can't open temporary file.";
const char OpenCRFerrMsg[]	= "Can't open cross reference file.";
const char OpenSRCerrMsg[]	= "Can't open source file.";
const char OpenHEXerrMsg[]	= "Can't open hex file.";
const char OpenBINerrMsg[]	= "Can't open binary file.";
const char OpenSYMerrMsg[]	= "Can't open sym file.";
const char OpenLSTerrMsg[]	= "Can't open list file.";
const char Tempoverflow[]	= "Temporary buffer overflow. Use '/t' option.";
const char Crefoverflow[]	= "Cref buffer overflow.";
char TableFileName[FILENAME_MAX]= "XASM.TBL";
const char TablereaderrMsg[]= "Read error : table file.";
const char *P1ErrorMsg[]	={" : Instruction or operand illegal.",	/*  0 */
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
const char LstOuterrMsg[]	= ".LST file write error.";
