#ifndef _AST_H_
#define _AST_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <variant>
#include <unordered_map>
#include <vector>
#include <string>

//define difference type for the same noe-terminal AST
typedef enum {Lor_LAndExp_Ty, Lor_LorExp_Ty} LorExpTy;
typedef enum {LAnd_EqExp_Ty, LAnd_LAndExp_Ty} LAndExpTy;
typedef enum {Eq_RelExp_Ty, Eq_EqExp_Ty} EqExpTy;
typedef enum {Rel_AddExp_Ty, Rel_RelExp_Ty} RelExpTy;
typedef enum {Add_MulExp_Ty, Add_AddExp_Ty} AddExpTy;
typedef enum {Mul_UnaryExp_Ty, Mul_MulExp_Ty} MulExpTy;
typedef enum {PrimaryExp_Ty, UnaryExp_Ty} UnaryExpTy;
typedef enum {Exp_Ty, LVal_Ty, Number_Ty} PrimaryExpTy;
typedef enum {Stmt_Block_Ty, Stmt_Exp_Ty, Stmt_Return_Ty, Stmt_LVal_Ty, Stmt_Comma_Ty, Stmt_If_Ty, Stmt_If_Else_Ty, Stmt_While_Ty, Stmt_Break_Ty, Stmt_Continue_Ty} StmtTy;

typedef enum {BlockItem_Decl_Ty, BlockItem_Stmt_Ty, BlockItem_Block_Decl_Ty, BlockItem_Block_Stmt_Ty} BlockItemTy;
typedef enum {ConstDef_Single_Ty, ConstDef_Mul_Ty} ConstDefTy;
typedef enum {Decl_ConstDecl_Ty, Decl_VarDecl_Ty} DeclTy;
typedef enum {VarDef_init_Ty, VarDef_noinit_Ty, VarDef_VarDef_init_Ty, VarDef_VarDef_noinit_Ty} VarDefTy;

//struct for a variable or a const
typedef struct _value{
  int const_val;          //if const, const value
  int variable_val;       //unused
  int is_const;           //whether this value is const
  std::string index_str;  //indentify for this value in koopa
}value;

static int count = 0; //the use of temp in koopa like %1, %2

static int if_count = 0;
static int block_end = 0;

static std::vector<int> while_nest;

//this is the symbol table for a function, which can has multiple block which corresponding to a map in this vector
//each map record current block, the map from ident to value(integer for a const and string in koopa for a variable)
static std::vector<std::unordered_map<std::string, value>> domain;
static int ident_num = 0;
//static std::unordered_map<std::string, value> val; //table for const or variable value

//struct to transfer temp result for Dump function to get result of sub expression
typedef struct para {
  bool is_data;
  int p1_temp;
  int p2_temp;
  int p3_temp;
  int p1;
  int p2;
  int p3;
  std::string str; // this is particularly design for LVal to get identifier which cannot implemented by Dump or parameter;
} parameter;

typedef parameter* parameter_t;

void print_operator(const parameter &p1, const parameter &p2);//function to print operator for a koopaIR clause
int GetBlockStr(std::string &str); //get the IR indet in map of the correct block for a particular identifier 

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0; //call when generate code
  virtual void Dump(parameter_t parameter_) const = 0; //call when generate code and need result
  virtual int CalcConst() const = 0; //calculate value for a expression
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    // std::cout << "CompUnitAST { ";
    func_def->Dump();
    // std::cout << " }";
  }
  void Dump(parameter_t parameter_) const override {} 
  int CalcConst() const override{return 0;}
};

// FuncDef 也是 BaseAST
// FuncDef -> FuncType IDENT "(" ")" Block
class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type; //function type
  std::string ident;                  //function name
  std::unique_ptr<BaseAST> block;     //block in this function


 void Dump() const override {
    //output information for this function
    std::cout << "fun ";
    std::cout << "@" << ident << "(): ";
    func_type->Dump();
    std::cout << "{" << std::endl;
    std::cout << "%entry: " << std::endl;
    //parameter p;
    block->Dump(); //call Dump to transfer the block into code
    std::cout << "}" << std::endl;
    // std::cout << " }";
  }
  void Dump(parameter_t parameter_)const override{} 
  int CalcConst() const override{return 0;}
};

