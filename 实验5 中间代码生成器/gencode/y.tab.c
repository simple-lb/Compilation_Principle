#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/02/91\n\
 Modified 11/4/96 by Wensong Zhang to make getenv() function right\n\
 and to remove the warning of unreferenced yyerrlab and yynewerror labels";
#endif
#define YYBYACC 1
#line 1 ".\compile.y"
 
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

int LineNo = 1; /*��ǰlookahead��ָ����ַ����ڵ��кţ�����ǰ���뵽���к�*/

int CompileFailed = 0;

void yyerror( char * ErrStr )
{
    CompileFailed = 1; /*����ʧ�ܱ�־*/
    printf("������Ϣ:%s, �к�:%d\n", ErrStr, LineNo);
}



/*�����ͳ����Ļ�������BASIC*/
#define CHAR     1
#define INT      2
#define FLOAT    3
#define BOOL     4

#define CHAR_WIDTH  1
#define INT_WIDTH   4
#define FLOAT_WIDTH 8  
#define BOOL_WIDTH  1

/*****************************���棺���ű�Ķ������غ���*******************************/

/*���������Ȳ�����ID_MAX_LEN ���ַ�*/
#define ID_MAX_LEN   64

/*���һ����ʶ��*/
struct SymbolElem {
    char name[ ID_MAX_LEN + 1 ]; /*������(���������)���Ȳ�����ID_MAX_LEN ���ַ�*/
    int type; /*�������������������int, �������ֻ��������ͣ���ʵ�ʵı������У�����Ҫ�������ṹ*/
    int  addr;      /*Ϊ�ñ�������Ŀռ���׵�ַ*/
	int  width;     /*�ñ����Ŀ�ȣ���ռ�ö��ٸ��ֽ�*/
    struct SymbolElem * next;  /*ָ����һ����ʶ��*/
};

/*��ʶ����*/
typedef struct SymbolList{
    struct SymbolElem * head;  /*ָ����ű�������ʵ�֣��ĵ�һ����㣬û��ͷ���,��ʼ��ΪNULL*/
    struct SymbolList * prev; /*��һ��ķ��ű�*/
    int beginaddr; /*�÷��ű��з������������ʱ�����ռ�Ŀ�ʼ��ַ*/
    int endaddr;    /*�÷��ű��з������������ʱ�����ռ�Ľ�����ַ*/
                   /*beginaddr~endaddr�Ŀռ��Ÿ÷��ű�����б�������ʱ����*/
} * SymbolList;  /*���ű�*/

SymbolList TopSymbolList=NULL; /*ȫ�ֱ�������ŵ�ǰ�ķ��ű�����ǰ�������ķ��ű�,��ӦΪ���ϵ�top*/

/*����������һ���µķ��ű�SymbolList�������ϵ�Env����PrevList�������һ����ű�*/
SymbolList CreateSymbolList( SymbolList PrevList, int StartAddr )
{ SymbolList list;
    list = (SymbolList) malloc( sizeof(struct SymbolList) );
    memset( list, 0, sizeof( struct SymbolList ) );
    list->prev = PrevList;
	list->endaddr = list->beginaddr = StartAddr;

    return list;
}

void DestroySymbolList( SymbolList List )
{struct SymbolElem * p, *q;
    
    if( List == NULL) return;
    p = List->head;
    while( p!=NULL ) {
        q = p->next; free(p); p=q;
    }
    free(List);    
}

/*�ڷ��ű�List�в����Ƿ���ڱ�ʶ��IdName��������ڣ��򷵻ظý��ָ�룬���򷵻ؿ�*/
struct SymbolElem * LookUpSymbolList( SymbolList List, char * IdName )
{struct SymbolElem * p;
    if( List==NULL ) return NULL;
    for( p = List->head; p!=NULL; p = p->next ) 
        if( strcmp( p->name, IdName ) == 0 ) break;
    return p;
}

/*�ӷ��ű�List��ʼ�����ϵ�����һ����ű��в����Ƿ���ڱ�ʶ��IdName��������ڣ��򷵻ظý��ָ�룬���򷵻ؿ�*/
struct SymbolElem * LookUpAllSymbolList( SymbolList List, char * IdName )
{ SymbolList env;
struct SymbolElem * p;
    env = List;
    while( env!=NULL ) {
        p = LookUpSymbolList( env, IdName );
        if(  p != NULL ) return p; /*�ҵ��÷���*/
        env = env->prev;
    }
    return NULL;
}


/*����һ���µķ��Ž��,����ӵ����ű��У����󷵻ظý��ָ��*/
struct SymbolElem * AddToSymbolList( SymbolList List, char * IdName,int IdType, int Width )
{struct SymbolElem * p;

    p = (struct SymbolElem *) malloc( sizeof(struct SymbolElem) );

    strcpy( p->name, IdName );
    p->type = IdType;
	p->width = Width;
	p->addr = List->endaddr;
	List->endaddr += Width;

    p->next = List->head;  /*���ñ�ʶ����ӵ����ű��ͷ*/
    List->head = p;

    return p;    
}

void PrintSymbolList( SymbolList List )
{struct SymbolElem * p;
    printf("***********************�����б�*************************\n");
    if( List ==NULL ) return ;
    for( p=List->head; p!=NULL; p=p->next ) {
        printf("������:%s, ����:", p->name);
		switch( p->type ) {
            case CHAR : printf("char");  break;
            case INT  : printf("int");   break;
            case FLOAT: printf("float"); break;
            case BOOL : printf("bool");  break;
		}
        printf(", ��ַ:%d, ���:%d\n", p->addr, p->width );
	}
    printf("*************�ñ����б�ռ��%d���ֽڿռ�***************\n", List->endaddr - List->beginaddr);
}

/*����һ����ʱ����,������ʱ�����ĵ�ַ����ʱ����������*/
int NewTemp( SymbolList List, char Name[], int Width )
{ static int TempID = 1;
  int addr;
    sprintf( Name, "T%d", TempID++ ); /*����T1��T2��*/
	addr = List->endaddr;
    List->endaddr += Width;
  
    return addr;
}

/*****************************���棺���ű�Ķ������غ���*****************************/




/*****************************���棺������Ķ������غ���*******************************/

union ConstVal {
        char    ch;    /*����ַ�����*/
        int     n;     /*������ͳ�������true=1��false=0 */
        double  f;     /*��Ÿ���������*/
};	

/*���һ������*/
struct ConstElem {
    char str[ID_MAX_LEN + 1 ]; /*�ñ������ڴ洢�������ı���ʽ����ʾ��ʱ���õ�,ʵ�ʵı���ϵͳ����Ҫ*/	   
    int type; /*�������������������int*/
    union ConstVal value;
    int  addr;      /*Ϊ�ó�������Ŀռ���׵�ַ*/
	int  width;     /*�ñ����Ŀ�ȣ���ռ�ö��ٸ��ֽ�*/
    struct ConstElem * next;  /*ָ����һ������*/
};

/*������*/
struct ConstList{
    struct ConstElem * head;  /*ָ������������ʵ�֣��ĵ�һ����㣬û��ͷ���,��ʼ��ΪNULL*/
    int beginaddr;  /*�÷��ű��з���������ռ�Ŀ�ʼ��ַ*/
    int endaddr;    /*�÷��ű��з���������ռ�Ľ�����ַ*/
                   /*beginaddr~endaddr�Ŀռ��Ÿó���������г���*/
} ConstList ;  /*������ȫ�ֱ�����ע����������ֻ��Ҫһ��������**/


