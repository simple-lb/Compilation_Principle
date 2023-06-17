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

int LineNo = 1; /*当前lookahead所指向的字符所在的行号，即当前编译到的行号*/

int CompileFailed = 0;

void yyerror( char * ErrStr )
{
    CompileFailed = 1; /*编译失败标志*/
    printf("错误信息:%s, 行号:%d\n", ErrStr, LineNo);
}



/*变量和常量的基本类型BASIC*/
#define CHAR     1
#define INT      2
#define FLOAT    3
#define BOOL     4

#define CHAR_WIDTH  1
#define INT_WIDTH   4
#define FLOAT_WIDTH 8  
#define BOOL_WIDTH  1

/*****************************下面：符号表的定义和相关函数*******************************/

/*变量名长度不超过ID_MAX_LEN 个字符*/
#define ID_MAX_LEN   64

/*存放一个标识符*/
struct SymbolElem {
    char name[ ID_MAX_LEN + 1 ]; /*符号名(例如变量名)长度不超过ID_MAX_LEN 个字符*/
    int type; /*用来存放类型名，例如int, 这个程序只处理简单类型，在实际的编译器中，这里要建立树结构*/
    int  addr;      /*为该变量分配的空间的首地址*/
	int  width;     /*该变量的宽度，即占用多少个字节*/
    struct SymbolElem * next;  /*指向下一个标识符*/
};

/*标识符表*/
typedef struct SymbolList{
    struct SymbolElem * head;  /*指向符号表（用链表实现）的第一个结点，没有头结点,初始化为NULL*/
    struct SymbolList * prev; /*上一层的符号表*/
    int beginaddr; /*该符号表中分配给变量和临时变量空间的开始地址*/
    int endaddr;    /*该符号表中分配给变量和临时变量空间的结束地址*/
                   /*beginaddr~endaddr的空间存放该符号表的所有变量和临时变量*/
} * SymbolList;  /*符号表*/

SymbolList TopSymbolList=NULL; /*全局变量，存放当前的符号表（即当前处于最顶层的符号表）,对应为书上的top*/

/*创建并返回一个新的符号表（SymbolList就是书上的Env），PrevList是其的上一层符号表*/
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

/*在符号表List中查找是否存在标识符IdName，如果存在，则返回该结点指针，否则返回空*/
struct SymbolElem * LookUpSymbolList( SymbolList List, char * IdName )
{struct SymbolElem * p;
    if( List==NULL ) return NULL;
    for( p = List->head; p!=NULL; p = p->next ) 
        if( strcmp( p->name, IdName ) == 0 ) break;
    return p;
}

/*从符号表List开始并不断地往上一层符号表中查找是否存在标识符IdName，如果存在，则返回该结点指针，否则返回空*/
struct SymbolElem * LookUpAllSymbolList( SymbolList List, char * IdName )
{ SymbolList env;
struct SymbolElem * p;
    env = List;
    while( env!=NULL ) {
        p = LookUpSymbolList( env, IdName );
        if(  p != NULL ) return p; /*找到该符号*/
        env = env->prev;
    }
    return NULL;
}


/*创建一个新的符号结点,并添加到符号表中，而后返回该结点指针*/
struct SymbolElem * AddToSymbolList( SymbolList List, char * IdName,int IdType, int Width )
{struct SymbolElem * p;

    p = (struct SymbolElem *) malloc( sizeof(struct SymbolElem) );

    strcpy( p->name, IdName );
    p->type = IdType;
	p->width = Width;
	p->addr = List->endaddr;
	List->endaddr += Width;

    p->next = List->head;  /*将该标识符添加到符号表表头*/
    List->head = p;

    return p;    
}

void PrintSymbolList( SymbolList List )
{struct SymbolElem * p;
    printf("***********************变量列表*************************\n");
    if( List ==NULL ) return ;
    for( p=List->head; p!=NULL; p=p->next ) {
        printf("变量名:%s, 类型:", p->name);
		switch( p->type ) {
            case CHAR : printf("char");  break;
            case INT  : printf("int");   break;
            case FLOAT: printf("float"); break;
            case BOOL : printf("bool");  break;
		}
        printf(", 地址:%d, 宽度:%d\n", p->addr, p->width );
	}
    printf("*************该变量列表共占用%d个字节空间***************\n", List->endaddr - List->beginaddr);
}