// FuncType ->  "VOID"
//          |   "INT"
class FuncTypeAST : public BaseAST {
public:
  std::string functype; //the function type

  void Dump() const override {
    // std::cout << "FuncTypeAST { ";
    // std::cout << functype;
    std::cout << "i32 ";
  }
  void Dump(parameter_t parameter_)const override {} 
  int CalcConst() const override{return 0;}
};

//Block ->  "{" BlockItem "}"
//      |   "{" "}"
class BlockAST : public BaseAST{
public:

  std::unique_ptr<BaseAST> blockitem;

  void Dump() const override {
    //std::cout << "{ " << std::endl;
    
    if(!blockitem){
      return;
    }
    std::unordered_map<std::string, value> val; 
    if(!domain.empty()){
      val = domain.back();
    }
    domain.push_back(val);
    blockitem->Dump();

    domain.pop_back();
    //std::cout << "}" << std::endl;
  }
  void Dump(parameter_t parameter_)const override{} 
  int CalcConst() const override{return -1;}
};

//BlockItem ->  Decl
//          |   Stmt
//          |   BlockItem Decl
//          |   BlockItem Stmt
class BlockItemAST : public BaseAST{
  public:
    BlockItemTy type;

    struct {
      struct {
        std::unique_ptr<BaseAST> decl;
      }decl_ty;

      struct {
        std::unique_ptr<BaseAST> stmt;
      }stmt_ty;

      struct {
        std::unique_ptr<BaseAST> blockitem;
        std::unique_ptr<BaseAST> decl;
      }block_decl_ty;

      struct {
        std::unique_ptr<BaseAST> blockitem;
        std::unique_ptr<BaseAST> stmt;
      }block_stmt_ty;
    } data;

  void Dump() const override {
    switch(type){
      case BlockItem_Decl_Ty: data.decl_ty.decl->Dump();break;
      case BlockItem_Stmt_Ty: data.stmt_ty.stmt->Dump();break;
      case BlockItem_Block_Decl_Ty: data.block_decl_ty.blockitem->Dump();data.block_decl_ty.decl->Dump();break;
      case BlockItem_Block_Stmt_Ty: data.block_stmt_ty.blockitem->Dump();data.block_stmt_ty.stmt->Dump();break;
    }
  }
  void Dump(parameter_t parameter_)const override{} 
  int CalcConst() const override{return -1;}
};


// Stmt ->  RETURN Exp ";"
//      |   Block
//      |   Exp ";"
//      |   ";"
//      |   LVal "=" Exp ";"
class StmtAST : public BaseAST{
 public:
  StmtTy type;
  
  struct {
    struct {
      std::unique_ptr<BaseAST> block;
    }block_ty;
    struct {
      std::unique_ptr<BaseAST> exp;
    }exp_ty;
    struct {
      std::unique_ptr<BaseAST> lval;
      std::unique_ptr<BaseAST> exp;
    }lval_ty;
    struct {
      std::unique_ptr<BaseAST> exp;
      std::unique_ptr<BaseAST> if_stmt;
      std::unique_ptr<BaseAST> else_stmt;
    }ifelse_ty;
    struct {
      std::unique_ptr<BaseAST> exp;
      std::unique_ptr<BaseAST> if_stmt;
    }if_ty;
    struct {
      std::unique_ptr<BaseAST> exp;
      std::unique_ptr<BaseAST> stmt;
    }while_ty;
    struct {
        std::unique_ptr<BaseAST> exp;
    }return_ty;
  }data;

