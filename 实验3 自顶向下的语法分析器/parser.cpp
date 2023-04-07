#include "lex.yy.h"
extern "C" void BeginCompileOneFile( const char * filename );
extern "C" void EndCompileOneFile(void);
extern "C" int yylex();
extern "C" int GetLineNo(void);
#include "lex.yy.c"

int lookahead;
void expr();
void stmt();
void Block();

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

void factor() {
    if( lookahead == '(' ) {
        printf("factor �� ( expr ) \n");
        match('(');
        expr();
        match(')');
        return;
    }
    if( lookahead == ID_TKN ) {
        printf("factor �� id \n");
        match(ID_TKN);
        return;
    }
    if( lookahead == NUM_TKN ) {
        printf("factor �� num \n");
        match(NUM_TKN);
        return;
    }
    error();
}

void D() {
    if( lookahead == '*' ) {
        printf("D �� * factor \n");
        match('*');
        factor();
        return;
    }
    if( lookahead == '/' ) {
        printf("D �� / factor \n");
        match('/');
        factor();
        return;
    }
    error();
}

void term1() {
    if( lookahead == '*' || lookahead == '/' ) {
        printf("term�� �� D term�� \n");
        D();
        term1();
        return;
    }
    if( lookahead == '+' || lookahead == '-' || lookahead == ')' || lookahead == RELOP_TKN || lookahead == ';') {
        printf("term�� �� empty \n");
        return;
    }
    error();
}

void term() {
    if( lookahead == '(' || lookahead == ID_TKN || lookahead == NUM_TKN) {
        printf("term �� factor term�� \n");
        factor();
        term1();
        return;
    }
    error();
}

void C() {
    if( lookahead == '+' ) {
        printf("C �� + term \n");
        match('+');
        term();
        return;
    }
    if( lookahead == '-' ) {
        printf("C �� - term \n");
        match('-');
        term();
        return;
    }
    error();
}

void expr1() {
    if( lookahead == '+' || lookahead == '-' ) {
        printf("expr�� �� C expr�� \n");
        C();
        expr1();
        return;
    }
    if( lookahead == ')' || lookahead == RELOP_TKN || lookahead == ';') {
        printf("expr�� �� empty \n");
        return;
    }
    error();
}

void expr() {
    if( lookahead == '(' || lookahead == ID_TKN || lookahead == NUM_TKN ) {
        printf("expr �� term expr�� \n");
        term();
        expr1();
        return;
    }
    error();
}

void B()
{
    if(lookahead==RELOP_TKN) {
        printf("B �� relop expr \n");
        match(RELOP_TKN);
        expr();
        return;
    }
    if(lookahead==')') {
        printf("B �� empty \n");
        return;
    }
    error();
}

void Bool()
{
    if(lookahead=='(' || lookahead==ID_TKN || lookahead==NUM_TKN) {
        printf("bool �� expr B \n");
        expr();
        B();
        return;
    }
    error();
}

void A()
{
    if(lookahead==ELSE_TKN) {
        printf("A �� else stmt; \n");
        match(ELSE_TKN);
        stmt();
        return;
    }
    if(lookahead==';') {
        printf("A �� empty \n");
        return;
    }
    error();
}

void stmt()
{
    if(lookahead==ID_TKN) {
		printf("stmt �� id = expr ; \n");
        match(ID_TKN);
        match('=');
        expr();
		match(';');
		return;
	}
    if(lookahead==IF_TKN) {
		printf("stmt �� if (bool) stmt A ; \n");
        match(IF_TKN);
        match('(');
        Bool();
        match(')');
        stmt();
        A();
		return;
	}
    if(lookahead==WHILE_TKN) {
		printf("stmt �� while (bool) stmt ; \n");
        match(WHILE_TKN);
        match('(');
        Bool();
        match(')');
        stmt();
		return;
	}
    if(lookahead==DO_TKN) {
		printf("stmt �� do stmt while (bool) ; \n");
        match(DO_TKN);
        stmt();
        match(WHILE_TKN);
        match('(');
        Bool();
        match(')');
		match(';');
		return;
	}
	if(lookahead==BREAK_TKN) {
		printf("stmt �� break ; \n");
        match(BREAK_TKN);
		match(';');
		return;
	}
    if( lookahead == '{' ) {
	    printf("stmt �� block\n");
	    Block();
        return;
    }
	error();
}

void stmts()
{   /*���lookahead��First(stmt stmts)�е�һ���ս������ôstmts��stmt stmts����*/
    if( lookahead==ID_TKN ||lookahead==IF_TKN ||
		lookahead==WHILE_TKN ||lookahead==DO_TKN ||
		lookahead==BREAK_TKN ||lookahead=='{' ) {
	    printf("stmts �� stmt stmts\n");
	    stmt();
	    stmts();
	    return;
    }
    if( lookahead=='}' ) { /*���lookahead��stmts��һ��follow,��ôstmts ����empty����*/
        printf("stmts �� empty \n");
	    return;
    } 
	/*����*/
    error();
}

void Block()
{
    if( lookahead=='{') {
	    printf("block �� { stmts }\n");
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
