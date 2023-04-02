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

%start  lines

%token  NUMBER

%left   '+' '-'
%left   '*' '/'
%right  UMINUS

%%
lines : lines expr '\n' { printf("%g\n", $2); }
      | lines '\n'
      | /*empty*/
      | error '\n' { yyerror("ǰ��һ�еı��ʽ����\n"); successFlag=0; yyerrok; }
      ;
expr  : expr '+' expr { $$ = $1 + $3;
                        printf("����ʽ��expr->expr+expr��%g=%g+%g\n",$$,$1,$3); }
      | expr '-' expr { $$ = $1 - $3; 
                        printf("����ʽ��expr->expr-expr��%g=%g-%g\n",$$,$1,$3); }
      | expr '*' expr { $$ = $1 * $3; 
                        printf("����ʽ��expr->expr*expr��%g=%g*%g\n",$$,$1,$3); }
      | expr '/' expr { $$ = $1 / $3; 
                        printf("����ʽ��expr->expr/expr��%g=%g/%g\n",$$,$1,$3); }
      | '(' expr ')'  { $$ = $2; 
                        printf("����ʽ��expr->(expr)��%g=(%g)\n",$$,$2);}
      | '-' expr %prec UMINUS { $$ = - $2; 
                        printf("����ʽ��expr-> -expr��%g=-%g\n",$$,$2);}
      | NUMBER        { $$ = $1;
                        printf("����ʽ��expr-> NUMBER��%f=%f\n",$$,$1 ); 
                        /* "%f"��"%g"���������������"%g"������������0, */ }
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





   