  void Dump() const override {
    if(block_end){
      return;
    }
    parameter p;   
    if(type == Stmt_Return_Ty){
      data.return_ty.exp->Dump(&p);
      std::cout << "  ";
      if(p.is_data){
        std::cout << "ret " << p.p1 << std::endl;
      }
      else
        std::cout << "ret %" << p.p1 << std::endl;
      block_end = 1;
      //std::cout << (p.p1_temp == 1) ? "%" : "@";
      //std::cout << p.p1 << endl;
    }
    else if(type == Stmt_LVal_Ty){
      p.is_data = 1;
      data.lval_ty.lval->Dump(&p);
      std::string str = p.str;
      data.lval_ty.exp->Dump(&p);

      int index;
      for(index = domain.size() - 1; index >= 0; --index){
        if(domain[index].find(str) != domain[index].end()){
          if(domain[index][str].is_const == 1){
            //error define a const with a variable
            //assign to a const value
          }
          str = domain[index][str].index_str;
          break;
          //return domain[index][ident].const_val;
        }
      }
      //search str's block
      if(-1 == index){
        //error, undefined ident str
        std::cout << "undefine " << str << std::endl;
      }

      if(p.is_data == 1){
        std::cout << "  store " << p.p1 << ", @" << str << std::endl;
      }
      else{
        std::cout << "  store " << "%" << p.p1 << ", @" << str << std::endl;
      } 
    }
    else if(type == Stmt_Block_Ty){
      if(!data.block_ty.block)
        std::cout << "null block" << std::endl;
      data.block_ty.block->Dump();
    }
    else if(type == Stmt_Exp_Ty){
      parameter p;
      data.exp_ty.exp->Dump(&p);
    }
    else if(type == Stmt_If_Ty){
      if(block_end){
        return;
      }
      int cur_if_count = if_count;
      if_count++;
      parameter p;
      data.if_ty.exp->Dump(&p);
      if(p.is_data){
        std::cout << "  br " << p.p1 << ", %then" << cur_if_count << ", %end" << cur_if_count << std::endl;
      }
      else{
        std::cout << "  br %" << p.p1 << ", %then" << cur_if_count << ", %end" << cur_if_count << std::endl;
      }
      
      //part of then
      std::cout << "%then" << cur_if_count << ":" << std::endl;
      block_end = 0; //reset when come to new block
      data.if_ty.if_stmt->Dump();
      if(!block_end){
        std::cout << "  jump %end" << cur_if_count << std::endl;
      }
      std::cout << "%end" << cur_if_count << ":" << std::endl;
      block_end = 0; //reset when come to new block
      //if_count++;

    }
    else if(type == Stmt_If_Else_Ty){
      if(block_end){
        return;
      }
      int cur_if_count = if_count;
      int if_stmt_end = 1;
      if_count++;
      parameter p;
      data.ifelse_ty.exp->Dump(&p);
      if(p.is_data){
        std::cout << "  br " << p.p1 << ", %then" << cur_if_count << ", %else" << cur_if_count << std::endl;
      }
      else{
        std::cout << "  br %" << p.p1 << ", %then" << cur_if_count << ", %else" << cur_if_count << std::endl;
      }
      
      //part of then
      std::cout << "%then" << cur_if_count << ":" << std::endl;
      block_end = 0;
      data.ifelse_ty.if_stmt->Dump();
      if(!block_end){
        std::cout << "  jump %end" << cur_if_count << std::endl;
        if_stmt_end = 0;
      }

      std::cout << "%else" << cur_if_count << ":" << std::endl;
      block_end = 0;
      data.ifelse_ty.else_stmt->Dump();
      if(!block_end){
        std::cout << "  jump %end" << cur_if_count << std::endl;
      }

      if(!(block_end && if_stmt_end)){
        std::cout << "%end" << cur_if_count << ":" << std::endl;
      }
      if(block_end && if_stmt_end){
        block_end = 1;
      }
      //if_count++;
      //block_end = 0;
    }
    else if(type == Stmt_While_Ty){
      if(block_end)
        return;
      int cur_while = if_count;
      if_count++;

      std::cout << "  jump %while_entry_" << cur_while << std::endl;
      std::cout << "%while_entry_" << cur_while << ":" << std::endl;
      parameter p;
      data.while_ty.exp->Dump(&p);
      
      if(p.is_data){
        std::cout << "  br " << p.p1 << ", %while_body_" << cur_while << ", %while_end_" << cur_while << std::endl;
      }
      else{
        std::cout << "  br %" << p.p1 << ", %while_body_" << cur_while << ", %while_end_" << cur_while << std::endl;
      }

      while_nest.push_back(cur_while);
      std::cout << "%while_body_" << cur_while << ":" << std::endl;
      data.while_ty.stmt->Dump();
      if(!block_end){
        std::cout << "  jump %while_entry_" << cur_while << std::endl;
      }
      while_nest.pop_back();
      std::cout << "%while_end_" << cur_while << ":" << std::endl;
      block_end = 0;
    }

    else if(type == Stmt_Break_Ty){
      if(while_nest.empty()){
        //error, a break not in while

      }
      std::cout << "  jump %while_end_" << while_nest.back() << std::endl;
      block_end = 1;
    }
    else if(type == Stmt_Continue_Ty){
      if(while_nest.empty()){
        //error, a break not in while

      }
      std::cout << "  jump %while_entry_" << while_nest.back() << std::endl;
      block_end = 1;
    }

    else if(type == Stmt_Comma_Ty){
      
    }
  }
  void Dump(parameter_t parameter_)const override {} 
  int CalcConst() const override{return 0;}
};