/*���������س�����*/
void CreateConstList( int StartAddr )
{ 
	ConstList.head = NULL;
	ConstList.endaddr = ConstList.beginaddr = StartAddr;
}

void DestroyConstList( void )
{struct ConstElem * p, *q;
    
    p = ConstList.head;
    while( p!=NULL ) {
        q = p->next; free(p); p=q;
    }
	memset( &ConstList, 0, sizeof(struct ConstList) );
}

/*�ڳ�����ConstList�в����Ƿ���ڳ�����������ڣ��򷵻ظý��ָ�룬���򷵻ؿ�*/
struct ConstElem * LookUpConstList( int ConstType, union ConstVal ConstValue, int Width )
{struct ConstElem * p;
    for( p = ConstList.head; p!=NULL; p = p->next ) 
        if( p->type == ConstType && memcmp( &p->value,&ConstValue, Width) == 0 )  break;
	
    return p;
}


/*����һ���µĳ������,����ӵ��������У����󷵻ظý��ָ��*/
struct ConstElem * AddToConstList( char * Str, int ConstType, union ConstVal ConstValue, int Width )
{struct ConstElem * p;

    p = (struct ConstElem *) malloc( sizeof(struct ConstElem) );

    strcpy( p->str, Str );
    p->type = ConstType;
    p->value = ConstValue;
	p->width = Width;

	p->addr = ConstList.endaddr;
	ConstList.endaddr += Width;

    p->next = ConstList.head;  /*���ó�����ӵ��������ͷ*/
    ConstList.head = p;

    return p;    
}

void PrintConstList(void)
{struct ConstElem * p;
    printf("***********************�����б�*************************\n");
    for( p=ConstList.head; p!=NULL; p=p->next ) {
	    printf("����:%s, ����:", p->str);
		switch( p->type ) {
            case CHAR : printf("char");  break;
            case INT  : printf("int");   break;
            case FLOAT: printf("float"); break;
            case BOOL : printf("bool");  break;
		}
        printf(", ��ַ:%d, ���:%d\n", p->addr, p->width );
	}
    printf("**************�ó����б�ռ��%d���ֽڿռ�***************\n", ConstList.endaddr - ConstList.beginaddr);
}

/*****************************���棺������Ķ������غ���*****************************/





/********************************����:��Ԫʽ�Ķ���ͺ���****************************/

/* ���ͼӼ��˳� */
#define OIntAdd          1001
#define OIntSub          1002
#define OIntMultiply     1003
#define OIntDivide       1004

/* �������Ӽ��˳� */
#define OFloatAdd        1011
#define OFloatSub        1012
#define OFloatMultiply   1013
#define OFloatDivide     1014

/*��ֵa=b*/
#define OIntEvaluation   1021
#define OFloatEvaluation 1022
#define OCharEvaluation  1023
#define OBoolEvaluation  1024

/* ������goto��� */
#define OGoto            1031

/* if a op b goto ��� */
#define OGTGoto          1041
#define OGEGoto          1042
#define OLTGoto          1043
#define OLEGoto          1044
#define OEQGoto          1045
#define ONEQGoto         1046

/*����ת�������*/
#define OCharToInt       1051
#define OCharToFloat     1052
#define OIntToFloat      1053
#define OIntToChar       1054
#define OFloatToChar     1055
#define OFloatToInt      1056
#define OCharToBool      1057
#define OIntToBool       1058
#define OFloatToBool     1059
#define OBoolToChar      1060
#define OBoolToInt       1061
#define OBoolToFloat     1062

#define HALT             	1063

/* ȡ�� */
#define OIntUminus			1071
#define OFloatUminus		1072

/*��Ԫʽ���ݽṹ*/
struct Quadruple {
    int op; /*�����*/
    int arg1; /*��ŵ�һ�������ĵ�ַ�������Ǳ�������ʱ�����ĵ�ַ*/
    int arg2;
    int arg3;/*��ŵ����������ĵ�ַ�������Ǳ�������ʱ�����ĵ�ַ������������Ԫʽ�ĵ�ַ(Goto �ĵ�ַ����)*/
    char arg1name[ID_MAX_LEN + 1]; /*������Ҫ��������ʾʱ����ʾarg1��Ӧ�ı�������ʱ����������(���еĻ���*/
    char arg2name[ID_MAX_LEN + 1]; /*������Ҫ��������ʾʱ����ʾarg2��Ӧ�ı�������ʱ����������(���еĻ���*/
    char arg3name[ID_MAX_LEN + 1]; /*������Ҫ��������ʾʱ����ʾarg3��Ӧ�ı�������ʱ����������(���еĻ���*/
};

/*��Ԫʽ��*/
struct QuadTable {
    int startaddr; /*��Ԫʽ��ʼ��ŵĵ�ַ,����100*/
    struct Quadruple * base; /*ָ��һ���ڴ棬������Ŷ����Ԫʽ����base[0]��ʼ���*/
    int size; /*base�п��Դ�ŵ���Ԫʽ�ĸ���*/
    int len; /*base[len]����һ����ԪʽҪ��ŵĿռ�*/

};

struct QuadTable QuadTable; /*ֻ��Ҫһ����Ԫʽ��*/

void CreateQuadTable(int StartAddr)
{
    QuadTable.startaddr = StartAddr; 
    QuadTable.size = 1000; /* һ��ʼ������Դ��1000����Ԫʽ*/
    QuadTable.base = ( struct Quadruple *)malloc( QuadTable.size * sizeof(struct Quadruple) );
    QuadTable.len = 0;
}

void DestroyQuadTable( void )
{
    QuadTable.startaddr = 0; 
    QuadTable.size = 0;
    if( QuadTable.base != NULL) free(QuadTable.base); 
    QuadTable.len = 0;   
}

/*��Arg1�Ǳ�������ʱ����ʱ��Arg1Name�Ǹñ���������,������ʾʱʹ�ã����������ͬ */
int Gen( int Op, int Arg1, int Arg2, int Arg3, char *Arg1Name, char *Arg2Name, char *Arg3Name )
{ 
    struct Quadruple * ptr; 
    int incr = 100;
    if( QuadTable.len >= QuadTable.size ) {
        ptr = realloc( QuadTable.base, QuadTable.size+incr );
        if( ptr==NULL ) return -1;
        QuadTable.base = ptr;
        QuadTable.size += incr;
    }
    ptr = & QuadTable.base[QuadTable.len];
    ptr->op = Op;
    ptr->arg1 = Arg1;
    ptr->arg2 = Arg2;
    ptr->arg3 = Arg3;
    strcpy( ptr->arg1name, Arg1Name);
    strcpy( ptr->arg2name, Arg2Name);
    strcpy( ptr->arg3name, Arg3Name);
    QuadTable.len++;

    return QuadTable.len - 1;
}

