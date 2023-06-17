%{ 
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
// ��������洢
struct backlist{
     int backaddr;// ������ʾ�������ݵ�ַ
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
       return p1; 	  //�����ݺϲ���p1 ��	   
}

void backpatch(struct backlist *p , int instr)
{
     // ��������
	struct backlist *temp;
    temp = p;
	QuadTable.base[temp->backaddr-QuadTable.startaddr].arg3 = instr; 
	while(temp->next != NULL) {
	   temp = temp->next;
	   QuadTable.base[temp->backaddr-QuadTable.startaddr]. arg3 = instr; //ѭ�������б�
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
	   struct backlist * breaklist;// ���ڴ��break�Ļ���ĵ�ַ
	} factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block;/*���ս��factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block���ۺ�����*/
        /*�����ķ����ŵ����Լ�¼����������������*/

    struct {
	   int instr;// ��һ��ָ�����ţ���QuadTable.len + QuadTable.startaddr  
	} M;
} ;

#define YYSTYPE union ParseStackNodeInfo 
       

/**************����:����䷨����ջ��Ԫ�ص���Ϣ�����ս���ͷ��ս�����ۺ�����****************/
 




%}

%token BASIC
%token CONST
%token ID

%token IF
%token ELSE
%token WHILE
%token DO
%token BREAK

%token RELOP_LT
%token RELOP_LE
%token RELOP_GT
%token RELOP_GE

%token RELOP_EQ
%token RELOP_NEQ

%token OR
%token AND



%right '='
%left '+' '-'
%left '*' '/'
%right '!' UMINUS

%%
program : block
        { 
            printf("����ʽ��program->block\n"); 

            if($1.block.nextlist != NULL) {
				backpatch($1.block.nextlist, QuadTable.startaddr + QuadTable.len);
		  	}
		  	Gen(HALT, 0, 0, 0, "", "", "");
        } 
        ;

block   : '{' blockM1 decls stmts blockM2 '}'
        { 
            printf("����ʽ��block->{decls stmts}\n"); 

            $$.block.nextlist = $4.stmts.nextlist;
        } 
        ;

blockM1 :                    
        { 
            TopSymbolList = CreateSymbolList( TopSymbolList, TopSymbolList->endaddr ); 
        }
        ;

blockM2 :                     
        { 
            SymbolList env;
            PrintSymbolList( TopSymbolList); 
            env = TopSymbolList->prev;
            DestroySymbolList( TopSymbolList ); 
            TopSymbolList = env;                 
        }
		;

decls   : decls decl          
        { 
            printf("����ʽ��decls->decls decl\n"); 
        }
        |                     
        { 
            printf("����ʽ��decls->null\n"); 
        }
        ;

decl    : type ID ';'         
        { 
            int width;
                                
            printf("����ʽ��decl->type ID; ID=%s\n",$2.id.name); 
            
            switch( $1.basic.type ) {
                case CHAR  : width = CHAR_WIDTH;  break;
                case INT   : width = INT_WIDTH;   break;
                case FLOAT : width = FLOAT_WIDTH; break;
                case BOOL  : width = BOOL_WIDTH;  break;
                default    : width = -1; break;
            }
            AddToSymbolList( TopSymbolList, $2.id.name, $1.basic.type, width );

        }
		;

type    : BASIC               
        { 
            printf("����ʽ��type->BASIC\n"); 
            
            $$.basic.type = $1.basic.type;
        }
        ;

stmts   : stmts M stmt    
        {
            printf("����ʽ��stmts->stmts stmt\n");

            if( $1.stmts.nextlist  != NULL) {
				backpatch($1.stmts.nextlist, $2.M.instr) ;
			}
			$$.stmts.nextlist = $3.stmt.nextlist;
        }
        
        | /*empty*/     
        {
            printf("����ʽ��stmts->null\n");
        }
        ;