// LVal ->  IDENT
class LValAST : public BaseAST {
  public:
    std::string ident;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      if(parameter_->is_data == 1){
        parameter_->str = ident;
        return;
      }

      int index;
      for(index = domain.size() - 1; index >= 0; --index){
        if(domain[index].find(ident) != domain[index].end()){
          break;
        }
      }

      if(index == -1){
        //error undefined ident
      }

      auto &val = domain[index];

      if(val[ident].is_const == 1){
        parameter_->is_data = 1;
        parameter_->p1 = val[ident].const_val;
        parameter_->str = ident;
      }
      else{
        //std::cout << "  %" << count++ << " = load @" << ident << "_" << index+1 << std::endl;
        std::cout << "  %" << count++ << " = load @" << val[ident].index_str << std::endl;
        parameter_->is_data = 0;
        parameter_->p1 = count - 1;
        parameter_->str = ident;
      }
    }
    int CalcConst() const override{
      int index;
      for(index = domain.size() - 1; index >= 0; --index){
        if(domain[index].find(ident) != domain[index].end()){
          if(domain[index][ident].is_const == 1){
            //error define a const with a variable
          }
          return domain[index][ident].const_val;
        }
      }
      if(index == -1){
        //error undefined ident
      }
      return -1;
    }
};

/*============================================ 3. oprating part begin ================================================*/

// ast for expression
// Exp -> UnaryExp
class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> lor_exp;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
        lor_exp->Dump(parameter_);
    }
    int CalcConst() const override{
      return lor_exp->CalcConst();
    }
};

// LOrExp ->  LAndExp
//        |   LOrExp LOR LAndExp
class LOrExpAST : public BaseAST {
  public:
    LorExpTy type;
    struct {
      struct {
        std::unique_ptr<BaseAST> land_exp;
      }landexp_ty;
      
      struct {
        std::unique_ptr<BaseAST> lor_exp;
        std::unique_ptr<BaseAST> land_exp;
      }lorexp_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == Lor_LAndExp_Ty){
        data.landexp_ty.land_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == Lor_LorExp_Ty){
        data.lorexp_ty.lor_exp->Dump(&p1);
        data.lorexp_ty.land_exp->Dump(&p2);

        std::cout << "  %" <<  count++ << " = or ";

        print_operator(p1, p2);
        std:: cout << " %" << count++ << " = ne 0, %" << count - 2 << std::endl;
        parameter_->is_data = false;
        parameter_->p1 = count - 1;
      }
    }
    int CalcConst() const override{
      if(type == Lor_LAndExp_Ty){
        return data.landexp_ty.land_exp->CalcConst();
      }
      else if(type == Lor_LorExp_Ty){
        return data.lorexp_ty.lor_exp->CalcConst() || data.lorexp_ty.land_exp->CalcConst();
      }
      return -1;
    }
};

// LAndExp -> EqExp
//         |  LAndExp LAND EqExp
class LAndExpAST : public BaseAST {
  public:
    LAndExpTy type;
    struct {
      struct {
        std::unique_ptr<BaseAST> eq_exp;
      }eqexp_ty;
      
