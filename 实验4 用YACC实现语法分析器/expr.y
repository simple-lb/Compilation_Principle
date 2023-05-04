%{ 

#include <ctype.h>
#include <stdio.h>


/*YYSTYPE是属性栈的元素的类型，该类型由我们定义,yyparse会使用该类型创建属性栈等等*/
#define YYSTYPE double

int successFlag = 1;

/*yyerror函数是yacc规定的一个函数，在yyparse函数中会调用该函数，但该函数的定义是我们提供的。该函数如下*/
void yyerror( char * ErrStr )
{
    printf("错误信息:%s\n", ErrStr);
}

%}

%start  program

%token  NUMBER
%token  ID_TKN
%token  RELOP_TKN
%token  RELOP_LT 
%token  RELOP_LE
%token  RELOP_EQ
%token  RELOP_NE
%token  RELOP_GT
%token  RELOP_GE
%token  IF_TKN
%token  ELSE_TKN  
%token  BREAK_TKN 
%token  WHILE_TKN
%token  DO_TKN

%left   '+' '-'
%left   '*' '/'
%right  UMINUS

%%
program : block { printf("program -> block\n"); }
      ;
block : '{' stmts '}' { printf("block -> {stmts}\n"); }
      ;
stmts : stmt stmts { printf("stmts -> {stmts}\n"); }
      |            { printf("stmts -> empty\n"); }
      ;
stmt  : ID_TKN '=' expr ';' { printf("stmt -> id = expr;\n"); }
      | IF_TKN '(' bool ')' stmt A  { printf("stmt -> if (bool) stmt A\n"); }
      | WHILE_TKN '(' bool ')' stmt   { printf("stmt -> while (bool) stmt\n"); }
      | DO_TKN stmt WHILE_TKN '(' bool ')' ';'  { printf("stmt -> do stmt while (bool); \n"); }
      | BREAK_TKN ';' { printf("stmt -> break;\n"); }
      | block   { printf("stmt -> block\n"); }
      ;
A     : ELSE_TKN stmt { printf("A -> else stmt\n"); }
      |               { printf("A -> empty\n"); }
      ;
bool  : expr B { printf("bool -> expr B\n"); }
      ;
B     : RELOP_TKN expr { printf("B -> relop expr\n"); }
      |                { printf("B -> empty\n"); }
      ;
expr  : term expr1 { printf("expr -> term expr1\n"); }
      ;
expr1 : C expr1 { printf("expr1 -> C expr1\n"); }
      |         { printf("expr1 -> empty\n"); }
      ;
C     : '+' term { printf("C -> + term\n"); }
      | '-' term { printf("C -> - term\n"); }
      ;
term  : factor term1 { printf("term -> factor term1\n"); }
      ;
term1 : D term1 { printf("term1 -> D term1\n"); }
      |         { printf("term1 -> empty\n"); }
      ;
D     : '*' factor { printf("D -> * factor\n"); }
      | '/' factor { printf("D -> / factor\n"); }
      ;
factor : '(' expr ')' { printf("factor -> (expr)\n"); }
      | ID_TKN { printf("factor -> id\n"); }
      | NUMBER { printf("factor -> num\n"); }
      ;
%%

/*如果把lex.yy.c包含在y.tab.c中，在工程中就只需要y.tab.c，不要再有lex.yy.c，否则yylex等函数会重复定义*/
#include "lex.yy.c"

int yyparse();  /*main函数要调用yyparse()函数，但该函数的定义在后面，所以要先声明(才能引用)*/

int main()
{ char filename[1000];

	printf("请输入要编译的源程序文件名："); gets(filename);
      /*因为lex.yy.c被包含在y.tab.c中，所以可以直接使用BeginCompileOneFile函数。
        否则，就要在main函数前面写声明: void BeginCompileOneFile(const char *); */
 	BeginCompileOneFile( filename );

        if( yyparse()==0 && successFlag==1 ) 
            printf("Successful!\n");
        else
            printf("Failed!\n");

	EndCompileOneFile();

	getchar();
      return 0;
}





   