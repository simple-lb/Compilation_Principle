#include "lex.yy.h"

extern "C" void BeginCompileOneFile( const char * filename );
extern "C" void EndCompileOneFile(void);
extern "C" int yylex();
extern "C" int GetLineNo(void);

int lookahead;

void error()
{
    printf("��%d�г��־䷨��������! \n",GetLineNo());
	EndCompileOneFile();
	printf("�밴�س����˳���");getchar();
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
		printf("stmt ��	break ; \n");
        match(BREAK_TKN);
		match(';');
		return;
	}
	error();
}

void stmts()
{   /*���lookahead��First(stmt stmts)�е�һ���ս������ôstmts��stmt stmts����*/
    if( lookahead==ID_TKN ||lookahead==IF_TKN ||
//		lookahead==WHILE_TKN ||lookahead==DO_TKN ||
		lookahead==BREAK_TKN ||lookahead=='{' ) {
	    printf("stmts 	����stmt stmts\n");
	    stmt();
	    stmts();
	    return;
    }
    if( lookahead=='}' ) { /*���lookahead��stmts��һ��follow,��ôstmts ����empty����*/
        printf("stmts ����empty \n");
	    return;
    } 
	/*����*/
    error();
}

void Block()
{
    if( lookahead=='{') {
	    printf("block ��  { stmts }\n");
	    match( '{' );
        stmts();
        match( '}' );
    }
    else error();
}

void Program()
{
    if( lookahead == '{' ) {
	    printf("program �� block\n");
	    Block();
    }
    else
        error();
}

int main()
{ 
char filename[1000];

    printf("������Ҫ�����Դ�����ļ�����"); gets(filename);
    BeginCompileOneFile( filename );
   
    //��flexɨ�赽�ļ�ĩβ��yylex��������0
    lookahead = yylex();

	/*����Program()��ʼ�䷨����,Program���ķ�����ʼ����*/
    Program();

	if( lookahead == 0 ) 
	    printf("\n�䷨�����ɹ���\n");
	else
		error();

    EndCompileOneFile();
    printf("�밴�س����˳���");getchar();//����䷨����ʧ�ܣ��ʹ�error()���˳������������˳���

    return 0;
}