      struct {
        std::unique_ptr<BaseAST> land_exp;
        std::unique_ptr<BaseAST> eq_exp;
      }landexp_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == LAnd_EqExp_Ty){
        data.eqexp_ty.eq_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == LAnd_LAndExp_Ty){
        data.landexp_ty.land_exp->Dump(&p1);
        data.landexp_ty.eq_exp->Dump(&p2);

//        print_operator(p1, p2);

        //compare first result with 0
        std::cout << "  %" << count++ << " = ne 0, ";
        if(p1.is_data == 0){
          std::cout << "%" << p1.p1 << std::endl;
        }
        else{
          std::cout << p1.p1 << std::endl;
        }

        //compare second result with 0
        std::cout << "  %" << count++ << " = ne 0, ";
        if(p2.is_data == 0){
          std::cout << "%" << p2.p1 << std::endl;
        }
        else{
          std::cout << p2.p1 << std::endl;
        }
        
        //compute final result
        std::cout << "  %" << count++ << " = and %" << count-3 << ", %" << count-2 << std::endl;
        parameter_->is_data = false;
        parameter_->p1 = count - 1;
      }
    }
    int CalcConst() const override{
      if(type == LAnd_EqExp_Ty){
        return data.eqexp_ty.eq_exp->CalcConst();
      }
      else if(type == LAnd_LAndExp_Ty){
        return data.landexp_ty.land_exp->CalcConst() && data.landexp_ty.eq_exp->CalcConst();
      }
      return -1;
    }
};

//  EqExp ->  RelExp
//        |   EqExp EQ RelExp
class EqExpAST : public BaseAST {
  public:
    EqExpTy type;
    struct {
      struct {
        std::unique_ptr<BaseAST> rel_exp;
      }relexp_ty;
      
      struct {
        int op; // 0 for '==', 1 for '!='
        std::unique_ptr<BaseAST> eq_exp;
        std::unique_ptr<BaseAST> rel_exp;
      }eqexp_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == Eq_RelExp_Ty){
        data.relexp_ty.rel_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == Eq_EqExp_Ty){
        data.eqexp_ty.eq_exp->Dump(&p1);
        data.eqexp_ty.rel_exp->Dump(&p2);

        if(data.eqexp_ty.op == 0){
          std::cout << "  %" << count++ << " = eq ";
        }
        else if(data.eqexp_ty.op == 1){
          std::cout << "  %" << count++ << " = ne ";
        }

        print_operator(p1, p2);

        parameter_->is_data = false;
        parameter_->p1 = count - 1;
      }
    }
    int CalcConst() const override{
      if(type == Eq_RelExp_Ty){
        return data.relexp_ty.rel_exp->CalcConst();
      }
      else if(type == Eq_EqExp_Ty){
        int eq_res = data.eqexp_ty.eq_exp->CalcConst();
        int rel_res = data.eqexp_ty.rel_exp->CalcConst();
        if(data.eqexp_ty.op == 0){
          return (eq_res == rel_res);
        }
        else{
          return (eq_res != rel_res);
        }
      }
      return -1;
    }
};

// RelExp -> AddExp
//        |  RelExp "<" AddExp
//        |  RelExp ">" AddExp
//        |  RelExp LE  AddExp
//        |  RelExp GE  AddExp
class RelExpAST : public BaseAST {
  public:
    RelExpTy type;
    struct {
      struct {
        std::unique_ptr<BaseAST> add_exp;
      }addexp_ty;
      
      struct {
        int op; // 0 for '<', 1 for '>', 2 for '<=', 3 for '>='
        std::unique_ptr<BaseAST> rel_exp;
        std::unique_ptr<BaseAST> add_exp;
      }relexp_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == Rel_AddExp_Ty){
        data.addexp_ty.add_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == Rel_RelExp_Ty){
        data.relexp_ty.rel_exp->Dump(&p1);
        data.relexp_ty.add_exp->Dump(&p2);

        if(data.relexp_ty.op == 0){
          std::cout << "  %" << count++ << " = lt ";
        }
        else if(data.relexp_ty.op == 1){
          std::cout << "  %" << count++ << " = gt ";
        }
        else if(data.relexp_ty.op == 2){
          std::cout << "  %" << count++ << " = le ";
        }
        else if(data.relexp_ty.op == 3){
          std::cout << "  %" << count++ << " = ge ";
        }

        print_operator(p1, p2);

        parameter_->is_data = false;
        parameter_->p1 = count - 1;
      }
    }
    int CalcConst() const override{
      if(type == Rel_AddExp_Ty){
        return data.addexp_ty.add_exp->CalcConst();
      }
      else if(type == Rel_RelExp_Ty){
        int rel_res = data.relexp_ty.rel_exp->CalcConst();
        int add_res = data.relexp_ty.add_exp->CalcConst();
        if(data.relexp_ty.op == 0){
          return (rel_res < add_res);
        }
        else if(data.relexp_ty.op == 1){
          return (rel_res > add_res);
        }
        else if(data.relexp_ty.op == 2){
          return (rel_res <= add_res);
        }
        else if(data.relexp_ty.op == 3){
          return (rel_res >= add_res);
        }
        return -1;
      }
      return -1;
    }
};

