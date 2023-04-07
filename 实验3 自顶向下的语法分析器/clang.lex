
%{

#include "lex.yy.h"

/*����ʹ�øñ���yylval�����ñ�������flex�Դ��ģ���������Ҫ�Լ������������*/
int yylval;
int lineNo=1;

/*��ο�������ҳ�е�˵��*/
/* http://www.ibm.com/developerworks/cn/linux/sdk/lex/index.html */

%}

delim    [" "\t]
ws       {delim}+
letter   [A-Za-z]
digit    [0-9]
id       {letter}({letter}|{digit})*
number   {digit}+(\.{digit}+)?(E[+-]?{digit}+)?
basic    "int"|"float"|"char"|"bool"

%%

{ws}       {/*no action and no return */ }
"\n"       {lineNo++;}
if         { printf("IF:%s\n",yytext);   return (IF_TKN); }
else       { printf("ELSE:%s\n",yytext); return (ELSE_TKN); }
break      { printf("BREAK:%s\n",yytext); return (BREAK_TKN); }
true       { printf("TRUE:%s\n",yytext); return (TRUE_TKN); }
false      { printf("FALSE:%s\n",yytext); return (FALSE_TKN); }
while      { printf("WHILE:%s\n",yytext); return (WHILE_TKN); }
do         { printf("DO:%s\n",yytext); return (DO_TKN); }
{basic}    { printf("BASIC:%s\n",yytext); return (BASIC_TKN); }
{id}       { printf("ID:%s\n",yytext); return (ID_TKN); }
{number}   { printf("Num:%s\n",yytext); return (NUM_TKN); } 

"<"        { yylval = RELOP_LT; printf("LT:%s\n",yytext);  return(RELOP_TKN); }
"<="       { yylval = RELOP_LE; printf("LE:%s\n",yytext);  return(RELOP_TKN); }
"=="        { yylval = RELOP_EQ; printf("EQ:%s\n",yytext);  return(RELOP_TKN); }
"<>"       { yylval = RELOP_NE; printf("NE:%s\n",yytext);  return(RELOP_TKN); }
">"        { yylval = RELOP_GT; printf("GT:%s\n",yytext);  return(RELOP_TKN); }
">="       { yylval = RELOP_GE; printf("GE:%s\n",yytext);  return(RELOP_TKN); }
"!="       { yylval = RELOP_NEQ; printf("NEQ:%s\n",yytext);  return(RELOP_TKN); }

[\/][\*]([^\*])*[\*]([^\*\/](([^\*])*)[\*]|[\*])*(\/)  { printf("��ע:%s\n",yytext); }

.          { printf("TKN:%s\n",yytext); return yytext[0]; }

%%

/*�ú�������yyin������fflex��yyin��������Ӧ���ļ����дʷ�����*/
void BeginCompileOneFile( const char * filename )
{  
   yyin = fopen( filename, "r");
   fseek( yyin, 0, SEEK_SET );

}

void EndCompileOneFile(void) 
{
   fclose(yyin);
   yyin = 0;
}

int yywrap()
{
	return 1;
}

/*���أ��ʷ�������ǰ�������кţ�һ���ھ䷨��������ʱʹ��*/
int GetLineNo(void)
{
    return lineNo;
}










