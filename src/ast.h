#ifndef _AST_H_
#define _AST_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>

typedef enum {Lor_LAndExp_Ty, Lor_LorExp_Ty} LorExpTy;
typedef enum {LAnd_EqExp_Ty, LAnd_LAndExp_Ty} LAndExpTy;
typedef enum {Eq_RelExp_Ty, Eq_EqExp_Ty} EqExpTy;
typedef enum {Rel_AddExp_Ty, Rel_RelExp_Ty} RelExpTy;
typedef enum {Add_MulExp_Ty, Add_AddExp_Ty} AddExpTy;
typedef enum {Mul_UnaryExp_Ty, Mul_MulExp_Ty} MulExpTy;
typedef enum {PrimaryExp_Ty, UnaryExp_Ty} UnaryExpTy;
typedef enum {Exp_Ty, LVal_Ty, Number_Ty} PrimaryExpTy;
typedef enum {Return_Ty} StmtTy;

typedef enum {BlockItem_Decl_Ty, BlockItem_Stmt_Ty, BlockItem_Block_Decl_Ty, BlockItem_Block_Stmt_Ty} BlockItemTy;
typedef enum {ConstDef_Single_Ty, ConstDef_Mul_Ty} ConstDefTy;

static int count = 0; //the use of temp
static std::unordered_map<std::string, int> const_val; //table for const value

typedef struct para {
  bool is_data;
  int p1_temp;
  int p2_temp;
  int p3_temp;
  int p1;
  int p2;
  int p3;
  std::string str;
} parameter;

typedef parameter* parameter_t;

void print_operator(const parameter &p1, const parameter &p2);

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual void Dump(parameter_t parameter_) const = 0;
  virtual int CalcConst() const = 0;
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
class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;


 void Dump() const override {
    std::cout << "fun ";
    std::cout << "@" << ident << "(): ";
    func_type->Dump();
    std::cout << "{" << std::endl;
    //parameter p;
    block->Dump();
    std::cout << "}" << std::endl;
    // std::cout << " }";
  }
  void Dump(parameter_t parameter_)const override{} 
  int CalcConst() const override{return 0;}
};

//
class FuncTypeAST : public BaseAST {
public:
  std::string functype;

  void Dump() const override {
    // std::cout << "FuncTypeAST { ";
    // std::cout << functype;
    std::cout << "i32 ";
  }
  void Dump(parameter_t parameter_)const override {} 
  int CalcConst() const override{return 0;}
};

class BlockAST : public BaseAST{
public:

  std::unique_ptr<BaseAST> blockitem;

  void Dump() const override {
    //std::cout << "{ " << std::endl;
    std::cout << "%entry: " << std::endl;
    blockitem->Dump();
    //std::cout << "}" << std::endl;
  }
  void Dump(parameter_t parameter_)const override{} 
  int CalcConst() const override{return -1;}
};

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


// Stmt -> "return" Exp ";"
class StmtAST : public BaseAST{
 public:
  StmtTy type;
  
  struct {
    struct {
        std::unique_ptr<BaseAST> exp;
    }return_ty;
  }data;

  void Dump() const override {
    parameter p;   
    if(type == Return_Ty){
      data.return_ty.exp->Dump(&p);
      std::cout << "  ";
      if(p.is_data){
        std::cout << "ret " << p.p1 << std::endl;
      }
      else
        std::cout << "ret %" << p.p1 << std::endl;
      //std::cout << (p.p1_temp == 1) ? "%" : "@";
      //std::cout << p.p1 << endl;
    }
  }
  void Dump(parameter_t parameter_)const override {} 
  int CalcConst() const override{return 0;}
};

class LValAST : public BaseAST {
  public:
    std::string ident;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter_->is_data = 1;
      parameter_->p1 = const_val[ident];
      parameter_->str = ident;
    }
    int CalcConst() const override{return -1;}
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
// UnaryExp -> PrimaryExp | UnaryOp UnaryExp
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
// PrimaryExp -> "(" Exp ")" | Number
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
        //auto identity = reinterpret_cast<LValAST&>(data.lval_ty.lval);
        parameter p;
        data.lval_ty.lval->Dump(&p);
        std::string str = p.str;
        //std::string str = *(std::string*)data.lval_ty.lval->CalcConst();
        auto ite = const_val.find(str);
        assert(ite != const_val.end());
        return const_val[str];
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
    std::unique_ptr<BaseAST> constdecl;

    void Dump()const override {
      constdecl->Dump();
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
      const_val[ident] = constinitval->CalcConst();
      if(type == ConstDef_Mul_Ty){
        constdef->Dump();
      }
    } 
    void Dump(parameter_t parameter_) const override {
    }
    int CalcConst() const override{return -1;}
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
