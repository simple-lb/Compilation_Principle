/*�����ܣ�����׺���ʽת��Ϊ��׺���ʽ�ķ�����
  �������룺��������(0~9)�Լ�'+'��'-"���ɵ���׺���ʽ
            Ϊ�˼򻯳�������Դʷ��������ܣ�����Ҫ����ʽ�в����пո�ȷָ�����
			����: �ڼ���������9-5+2
  ԭʼ�ķ�������
           expr --> expr + term
                  | expr - term
                  | term
           term --> 0
                  | 1
                  | 2
	              ......
                  | 9

  ������ݹ����﷨�Ƶ����壺
            ����ʽ                       �������
            expr --> term  rest       expr.syn = term.syn || rest.syn

            rest --> + term rest1     rest.syn = term.syn || '+' || rest1.syn
                   | - term rest1     rest.syn = term.syn || '-' || rest1.syn
   		           | ��               rest.syn = ""

            term --> 0                term.syn = "0"
                   | 1                term.syn = "1" 
                   | 2                term.syn = "2"
		           ......
                   | 9                term.syn = "9"
   ˵�����ۺ����Ե�Ӣ�ķ���Ϊ: synthesized attribute
         expr.syn��ʾexpr��һ���ۺ�����Ϊsyn�������Դ洢expr�ĺ�׺���ʽ��
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
using std::string;

char LookAhead;

void Match(char t)
{
	if( LookAhead==t ) 
		LookAhead = getchar(); //������ǰ����һ���ַ�
    else {
		printf("\n���ʽ����Match��������Ҫ������ַ�Ϊ%c������ʵ���������%c\n", t, LookAhead );
		exit(1); //��������
	}

}

void Term(string & TermSyn)  
{char t;
	if( LookAhead>='0' && LookAhead<='9' ) {
		t = LookAhead; Match(t); TermSyn = string(1,t); //���磺t='3',�򴴽�����3����
	}
    else {
		printf("\n���ʽ����Term��������Ҫ���������ַ��������������%c\n", LookAhead );
		exit(1); //��������
	}
}

void Rest(string & RestSyn)  
{string term_syn,rest_syn;
	switch( LookAhead ) {
	    case '+':
			Match('+'); Term(term_syn); Rest(rest_syn); // rest.syn = term.syn || '+' || rest1.syn
			RestSyn = term_syn + "+" + rest_syn;
			break;
		case '-':
			Match('-'); Term(term_syn); Rest(rest_syn); // rest.syn = term.syn || '-' || rest1.syn
			RestSyn = term_syn + "-" + rest_syn;
			break;
		default:   // rest --> ��   rest.syn = ""
			RestSyn = "";
			break;
	}
}

void Expr(string & ExprSyn) 
{string term_syn, rest_syn;
    Term(term_syn);
	Rest(rest_syn);
	ExprSyn = term_syn + rest_syn;
}

void main()
{string expr_syn;
	printf("��������׺���ʽ��");
	LookAhead = getchar();

	printf("���׺���ʽΪ��");
	Expr(expr_syn);
    printf( "%s", expr_syn.c_str() );

	if( LookAhead !='\n' ) {
	    //���磺3+45
		printf("\n����ı��ʽ���󣬴�����ַ���%c\n", LookAhead);
		exit(1);
	}

	printf("\n���ʽ�����ɹ���\n");

}