// AddExp -> MulExp
//        |  AddExp "+" MulExp
//        |  AddExp "-" MulExp
class AddExpAST : public BaseAST {
  public:
    AddExpTy type;
    struct{
      struct{
        std::unique_ptr<BaseAST> mul_exp;
      }mulexp_ty;
      
      struct {
        std::unique_ptr<BaseAST> add_exp;
        char op;
        std::unique_ptr<BaseAST> mul_exp;
      }addexp_ty;
    }data;

    void Dump() const override{}
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == Add_MulExp_Ty) {
        data.mulexp_ty.mul_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == Add_AddExp_Ty) {
        data.addexp_ty.add_exp->Dump(&p1);
        data.addexp_ty.mul_exp->Dump(&p2);

        std::cout << "  %" << count++ << " = ";
        if(data.addexp_ty.op == '+'){
          std::cout << "add ";
        }
        else if(data.addexp_ty.op == '-') {
          std::cout << "sub ";
        }

        print_operator(p1, p2);
        parameter_->is_data = false;
        parameter_->p1 = count-1;
      }
    } 
    int CalcConst() const override{

      if(type == Add_MulExp_Ty){
        return data.mulexp_ty.mul_exp->CalcConst();
      }
      else if(type == Add_AddExp_Ty){
        int add_res = data.addexp_ty.add_exp->CalcConst();
        int mul_res = data.addexp_ty.mul_exp->CalcConst();
        if(data.addexp_ty.op == '+'){
          return add_res + mul_res;
        }
        else if(data.addexp_ty.op == '-'){
          return add_res - mul_res;
        }
        return -1;
      }
      return -1;
    }
};

// MulExp -> UnaryExp
//        |  MulExp "*" UnaryExp
//        |  MulExp "/" UnaryExp
//        |  MulExp "%" UnaryExp
class MulExpAST : public BaseAST {
  public:
    MulExpTy type;
    struct{
      struct {
        std::unique_ptr<BaseAST> unary_exp;
      }unaryexp_ty;

      struct {
        std::unique_ptr<BaseAST> mul_exp;
        char op;
        std::unique_ptr<BaseAST> unary_exp;
      }mulexp_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p1, p2;
      if(type == Mul_UnaryExp_Ty) {
        data.unaryexp_ty.unary_exp->Dump(&p1);
        *parameter_ = p1;
      }
      else if(type == Mul_MulExp_Ty) {
        data.mulexp_ty.mul_exp->Dump(&p1);
        data.mulexp_ty.unary_exp->Dump(&p2);

        std::cout << "  %" << count++ << " = ";
        if(data.mulexp_ty.op == '*'){
          std::cout << "mul ";
        }
        else if(data.mulexp_ty.op == '/') {
          std::cout << "div ";
        }
        else if(data.mulexp_ty.op == '%') {
          std::cout << "mod ";
        }

        if(p1.is_data == 0){
          std::cout << "%" << p1.p1 << ", ";
        }
        else{
          std::cout << p1.p1 << ", ";
        }

        if(p2.is_data == 0) {
          std::cout << "%" << p2.p1 << std::endl;
        }
        else{
          std::cout << p2.p1 << std::endl;
        }
        parameter_->is_data = false;
        parameter_->p1 = count-1;
      }
    }
    int CalcConst() const override{
      if(type == Mul_UnaryExp_Ty){
        return data.unaryexp_ty.unary_exp->CalcConst();
      }
      else if(type == Mul_MulExp_Ty){
        int mul_res = data.mulexp_ty.mul_exp->CalcConst();
        int unary_res = data.mulexp_ty.unary_exp->CalcConst();
        if(data.mulexp_ty.op == '*'){
          return mul_res * unary_res;
        }
        else if(data.mulexp_ty.op == '/'){
          return mul_res / unary_res;
        }
        else if(data.mulexp_ty.op == '%'){
          return mul_res % unary_res;
        }
        return -1;
      }
      return -1;
    }
};