/*����Ԫʽ����Ӧ������ַ����д�뵽�ļ���*/
void WriteQuadTableToFile( const char * FileName )
{
    FILE * fp;
    struct Quadruple * ptr;
    int i,op;
    char str[1000],ch;
    fp = fopen( FileName, "w" );
    if( fp==NULL ) return;
    for( i=0, ptr = QuadTable.base; i < QuadTable.len; i++,ptr++ ) {
        fprintf(fp, "%5d:  ", QuadTable.startaddr + i);
        printf("%5d:  ", QuadTable.startaddr + i);
        op = ptr->op;
        switch( op ) {
            case OIntAdd: 
         	case OIntSub: 
         	case OIntMultiply: 
         	case OIntDivide:
            case OFloatAdd: 
            case OFloatSub: 
            case OFloatMultiply: 
            case OFloatDivide:
           		if(op == OIntAdd || op == OFloatAdd) ch = '+';
           		if(op == OIntSub || op == OFloatSub) ch = '-';
           		if(op == OIntMultiply || op == OFloatMultiply) ch = '*';
           		if(op == OIntDivide || op == OFloatDivide) ch = '/';
                sprintf(str, "[%d] = [%d] %c [%d]", ptr->arg3, ptr->arg1, ch, ptr->arg2);
				printf("%s = %s %c %s\n", ptr->arg3name, ptr->arg1name, ch, ptr->arg2name);
                break;

            case OIntEvaluation:
            case OFloatEvaluation:
            case OCharEvaluation:
            case OBoolEvaluation:
            	sprintf(str, "[%d] = [%d]", ptr->arg3, ptr->arg1);
				printf("%s = %s\n", ptr->arg3name, ptr->arg1name);
                break;

            case OIntUminus:
            case OFloatUminus: 
            	sprintf(str, "[%d] = - [%d]", ptr->arg3, ptr->arg1);
            	printf("%s = %s\n", ptr->arg3name, ptr->arg1name);
                break;

            case OGoto:
				sprintf(str, "Goto %d", ptr->arg3);
				printf("Goto %d\n", ptr->arg3);
                break;

            case OLTGoto: 
				sprintf(str, "if [%d] < [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s < %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break; 
			case OLEGoto: 
				sprintf(str, "if [%d] <= [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s <= %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break; 
			case OGTGoto: 
				sprintf(str, "if [%d] > [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s > %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break; 
			case OGEGoto: 
				sprintf(str, "if [%d] >= [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s >= %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break;
			case OEQGoto: 
				sprintf(str, "if [%d] == [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s == %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break; 
			case ONEQGoto: 
				sprintf(str, "if [%d] != [%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  
				printf("if %s != %s Goto %d\n", ptr->arg1name, ptr->arg2name, ptr->arg3 );  
				break; 

            case OCharToBool:
            case OIntToBool:
            case OFloatToBool:
            	sprintf( str,"[%d] = (bool) [%d]", ptr->arg3, ptr->arg1); 
				printf("%s = (bool) %s\n", ptr->arg3name, ptr->arg1name);  
				break;
			case OBoolToChar:
            case OIntToChar:
            case OFloatToChar:
            	sprintf( str,"[%d] = (char) [%d]", ptr->arg3, ptr->arg1); 
				printf("%s = (char) %s\n", ptr->arg3name, ptr->arg1name);  
				break; 
			case OBoolToInt:
            case OCharToInt:
            case OFloatToInt:
            	sprintf( str,"[%d] = (int) [%d]", ptr->arg3, ptr->arg1); 
				printf("%s = (int) %s\n", ptr->arg3name, ptr->arg1name);  
				break;
			case OBoolToFloat:
            case OCharToFloat:
            case OIntToFloat:
            	sprintf( str,"[%d] = (float) [%d]", ptr->arg3, ptr->arg1); 
				printf("%s = (float) %s\n", ptr->arg3name, ptr->arg1name);  
				break; 

			case HALT : 
				sprintf(str, "HALT"); 
				printf("HALT\n");
				break;

            default:
            	yyerror("������󣺳��ֲ���ʶ���������"); 
                strcpy(str, "error: Unknown operator");break;
        }
        fprintf(fp,"%s\n",str);
    }

    fclose(fp);
}

/********************************����:��Ԫʽ�Ķ���ͺ���****************************/


/********************************����:�������ͼ����غ���****************************/

int typeMax(int type1, int type2)
{
	return type1 > type2 ? type1 : type2;
}

int typeWiden(int addr, int type, char name[], int wideType, char tmpName[], SymbolList TopSymbolList)
{
	if(type == wideType) {
		return addr;
	} else {
		int op;
		int tmpWidth;
		int tmpAddr;
		if(type == BOOL && wideType == CHAR) {
			op = OBoolToChar;
			tmpWidth = CHAR_WIDTH;
		} else if(type == BOOL && wideType == INT) {
			op = OBoolToInt;
			tmpWidth = INT_WIDTH;
		} else if(type == BOOL && wideType == FLOAT) {
			op = OBoolToFloat;
			tmpWidth = FLOAT_WIDTH;
		} else if(type == CHAR && wideType == INT) {
			op = OCharToInt;
			tmpWidth = INT_WIDTH;
		} else if(type == CHAR && wideType == FLOAT) {
			op = OCharToFloat;
			tmpWidth = FLOAT_WIDTH;
		} else if(type == INT && wideType == FLOAT) {
			op = OIntToFloat;
			tmpWidth = FLOAT_WIDTH;
		}
		tmpAddr = NewTemp(TopSymbolList, tmpName, tmpWidth);
		Gen(op, addr, 0, tmpAddr, name, "", tmpName);

		return tmpAddr;
	}
}

/********************************����:�������ͼ����غ���****************************/


/******************************����:���������ر����ͺ���*******************************************/
/* ��������洢*/
struct backlist{
     int backaddr;/* ������ʾ�������ݵ�ַ*/
     struct backlist *next ; 
};

struct backlist * makelist(int i)
{
    struct backlist *x  = (struct  backlist *)malloc( sizeof( struct backlist)) ; 
	x->backaddr = i ; 
    x->next = NULL;
	return x;
}

struct backlist *  merge(struct backlist * p1 , struct backlist *p2 )
{
       struct backlist * tail = p1;
       while( tail->next != NULL){
	        tail = tail->next; 
	   }	   
	   tail->next = p2;
       return p1; 	  /*�����ݺϲ���p1 ��	   */
}

void backpatch(struct backlist *p , int instr)
{
     /* ��������*/
	struct backlist *temp;
    temp = p;
	QuadTable.base[temp->backaddr-QuadTable.startaddr].arg3 = instr; 
	while(temp->next != NULL) {
	   temp = temp->next;
	   QuadTable.base[temp->backaddr-QuadTable.startaddr]. arg3 = instr; /*ѭ�������б�*/
	}
}


/******************************����:���������ر����ͺ���*******************************************/


/**************����:����䷨����ջ��Ԫ�ص���Ϣ�����ս���ͷ��ս�����ۺ�����****************/

 union ParseStackNodeInfo{
    struct {
	    /*������(���������)���Ȳ�����ID_MAX_LEN ���ַ�*/
        /*�������hash�����洢���еĲ�ͬ���ֵı�ʶ�������֣�����Ϳ���ʹ��һ��ָ��ָ��ñ�ʶ�������֣�
		  �ô��Ǽ��ٷ���ջ��Ԫ�صĿռ��С���Ӷ���ʡ�ռ�����߱���Ч�ʣ�*/
        char name[ID_MAX_LEN + 1 ]; 
    }id;  /*��ʶ��:�ս��ID���ۺ�����*/

    struct {
	   char str[ID_MAX_LEN + 1 ]; /*�ñ������ڴ洢�������ı���ʽ����ʾ��ʱ���õ�,ʵ�ʵı���ϵͳ����Ҫ*/	   
       int type; /*�������������������INT*/
	   union ConstVal value; /*�������ս��CONST����Ϣ*/
	   int width;
	} constval; /*�ս��const���ۺ�����*/

    struct {
        int type; /*�������������������INT*/
    }basic; /*�����������ͣ��ս��BASIC���ۺ�����*/

	struct {
	   char str[ID_MAX_LEN + 1 ]; /*�ñ������ڴ洢����������ʱ�������������ı���ʽ����ʾ��ʱ���õ�,ʵ�ʵı���ϵͳ����Ҫ*/
	   int type;
	   int addr;
	   int width;
       struct backlist * truelist;
	   struct backlist * falselist;
	   struct backlist * nextlist;
	   struct backlist * breaklist;/* ���ڴ��break�Ļ���ĵ�ַ*/
	} factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block;/*���ս��factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block���ۺ�����*/
        /*�����ķ����ŵ����Լ�¼����������������*/

    struct {
	   int instr;/* ��һ��ָ�����ţ���QuadTable.len + QuadTable.startaddr  */
	} M;
} ;

#define YYSTYPE union ParseStackNodeInfo 
       

/**************����:����䷨����ջ��Ԫ�ص���Ϣ�����ս���ͷ��ս�����ۺ�����****************/
 




#line 617 "y.tab.c"
#define BASIC 257
#define CONST 258
#define ID 259
#define IF 260
#define ELSE 261
#define WHILE 262
#define DO 263
#define BREAK 264
#define RELOP_LT 265
#define RELOP_LE 266
#define RELOP_GT 267
#define RELOP_GE 268
#define RELOP_EQ 269
#define RELOP_NEQ 270
#define OR 271
#define AND 272
#define UMINUS 273
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    1,    2,    5,    3,    3,    6,    7,    4,    4,
    9,    9,    9,    9,    9,    9,    9,   11,   11,   13,
   13,   14,   14,   14,   15,   15,   15,   15,   15,    8,
   12,   10,   10,   10,   16,   16,   16,   17,   17,   17,
   18,   18,   18,
};
short yylen[] = {                                         2,
    1,    6,    0,    0,    2,    0,    3,    1,    3,    0,
    4,    6,   10,    7,    8,    2,    1,    4,    1,    4,
    1,    3,    3,    1,    3,    3,    3,    3,    1,    0,
    0,    3,    3,    1,    3,    3,    1,    2,    2,    1,
    3,    1,    1,
};
short yydefred[] = {                                      0,
    3,    0,    1,    6,    0,    8,    0,    5,    0,    0,
    0,    0,    2,    0,    0,   30,   30,    0,   17,    9,
    7,    0,    0,    0,    0,   16,   43,   42,    0,    0,
    0,    0,    0,   37,   40,    0,    0,    0,    0,   24,
    0,    0,   39,   38,    0,    0,    0,   11,    0,    0,
    0,    0,    0,    0,   30,   30,   30,    0,    0,    0,
    0,   41,    0,    0,   35,   36,    0,    0,    0,    0,
    0,    0,    0,   22,   23,   30,    0,    0,    0,    0,
    0,    0,   31,   14,    0,   30,   15,    0,   13,
};
short yydgoto[] = {                                       2,
   19,    4,    5,    7,   10,    8,    9,   11,   20,   36,
   37,   86,   38,   39,   40,   33,   34,   35,
};
short yysindex[] = {                                   -117,
    0,    0,    0,    0, -246,    0,    0,    0, -245,  -82,
 -103,   11,    0,  -16,   19,    0,    0,   15,    0,    0,
    0,   21,   21,   25, -103,    0,    0,    0,   21,   21,
   21,  -20,    9,    0,    0,   17,  -17, -195, -220,    0,
   21, -184,    0,    0,  -11,   21,   21,    0,   21,   21,
   21,   21,   21,   21,    0,    0,    0,   21,   21,   -8,
   40,    0,    9,    9,    0,    0,   28,   28,   28,   28,
   21, -103,   21,    0,    0,    0,   21, -195, -180, -220,
 -103,    5,    0,    0,   23,    0,    0, -103,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0, -122,    0, -116,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -41,    0,    0,  -19,    0,    6,    1,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -33,  -24,    0,    0,  -14,  -10,   -5,   -1,
    0,    0,    0,    0,    0,    0,    0,    7, -110,    3,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   83,    0,    0,    0,    0,    0,    0,   12,   -9,   33,
  -36,    0,   18,   20,   -6,   29,    8,    0,
};
#define YYTABLESIZE 285
short yytable[] = {                                      34,
   10,   34,   10,   34,   60,    1,   30,   32,    4,   32,
    6,   32,   12,   12,   12,   42,   33,   34,   33,    1,
   33,   29,   46,   56,   47,   32,   25,   24,   25,   62,
   26,   46,   76,   47,   33,   27,   43,   44,   48,   28,
   82,   21,   13,   20,   22,   85,   19,   18,   58,   59,
   49,   74,   75,   30,   32,   50,   65,   66,   23,   46,
   31,   47,   79,   45,   41,   29,   71,   72,   73,   21,
   46,   84,   47,   26,   63,   64,   57,   61,   89,   77,
   83,   87,    3,   67,   68,   69,   70,   81,   78,    0,
    0,    0,   80,    0,    0,    0,    0,   88,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   10,   10,    0,   10,
   10,   10,   30,   30,    0,   30,   30,   30,   12,   12,
    0,   12,   12,   12,    0,   14,   15,    0,   16,   17,
   18,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   34,   34,   34,   34,   34,   34,   34,
   34,   32,   32,   32,   32,   32,   32,   32,   32,    0,
   33,   33,   33,   33,   33,   33,   33,   33,    0,   29,
   29,   29,   29,   55,   25,   25,   25,   25,   26,   26,
   26,   26,   55,   27,   27,   27,   27,   28,   28,   28,
   28,   21,   21,   20,   20,   55,   19,   18,   27,   28,
    0,   51,   52,   53,   54,
};
short yycheck[] = {                                      41,
  123,   43,  125,   45,   41,  123,  123,   41,  125,   43,
  257,   45,  123,  259,  125,   25,   41,   59,   43,  123,
   45,   41,   43,   41,   45,   59,   41,   16,   17,   41,
   41,   43,   41,   45,   59,   41,   29,   30,   59,   41,
   77,   41,  125,   41,   61,   41,   41,   41,  269,  270,
   42,   58,   59,   33,   22,   47,   49,   50,   40,   43,
   40,   45,   72,   31,   40,   45,   55,   56,   57,   59,
   43,   81,   45,   59,   46,   47,  272,  262,   88,   40,
  261,   59,    0,   51,   52,   53,   54,   76,   71,   -1,
   -1,   -1,   73,   -1,   -1,   -1,   -1,   86,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  259,  260,   -1,  262,
  263,  264,  259,  260,   -1,  262,  263,  264,  259,  260,
   -1,  262,  263,  264,   -1,  259,  260,   -1,  262,  263,
  264,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  265,  266,  267,  268,  269,  270,  271,
  272,  265,  266,  267,  268,  269,  270,  271,  272,   -1,
  265,  266,  267,  268,  269,  270,  271,  272,   -1,  269,
  270,  271,  272,  271,  269,  270,  271,  272,  269,  270,
  271,  272,  271,  269,  270,  271,  272,  269,  270,  271,
  272,  271,  272,  271,  272,  271,  271,  271,  258,  259,
   -1,  265,  266,  267,  268,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 273
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,0,0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,
"';'",0,"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"BASIC","CONST","ID","IF","ELSE","WHILE","DO","BREAK","RELOP_LT",
"RELOP_LE","RELOP_GT","RELOP_GE","RELOP_EQ","RELOP_NEQ","OR","AND","UMINUS",
};
char *yyrule[] = {
"$accept : program",
"program : block",
"block : '{' blockM1 decls stmts blockM2 '}'",
"blockM1 :",
"blockM2 :",
"decls : decls decl",
"decls :",
"decl : type ID ';'",
"type : BASIC",
"stmts : stmts M stmt",
"stmts :",
"stmt : ID '=' expr ';'",
"stmt : IF '(' bool ')' M stmt",
"stmt : IF '(' bool ')' M stmt ELSE N M stmt",
"stmt : WHILE M '(' bool ')' M stmt",
"stmt : DO M stmt WHILE '(' bool ')' ';'",
"stmt : BREAK ';'",
"stmt : block",
"bool : bool OR M join",
"bool : join",
"join : join AND M equality",
"join : equality",
"equality : equality RELOP_EQ rel",
"equality : equality RELOP_NEQ rel",
"equality : rel",
"rel : expr RELOP_LT expr",
"rel : expr RELOP_LE expr",
"rel : expr RELOP_GT expr",
"rel : expr RELOP_GE expr",
"rel : expr",
"M :",
"N :",
"expr : expr '+' term",
"expr : expr '-' term",
"expr : term",
"term : term '*' unary",
"term : term '/' unary",
"term : unary",
"unary : '!' unary",
"unary : '-' unary",
"unary : factor",
"factor : '(' expr ')'",
"factor : ID",
"factor : CONST",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifdef YYSTACKSIZE
#ifndef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#endif
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 600
#define YYMAXDEPTH 600
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 1336 ".\compile.y"
#include "lex.yy.c"

int yyparse();  /*main����Ҫ����yyparse()���������ú����Ķ����ں��棬����Ҫ������(��������)*/

int main()
{
	char sourcefile[1000],destfile[1000];

	printf("������Ҫ�����Դ�����ļ���(�س�Ĭ��Ϊcode.txt)��"); gets(sourcefile);
        if( strcmp( sourcefile,"")== 0 ) 
            strcpy( sourcefile, "code.txt");
	printf("���������м������ļ���(�س�Ĭ��Ϊgencode.txt)��"); gets(destfile);
        if( strcmp( destfile,"")== 0 ) 
            strcpy( destfile, "gencode.txt");

	BeginCompileOneFile( sourcefile );

    CreateConstList(3000);/*����������,����ӵ�ַ3000��ʼ����ռ������*/
    /*��C���Ա������У��÷��ű����ڴ���ⲿ�������������ȡ�
      �����ǵ��﷨�в���֧���ⲿ�����ͺ��������Ըñ�û�б��õ�*/
    TopSymbolList = CreateSymbolList( NULL, 2000 ); /*����ӵ�ַ2000��ʼ����ռ������*/
    CreateQuadTable(100); /*������Ԫʽ��������Ԫʽ�ӵ�ַ�ռ�100��ʼ���*/

    yyparse();

    PrintConstList();
    WriteQuadTableToFile( destfile ); /*����Ԫʽд�뵽�ļ�destfile��*/

    DestroyQuadTable();
    DestroySymbolList(TopSymbolList);
	DestroyConstList();

    if( CompileFailed == 0 ) 
	    printf("����ɹ������ɵ���Ԫʽ���ļ�%s�С�\n", destfile );
	else
	    printf("Դ�ļ�%s�д��󣬱���ʧ�ܡ�\n", sourcefile );

	EndCompileOneFile();
        
	getchar();
    return 0;
}
#line 889 "y.tab.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
yynewerror:
#endif
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
yyerrlab:
#endif
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 641 ".\compile.y"
{ 
            printf("����ʽ��program->block\n"); 

            if(yyvsp[0].block.nextlist != NULL) {
				backpatch(yyvsp[0].block.nextlist, QuadTable.startaddr + QuadTable.len);
		  	}
		  	Gen(HALT, 0, 0, 0, "", "", "");
        }
break;
case 2:
#line 652 ".\compile.y"
{ 
            printf("����ʽ��block->{decls stmts}\n"); 

            yyval.block.nextlist = yyvsp[-2].stmts.nextlist;
        }
break;
case 3:
#line 660 ".\compile.y"
{ 
            TopSymbolList = CreateSymbolList( TopSymbolList, TopSymbolList->endaddr ); 
        }
break;
case 4:
#line 666 ".\compile.y"
{ 
            SymbolList env;
            PrintSymbolList( TopSymbolList); 
            env = TopSymbolList->prev;
            DestroySymbolList( TopSymbolList ); 
            TopSymbolList = env;                 
        }
break;
case 5:
#line 676 ".\compile.y"
{ 
            printf("����ʽ��decls->decls decl\n"); 
        }
break;
case 6:
#line 680 ".\compile.y"
{ 
            printf("����ʽ��decls->null\n"); 
        }
break;
case 7:
#line 686 ".\compile.y"
{ 
            int width;
                                
            printf("����ʽ��decl->type ID; ID=%s\n",yyvsp[-1].id.name); 
            
            switch( yyvsp[-2].basic.type ) {
                case CHAR  : width = CHAR_WIDTH;  break;
                case INT   : width = INT_WIDTH;   break;
                case FLOAT : width = FLOAT_WIDTH; break;
                case BOOL  : width = BOOL_WIDTH;  break;
                default    : width = -1; break;
            }
            AddToSymbolList( TopSymbolList, yyvsp[-1].id.name, yyvsp[-2].basic.type, width );

        }
break;
case 8:
#line 704 ".\compile.y"
{ 
            printf("����ʽ��type->BASIC\n"); 
            
            yyval.basic.type = yyvsp[0].basic.type;
        }
break;
case 9:
#line 712 ".\compile.y"
{
            printf("����ʽ��stmts->stmts stmt\n");

            if( yyvsp[-2].stmts.nextlist  != NULL) {
				backpatch(yyvsp[-2].stmts.nextlist, yyvsp[-1].M.instr) ;
			}
			yyval.stmts.nextlist = yyvsp[0].stmt.nextlist;
        }
break;
case 10:
#line 722 ".\compile.y"
{
            printf("����ʽ��stmts->null\n");
        }
break;
case 11:
#line 728 ".\compile.y"
{ 
            printf("����ʽ��stmt->id = expr;\n"); 
            
            struct SymbolElem * p;
			p = LookUpAllSymbolList(TopSymbolList, yyvsp[-3].id.name );
      		if(p != NULL) {
                if (p->type == yyvsp[-1].expr.type) {
                    switch (p->type) {
                        case BOOL   : Gen(OBoolEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);   break;
                        case CHAR   : Gen(OCharEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);   break;
                        case INT    : Gen(OIntEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);    break;
                        case FLOAT  : Gen(OFloatEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);  break;
                        default: yyerror("�������ͷǷ�");
                    }
                } else {
                    int op;
                    char tmpName[10];
                    int tmpWidth;
                    int tmpAddr;

                    if(p->type == BOOL) {
                        tmpWidth = BOOL_WIDTH;
                        switch (yyvsp[-1].expr.type) {
                            case CHAR : op = OCharToBool;   break;
                            case INT  : op = OIntToBool;    break;
                            case FLOAT: op = OFloatToBool;  break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == CHAR) {
                        tmpWidth = CHAR_WIDTH;
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToChar;   break;
                            case INT  : op = OIntToChar;    break;
                            case FLOAT: op = OFloatToChar;  break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == INT) {
                        tmpWidth = INT_WIDTH; 
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToInt;    break;
                            case CHAR : op = OCharToInt;    break;
                            case FLOAT: op = OFloatToInt;   break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == FLOAT) {
                        tmpWidth = FLOAT_WIDTH;
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToFloat;  break;
                            case CHAR : op = OCharToFloat;  break;
                            case INT  : op = OIntToFloat;   break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else{
                        yyerror("�����Ƿ�����");
                    } 
                    tmpAddr = NewTemp(TopSymbolList, tmpName, tmpWidth);
                    Gen(op, yyvsp[-1].expr.addr, 0, tmpAddr, yyvsp[-1].expr.str, "", tmpName);

                    switch (p->type) {
                        case BOOL : Gen(OBoolEvaluation , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case CHAR : Gen(OCharEvaluation , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case INT  : Gen(OIntEvaluation  , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case FLOAT: Gen(OFloatEvaluation, tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        default: yyerror("�������ͷǷ�");
                    }
                }
			} else {
				yyerror( "������û�ж���" );
				strcpy( yyval.factor.str, "no_id_defined" ); /*�ݴ���*/
				yyval.factor.type = INT;
				yyval.factor.addr = -1;
				yyval.factor.width = INT_WIDTH;						
			 
				Gen(OIntEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);  
				yyval.stmt.nextlist = NULL; 
        	}
        }
break;
case 12:
#line 807 ".\compile.y"
{ 
            printf("����ʽ��stmt->if (bool) stmt\n");

            backpatch(yyvsp[-3].bool.truelist, yyvsp[-1].M.instr);
			yyval.stmt.nextlist = merge(yyvsp[-3].bool.falselist, yyvsp[0].stmt.nextlist);
        }
break;
case 13:
#line 815 ".\compile.y"
{ 
            printf("����ʽ��stmt->if (bool) stmt esle stmt\n"); 
            
            backpatch(yyvsp[-7].bool.truelist, yyvsp[-5].M.instr);
			backpatch(yyvsp[-7].bool.falselist, yyvsp[-1].M.instr);
			struct backlist *temp ;
			temp = merge(yyvsp[-2].N.nextlist, yyvsp[-4].stmt.nextlist);
			yyval.stmt.nextlist = merge(temp, yyvsp[0].stmt.nextlist);
        }
break;
case 14:
#line 826 ".\compile.y"
{ 
            printf("����ʽ��stmt->while (bool) stmt\n"); 
            
            backpatch(yyvsp[0].stmt.nextlist, yyvsp[-5].M.instr);
			backpatch(yyvsp[-3].bool.truelist, yyvsp[-1].M.instr);
			yyval.stmt.nextlist = yyvsp[-3].bool.falselist;
			Gen(OGoto, 0, 0, yyvsp[-5].M.instr, "", "", "");
        }
break;
case 15:
#line 836 ".\compile.y"
{ 
            printf("����ʽ��stmt->do stmt while (bool)\n"); 
            
            backpatch(yyvsp[-2].bool.truelist, yyvsp[-6].M.instr);
            yyval.stmt.nextlist = yyvsp[-2].bool.falselist;
        }
break;
case 16:
#line 844 ".\compile.y"
{ 
            printf("����ʽ��stmt->break ;\n"); 
        }
break;
case 17:
#line 849 ".\compile.y"
{ 
            printf("����ʽ��stmt->block\n"); 
        
            yyval.stmt.nextlist = yyvsp[0].block.nextlist;
        }
break;
case 18:
#line 857 ".\compile.y"
{ 
            printf("����ʽ��bool->bool || join\n"); 
            
            strcpy(yyval.bool.str, "");
			yyval.bool.type = 0;
			yyval.bool.addr = 0;
			yyval.bool.width = 0;  
			backpatch(yyvsp[-3].bool.falselist, yyvsp[-1].M.instr);
			yyval.bool.truelist = merge(yyvsp[-3].bool.truelist, yyvsp[0].join.truelist);
			yyval.bool.falselist = yyvsp[0].join.falselist;
        }
break;
case 19:
#line 870 ".\compile.y"
{ 
            printf("����ʽ��bool->join\n"); 
            
            strcpy( yyval.bool.str, yyvsp[0].join.str );
			yyval.bool.type = yyvsp[0].join.type;
			yyval.bool.addr = yyvsp[0].join.addr;
			yyval.bool.width = yyvsp[0].join.width;
			yyval.bool.truelist = yyvsp[0].join.truelist;
			yyval.bool.falselist = yyvsp[0].join.falselist;    
        }
break;
case 20:
#line 883 ".\compile.y"
{ 
            printf("����ʽ��join->join && equality\n"); 
        
            strcpy(yyval.join.str, "");
			yyval.join.type = 0;
			yyval.join.addr = 0;
			yyval.join.width = 0;  
            backpatch(yyvsp[-3].join.truelist, yyvsp[-1].M.instr);
			yyval.join.truelist = yyvsp[0].equality.truelist;
			yyval.join.falselist = merge(yyvsp[-3].join.falselist, yyvsp[0].equality.falselist);
        }
break;
case 21:
#line 896 ".\compile.y"
{ 
            printf("����ʽ��join->equality\n"); 
        
            strcpy(yyval.join.str, yyvsp[0].equality.str);
			yyval.join.type = yyvsp[0].equality.type;	
			yyval.join.addr = yyvsp[0].equality.addr;
		  	yyval.join.width = yyvsp[0].equality.width;
			yyval.join.truelist = yyvsp[0].equality.truelist;
			yyval.join.falselist = yyvsp[0].equality.falselist;
        }
break;
case 22:
#line 909 ".\compile.y"
{
            printf("����ʽ��equality -> equality == rel\n");

            strcpy(yyval.equality.str, "");
			yyval.equality.type = 0;
			yyval.equality.addr = 0;
			yyval.equality.width = 0;
			yyval.equality.truelist = makelist(QuadTable.len + QuadTable.startaddr);	
			yyval.equality.falselist = makelist(QuadTable.len + QuadTable.startaddr + 1);	
			Gen(OEQGoto, yyvsp[-2].equality.addr, yyvsp[0].rel.addr, 0, yyvsp[-2].equality.str, yyvsp[0].rel.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 23:
#line 923 ".\compile.y"
{
            printf("����ʽ��equality -> equality != rel\n");

            strcpy(yyval.equality.str, "");
            yyval.equality.type = 0;
			yyval.equality.addr = 0;
			yyval.equality.width = 0;
			yyval.equality.truelist = makelist(QuadTable.len + QuadTable.startaddr);	
			yyval.equality.falselist = makelist(QuadTable.len + QuadTable.startaddr + 1);	
			Gen(ONEQGoto, yyvsp[-2].equality.addr, yyvsp[0].rel.addr, 0, yyvsp[-2].equality.str, yyvsp[0].rel.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 24:
#line 937 ".\compile.y"
{
            printf("����ʽ��equality -> rel\n");

            strcpy(yyval.equality.str, yyvsp[0].rel.str);
			yyval.equality.type = yyvsp[0].rel.type;
			yyval.equality.addr = yyvsp[0].rel.addr;
		  	yyval.equality.width = yyvsp[0].rel.width;
			yyval.equality.truelist = yyvsp[0].rel.truelist;
			yyval.equality.falselist = yyvsp[0].rel.falselist;
        }
break;
case 25:
#line 950 ".\compile.y"
{
            printf("����ʽ��rel -> expr < expr\n");

            strcpy(yyval.rel.str, "");
            yyval.rel.type = 0;
			yyval.rel.addr = 0;
			yyval.rel.width = 0;
			yyval.rel.truelist =  makelist(QuadTable.startaddr + QuadTable.len);	
			yyval.rel.falselist =  makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OLTGoto, yyvsp[-2].expr.addr, yyvsp[0].expr.addr , 0 , yyvsp[-2].expr.str, yyvsp[0].expr.str, "_") ;
			Gen(OGoto, 0, 0, 0, "", "", "_");
		}
break;
case 26:
#line 964 ".\compile.y"
{
            printf("����ʽ��rel -> expr <= expr\n");

            strcpy(yyval.rel.str, "");
            yyval.rel.type = 0;
			yyval.rel.addr = 0;
			yyval.rel.width = 0;
            yyval.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			yyval.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OLEGoto, yyvsp[-2].expr.addr, yyvsp[0].expr.addr, 0, yyvsp[-2].expr.str, yyvsp[0].expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 27:
#line 978 ".\compile.y"
{
            printf("����ʽ��rel -> expr > expr\n");

            strcpy(yyval.rel.str, "");
            yyval.rel.type = 0;
			yyval.rel.addr = 0;
			yyval.rel.width = 0;
            yyval.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			yyval.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OGTGoto, yyvsp[-2].expr.addr, yyvsp[0].expr.addr, 0, yyvsp[-2].expr.str, yyvsp[0].expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 28:
#line 992 ".\compile.y"
{
            printf("����ʽ��rel -> expr >= expr\n");

            strcpy(yyval.rel.str, "");
            yyval.rel.type = 0;
			yyval.rel.addr = 0;
			yyval.rel.width = 0;
            yyval.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			yyval.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OGEGoto, yyvsp[-2].expr.addr, yyvsp[0].expr.addr, 0, yyvsp[-2].expr.str, yyvsp[0].expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 29:
#line 1006 ".\compile.y"
{ 	
            printf("����ʽ��rel -> expr\n"); 
	  		
			strcpy(yyval.rel.str,yyvsp[0].expr.str);
			yyval.rel.type = yyvsp[0].expr.type;
			yyval.rel.addr = yyvsp[0].expr.addr;
			yyval.rel.width = yyvsp[0].expr.width;
			yyval.rel.truelist = NULL;
			yyval.rel.falselist = NULL;
 	 	}
break;
case 30:
#line 1019 ".\compile.y"
{
            yyval.M.instr = QuadTable.startaddr + QuadTable.len;
        }
break;
case 31:
#line 1025 ".\compile.y"
{
            yyval.N.nextlist = makelist(QuadTable.len + QuadTable.startaddr);  
            Gen(OGoto, 0, 0, 0, "", "", "");
        }
break;
case 32:
#line 1033 ".\compile.y"
{ 
            printf("����ʽ��expr->expr + term\n"); 
                           
            yyval.expr.type = typeMax(yyvsp[-2].expr.type, yyvsp[0].term.type);
			switch (yyval.expr.type) {
				case BOOL 	: yyval.expr.width = BOOL_WIDTH; break;
				case CHAR	: yyval.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.expr.width = INT_WIDTH; break;
				case FLOAT 	: yyval.expr.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			yyval.expr.addr = NewTemp(TopSymbolList, yyval.expr.str, yyval.expr.width);
			if(yyvsp[-2].expr.type == yyvsp[0].term.type) {
				if (yyvsp[-2].expr.type == INT) {
					Gen(OIntAdd, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str); 
				} else if (yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatAdd, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if(yyvsp[-2].expr.type > yyvsp[0].term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].term.addr, yyvsp[0].term.type, yyvsp[0].term.str, yyvsp[-2].expr.type, tmpName, TopSymbolList);
				if(yyvsp[-2].expr.type == INT) {
					Gen(OIntAdd, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);	
				} else if(yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatAdd, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].expr.addr, yyvsp[-2].expr.type, yyvsp[-2].expr.str, yyvsp[0].term.type, tmpName, TopSymbolList);
				if(yyvsp[0].term.type == INT) {
					Gen(OIntAdd, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else if(yyvsp[0].term.type == FLOAT) {
					Gen(OFloatAdd, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			yyval.expr.truelist = NULL;
			yyval.expr.falselist = NULL;
        }
break;
case 33:
#line 1079 ".\compile.y"
{ 
            printf("����ʽ��expr->expr - term\n"); 
	  
            yyval.expr.type = typeMax(yyvsp[-2].expr.type, yyvsp[0].term.type);
			switch (yyval.expr.type) {
				case BOOL 	: yyval.expr.width = BOOL_WIDTH; break;
				case CHAR	: yyval.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.expr.width = INT_WIDTH; break;
				case FLOAT 	: yyval.expr.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			yyval.expr.addr = NewTemp(TopSymbolList, yyval.expr.str, yyval.expr.width);
			if(yyvsp[-2].expr.type == yyvsp[0].term.type) {
				if (yyvsp[-2].expr.type == INT) {
					Gen(OIntSub, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str); 
				} else if (yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatSub, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if(yyvsp[-2].expr.type > yyvsp[0].term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].term.addr, yyvsp[0].term.type, yyvsp[0].term.str, yyvsp[-2].expr.type, tmpName, TopSymbolList);
				if(yyvsp[-2].expr.type == INT) {
					Gen(OIntSub, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);	
				} else if(yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatSub, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].expr.addr, yyvsp[-2].expr.type, yyvsp[-2].expr.str, yyvsp[0].term.type, tmpName, TopSymbolList);
				if(yyvsp[0].term.type == INT) {
					Gen(OIntSub, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else if(yyvsp[0].term.type == FLOAT) {
					Gen(OFloatSub, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			yyval.expr.truelist = NULL;
			yyval.expr.falselist = NULL;
	    }
break;
case 34:
#line 1125 ".\compile.y"
{ 
            printf("����ʽ��expr->term\n");

            strcpy(yyval.expr.str, yyvsp[0].term.str);
		  	yyval.expr.type = yyvsp[0].term.type;
			yyval.expr.addr = yyvsp[0].term.addr;
		 	yyval.expr.width = yyvsp[0].term.width;
			yyval.expr.truelist = yyvsp[0].term.truelist;
			yyval.expr.falselist = yyvsp[0].term.falselist;
	  
	    }
break;
case 35:
#line 1139 ".\compile.y"
{ 
            printf("����ʽ��term->term*unary\n"); 
            
            yyval.term.type = typeMax(yyvsp[-2].term.type, yyvsp[0].unary.type);
			switch (yyval.term.type) {
				case BOOL 	: yyval.term.width = BOOL_WIDTH; break;
				case CHAR	: yyval.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.term.width = INT_WIDTH; break;
				case FLOAT 	: yyval.term.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			yyval.term.addr = NewTemp(TopSymbolList, yyval.term.str, yyval.term.width);
			if(yyvsp[-2].term.type == yyvsp[0].unary.type) {
				if (yyvsp[-2].term.type == INT) {
					Gen(OIntMultiply, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str); 
				} else if (yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatMultiply, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if(yyvsp[-2].term.type > yyvsp[0].unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].unary.addr, yyvsp[0].unary.type, yyvsp[0].unary.str, yyvsp[-2].term.type, tmpName, TopSymbolList);
				if(yyvsp[-2].term.type == INT) {
					Gen(OIntMultiply, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);	
				} else if(yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatMultiply, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].term.addr, yyvsp[-2].term.type, yyvsp[-2].term.str, yyvsp[0].unary.type, tmpName, TopSymbolList);
				if(yyvsp[0].unary.type == INT) {
					Gen(OIntMultiply, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else if(yyvsp[0].unary.type == FLOAT) {
					Gen(OFloatMultiply, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			yyval.term.truelist = NULL;
			yyval.term.falselist = NULL;    
        }
break;
case 36:
#line 1185 ".\compile.y"
{ 
            printf("����ʽ��term->term/unary\n"); 
        
            yyval.term.type = typeMax(yyvsp[-2].term.type, yyvsp[0].unary.type);
			switch (yyval.term.type) {
				case BOOL 	: yyval.term.width = BOOL_WIDTH; break;
				case CHAR	: yyval.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.term.width = INT_WIDTH; break;
				case FLOAT 	: yyval.term.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			yyval.term.addr = NewTemp(TopSymbolList, yyval.term.str, yyval.term.width);
			if(yyvsp[-2].term.type == yyvsp[0].unary.type) {
				if (yyvsp[-2].term.type == INT) {
					Gen(OIntDivide, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str); 
				} else if (yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatDivide, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if(yyvsp[-2].term.type > yyvsp[0].unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].unary.addr, yyvsp[0].unary.type, yyvsp[0].unary.str, yyvsp[-2].term.type, tmpName, TopSymbolList);
				if(yyvsp[-2].term.type == INT) {
					Gen(OIntDivide, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);	
				} else if(yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatDivide, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].term.addr, yyvsp[-2].term.type, yyvsp[-2].term.str, yyvsp[0].unary.type, tmpName, TopSymbolList);
				if(yyvsp[0].unary.type == INT) {
					Gen(OIntDivide, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else if(yyvsp[0].unary.type == FLOAT) {
					Gen(OFloatDivide, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			yyval.term.truelist = NULL;
			yyval.term.falselist = NULL;
        }
break;
case 37:
#line 1231 ".\compile.y"
{ 
            printf("����ʽ��term->unary\n");
            
            strcpy(yyval.term.str, yyvsp[0].unary.str);
			yyval.term.type = yyvsp[0].unary.type;	
			yyval.term.addr = yyvsp[0].unary.addr;
			yyval.term.width = yyvsp[0].unary.width;
			yyval.term.truelist = yyvsp[0].unary.truelist;
			yyval.term.falselist = yyvsp[0].unary.falselist;
        }
break;
case 38:
#line 1244 ".\compile.y"
{
            printf("����ʽ��unary -> !unary\n");
            strcpy(yyval.unary.str, yyvsp[0].unary.str);
			yyval.unary.type = yyvsp[0].unary.type;
			yyval.unary.addr = yyvsp[0].unary.addr;
		  	yyval.unary.width = yyvsp[0].unary.width;
			yyval.unary.truelist = yyvsp[0].unary.falselist;
			yyval.unary.falselist = yyvsp[0].unary.truelist;
        }
break;
case 39:
#line 1255 ".\compile.y"
{
            printf("����ʽ��unary -> -unary\n");
            yyval.unary.type = yyvsp[0].unary.type;
			yyval.unary.width = yyvsp[0].unary.width;
			yyval.unary.truelist = NULL;
			yyval.unary.falselist = NULL;	
		    yyval.unary.addr = NewTemp(TopSymbolList, yyval.unary.str, yyval.unary.width);
		    if(yyvsp[0].unary.type == INT) {
		    	Gen(OIntUminus, yyvsp[0].unary.addr, 0, yyval.unary.addr, yyvsp[0].unary.str, "", yyval.unary.str);
		    } else if(yyvsp[0].unary.type == FLOAT) {
				Gen(OFloatUminus, yyvsp[0].unary.addr, 0, yyval.unary.addr, yyvsp[0].unary.str, "", yyval.unary.str);
		    } else {
		    	yyerror("�����ͻ򸡵�������");
		    }			  
        }
break;
case 40:
#line 1272 ".\compile.y"
{
            printf("����ʽ��unary -> factor\n");
            strcpy( yyval.unary.str, yyvsp[0].factor.str );
			yyval.unary.type = yyvsp[0].factor.type;
			yyval.unary.addr = yyvsp[0].factor.addr;
			yyval.unary.width = yyvsp[0].factor.width;
			yyval.unary.truelist = yyvsp[0].factor.truelist;
			yyval.unary.falselist = yyvsp[0].factor.falselist;
        }
break;
case 41:
#line 1284 ".\compile.y"
{
            printf("����ʽ��factor->(expr)\n" );
            
            strcpy(yyval.factor.str, yyvsp[-1].expr.str);
			yyval.factor.type = yyvsp[-1].expr.type;
			yyval.factor.addr = yyvsp[-1].expr.addr;
			yyval.factor.width = yyvsp[-1].expr.width;
			yyval.factor.truelist = yyvsp[-1].expr.truelist;
			yyval.factor.falselist = yyvsp[-1].expr.falselist;
        }
break;
case 42:
#line 1296 ".\compile.y"
{ 
            struct SymbolElem * p;
            printf("����ʽ��factor->id\n"); 
            p = LookUpAllSymbolList( TopSymbolList, yyvsp[0].id.name );
            if( p != NULL ) {
                strcpy( yyval.factor.str, p->name );
                yyval.factor.type  = p->type;
                yyval.factor.addr  = p->addr;
                yyval.factor.width = p->width;
            }							    
            else {
                yyerror( "������û�ж���" );
                strcpy( yyval.factor.str, "no_id_defined" ); /*�ݴ���*/
                yyval.factor.type = INT;
                yyval.factor.addr = -1;
                yyval.factor.width = INT_WIDTH;							    
            }
            yyval.factor.truelist = NULL;
			yyval.factor.falselist = NULL;
        }
break;
case 43:
#line 1318 ".\compile.y"
{                        
            struct ConstElem * p; 
            printf("����ʽ��factor->CONST\n");

            p = LookUpConstList( yyvsp[0].constval.type, yyvsp[0].constval.value, yyvsp[0].constval.width ) ;
            if( p== NULL )
                p = AddToConstList( yyvsp[0].constval.str, yyvsp[0].constval.type, yyvsp[0].constval.value, yyvsp[0].constval.width );

            strcpy( yyval.factor.str, yyvsp[0].constval.str );
            yyval.factor.type  = yyvsp[0].constval.type;
            yyval.factor.addr  = p->addr;
            yyval.factor.width = p->width;
            yyval.factor.truelist = NULL;
			yyval.factor.falselist = NULL;
        }
break;
#line 1747 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