/*分配一个临时变量,返回临时变量的地址、临时变量的名称*/
int NewTemp( SymbolList List, char Name[], int Width )
{ static int TempID = 1;
  int addr;
    sprintf( Name, "T%d", TempID++ ); /*例如T1，T2等*/
	addr = List->endaddr;
    List->endaddr += Width;
  
    return addr;
}

/*****************************上面：符号表的定义和相关函数*****************************/




/*****************************下面：常数表的定义和相关函数*******************************/

union ConstVal {
        char    ch;    /*存放字符常量*/
        int     n;     /*存放整型常量，或true=1，false=0 */
        double  f;     /*存放浮点数常量*/
};	

/*存放一个常数*/
struct ConstElem {
    char str[ID_MAX_LEN + 1 ]; /*该变量用于存储常数的文本形式，演示的时候用的,实际的编译系统不需要*/	   
    int type; /*用来存放类型名，例如int*/
    union ConstVal value;
    int  addr;      /*为该常量分配的空间的首地址*/
	int  width;     /*该变量的宽度，即占用多少个字节*/
    struct ConstElem * next;  /*指向下一个常量*/
};

/*常量表*/
struct ConstList{
    struct ConstElem * head;  /*指向常量表（用链表实现）的第一个结点，没有头结点,初始化为NULL*/
    int beginaddr;  /*该符号表中分配给常量空间的开始地址*/
    int endaddr;    /*该符号表中分配给常量空间的结束地址*/
                   /*beginaddr~endaddr的空间存放该常量表的所有常量*/
} ConstList ;  /*常量表，全局变量，注意整个程序只需要一个常量表**/


/*创建并返回常量表*/
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

/*在常量表ConstList中查找是否存在常量，如果存在，则返回该结点指针，否则返回空*/
struct ConstElem * LookUpConstList( int ConstType, union ConstVal ConstValue, int Width )
{struct ConstElem * p;
    for( p = ConstList.head; p!=NULL; p = p->next ) 
        if( p->type == ConstType && memcmp( &p->value,&ConstValue, Width) == 0 )  break;
	
    return p;
}


/*创建一个新的常数结点,并添加到常数表中，而后返回该结点指针*/
struct ConstElem * AddToConstList( char * Str, int ConstType, union ConstVal ConstValue, int Width )
{struct ConstElem * p;

    p = (struct ConstElem *) malloc( sizeof(struct ConstElem) );

    strcpy( p->str, Str );
    p->type = ConstType;
    p->value = ConstValue;
	p->width = Width;

	p->addr = ConstList.endaddr;
	ConstList.endaddr += Width;

    p->next = ConstList.head;  /*将该常量添加到常量表表头*/
    ConstList.head = p;

    return p;    
}

void PrintConstList(void)
{struct ConstElem * p;
    printf("***********************常量列表*************************\n");
    for( p=ConstList.head; p!=NULL; p=p->next ) {
	    printf("常量:%s, 类型:", p->str);
		switch( p->type ) {
            case CHAR : printf("char");  break;
            case INT  : printf("int");   break;
            case FLOAT: printf("float"); break;
            case BOOL : printf("bool");  break;
		}
        printf(", 地址:%d, 宽度:%d\n", p->addr, p->width );
	}
    printf("**************该常量列表共占用%d个字节空间***************\n", ConstList.endaddr - ConstList.beginaddr);
}

/*****************************上面：常数表的定义和相关函数*****************************/





/********************************下面:四元式的定义和函数****************************/

/* 整型加减乘除 */
#define OIntAdd          1001
#define OIntSub          1002
#define OIntMultiply     1003
#define OIntDivide       1004

/* 浮点数加减乘除 */
#define OFloatAdd        1011
#define OFloatSub        1012
#define OFloatMultiply   1013
#define OFloatDivide     1014

/*赋值a=b*/
#define OIntEvaluation   1021
#define OFloatEvaluation 1022
#define OCharEvaluation  1023
#define OBoolEvaluation  1024

/* 无条件goto语句 */
#define OGoto            1031

/* if a op b goto 语句 */
#define OGTGoto          1041
#define OGEGoto          1042
#define OLTGoto          1043
#define OLEGoto          1044
#define OEQGoto          1045
#define ONEQGoto         1046

