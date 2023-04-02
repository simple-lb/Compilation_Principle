
%{
#include <stdio.h>
#include <stdlib.h>



/*请参考下面网页中的说明*/
/* http://www.ibm.com/developerworks/cn/linux/sdk/lex/index.html */

%}

delim    [" "\t]
ws       {delim}+
digit    [0-9]
id       {letter}({letter}|{digit})*
number   {digit}+(\.{digit}+)?(E[+-]?{digit}+)?

%%

{ws}       {/*no action and no return */ }
{number}   { sscanf( yytext, "%lf", &yylval ); return (NUMBER); } 
\n         { return yytext[0]; }

[\/][\*]([^\*])*[\*]([^\*\/](([^\*])*)[\*]|[\*])*(\/)  { printf("备注:%s\n",yytext); }

.          { return yytext[0]; }

%%

/*该函数设置yyin变量，fflex对yyin变量所对应的文件进行词法分析*/
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









