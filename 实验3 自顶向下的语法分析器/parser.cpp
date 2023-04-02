#include "lex.yy.h"

extern "C" void BeginCompileOneFile( const char * filename );
extern "C" void EndCompileOneFile(void);
extern "C" int yylex();
extern "C" int GetLineNo(void);

int lookahead;

void error()
{
    printf("在%d行出现句法分析错误! \n",GetLineNo());
	EndCompileOneFile();
	printf("请按回车键退出！");getchar();
    exit(1);
}

void match(int TokenID)
{
    if( lookahead == TokenID )
        lookahead = yylex();
    else 
        error();
}

void stmt()
{
	if(lookahead==BREAK_TKN) {
		printf("stmt →	break ; \n");
        match(BREAK_TKN);
		match(';');
		return;
	}
	error();
}

void stmts()
{   /*如果lookahead是First(stmt stmts)中的一个终结符，那么stmts→stmt stmts可用*/
    if( lookahead==ID_TKN ||lookahead==IF_TKN ||
//		lookahead==WHILE_TKN ||lookahead==DO_TKN ||
		lookahead==BREAK_TKN ||lookahead=='{' ) {
	    printf("stmts 	→　stmt stmts\n");
	    stmt();
	    stmts();
	    return;
    }
    if( lookahead=='}' ) { /*如果lookahead是stmts的一个follow,那么stmts →　empty可用*/
        printf("stmts →　empty \n");
	    return;
    } 
	/*否则*/
    error();
}

void Block()
{
    if( lookahead=='{') {
	    printf("block →  { stmts }\n");
	    match( '{' );
        stmts();
        match( '}' );
    }
    else error();
}

void Program()
{
    if( lookahead == '{' ) {
	    printf("program → block\n");
	    Block();
    }
    else
        error();
}

int main()
{ 
char filename[1000];

    printf("请输入要编译的源程序文件名："); gets(filename);
    BeginCompileOneFile( filename );
   
    //当flex扫描到文件末尾，yylex函数返回0
    lookahead = yylex();

	/*调用Program()开始句法分析,Program是文法的起始符号*/
    Program();

	if( lookahead == 0 ) 
	    printf("\n句法分析成功！\n");
	else
		error();

    EndCompileOneFile();
    printf("请按回车键退出！");getchar();//如果句法分析失败，就从error()中退出，不在这里退出。

    return 0;
}