/*类型转换运算符*/
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

/* 取反 */
#define OIntUminus			1071
#define OFloatUminus		1072

/*四元式数据结构*/
struct Quadruple {
    int op; /*运算符*/
    int arg1; /*存放第一个参数的地址，可能是变量、临时变量的地址*/
    int arg2;
    int arg3;/*存放第三个参数的地址，可能是变量、临时变量的地址，还可能是四元式的地址(Goto 的地址参数)*/
    char arg1name[ID_MAX_LEN + 1]; /*本不需要，用于演示时能显示arg1对应的变量或临时变量的名称(若有的话）*/
    char arg2name[ID_MAX_LEN + 1]; /*本不需要，用于演示时能显示arg2对应的变量或临时变量的名称(若有的话）*/
    char arg3name[ID_MAX_LEN + 1]; /*本不需要，用于演示时能显示arg3对应的变量或临时变量的名称(若有的话）*/
};

/*四元式表*/
struct QuadTable {
    int startaddr; /*四元式开始存放的地址,比如100*/
    struct Quadruple * base; /*指向一块内存，用来存放多个四元式，从base[0]开始存放*/
    int size; /*base中可以存放的四元式的个数*/
    int len; /*base[len]是下一个四元式要存放的空间*/

};

struct QuadTable QuadTable; /*只需要一个四元式表*/

void CreateQuadTable(int StartAddr)
{
    QuadTable.startaddr = StartAddr; 
    QuadTable.size = 1000; /* 一开始假设可以存放1000个四元式*/
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

/*当Arg1是变量或临时变量时，Arg1Name是该变量的名称,用于演示时使用，其余参数类同 */
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

/*把四元式所对应的三地址代码写入到文件中*/
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
            	yyerror("程序错误：出现不认识的运算符！"); 
                strcpy(str, "error: Unknown operator");break;
        }
        fprintf(fp,"%s\n",str);
    }

    fclose(fp);
}

/********************************上面:四元式的定义和函数****************************/


/********************************下面:定义类型检查相关函数****************************/

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

/********************************上面:定义类型检查相关函数****************************/


/******************************下面:定义回填相关变量和函数*******************************************/
/* 采用链表存储*/
struct backlist{
     int backaddr;/* 用来表示回填数据地址*/
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
       return p1; 	  /*将数据合并到p1 中	   */
}

void backpatch(struct backlist *p , int instr)
{
     /* 回填数据*/
	struct backlist *temp;
    temp = p;
	QuadTable.base[temp->backaddr-QuadTable.startaddr].arg3 = instr; 
	while(temp->next != NULL) {
	   temp = temp->next;
	   QuadTable.base[temp->backaddr-QuadTable.startaddr]. arg3 = instr; /*循环回填列表*/
	}
}


/******************************上面:定义回填相关变量和函数*******************************************/


/**************下面:定义句法分析栈中元素的信息，即终结符和非终结符的综合属性****************/

 union ParseStackNodeInfo{
    struct {
	    /*符号名(例如变量名)长度不超过ID_MAX_LEN 个字符*/
        /*如果建立hash表来存储所有的不同名字的标识符的名字，这里就可以使用一个指针指向该标识符的名字，
		  好处是减少分析栈中元素的空间大小，从而节省空间且提高编译效率，*/
        char name[ID_MAX_LEN + 1 ]; 
    }id;  /*标识符:终结符ID的综合属性*/

    struct {
	   char str[ID_MAX_LEN + 1 ]; /*该变量用于存储常数的文本形式，演示的时候用的,实际的编译系统不需要*/	   
       int type; /*用来存放类型名，例如INT*/
	   union ConstVal value; /*常量：终结符CONST的信息*/
	   int width;
	} constval; /*终结符const的综合属性*/

    struct {
        int type; /*用来存放类型名，例如INT*/
    }basic; /*基本数据类型：终结符BASIC的综合属性*/

	struct {
	   char str[ID_MAX_LEN + 1 ]; /*该变量用于存储变量名、临时变量名或常数的文本形式，演示的时候用的,实际的编译系统不需要*/
	   int type;
	   int addr;
	   int width;
       struct backlist * truelist;
	   struct backlist * falselist;
	   struct backlist * nextlist;
	   struct backlist * breaklist;/* 用于存放break的回填的地址*/
	} factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block;/*非终结符factor, term, expr, unary, rel, equality, join, bool, N, stmt, stmts, block的综合属性*/
        /*其它文法符号的属性记录可以在下面继续添加*/

    struct {
	   int instr;/* 下一条指令的序号，即QuadTable.len + QuadTable.startaddr  */
	} M;
} ;