// ast for unaryexp
// UnaryExp -> PrimaryExp 
//          |  UnaryOp UnaryExp
class UnaryExpAST : public BaseAST {
  public:
    UnaryExpTy type;
    struct{
      struct {
        std::unique_ptr<BaseAST> primary_exp;
      }primary_ty;
      
      struct {
        std::unique_ptr<BaseAST> unary_op;
        std::unique_ptr<BaseAST> unary_exp;
      }unary_ty;
    }data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p;
      parameter unary_operator;
      if(type == PrimaryExp_Ty){
        data.primary_ty.primary_exp->Dump(&p);   
        *parameter_ = p;
      }
      else if(type == UnaryExp_Ty){
        data.unary_ty.unary_exp->Dump(&p);
        data.unary_ty.unary_op->Dump(&unary_operator);
        char op = unary_operator.p1;
        if(op != '+')
          std::cout << "  ";

        parameter_->is_data = false;
        parameter_->p1 = count;
        if(p.is_data == 0){
          if(op == '-'){
            std::cout << "%" << count++ << " = sub 0, %" << p.p1 << std::endl;
          }
          else if(op == '!'){
            std::cout << "%" << count++ << " = eq 0, %" << p.p1 << std::endl;  
          }
          else if(op == '+'){
            *parameter_ = p;
          }
        }
        else{
          if(op == '-'){
            std::cout << "%" << count++ << " = sub 0, " << p.p1 << std::endl;
          }
          else if(op == '!'){
            std::cout << "%" << count++ << " = eq 0, " << p.p1 << std::endl;
          }
          else if(op == '+'){
            *parameter_ = p;
            //parameter_->is_data = true;
            //parameter_->p1 = p.p1;
          }
        }
      }
    }
    int CalcConst() const override{
      if(type == PrimaryExp_Ty){
        return data.primary_ty.primary_exp->CalcConst();
      }
      else if(type == UnaryExp_Ty){
        int unary_res = data.unary_ty.unary_exp->CalcConst();
        int op = data.unary_ty.unary_op->CalcConst();

        if(op == '+'){
          return unary_res;
        }
        else if(op == '-'){
          return -1 * unary_res;
        }
        else if(op == '!'){
          return !unary_res;
        }
        return -1;
      }
      return -1;
    }
};

// ast for unaryop
// UnaryOp -> '+' | '-' | '!'
class UnaryOpAST : public BaseAST {
  public:
    char op;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter_->p1 = op;
    }
    int CalcConst() const override{
      return (int)op;
    }
};

// ast for PrimaryExp
// PrimaryExp -> "(" Exp ")" 
//            |   Number
class PrimaryExpAST : public BaseAST {
  public:
    PrimaryExpTy type;

    struct {
      struct {
        std::unique_ptr<BaseAST> exp;
      } exp_ty;

      struct {
        std::unique_ptr<BaseAST> lval;
      } lval_ty;

      struct {
        std::unique_ptr<BaseAST> number;
      } number_ty;
    } data;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p;
      if(type == Number_Ty){
        parameter_->is_data = 1;
        data.number_ty.number->Dump(&p);
        parameter_->p1 = p.p1;
      }
      else if(type == LVal_Ty){
        p.is_data = 0;
        data.lval_ty.lval->Dump(&p);
        *parameter_ = p;
      }
      else if(type == Exp_Ty){
        data.exp_ty.exp->Dump(&p);
        *parameter_ = p;
      }
    }

    int CalcConst() const override{
      if(type == Number_Ty){
        return data.number_ty.number->CalcConst();
      }
      else if(type == LVal_Ty){
        //parameter p;
        //data.lval_ty.lval->Dump(&p);
        //std::string str = p.str;
        //auto ite = const_val.find(str);
        //assert(ite != const_val.end());
        //return val[str].is_const ? val[str].const_val : val[str].variable_val;
        return data.lval_ty.lval->CalcConst();
      }
      else if(type == Exp_Ty){
        return data.exp_ty.exp->CalcConst();
      }
      return -1;
    }
};