stmt    : ID '=' expr ';'                  
        { 
            printf("����ʽ��stmt->id = expr;\n"); 
            
            struct SymbolElem * p;
			p = LookUpAllSymbolList(TopSymbolList, $1.id.name );
      		if(p != NULL) {
                if (p->type == $3.expr.type) {
                    switch (p->type) {
                        case BOOL   : Gen(OBoolEvaluation , $3.expr.addr, 0, p->addr, $3.expr.str, "",  p->name);   break;
                        case CHAR   : Gen(OCharEvaluation , $3.expr.addr, 0, p->addr, $3.expr.str, "",  p->name);   break;
                        case INT    : Gen(OIntEvaluation , $3.expr.addr, 0, p->addr, $3.expr.str, "",  p->name);    break;
                        case FLOAT  : Gen(OFloatEvaluation , $3.expr.addr, 0, p->addr, $3.expr.str, "",  p->name);  break;
                        default: yyerror("�������ͷǷ�");
                    }
                } else {
                    int op;
                    char tmpName[10];
                    int tmpWidth;
                    int tmpAddr;

                    if(p->type == BOOL) {
                        tmpWidth = BOOL_WIDTH;
                        switch ($3.expr.type) {
                            case CHAR : op = OCharToBool;   break;
                            case INT  : op = OIntToBool;    break;
                            case FLOAT: op = OFloatToBool;  break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == CHAR) {
                        tmpWidth = CHAR_WIDTH;
                        switch ($3.expr.type) {
                            case BOOL : op = OBoolToChar;   break;
                            case INT  : op = OIntToChar;    break;
                            case FLOAT: op = OFloatToChar;  break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == INT) {
                        tmpWidth = INT_WIDTH; 
                        switch ($3.expr.type) {
                            case BOOL : op = OBoolToInt;    break;
                            case CHAR : op = OCharToInt;    break;
                            case FLOAT: op = OFloatToInt;   break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else if(p->type == FLOAT) {
                        tmpWidth = FLOAT_WIDTH;
                        switch ($3.expr.type) {
                            case BOOL : op = OBoolToFloat;  break;
                            case CHAR : op = OCharToFloat;  break;
                            case INT  : op = OIntToFloat;   break;
                            default: yyerror("�������ͷǷ�");
                        }
                    } else{
                        yyerror("�����Ƿ�����");
                    } 
                    tmpAddr = NewTemp(TopSymbolList, tmpName, tmpWidth);
                    Gen(op, $3.expr.addr, 0, tmpAddr, $3.expr.str, "", tmpName);

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
				strcpy( $$.factor.str, "no_id_defined" ); /*�ݴ���*/
				$$.factor.type = INT;
				$$.factor.addr = -1;
				$$.factor.width = INT_WIDTH;						
			 
				Gen(OIntEvaluation , $3.expr.addr, 0, p->addr, $3.expr.str, "",  p->name);  
				$$.stmt.nextlist = NULL; 
        	}
        }

        | IF '(' bool ')' M stmt             
        { 
            printf("����ʽ��stmt->if (bool) stmt\n");

            backpatch($3.bool.truelist, $5.M.instr);
			$$.stmt.nextlist = merge($3.bool.falselist, $6.stmt.nextlist);
        }

        | IF '(' bool ')' M stmt ELSE N M stmt   
        { 
            printf("����ʽ��stmt->if (bool) stmt esle stmt\n"); 
            
            backpatch($3.bool.truelist, $5.M.instr);
			backpatch($3.bool.falselist, $9.M.instr);
			struct backlist *temp ;
			temp = merge($8.N.nextlist, $6.stmt.nextlist);
			$$.stmt.nextlist = merge(temp, $10.stmt.nextlist);
        }

        | WHILE M '(' bool ')' M stmt          
        { 
            printf("����ʽ��stmt->while (bool) stmt\n"); 
            
            backpatch($7.stmt.nextlist, $2.M.instr);
			backpatch($4.bool.truelist, $6.M.instr);
			$$.stmt.nextlist = $4.bool.falselist;
			Gen(OGoto, 0, 0, $2.M.instr, "", "", "");
        }

        | DO M stmt WHILE '(' bool ')' ';'   
        { 
            printf("����ʽ��stmt->do stmt while (bool)\n"); 
            
            backpatch($6.bool.truelist, $2.M.instr);
            $$.stmt.nextlist = $6.bool.falselist;
        }
        
        | BREAK  ';'                       
        { 
            printf("����ʽ��stmt->break ;\n"); 
        }

        | block                            
        { 
            printf("����ʽ��stmt->block\n"); 
        
            $$.stmt.nextlist = $1.block.nextlist;
        }
        ;

bool    : bool OR M join                     
        { 
            printf("����ʽ��bool->bool || join\n"); 
            
            strcpy($$.bool.str, "");
			$$.bool.type = 0;
			$$.bool.addr = 0;
			$$.bool.width = 0;  
			backpatch($1.bool.falselist, $3.M.instr);
			$$.bool.truelist = merge($1.bool.truelist, $4.join.truelist);
			$$.bool.falselist = $4.join.falselist;
        }

        |  join                             
        { 
            printf("����ʽ��bool->join\n"); 
            
            strcpy( $$.bool.str, $1.join.str );
			$$.bool.type = $1.join.type;
			$$.bool.addr = $1.join.addr;
			$$.bool.width = $1.join.width;
			$$.bool.truelist = $1.join.truelist;
			$$.bool.falselist = $1.join.falselist;    
        }  
		;

join    :  join AND M equality                
        { 
            printf("����ʽ��join->join && equality\n"); 
        
            strcpy($$.join.str, "");
			$$.join.type = 0;
			$$.join.addr = 0;
			$$.join.width = 0;  
            backpatch($1.join.truelist, $3.M.instr);
			$$.join.truelist = $4.equality.truelist;
			$$.join.falselist = merge($1.join.falselist, $4.equality.falselist);
        } 

        |  equality                         
        { 
            printf("����ʽ��join->equality\n"); 
        
            strcpy($$.join.str, $1.equality.str);
			$$.join.type = $1.equality.type;	
			$$.join.addr = $1.equality.addr;
		  	$$.join.width = $1.equality.width;
			$$.join.truelist = $1.equality.truelist;
			$$.join.falselist = $1.equality.falselist;
        } 
        ;

equality: equality RELOP_EQ rel
        {
            printf("����ʽ��equality -> equality == rel\n");

            strcpy($$.equality.str, "");
			$$.equality.type = 0;
			$$.equality.addr = 0;
			$$.equality.width = 0;
			$$.equality.truelist = makelist(QuadTable.len + QuadTable.startaddr);	
			$$.equality.falselist = makelist(QuadTable.len + QuadTable.startaddr + 1);	
			Gen(OEQGoto, $1.equality.addr, $3.rel.addr, 0, $1.equality.str, $3.rel.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }

        | equality RELOP_NEQ rel
        {
            printf("����ʽ��equality -> equality != rel\n");

            strcpy($$.equality.str, "");
            $$.equality.type = 0;
			$$.equality.addr = 0;
			$$.equality.width = 0;
			$$.equality.truelist = makelist(QuadTable.len + QuadTable.startaddr);	
			$$.equality.falselist = makelist(QuadTable.len + QuadTable.startaddr + 1);	
			Gen(ONEQGoto, $1.equality.addr, $3.rel.addr, 0, $1.equality.str, $3.rel.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }

        | rel
        {
            printf("����ʽ��equality -> rel\n");

            strcpy($$.equality.str, $1.rel.str);
			$$.equality.type = $1.rel.type;
			$$.equality.addr = $1.rel.addr;
		  	$$.equality.width = $1.rel.width;
			$$.equality.truelist = $1.rel.truelist;
			$$.equality.falselist = $1.rel.falselist;
        }
        ;

rel     : expr RELOP_LT expr   
		{
            printf("����ʽ��rel -> expr < expr\n");

            strcpy($$.rel.str, "");
            $$.rel.type = 0;
			$$.rel.addr = 0;
			$$.rel.width = 0;
			$$.rel.truelist =  makelist(QuadTable.startaddr + QuadTable.len);	
			$$.rel.falselist =  makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OLTGoto, $1.expr.addr, $3.expr.addr , 0 , $1.expr.str, $3.expr.str, "_") ;
			Gen(OGoto, 0, 0, 0, "", "", "_");
		}

        | expr RELOP_LE expr   
        {
            printf("����ʽ��rel -> expr <= expr\n");

            strcpy($$.rel.str, "");
            $$.rel.type = 0;
			$$.rel.addr = 0;
			$$.rel.width = 0;
            $$.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			$$.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OLEGoto, $1.expr.addr, $3.expr.addr, 0, $1.expr.str, $3.expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }

        | expr RELOP_GT expr   
        {
            printf("����ʽ��rel -> expr > expr\n");

            strcpy($$.rel.str, "");
            $$.rel.type = 0;
			$$.rel.addr = 0;
			$$.rel.width = 0;
            $$.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			$$.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OGTGoto, $1.expr.addr, $3.expr.addr, 0, $1.expr.str, $3.expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }

        | expr RELOP_GE expr   
        {
            printf("����ʽ��rel -> expr >= expr\n");

            strcpy($$.rel.str, "");
            $$.rel.type = 0;
			$$.rel.addr = 0;
			$$.rel.width = 0;
            $$.rel.truelist = makelist(QuadTable.startaddr + QuadTable.len);
			$$.rel.falselist = makelist(QuadTable.startaddr + QuadTable.len + 1 );
			Gen(OGEGoto, $1.expr.addr, $3.expr.addr, 0, $1.expr.str, $3.expr.str, "");
			Gen(OGoto, 0, 0, 0, "", "", "");
        }

        | expr                 
		{ 	
            printf("����ʽ��rel -> expr\n"); 
	  		
			strcpy($$.rel.str,$1.expr.str);
			$$.rel.type = $1.expr.type;
			$$.rel.addr = $1.expr.addr;
			$$.rel.width = $1.expr.width;
			$$.rel.truelist = NULL;
			$$.rel.falselist = NULL;
 	 	}
        ;

M       : /* epsilon */
        {
            $$.M.instr = QuadTable.startaddr + QuadTable.len;
        }
        ;

N       : /* epsilon */     
        {
            $$.N.nextlist = makelist(QuadTable.len + QuadTable.startaddr);  
            Gen(OGoto, 0, 0, 0, "", "", "");
        }
        ;


expr    : expr  '+' term    
        { 
            printf("����ʽ��expr->expr + term\n"); 
                           
            $$.expr.type = typeMax($1.expr.type, $3.term.type);
			switch ($$.expr.type) {
				case BOOL 	: $$.expr.width = BOOL_WIDTH; break;
				case CHAR	: $$.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: $$.expr.width = INT_WIDTH; break;
				case FLOAT 	: $$.expr.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			$$.expr.addr = NewTemp(TopSymbolList, $$.expr.str, $$.expr.width);
			if($1.expr.type == $3.term.type) {
				if ($1.expr.type == INT) {
					Gen(OIntAdd, $1.expr.addr, $3.term.addr, $$.expr.addr, $1.expr.str, $3.term.str, $$.expr.str); 
				} else if ($1.expr.type == FLOAT) {
					Gen(OFloatAdd, $1.expr.addr, $3.term.addr, $$.expr.addr, $1.expr.str, $3.term.str, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if($1.expr.type > $3.term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden($3.term.addr, $3.term.type, $3.term.str, $1.expr.type, tmpName, TopSymbolList);
				if($1.expr.type == INT) {
					Gen(OIntAdd, $1.expr.addr, tmpAddr, $$.expr.addr, $1.expr.str, tmpName, $$.expr.str);	
				} else if($1.expr.type == FLOAT) {
					Gen(OFloatAdd, $1.expr.addr, tmpAddr, $$.expr.addr, $1.expr.str, tmpName, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden($1.expr.addr, $1.expr.type, $1.expr.str, $3.term.type, tmpName, TopSymbolList);
				if($3.term.type == INT) {
					Gen(OIntAdd, tmpAddr, $3.term.addr, $$.expr.addr, tmpName, $3.term.str, $$.expr.str);
				} else if($3.term.type == FLOAT) {
					Gen(OFloatAdd, tmpAddr, $3.term.addr, $$.expr.addr, tmpName, $3.term.str, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			$$.expr.truelist = NULL;
			$$.expr.falselist = NULL;
        }

        | expr  '-' term    
        { 
            printf("����ʽ��expr->expr - term\n"); 
	  
            $$.expr.type = typeMax($1.expr.type, $3.term.type);
			switch ($$.expr.type) {
				case BOOL 	: $$.expr.width = BOOL_WIDTH; break;
				case CHAR	: $$.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: $$.expr.width = INT_WIDTH; break;
				case FLOAT 	: $$.expr.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			$$.expr.addr = NewTemp(TopSymbolList, $$.expr.str, $$.expr.width);
			if($1.expr.type == $3.term.type) {
				if ($1.expr.type == INT) {
					Gen(OIntSub, $1.expr.addr, $3.term.addr, $$.expr.addr, $1.expr.str, $3.term.str, $$.expr.str); 
				} else if ($1.expr.type == FLOAT) {
					Gen(OFloatSub, $1.expr.addr, $3.term.addr, $$.expr.addr, $1.expr.str, $3.term.str, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if($1.expr.type > $3.term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden($3.term.addr, $3.term.type, $3.term.str, $1.expr.type, tmpName, TopSymbolList);
				if($1.expr.type == INT) {
					Gen(OIntSub, $1.expr.addr, tmpAddr, $$.expr.addr, $1.expr.str, tmpName, $$.expr.str);	
				} else if($1.expr.type == FLOAT) {
					Gen(OFloatSub, $1.expr.addr, tmpAddr, $$.expr.addr, $1.expr.str, tmpName, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden($1.expr.addr, $1.expr.type, $1.expr.str, $3.term.type, tmpName, TopSymbolList);
				if($3.term.type == INT) {
					Gen(OIntSub, tmpAddr, $3.term.addr, $$.expr.addr, tmpName, $3.term.str, $$.expr.str);
				} else if($3.term.type == FLOAT) {
					Gen(OFloatSub, tmpAddr, $3.term.addr, $$.expr.addr, tmpName, $3.term.str, $$.expr.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			$$.expr.truelist = NULL;
			$$.expr.falselist = NULL;
	    }
 
        | term              
        { 
            printf("����ʽ��expr->term\n");

            strcpy($$.expr.str, $1.term.str);
		  	$$.expr.type = $1.term.type;
			$$.expr.addr = $1.term.addr;
		 	$$.expr.width = $1.term.width;
			$$.expr.truelist = $1.term.truelist;
			$$.expr.falselist = $1.term.falselist;
	  
	    }
        ;

term    : term  '*' unary  
        { 
            printf("����ʽ��term->term*unary\n"); 
            
            $$.term.type = typeMax($1.term.type, $3.unary.type);
			switch ($$.term.type) {
				case BOOL 	: $$.term.width = BOOL_WIDTH; break;
				case CHAR	: $$.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: $$.term.width = INT_WIDTH; break;
				case FLOAT 	: $$.term.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			$$.term.addr = NewTemp(TopSymbolList, $$.term.str, $$.term.width);
			if($1.term.type == $3.unary.type) {
				if ($1.term.type == INT) {
					Gen(OIntMultiply, $1.term.addr, $3.unary.addr, $$.term.addr, $1.term.str, $3.unary.str, $$.term.str); 
				} else if ($1.term.type == FLOAT) {
					Gen(OFloatMultiply, $1.term.addr, $3.unary.addr, $$.term.addr, $1.term.str, $3.unary.str, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if($1.term.type > $3.unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden($3.unary.addr, $3.unary.type, $3.unary.str, $1.term.type, tmpName, TopSymbolList);
				if($1.term.type == INT) {
					Gen(OIntMultiply, $1.term.addr, tmpAddr, $$.term.addr, $1.term.str, tmpName, $$.term.str);	
				} else if($1.term.type == FLOAT) {
					Gen(OFloatMultiply, $1.term.addr, tmpAddr, $$.term.addr, $1.term.str, tmpName, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden($1.term.addr, $1.term.type, $1.term.str, $3.unary.type, tmpName, TopSymbolList);
				if($3.unary.type == INT) {
					Gen(OIntMultiply, tmpAddr, $3.unary.addr, $$.term.addr, tmpName, $3.unary.str, $$.term.str);
				} else if($3.unary.type == FLOAT) {
					Gen(OFloatMultiply, tmpAddr, $3.unary.addr, $$.term.addr, tmpName, $3.unary.str, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			$$.term.truelist = NULL;
			$$.term.falselist = NULL;    
        }

        | term  '/' unary 
        { 
            printf("����ʽ��term->term/unary\n"); 
        
            $$.term.type = typeMax($1.term.type, $3.unary.type);
			switch ($$.term.type) {
				case BOOL 	: $$.term.width = BOOL_WIDTH; break;
				case CHAR	: $$.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: $$.term.width = INT_WIDTH; break;
				case FLOAT 	: $$.term.width = FLOAT_WIDTH; break;
				default: yyerror("�������ͷǷ�");
			}
			$$.term.addr = NewTemp(TopSymbolList, $$.term.str, $$.term.width);
			if($1.term.type == $3.unary.type) {
				if ($1.term.type == INT) {
					Gen(OIntDivide, $1.term.addr, $3.unary.addr, $$.term.addr, $1.term.str, $3.unary.str, $$.term.str); 
				} else if ($1.term.type == FLOAT) {
					Gen(OFloatDivide, $1.term.addr, $3.unary.addr, $$.term.addr, $1.term.str, $3.unary.str, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");
				}
			} else if($1.term.type > $3.unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden($3.unary.addr, $3.unary.type, $3.unary.str, $1.term.type, tmpName, TopSymbolList);
				if($1.term.type == INT) {
					Gen(OIntDivide, $1.term.addr, tmpAddr, $$.term.addr, $1.term.str, tmpName, $$.term.str);	
				} else if($1.term.type == FLOAT) {
					Gen(OFloatDivide, $1.term.addr, tmpAddr, $$.term.addr, $1.term.str, tmpName, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden($1.term.addr, $1.term.type, $1.term.str, $3.unary.type, tmpName, TopSymbolList);
				if($3.unary.type == INT) {
					Gen(OIntDivide, tmpAddr, $3.unary.addr, $$.term.addr, tmpName, $3.unary.str, $$.term.str);
				} else if($3.unary.type == FLOAT) {
					Gen(OFloatDivide, tmpAddr, $3.unary.addr, $$.term.addr, tmpName, $3.unary.str, $$.term.str);
				} else {
					yyerror("�����ͻ򸡵�������");	
				}			  
			}
			$$.term.truelist = NULL;
			$$.term.falselist = NULL;
        }

        | unary            
        { 
            printf("����ʽ��term->unary\n");
            
            strcpy($$.term.str, $1.unary.str);
			$$.term.type = $1.unary.type;	
			$$.term.addr = $1.unary.addr;
			$$.term.width = $1.unary.width;
			$$.term.truelist = $1.unary.truelist;
			$$.term.falselist = $1.unary.falselist;
        }
        ;

unary   : '!' unary
        {
            printf("����ʽ��unary -> !unary\n");
            strcpy($$.unary.str, $2.unary.str);
			$$.unary.type = $2.unary.type;
			$$.unary.addr = $2.unary.addr;
		  	$$.unary.width = $2.unary.width;
			$$.unary.truelist = $2.unary.falselist;
			$$.unary.falselist = $2.unary.truelist;
        }

        | '-' unary
        {
            printf("����ʽ��unary -> -unary\n");
            $$.unary.type = $2.unary.type;
			$$.unary.width = $2.unary.width;
			$$.unary.truelist = NULL;
			$$.unary.falselist = NULL;	
		    $$.unary.addr = NewTemp(TopSymbolList, $$.unary.str, $$.unary.width);
		    if($2.unary.type == INT) {
		    	Gen(OIntUminus, $2.unary.addr, 0, $$.unary.addr, $2.unary.str, "", $$.unary.str);
		    } else if($2.unary.type == FLOAT) {
				Gen(OFloatUminus, $2.unary.addr, 0, $$.unary.addr, $2.unary.str, "", $$.unary.str);
		    } else {
		    	yyerror("�����ͻ򸡵�������");
		    }			  
        }

        | factor
        {
            printf("����ʽ��unary -> factor\n");
            strcpy( $$.unary.str, $1.factor.str );
			$$.unary.type = $1.factor.type;
			$$.unary.addr = $1.factor.addr;
			$$.unary.width = $1.factor.width;
			$$.unary.truelist = $1.factor.truelist;
			$$.unary.falselist = $1.factor.falselist;
        }
        ;

factor  : '(' expr ')'      
        {
            printf("����ʽ��factor->(expr)\n" );
            
            strcpy($$.factor.str, $2.expr.str);
			$$.factor.type = $2.expr.type;
			$$.factor.addr = $2.expr.addr;
			$$.factor.width = $2.expr.width;
			$$.factor.truelist = $2.expr.truelist;
			$$.factor.falselist = $2.expr.falselist;
        }

        | ID                
        { 
            struct SymbolElem * p;
            printf("����ʽ��factor->id\n"); 
            p = LookUpAllSymbolList( TopSymbolList, $1.id.name );
            if( p != NULL ) {
                strcpy( $$.factor.str, p->name );
                $$.factor.type  = p->type;
                $$.factor.addr  = p->addr;
                $$.factor.width = p->width;
            }							    
            else {
                yyerror( "������û�ж���" );
                strcpy( $$.factor.str, "no_id_defined" ); /*�ݴ���*/
                $$.factor.type = INT;
                $$.factor.addr = -1;
                $$.factor.width = INT_WIDTH;							    
            }
            $$.factor.truelist = NULL;
			$$.factor.falselist = NULL;
        }

        | CONST              
        {                        
            struct ConstElem * p; 
            printf("����ʽ��factor->CONST\n");

            p = LookUpConstList( $1.constval.type, $1.constval.value, $1.constval.width ) ;
            if( p== NULL )
                p = AddToConstList( $1.constval.str, $1.constval.type, $1.constval.value, $1.constval.width );

            strcpy( $$.factor.str, $1.constval.str );
            $$.factor.type  = $1.constval.type;
            $$.factor.addr  = p->addr;
            $$.factor.width = p->width;
            $$.factor.truelist = NULL;
			$$.factor.falselist = NULL;
        }
        ; 

%%
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
