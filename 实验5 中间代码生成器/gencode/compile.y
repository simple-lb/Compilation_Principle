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
{ struct Quadruple * ptr; 
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
{FILE * fp;
struct Quadruple * ptr;
int i,op;
char str[1000],ch;
    fp = fopen( FileName, "w" );
    if( fp==NULL ) return;
    for( i=0, ptr = QuadTable.base; i < QuadTable.len; i++,ptr++ ) {
        fprintf(fp, "%5d:  ", QuadTable.startaddr + i);
        op = ptr->op;
        switch( op ) {
            case OIntAdd        :
            case OIntSub        :
            case OIntMultiply   :
            case OIntDivide     :
            case OFloatAdd      :
            case OFloatSub      :
            case OFloatMultiply :
            case OFloatDivide   : if( op==OIntAdd || op==OFloatAdd) ch = '+';
                                  if( op==OIntSub || op==OFloatSub) ch = '-';
                                  if( op==OIntMultiply || op==OFloatMultiply) ch = '*';
                                  if( op==OIntDivide || op==OFloatDivide) ch = '/';
                                  sprintf(str,"[%d] = [%d] %c [%d]", ptr->arg3, ptr->arg1, ch, ptr->arg2);
                                  break;
            case OIntEvaluation   :
            case OFloatEvaluation :
            case OCharEvaluation  :
            case OBoolEvaluation  : sprintf(str,"[%d] = [%d]", ptr->arg3, ptr->arg1);
                                    break;
            case OGoto            : sprintf(str,"Goto %d", ptr->arg3);
                                    break;
            case OGTGoto  : sprintf(str,"if [%d]>[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  break;
            case OGEGoto  : sprintf(str,"if [%d]>=[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 ); break;
            case OLTGoto  : sprintf(str,"if [%d]<[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 );  break;
            case OLEGoto  : sprintf(str,"if [%d]<=[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 ); break;
            case OEQGoto  : sprintf(str,"if [%d]==[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 ); break;
            case ONEQGoto : sprintf(str,"if [%d]<>[%d] Goto %d", ptr->arg1, ptr->arg2, ptr->arg3 ); break;

            case OCharToInt   : sprintf( str,"[%d] = (int) [%d]",   ptr->arg3, ptr->arg1 );  break;
            case OCharToFloat : sprintf( str,"[%d] = (float) [%d]", ptr->arg3, ptr->arg1 );  break;
            case OIntToFloat  : sprintf( str,"[%d] = (float) [%d]", ptr->arg3, ptr->arg1 );  break;
            case OIntToChar   : sprintf( str,"[%d] = (char) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OFloatToChar : sprintf( str,"[%d] = (char) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OFloatToInt  : sprintf( str,"[%d] = (int) [%d]",   ptr->arg3, ptr->arg1 );  break;

            case OCharToBool   : sprintf( str,"[%d] = (bool) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OIntToBool    : sprintf( str,"[%d] = (bool) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OFloatToBool  : sprintf( str,"[%d] = (bool) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OBoolToChar   : sprintf( str,"[%d] = (char) [%d]",  ptr->arg3, ptr->arg1 );  break;
            case OBoolToInt    : sprintf( str,"[%d] = (int) [%d]",   ptr->arg3, ptr->arg1 );  break;
            case OBoolToFloat  : sprintf( str,"[%d] = (float) [%d]", ptr->arg3, ptr->arg1 );  break;

            default: yyerror("������󣺳��ֲ���ʶ���������"); strcpy(str, "error: Unknown operator");break;
        }
        fprintf(fp,"%s\n",str);
    }

    fclose(fp);
}

/********************************����:��Ԫʽ�Ķ���ͺ���****************************/



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
	} factor, term, expr;/*���ս��factor, term, expr���ۺ�����*/
        /*�����ķ����ŵ����Լ�¼����������������*/


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
program : block                                 { printf("����ʽ��program->block\n"); } 
        ;

block   : '{' blockM1 decls stmts blockM2 '}'   { printf("����ʽ��block->{decls stmts}\n"); } 
        ;

blockM1 :                     { TopSymbolList = CreateSymbolList( TopSymbolList, TopSymbolList->endaddr ); }
        ;

blockM2 :                     { SymbolList env;
                                 PrintSymbolList( TopSymbolList); 
                                 env = TopSymbolList->prev;
                                 DestroySymbolList( TopSymbolList ); 
                                 TopSymbolList = env;                 
                              }
		;

decls   : decls decl          { printf("����ʽ��decls->decls decl\n"); }
        |                     { printf("����ʽ��decls->null\n"); }
        ;

decl    : type ID ';'         { int width;
                                
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

type    : BASIC               { printf("����ʽ��type->BASIC\n"); $$.basic.type = $1.basic.type; }
        ;

stmts   : stmts stmt    {printf("����ʽ��stmts->stmts stmt\n");}
        | /*empty*/     {printf("����ʽ��stmts->null\n");}
        ;

stmt    : ID '=' expr ';'                  { printf("����ʽ��stmt->id = expr;\n"); }
        | IF '(' bool ')' stmt             { printf("����ʽ��stmt->if (bool) stmt\n");}
        | IF '(' bool ')' stmt ELSE stmt   { printf("����ʽ��stmt->if (bool) stmt esle stmt\n"); }
        | WHILE '(' bool ')' stmt          { printf("����ʽ��stmt->while (bool) stmt\n"); }
        | DO stmt WHILE '(' bool ')' ';'   { printf("����ʽ��stmt->do stmt while (bool)\n"); }
        | BREAK  ';'                       { printf("����ʽ��stmt->break ;\n"); }
        | block                            { printf("����ʽ��stmt->block\n"); }
        ;

bool  :   bool OR join                     { printf("����ʽ��bool->bool || join\n"); }
        | join                             { printf("����ʽ��bool->join\n"); }  
		;

join  :   join AND equality                { printf("����ʽ��join->join && equality\n"); } 
        | equality                         { printf("����ʽ��join->equality\n"); } 
        ;

equality : equality RELOP_EQ  rel          { printf("����ʽ��equality->equality == rel\n"); }
        |  equality RELOP_NEQ rel          { printf("����ʽ��equality->equality != rel\n"); }
        |  rel                             { printf("����ʽ��equality->rel\n"); }
		;
 
rel   :  expr RELOP_LT expr   { printf("����ʽ��rel -> expr < expr\n"); }
      |  expr RELOP_LE expr   { printf("����ʽ��rel -> expr <= expr\n"); }
      |  expr RELOP_GT expr   { printf("����ʽ��rel -> expr > expr\n"); }
      |  expr RELOP_GE expr   { printf("����ʽ��rel -> expr >= expr\n"); }
      |  expr                 { printf("����ʽ��rel -> expr\n"); }
      ;



expr  : expr  '+' term    { printf("����ʽ��expr->expr + term\n"); 
                           
						   }

      | expr  '-' term    { printf("����ʽ��expr->expr - term\n"); 
	  
	  
	                      }
 
      | term              { printf("����ʽ��expr->term\n");
							strcpy( $$.expr.str, $1.term.str );
							$$.expr.type = $1.term.type;
							$$.expr.addr = $1.term.addr;
							$$.expr.width = $1.term.width;	
	  
	                      }
      ;

term  : term  '*' factor  { printf("����ʽ��term->term*factor\n"); }

      | term  '/' factor  { printf("����ʽ��term->term/factor\n"); }

      | factor            { printf("����ʽ��term->factor\n");
							strcpy( $$.term.str, $1.factor.str );
							$$.term.type = $1.factor.type;
							$$.term.addr = $1.factor.addr;
							$$.term.width = $1.factor.width;	
	                      }
      ;

factor: '(' expr ')'      { printf("����ʽ��factor->(expr)\n" );
							strcpy( $$.factor.str, $2.expr.str );
							$$.factor.type  = $2.expr.type;
							$$.factor.addr  = $2.expr.addr;
							$$.factor.width = $2.expr.width;
                          }

      | ID                { 
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
	                      }

      |CONST              {                        
							struct ConstElem * p; 
							    printf("����ʽ��factor->CONST\n");

								p = LookUpConstList( $1.constval.type, $1.constval.value, $1.constval.width ) ;
								if( p== NULL )
                                    p = AddToConstList( $1.constval.str, $1.constval.type, $1.constval.value, $1.constval.width );

								strcpy( $$.factor.str, $1.constval.str );
								$$.factor.type  = $1.constval.type;
								$$.factor.addr  = p->addr;
								$$.factor.width = p->width;
                          }
      ; 

%%
#include "lex.yy.c"

int yyparse();  /*main����Ҫ����yyparse()���������ú����Ķ����ں��棬����Ҫ������(��������)*/

int main()
{
	char sourcefile[1000],destfile[1000];

	printf("������Ҫ�����Դ�����ļ���(�س�Ĭ��Ϊd:\\gencode\\code.txt)��"); gets(sourcefile);
        if( strcmp( sourcefile,"")== 0 ) 
            strcpy( sourcefile, "d:\\gencode\\code.txt");
	printf("���������м������ļ���(�س�Ĭ��Ϊd:\\gencode\\gencode.txt)��"); gets(destfile);
        if( strcmp( destfile,"")== 0 ) 
            strcpy( destfile, "d:\\gencode\\gencode.txt");

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





   