class NumberAST : public BaseAST {
  public:
    int number;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter_->p1 = number;
    }
    int CalcConst() const override{return number;}
};
/*============================================ 3. oprating part end ================================================*/
/*============================================ 4. variable part begin ================================================*/
class DeclAST : public BaseAST {
  public:
    DeclTy type;

    struct {
      struct {
        std::unique_ptr<BaseAST> constdecl;
      }constdecl_ty;

      struct {
        std::unique_ptr<BaseAST> vardecl;
      }vardecl_ty;
    }data;

    void Dump()const override {
      if(block_end){
        return;
      }
      if(type == Decl_ConstDecl_Ty)
        data.constdecl_ty.constdecl->Dump();
      else
        data.vardecl_ty.vardecl->Dump();
    } 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{return -1;}
};

class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> constdef;

    void Dump()const override {
      constdef->Dump();
    } 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{return -1;}
};

class VarDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> vardef;

    void Dump() const override {
      vardef->Dump();
    }
    void Dump(parameter_t parameter_) const override {}
    int CalcConst() const override {return -1;}
};

class BTypeAST : public BaseAST {
  public:
    std::string btype;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{return -1;}
};

class ConstDefAST : public BaseAST {
  public:
    ConstDefTy type;

    std::string ident;
    std::unique_ptr<BaseAST> constinitval;
    std::unique_ptr<BaseAST> constdef;

    void Dump()const override {
      auto &val = domain.back();
      int cur_value = constinitval->CalcConst();
      std::string cur_str = ident + "_" + std::to_string(domain.size()) + "_" + std::to_string(ident_num++);
      if((val.find(ident) != val.end()) && val[ident].index_str == cur_str){
        //error, redefine a variable in same block
      }
      val[ident].is_const = 1;
      val[ident].const_val = cur_value;
      val[ident].index_str = cur_str;
      //domain.push_back(val);
      if(type == ConstDef_Mul_Ty){
        constdef->Dump();
      }
    } 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{return -1;}
};

class VarDefAST : public BaseAST {
  public:
    VarDefTy type;
    std::string ident;
    std::unique_ptr<BaseAST> initval;
    std::unique_ptr<BaseAST> vardef;

    void Dump() const override {
      auto &val = domain.back();
      std::string cur_str = ident + "_" + std::to_string(domain.size()) + "_" + std::to_string(ident_num++);
      //std::cout << "cur_str is " << cur_str << std::endl;
      //std::cout << "  @" << ident << "_" << index << " = alloc i32" << std::endl;
      std::cout << "  @" << cur_str << " = alloc i32" << std::endl;

      val[ident].is_const = 0; //not a const value
      val[ident].index_str = cur_str;
      //domain.push_back(val);
      if(type == VarDef_init_Ty || type == VarDef_VarDef_init_Ty){
        parameter p;
        initval->Dump(&p);

        //val[ident].variable_val = initval->Dump();
        if(p.is_data){
          std::cout << "  store " << p.p1 << ", @" << cur_str << std::endl;
        }
        else{
          std::cout << "  store " << "%" << p.p1 << ", @" << cur_str << std::endl;
        }
      }
      if(type == VarDef_VarDef_noinit_Ty || type == VarDef_VarDef_init_Ty){
        vardef->Dump();
      }
    }
    void Dump(parameter_t parameter_) const override{}
    int CalcConst() const override {return -1;}
};

class ConstInitValAST: public BaseAST {
  public:
    std::unique_ptr<BaseAST> constexp;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{
      return constexp->CalcConst();
    }
};

class InitValAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter p;
      exp->Dump(&p);
      *parameter_ = p;
    }
    int CalcConst() const override{
      return exp->CalcConst();
    }
};
    

class ConstExpAST: public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{
      return exp->CalcConst();
    }
};

#endif
