
%{
#include <stdio.h>
#include <stdlib.h>


#define ID_TKN      500
#define NUM_TKN     510

#define RELOP_TKN   520
#define RELOP_LT    521
#define RELOP_LE    522
#define RELOP_EQ    523
#define RELOP_NE    524
#define RELOP_GT    525
#define RELOP_GE    526

#define RELOP_DEQ   527
#define RELOP_NEQ   528



#define IF_TKN      610
#define ELSE_TKN    612

#define WHILE_TKN   613
#define DO_TKN      614
#define BREAK_TKN   615
#define TRUE_TKN    616
#define FALSE_TKN   617
#define BASIC_TKN   618

/*����ʹ�øñ���yylval�����ñ�������flex�Դ��ģ���������Ҫ�Լ������������*/
int yylval;

/*�ú������Խ��������ӵ����ű��У�����������û��ʵ�ָù���*/
int installID() ;

/*�ú������Խ���ֵ���ӵ���ֵ���У�����������û��ʵ�ָù���*/
int installNum() ;

/*��ο�������ҳ�е�˵��*/
/* http://www.ibm.com/developerworks/cn/linux/sdk/lex/index.html */

%}

delim    [" "\t\n]
ws       {delim}+
letter   [A-Za-z]
digit    [0-9]
id       {letter}({letter}|{digit})*
number   {digit}+(\.{digit}+)?(E[+-]?{digit}+)?
basic    "int"|"float"|"char"|"bool"

%%

{ws}       {/*no action and no return */ }
if         { printf("IF:%s\n",yytext);   return (IF_TKN); }
else       { printf("ELSE:%s\n",yytext); return (ELSE_TKN); }

true       { printf("TRUE:%s\n",yytext); return (TRUE_TKN); }
false      { printf("FALSE:%s\n",yytext); return (FALSE_TKN); }
while      { printf("WHILE:%s\n",yytext); return (WHILE_TKN); }
do         { printf("DO:%s\n",yytext); return (DO_TKN); }
break      { printf("BREAK:%s\n",yytext); return (BREAK_TKN); }



{basic}    { printf("BASIC:%s\n",yytext); return (BASIC_TKN); }

{id}       { yylval = (int) installID(); printf("ID:%s\n",yytext); return (ID_TKN); }
{number}   { yylval = (int) installNum();printf("Num:%s\n",yytext); return (NUM_TKN); } 


"<"        { yylval = RELOP_LT; printf("LT:%s\n",yytext);  return(RELOP_TKN); }
"<="       { yylval = RELOP_LE; printf("LE:%s\n",yytext);  return(RELOP_TKN); }
"="        { yylval = RELOP_EQ; printf("EQ:%s\n",yytext);  return(RELOP_TKN); }
"<>"       { yylval = RELOP_NE; printf("NE:%s\n",yytext);  return(RELOP_TKN); }
">"        { yylval = RELOP_GT; printf("GT:%s\n",yytext);  return(RELOP_TKN); }
">="       { yylval = RELOP_GE; printf("GE:%s\n",yytext);  return(RELOP_TKN); }

"=="       { yylval = RELOP_DEQ; printf("DEQ:%s\n",yytext);  return(RELOP_TKN); }
"!="       { yylval = RELOP_NEQ; printf("NEQ:%s\n",yytext);  return(RELOP_TKN); }


[\/][\*]([^\*])*[\*]([^\*\/](([^\*])*)[\*]|[\*])*(\/)  { printf("Remark:%s\n",yytext); }

.          { printf("%c:%c\n",yytext[0],yytext[0]); return yytext[0]; }

%%

/*�ú�������yyin������fflex��yyin��������Ӧ���ļ����дʷ�����*/
void BeginCompileOneFile( const char * filename )
{  
   yyin = fopen( filename, "r");
   fseek( yyin, 0, SEEK_SET );

/* ȱʡ����£�yyin �� yyout ��ָ���׼����������
   yyout = fopen( outputfilename, "w");
   fseek( yyout, 0, SEEK_SET );
*/
}

void EndCompileOneFile(void) 
{
   fclose(yyin);
   yyin = 0;
}

/*��������ķ���ֵ��1����ֹͣ������ ���������������������ļ���*/
int yywrap()
{
	return 1;
}

int installID() 
{
   return 1;
}

int installNum() 
{
   return 1;
}


/*��Ϊlex.yy.c��C����д�ģ�������Щ��������C���Եĺ������÷�ʽ�����
#ifdef __cplusplus
    extern "C" int yylex(void);
    extern "C" void BeginCompileOneFile( const char * filename );
    extern "C" void EndCompileOneFile( void);
#endif
*/


int main()
{ int token;
char filename[1000];

	printf("������Ҫ�����Դ�����ļ�����"); gets(filename);
	BeginCompileOneFile( filename );

	//��flexɨ�赽�ļ�ĩβ��yylex��������0
    while( ( token = yylex() ) > 0 ) ;

	EndCompileOneFile();

	getchar();
        return 0;
}









