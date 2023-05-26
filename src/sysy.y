%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST LOR LAND EQ NEQ LE GE IF ELSE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp LOrExp LAndExp EqExp RelExp AddExp MulExp PrimaryExp LVal UnaryExp UnaryOp Number
%type <ast_val> BlockItem Decl ConstDecl VarDecl BType ConstDef VarDef ConstInitVal InitVal ConstExp
//%type <int_val> Number
//%nonassoc ELSE
//%nonassoc LOWER_TNAH_ELSE

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->functype = "int"; 
    $$ = ast;
  }
  ;

//Block
//  : '{' Stmt '}' {
//    auto ast = new BlockAST();
//    ast->stmt = unique_ptr<BaseAST>($2);
//    $$ = ast;
//  }
//  ;
Block
  : '{' BlockItem '}' {
    auto ast = new BlockAST();
    ast->blockitem = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | '{' '}'{
    auto ast = new BlockAST();
    ast->blockitem = NULL;
    $$ = ast;
  }

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->type = BlockItem_Decl_Ty;
    ast->data.decl_ty.decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->type = BlockItem_Stmt_Ty;
    ast->data.stmt_ty.stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | BlockItem Decl{
    auto ast = new BlockItemAST();
    ast->type = BlockItem_Block_Decl_Ty;
    ast->data.block_decl_ty.blockitem = unique_ptr<BaseAST>($1);
    ast->data.block_decl_ty.decl = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | BlockItem Stmt{
    auto ast = new BlockItemAST();
    ast->type = BlockItem_Block_Stmt_Ty;
    ast->data.block_stmt_ty.blockitem = unique_ptr<BaseAST>($1);
    ast->data.block_stmt_ty.stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

Decl 
  : ConstDecl {
    auto ast = new DeclAST();
    ast->type = Decl_ConstDecl_Ty;
    ast->data.constdecl_ty.constdecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->type = Decl_VarDecl_Ty;
    ast->data.vardecl_ty.vardecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstDecl
  : CONST BType ConstDef ';' {
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    ast->constdef = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

VarDecl
  : BType VarDef ';' {
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->vardef = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

BType
  : INT {
    auto ast = new BTypeAST();
    ast->btype = "int";
    $$ = ast;
  }

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->type = ConstDef_Single_Ty;
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '=' ConstInitVal ',' ConstDef {
    auto ast = new ConstDefAST();
    ast->type = ConstDef_Mul_Ty;
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    ast->constdef = unique_ptr<BaseAST>($5);
    $$ = ast;
  }

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->type = VarDef_noinit_Ty;
    ast->ident = *unique_ptr<std::string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->type = VarDef_init_Ty;
    ast->ident = *unique_ptr<std::string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ',' VarDef {
    auto ast = new VarDefAST();
    ast->type = VarDef_VarDef_noinit_Ty;
    ast->ident = *unique_ptr<std::string>($1);
    ast->vardef = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '=' InitVal ',' VarDef {
    auto ast = new VarDefAST();
    ast->type = VarDef_VarDef_init_Ty;
    ast->ident = *unique_ptr<std::string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    ast->vardef = unique_ptr<BaseAST>($5);
    $$ = ast;
  }

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
    

// Stmt
//   : RETURN Number ';' {
//     auto ast = new StmtAST();
//     ast->number = ($2);
//     $$ = ast;
//   }
//   ;
Stmt
  : LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->type = Stmt_LVal_Ty;
    ast->data.lval_ty.lval = unique_ptr<BaseAST>($1);
    ast->data.lval_ty.exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->type = Stmt_Block_Ty;
    ast->data.block_ty.block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->type = Stmt_Exp_Ty;
    ast->data.exp_ty.exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type = Stmt_If_Ty;
    ast->data.if_ty.exp = unique_ptr<BaseAST>($3);
    ast->data.if_ty.if_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->type = Stmt_If_Else_Ty;
    ast->data.ifelse_ty.exp = unique_ptr<BaseAST>($3);
    ast->data.ifelse_ty.if_stmt = unique_ptr<BaseAST>($5);
    ast->data.ifelse_ty.else_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->type = Stmt_Comma_Ty;
    $$ = ast;
  }
  | RETURN Exp ';'{
    auto ast = new StmtAST();
    ast->type = Stmt_Return_Ty;
    ast->data.return_ty.exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

/*======================= 3. operation part begin ==================*/
Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->type = Lor_LAndExp_Ty;
    ast->data.landexp_ty.land_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp LOR LAndExp {
    auto ast = new LOrExpAST();
    ast->type = Lor_LorExp_Ty;
    ast->data.lorexp_ty.lor_exp = unique_ptr<BaseAST>($1);
    ast->data.lorexp_ty.land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->type = LAnd_EqExp_Ty;
    ast->data.eqexp_ty.eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp LAND EqExp {
    auto ast = new LAndExpAST();
    ast->type = LAnd_LAndExp_Ty;
    ast->data.landexp_ty.land_exp = unique_ptr<BaseAST>($1);
    ast->data.landexp_ty.eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->type = Eq_RelExp_Ty;
    ast->data.relexp_ty.rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExpAST();
    ast->type = Eq_EqExp_Ty;
    ast->data.eqexp_ty.eq_exp = unique_ptr<BaseAST>($1);
    ast->data.eqexp_ty.rel_exp = unique_ptr<BaseAST>($3);
    ast->data.eqexp_ty.op = 0;
    $$ = ast;
  }
  | EqExp NEQ RelExp {
    auto ast = new EqExpAST();
    ast->type = Eq_EqExp_Ty;
    ast->data.eqexp_ty.eq_exp = unique_ptr<BaseAST>($1);
    ast->data.eqexp_ty.rel_exp = unique_ptr<BaseAST>($3);
    ast->data.eqexp_ty.op = 1;
    $$ = ast;
  }

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->type = Rel_AddExp_Ty;
    ast->data.addexp_ty.add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp '<' AddExp {
    auto ast = new RelExpAST();
    ast->type = Rel_RelExp_Ty;
    ast->data.relexp_ty.rel_exp = unique_ptr<BaseAST>($1);
    ast->data.relexp_ty.add_exp = unique_ptr<BaseAST>($3);
    ast->data.relexp_ty.op = 0;
    $$ = ast;
  }
  | RelExp '>' AddExp {
    auto ast = new RelExpAST();
    ast->type = Rel_RelExp_Ty;
    ast->data.relexp_ty.rel_exp = unique_ptr<BaseAST>($1);
    ast->data.relexp_ty.add_exp = unique_ptr<BaseAST>($3);
    ast->data.relexp_ty.op = 1;
    $$ = ast;
  }
  | RelExp LE AddExp {
    auto ast = new RelExpAST();
    ast->type = Rel_RelExp_Ty;
    ast->data.relexp_ty.rel_exp = unique_ptr<BaseAST>($1);
    ast->data.relexp_ty.add_exp = unique_ptr<BaseAST>($3);
    ast->data.relexp_ty.op = 2;
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new RelExpAST();
    ast->type = Rel_RelExp_Ty;
    ast->data.relexp_ty.rel_exp = unique_ptr<BaseAST>($1);
    ast->data.relexp_ty.add_exp = unique_ptr<BaseAST>($3);
    ast->data.relexp_ty.op = 3;
    $$ = ast;
  }


AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->type = Add_MulExp_Ty;
    ast->data.mulexp_ty.mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp{
    auto ast = new AddExpAST();
    ast->type = Add_AddExp_Ty;
    ast->data.addexp_ty.add_exp = unique_ptr<BaseAST>($1);
    ast->data.addexp_ty.mul_exp = unique_ptr<BaseAST>($3);
    ast->data.addexp_ty.op = '+';
    $$ = ast;
  }
  | AddExp '-' MulExp{
    auto ast = new AddExpAST();
    ast->type = Add_AddExp_Ty;
    ast->data.addexp_ty.add_exp = unique_ptr<BaseAST>($1);
    ast->data.addexp_ty.mul_exp = unique_ptr<BaseAST>($3);
    ast->data.addexp_ty.op = '-';
    $$ = ast;
  }

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->type = Mul_UnaryExp_Ty;
    ast->data.unaryexp_ty.unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp{
    auto ast = new MulExpAST();
    ast->type = Mul_MulExp_Ty;
    ast->data.mulexp_ty.mul_exp = unique_ptr<BaseAST>($1);
    ast->data.mulexp_ty.unary_exp = unique_ptr<BaseAST>($3);
    ast->data.mulexp_ty.op = '*';
    $$ = ast;
  }
  | MulExp '/' UnaryExp{
    auto ast = new MulExpAST();
    ast->type = Mul_MulExp_Ty;
    ast->data.mulexp_ty.mul_exp = unique_ptr<BaseAST>($1);
    ast->data.mulexp_ty.unary_exp = unique_ptr<BaseAST>($3);
    ast->data.mulexp_ty.op = '/';
    $$ = ast;
  }
  | MulExp '%' UnaryExp{
    auto ast = new MulExpAST();
    ast->type = Mul_MulExp_Ty;
    ast->data.mulexp_ty.mul_exp = unique_ptr<BaseAST>($1);
    ast->data.mulexp_ty.unary_exp = unique_ptr<BaseAST>($3);
    ast->data.mulexp_ty.op = '%';
    $$ = ast;
  }
  

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->type = PrimaryExp_Ty;
    ast->data.primary_ty.primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->type = UnaryExp_Ty;
    ast->data.unary_ty.unary_op = unique_ptr<BaseAST>($1);
    ast->data.unary_ty.unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }

UnaryOp
  : '+' {
    auto ast = new UnaryOpAST();
    ast->op = '+';
    $$ = ast; //forget return cause segment false
  }
  | '-' {
    auto ast = new UnaryOpAST();
    ast->op = '-';
    $$ = ast;
  }
  | '!' {
    auto ast = new UnaryOpAST();
    ast->op = '!';
    $$ = ast;
  }

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->type = Exp_Ty;
    ast->data.exp_ty.exp = unique_ptr<BaseAST> ($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->type = LVal_Ty;
    ast->data.lval_ty.lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->type = Number_Ty;
    ast->data.number_ty.number = unique_ptr<BaseAST> ($1);
    $$ = ast;
  }

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->number = ($1);
    $$ = ast;
  }
  ;
/*======================= 3. operation part end  ==================*/
/*======================= 4. variable part begin ==================*/

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