#define YYSTYPE union ParseStackNodeInfo 
       

/**************上面:定义句法分析栈中元素的信息，即终结符和非终结符的综合属性****************/
 




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

int yyparse();  /*main函数要调用yyparse()函数，但该函数的定义在后面，所以要先声明(才能引用)*/

int main()
{
	char sourcefile[1000],destfile[1000];

	printf("请输入要编译的源程序文件名(回车默认为code.txt)："); gets(sourcefile);
        if( strcmp( sourcefile,"")== 0 ) 
            strcpy( sourcefile, "code.txt");
	printf("请输入存放中间代码的文件名(回车默认为gencode.txt)："); gets(destfile);
        if( strcmp( destfile,"")== 0 ) 
            strcpy( destfile, "gencode.txt");

	BeginCompileOneFile( sourcefile );

    CreateConstList(3000);/*创建常量表,假设从地址3000开始分配空间给常量*/
    /*在C语言编译器中，该符号表用于存放外部变量，函数名等。
      在我们的语法中并不支持外部变量和函数，所以该表没有被用到*/
    TopSymbolList = CreateSymbolList( NULL, 2000 ); /*假设从地址2000开始分配空间给变量*/
    CreateQuadTable(100); /*创建四元式表，假设四元式从地址空间100开始存放*/

    yyparse();

    PrintConstList();
    WriteQuadTableToFile( destfile ); /*把四元式写入到文件destfile中*/

    DestroyQuadTable();
    DestroySymbolList(TopSymbolList);
	DestroyConstList();

    if( CompileFailed == 0 ) 
	    printf("编译成功，生成的四元式在文件%s中。\n", destfile );
	else
	    printf("源文件%s有错误，编译失败。\n", sourcefile );

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
            printf("产生式：program->block\n"); 

            if(yyvsp[0].block.nextlist != NULL) {
				backpatch(yyvsp[0].block.nextlist, QuadTable.startaddr + QuadTable.len);
		  	}
		  	Gen(HALT, 0, 0, 0, "", "", "");
        }
break;
case 2:
#line 652 ".\compile.y"
{ 
            printf("产生式：block->{decls stmts}\n"); 

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
            printf("产生式：decls->decls decl\n"); 
        }
break;
case 6:
#line 680 ".\compile.y"
{ 
            printf("产生式：decls->null\n"); 
        }
break;
case 7:
#line 686 ".\compile.y"
{ 
            int width;
                                
            printf("产生式：decl->type ID; ID=%s\n",yyvsp[-1].id.name); 
            
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
            printf("产生式：type->BASIC\n"); 
            
            yyval.basic.type = yyvsp[0].basic.type;
        }
break;
case 9:
#line 712 ".\compile.y"
{
            printf("产生式：stmts->stmts stmt\n");

            if( yyvsp[-2].stmts.nextlist  != NULL) {
				backpatch(yyvsp[-2].stmts.nextlist, yyvsp[-1].M.instr) ;
			}
			yyval.stmts.nextlist = yyvsp[0].stmt.nextlist;
        }
break;
case 10:
#line 722 ".\compile.y"
{
            printf("产生式：stmts->null\n");
        }
