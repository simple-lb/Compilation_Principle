#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/02/91\n\
 Modified 11/4/96 by Wensong Zhang to make getenv() function right\n\
 and to remove the warning of unreferenced yyerrlab and yynewerror labels";
#endif
#define YYBYACC 1
#line 1 ".\expr.y"
 

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

#line 26 "y.tab.c"
#define NUMBER 257
#define ID_TKN 258
#define RELOP_TKN 259
#define RELOP_LT 260
#define RELOP_LE 261
#define RELOP_EQ 262
#define RELOP_NE 263
#define RELOP_GT 264
#define RELOP_GE 265
#define IF_TKN 266
#define ELSE_TKN 267
#define BREAK_TKN 268
#define WHILE_TKN 269
#define DO_TKN 270
#define UMINUS 271
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    1,    2,    2,    3,    3,    3,    3,    3,    3,
    6,    6,    5,    7,    7,    4,    9,    9,   10,   10,
    8,   12,   12,   13,   13,   11,   11,   11,
};
short yylen[] = {                                         2,
    1,    3,    2,    0,    4,    6,    5,    7,    2,    1,
    2,    0,    2,    2,    0,    2,    2,    0,    2,    2,
    2,    2,    0,    2,    2,    3,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    1,    0,    0,    0,    0,    0,   10,    0,
    0,    0,    0,    9,    0,    0,    2,    3,   28,   27,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    5,
    0,    0,   16,    0,    0,    0,   21,    0,    0,   13,
    0,    0,    0,   26,   19,   20,   17,   24,   25,   22,
   14,    0,    7,    0,    0,    6,    0,   11,    8,
};
short yydgoto[] = {                                       2,
    9,   10,   11,   25,   26,   56,   40,   23,   33,   34,
   24,   37,   38,
};
short yysindex[] = {                                   -111,
 -113,    0,    0,  -38,  -16,  -34,  -13, -113,    0,  -99,
 -113,  -37,  -37,    0,  -37, -241,    0,    0,    0,    0,
  -37,  -29,  -30,  -33, -230,  -10,   -9,   -6,   -8,    0,
  -37,  -37,    0,  -30,  -37,  -37,    0,  -33,  -37,    0,
 -113, -113,  -37,    0,    0,    0,    0,    0,    0,    0,
    0, -232,    0,   -4, -113,    0,  -18,    0,    0,
};
short yyrindex[] = {                                      0,
  -83,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -83,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -40,  -41,    2,    0,    0,    0,    0,    0,
    0,    0,    0,  -40,    0,    0,    0,  -41,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -118,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   44,   34,   -2,   -1,   -7,    0,    0,  -15,   12,    0,
  -14,    9,    0,
};
#define YYTABLESIZE 221
short yytable[] = {                                      23,
   18,   23,   21,   23,   12,   16,   12,   27,   35,    1,
   22,    1,   31,   36,   32,   45,   46,   23,   18,   29,
   48,   49,   12,   13,   14,   17,   15,   28,   39,   30,
   41,   42,   44,   43,   55,   54,   57,   51,   52,   53,
   59,    4,   15,    3,   18,   47,   50,    0,    0,    0,
    0,    0,   58,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   12,
    0,    0,    0,    0,    4,    0,    0,   12,    0,   12,
   12,   12,    5,    0,    6,    7,    8,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   23,   18,   19,
   20,
};
short yycheck[] = {                                      41,
   41,   43,   40,   45,  123,    8,  125,   15,   42,  123,
   12,  123,   43,   47,   45,   31,   32,   59,   59,   21,
   35,   36,   61,   40,   59,  125,   40,  269,  259,   59,
   41,   41,   41,   40,  267,   43,   41,   39,   41,   42,
   59,  125,   41,    0,   11,   34,   38,   -1,   -1,   -1,
   -1,   -1,   55,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,
   -1,   -1,   -1,   -1,  258,   -1,   -1,  266,   -1,  268,
  269,  270,  266,   -1,  268,  269,  270,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  259,  259,  257,
  258,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 271
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'","'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,"';'",
0,"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"NUMBER","ID_TKN","RELOP_TKN","RELOP_LT","RELOP_LE","RELOP_EQ","RELOP_NE",
"RELOP_GT","RELOP_GE","IF_TKN","ELSE_TKN","BREAK_TKN","WHILE_TKN","DO_TKN",
"UMINUS",
};
char *yyrule[] = {
"$accept : program",
"program : block",
"block : '{' stmts '}'",
"stmts : stmt stmts",
"stmts :",
"stmt : ID_TKN '=' expr ';'",
"stmt : IF_TKN '(' bool ')' stmt A",
"stmt : WHILE_TKN '(' bool ')' stmt",
"stmt : DO_TKN stmt WHILE_TKN '(' bool ')' ';'",
"stmt : BREAK_TKN ';'",
"stmt : block",
"A : ELSE_TKN stmt",
"A :",
"bool : expr B",
"B : RELOP_TKN expr",
"B :",
"expr : term expr1",
"expr1 : C expr1",
"expr1 :",
"C : '+' term",
"C : '-' term",
"term : factor term1",
"term1 : D term1",
"term1 :",
"D : '*' factor",
"D : '/' factor",
"factor : '(' expr ')'",
"factor : ID_TKN",
"factor : NUMBER",
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
#line 85 ".\expr.y"

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





   
#line 245 "y.tab.c"
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
#line 42 ".\expr.y"
{ printf("program -> block\n"); }
break;
case 2:
#line 44 ".\expr.y"
{ printf("block -> {stmts}\n"); }
break;
case 3:
#line 46 ".\expr.y"
{ printf("stmts -> {stmts}\n"); }
break;
case 4:
#line 47 ".\expr.y"
{ printf("stmts -> empty\n"); }
break;
case 5:
#line 49 ".\expr.y"
{ printf("stmt -> id = expr;\n"); }
break;
case 6:
#line 50 ".\expr.y"
{ printf("stmt -> if (bool) stmt A\n"); }
break;
case 7:
#line 51 ".\expr.y"
{ printf("stmt -> while (bool) stmt\n"); }
break;
case 8:
#line 52 ".\expr.y"
{ printf("stmt -> do stmt while (bool); \n"); }
break;
case 9:
#line 53 ".\expr.y"
{ printf("stmt -> break;\n"); }
break;
case 10:
#line 54 ".\expr.y"
{ printf("stmt -> block\n"); }
break;
case 11:
#line 56 ".\expr.y"
{ printf("A -> else stmt\n"); }
break;
case 12:
#line 57 ".\expr.y"
{ printf("A -> empty\n"); }
break;
case 13:
#line 59 ".\expr.y"
{ printf("bool -> expr B\n"); }
break;
case 14:
#line 61 ".\expr.y"
{ printf("B -> relop expr\n"); }
break;
case 15:
#line 62 ".\expr.y"
{ printf("B -> empty\n"); }
break;
case 16:
#line 64 ".\expr.y"
{ printf("expr -> term expr1\n"); }
break;
case 17:
#line 66 ".\expr.y"
{ printf("expr1 -> C expr1\n"); }
break;
case 18:
#line 67 ".\expr.y"
{ printf("expr1 -> empty\n"); }
break;
case 19:
#line 69 ".\expr.y"
{ printf("C -> + term\n"); }
break;
case 20:
#line 70 ".\expr.y"
{ printf("C -> - term\n"); }
break;
case 21:
#line 72 ".\expr.y"
{ printf("term -> factor term1\n"); }
break;
case 22:
#line 74 ".\expr.y"
{ printf("term1 -> D term1\n"); }
break;
case 23:
#line 75 ".\expr.y"
{ printf("term1 -> empty\n"); }
break;
case 24:
#line 77 ".\expr.y"
{ printf("D -> * factor\n"); }
break;
case 25:
#line 78 ".\expr.y"
{ printf("D -> / factor\n"); }
break;
case 26:
#line 80 ".\expr.y"
{ printf("factor -> (expr)\n"); }
break;
case 27:
#line 81 ".\expr.y"
{ printf("factor -> id\n"); }
break;
case 28:
#line 82 ".\expr.y"
{ printf("factor -> num\n"); }
break;
#line 496 "y.tab.c"
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
