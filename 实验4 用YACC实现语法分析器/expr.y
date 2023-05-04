%{ 

#include <ctype.h>
#include <stdio.h>


/*YYSTYPE������ջ��Ԫ�ص����ͣ������������Ƕ���,yyparse��ʹ�ø����ʹ�������ջ�ȵ�*/
#define YYSTYPE double

int successFlag = 1;

/*yyerror������yacc�涨��һ����������yyparse�����л���øú��������ú����Ķ����������ṩ�ġ��ú�������*/
void yyerror( char * ErrStr )
{
    printf("������Ϣ:%s\n", ErrStr);
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

/*�����lex.yy.c������y.tab.c�У��ڹ����о�ֻ��Ҫy.tab.c����Ҫ����lex.yy.c������yylex�Ⱥ������ظ�����*/
#include "lex.yy.c"

int yyparse();  /*main����Ҫ����yyparse()���������ú����Ķ����ں��棬����Ҫ������(��������)*/

int main()
{ char filename[1000];

	printf("������Ҫ�����Դ�����ļ�����"); gets(filename);
      /*��Ϊlex.yy.c��������y.tab.c�У����Կ���ֱ��ʹ��BeginCompileOneFile������
        ���򣬾�Ҫ��main����ǰ��д����: void BeginCompileOneFile(const char *); */
 	BeginCompileOneFile( filename );

        if( yyparse()==0 && successFlag==1 ) 
            printf("Successful!\n");
        else
            printf("Failed!\n");

	EndCompileOneFile();

	getchar();
      return 0;
}





   