break;
case 11:
#line 728 ".\compile.y"
{ 
            printf("产生式：stmt->id = expr;\n"); 
            
            struct SymbolElem * p;
			p = LookUpAllSymbolList(TopSymbolList, yyvsp[-3].id.name );
      		if(p != NULL) {
                if (p->type == yyvsp[-1].expr.type) {
                    switch (p->type) {
                        case BOOL   : Gen(OBoolEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);   break;
                        case CHAR   : Gen(OCharEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);   break;
                        case INT    : Gen(OIntEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);    break;
                        case FLOAT  : Gen(OFloatEvaluation , yyvsp[-1].expr.addr, 0, p->addr, yyvsp[-1].expr.str, "",  p->name);  break;
                        default: yyerror("变量类型非法");
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
                            default: yyerror("变量类型非法");
                        }
                    } else if(p->type == CHAR) {
                        tmpWidth = CHAR_WIDTH;
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToChar;   break;
                            case INT  : op = OIntToChar;    break;
                            case FLOAT: op = OFloatToChar;  break;
                            default: yyerror("变量类型非法");
                        }
                    } else if(p->type == INT) {
                        tmpWidth = INT_WIDTH; 
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToInt;    break;
                            case CHAR : op = OCharToInt;    break;
                            case FLOAT: op = OFloatToInt;   break;
                            default: yyerror("变量类型非法");
                        }
                    } else if(p->type == FLOAT) {
                        tmpWidth = FLOAT_WIDTH;
                        switch (yyvsp[-1].expr.type) {
                            case BOOL : op = OBoolToFloat;  break;
                            case CHAR : op = OCharToFloat;  break;
                            case INT  : op = OIntToFloat;   break;
                            default: yyerror("变量类型非法");
                        }
                    } else{
                        yyerror("变量非法类型");
                    } 
                    tmpAddr = NewTemp(TopSymbolList, tmpName, tmpWidth);
                    Gen(op, yyvsp[-1].expr.addr, 0, tmpAddr, yyvsp[-1].expr.str, "", tmpName);

                    switch (p->type) {
                        case BOOL : Gen(OBoolEvaluation , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case CHAR : Gen(OCharEvaluation , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case INT  : Gen(OIntEvaluation  , tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        case FLOAT: Gen(OFloatEvaluation, tmpAddr, 0, p->addr, tmpName, "",  p->name); break;
                        default: yyerror("变量类型非法");
                    }
                }
			} else {
				yyerror( "变量名没有定义" );
				strcpy( yyval.factor.str, "no_id_defined" ); /*容错处理*/
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
            printf("产生式：stmt->if (bool) stmt\n");

            backpatch(yyvsp[-3].bool.truelist, yyvsp[-1].M.instr);
			yyval.stmt.nextlist = merge(yyvsp[-3].bool.falselist, yyvsp[0].stmt.nextlist);
        }
break;
case 13:
#line 815 ".\compile.y"
{ 
            printf("产生式：stmt->if (bool) stmt esle stmt\n"); 
            
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
            printf("产生式：stmt->while (bool) stmt\n"); 
            
            backpatch(yyvsp[0].stmt.nextlist, yyvsp[-5].M.instr);
			backpatch(yyvsp[-3].bool.truelist, yyvsp[-1].M.instr);
			yyval.stmt.nextlist = yyvsp[-3].bool.falselist;
			Gen(OGoto, 0, 0, yyvsp[-5].M.instr, "", "", "");
        }
break;
case 15:
#line 836 ".\compile.y"
{ 
            printf("产生式：stmt->do stmt while (bool)\n"); 
            
            backpatch(yyvsp[-2].bool.truelist, yyvsp[-6].M.instr);
            yyval.stmt.nextlist = yyvsp[-2].bool.falselist;
        }
break;
case 16:
#line 844 ".\compile.y"
{ 
            printf("产生式：stmt->break ;\n"); 
        }
break;
case 17:
#line 849 ".\compile.y"
{ 
            printf("产生式：stmt->block\n"); 
        
            yyval.stmt.nextlist = yyvsp[0].block.nextlist;
        }
break;
case 18:
#line 857 ".\compile.y"
{ 
            printf("产生式：bool->bool || join\n"); 
            
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
            printf("产生式：bool->join\n"); 
            
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
            printf("产生式：join->join && equality\n"); 
        
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
            printf("产生式：join->equality\n"); 
        
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
            printf("产生式：equality -> equality == rel\n");

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
            printf("产生式：equality -> equality != rel\n");

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
            printf("产生式：equality -> rel\n");

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
            printf("产生式：rel -> expr < expr\n");

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
            printf("产生式：rel -> expr <= expr\n");

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
            printf("产生式：rel -> expr > expr\n");

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
            printf("产生式：rel -> expr >= expr\n");

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
            printf("产生式：rel -> expr\n"); 
	  		
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
            printf("产生式：expr->expr + term\n"); 
                           
            yyval.expr.type = typeMax(yyvsp[-2].expr.type, yyvsp[0].term.type);
			switch (yyval.expr.type) {
				case BOOL 	: yyval.expr.width = BOOL_WIDTH; break;
				case CHAR	: yyval.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.expr.width = INT_WIDTH; break;
				case FLOAT 	: yyval.expr.width = FLOAT_WIDTH; break;
				default: yyerror("变量类型非法");
			}
			yyval.expr.addr = NewTemp(TopSymbolList, yyval.expr.str, yyval.expr.width);
			if(yyvsp[-2].expr.type == yyvsp[0].term.type) {
				if (yyvsp[-2].expr.type == INT) {
					Gen(OIntAdd, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str); 
				} else if (yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatAdd, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");
				}
			} else if(yyvsp[-2].expr.type > yyvsp[0].term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].term.addr, yyvsp[0].term.type, yyvsp[0].term.str, yyvsp[-2].expr.type, tmpName, TopSymbolList);
				if(yyvsp[-2].expr.type == INT) {
					Gen(OIntAdd, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);	
				} else if(yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatAdd, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].expr.addr, yyvsp[-2].expr.type, yyvsp[-2].expr.str, yyvsp[0].term.type, tmpName, TopSymbolList);
				if(yyvsp[0].term.type == INT) {
					Gen(OIntAdd, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else if(yyvsp[0].term.type == FLOAT) {
					Gen(OFloatAdd, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}			  
			}
			yyval.expr.truelist = NULL;
			yyval.expr.falselist = NULL;
        }
break;
case 33:
#line 1079 ".\compile.y"
{ 
            printf("产生式：expr->expr - term\n"); 
	  
            yyval.expr.type = typeMax(yyvsp[-2].expr.type, yyvsp[0].term.type);
			switch (yyval.expr.type) {
				case BOOL 	: yyval.expr.width = BOOL_WIDTH; break;
				case CHAR	: yyval.expr.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.expr.width = INT_WIDTH; break;
				case FLOAT 	: yyval.expr.width = FLOAT_WIDTH; break;
				default: yyerror("变量类型非法");
			}
			yyval.expr.addr = NewTemp(TopSymbolList, yyval.expr.str, yyval.expr.width);
			if(yyvsp[-2].expr.type == yyvsp[0].term.type) {
				if (yyvsp[-2].expr.type == INT) {
					Gen(OIntSub, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str); 
				} else if (yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatSub, yyvsp[-2].expr.addr, yyvsp[0].term.addr, yyval.expr.addr, yyvsp[-2].expr.str, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");
				}
			} else if(yyvsp[-2].expr.type > yyvsp[0].term.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].term.addr, yyvsp[0].term.type, yyvsp[0].term.str, yyvsp[-2].expr.type, tmpName, TopSymbolList);
				if(yyvsp[-2].expr.type == INT) {
					Gen(OIntSub, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);	
				} else if(yyvsp[-2].expr.type == FLOAT) {
					Gen(OFloatSub, yyvsp[-2].expr.addr, tmpAddr, yyval.expr.addr, yyvsp[-2].expr.str, tmpName, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].expr.addr, yyvsp[-2].expr.type, yyvsp[-2].expr.str, yyvsp[0].term.type, tmpName, TopSymbolList);
				if(yyvsp[0].term.type == INT) {
					Gen(OIntSub, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else if(yyvsp[0].term.type == FLOAT) {
					Gen(OFloatSub, tmpAddr, yyvsp[0].term.addr, yyval.expr.addr, tmpName, yyvsp[0].term.str, yyval.expr.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}			  
			}
			yyval.expr.truelist = NULL;
			yyval.expr.falselist = NULL;
	    }
break;
case 34:
#line 1125 ".\compile.y"
{ 
            printf("产生式：expr->term\n");

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
            printf("产生式：term->term*unary\n"); 
            
            yyval.term.type = typeMax(yyvsp[-2].term.type, yyvsp[0].unary.type);
			switch (yyval.term.type) {
				case BOOL 	: yyval.term.width = BOOL_WIDTH; break;
				case CHAR	: yyval.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.term.width = INT_WIDTH; break;
				case FLOAT 	: yyval.term.width = FLOAT_WIDTH; break;
				default: yyerror("变量类型非法");
			}
			yyval.term.addr = NewTemp(TopSymbolList, yyval.term.str, yyval.term.width);
			if(yyvsp[-2].term.type == yyvsp[0].unary.type) {
				if (yyvsp[-2].term.type == INT) {
					Gen(OIntMultiply, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str); 
				} else if (yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatMultiply, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");
				}
			} else if(yyvsp[-2].term.type > yyvsp[0].unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].unary.addr, yyvsp[0].unary.type, yyvsp[0].unary.str, yyvsp[-2].term.type, tmpName, TopSymbolList);
				if(yyvsp[-2].term.type == INT) {
					Gen(OIntMultiply, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);	
				} else if(yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatMultiply, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].term.addr, yyvsp[-2].term.type, yyvsp[-2].term.str, yyvsp[0].unary.type, tmpName, TopSymbolList);
				if(yyvsp[0].unary.type == INT) {
					Gen(OIntMultiply, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else if(yyvsp[0].unary.type == FLOAT) {
					Gen(OFloatMultiply, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}			  
			}
			yyval.term.truelist = NULL;
			yyval.term.falselist = NULL;    
        }
break;
case 36:
#line 1185 ".\compile.y"
{ 
            printf("产生式：term->term/unary\n"); 
        
            yyval.term.type = typeMax(yyvsp[-2].term.type, yyvsp[0].unary.type);
			switch (yyval.term.type) {
				case BOOL 	: yyval.term.width = BOOL_WIDTH; break;
				case CHAR	: yyval.term.width = CHAR_WIDTH; break;
	  	 	  	case INT	: yyval.term.width = INT_WIDTH; break;
				case FLOAT 	: yyval.term.width = FLOAT_WIDTH; break;
				default: yyerror("变量类型非法");
			}
			yyval.term.addr = NewTemp(TopSymbolList, yyval.term.str, yyval.term.width);
			if(yyvsp[-2].term.type == yyvsp[0].unary.type) {
				if (yyvsp[-2].term.type == INT) {
					Gen(OIntDivide, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str); 
				} else if (yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatDivide, yyvsp[-2].term.addr, yyvsp[0].unary.addr, yyval.term.addr, yyvsp[-2].term.str, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");
				}
			} else if(yyvsp[-2].term.type > yyvsp[0].unary.type) {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[0].unary.addr, yyvsp[0].unary.type, yyvsp[0].unary.str, yyvsp[-2].term.type, tmpName, TopSymbolList);
				if(yyvsp[-2].term.type == INT) {
					Gen(OIntDivide, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);	
				} else if(yyvsp[-2].term.type == FLOAT) {
					Gen(OFloatDivide, yyvsp[-2].term.addr, tmpAddr, yyval.term.addr, yyvsp[-2].term.str, tmpName, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}				  
			} else {
				char tmpName[10];
     			int tmpAddr = typeWiden(yyvsp[-2].term.addr, yyvsp[-2].term.type, yyvsp[-2].term.str, yyvsp[0].unary.type, tmpName, TopSymbolList);
				if(yyvsp[0].unary.type == INT) {
					Gen(OIntDivide, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else if(yyvsp[0].unary.type == FLOAT) {
					Gen(OFloatDivide, tmpAddr, yyvsp[0].unary.addr, yyval.term.addr, tmpName, yyvsp[0].unary.str, yyval.term.str);
				} else {
					yyerror("非整型或浮点型运算");	
				}			  
			}
			yyval.term.truelist = NULL;
			yyval.term.falselist = NULL;
        }
break;
case 37:
#line 1231 ".\compile.y"
{ 
            printf("产生式：term->unary\n");
            
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
            printf("产生式：unary -> !unary\n");
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
            printf("产生式：unary -> -unary\n");
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
		    	yyerror("非整型或浮点型运算");
		    }			  
        }
break;
case 40:
#line 1272 ".\compile.y"
{
            printf("产生式：unary -> factor\n");
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
            printf("产生式：factor->(expr)\n" );
            
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
            printf("产生式：factor->id\n"); 
            p = LookUpAllSymbolList( TopSymbolList, yyvsp[0].id.name );
            if( p != NULL ) {
                strcpy( yyval.factor.str, p->name );
                yyval.factor.type  = p->type;
                yyval.factor.addr  = p->addr;
                yyval.factor.width = p->width;
            }							    
            else {
                yyerror( "变量名没有定义" );
                strcpy( yyval.factor.str, "no_id_defined" ); /*容错处理*/
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
            printf("产生式：factor->CONST\n");

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
