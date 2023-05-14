#ifndef _AST_H_
#define _AST_H_

#include <iostream>
#include <memory>
#include <string>

typedef enum {Add_MulExp_Ty, Add_AddExp_Ty} AddExpTy;
typedef enum {Mul_UnaryExp_Ty, Mul_MulExp_Ty} MulExpTy;
typedef enum {PrimaryExp_Ty, UnaryExp_Ty} UnaryExpTy;
typedef enum {Exp_Ty, Number_Ty} PrimaryExpTy;
typedef enum {Return_Ty} StmtTy;


static int count = 0;
typedef struct para {
  bool is_data;
  int p1_temp;
  int p2_temp;
  int p3_temp;
  int p1;
  int p2;
  int p3;
} parameter;

typedef parameter* parameter_t;
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual void Dump(parameter_t parameter_) const = 0;
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
    //parameter p;
    block->Dump();
    // std::cout << " }";
  }
  void Dump(parameter_t parameter_)const override{} 
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
};

class BlockAST : public BaseAST{
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "{ " << std::endl;
    std::cout << "%entry: " << std::endl;
    stmt->Dump();
    std::cout << "}" << std::endl;;
  }
  void Dump(parameter_t parameter_)const override{} 
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
};


/*============================================ 3. oprating part begin ================================================*/

// ast for expression
// Exp -> UnaryExp
class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> add_exp;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
        add_exp->Dump(parameter_);
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
      else if(type == Exp_Ty){
        data.exp_ty.exp->Dump(&p);
        *parameter_ = p;
      //  if(p.is_data == 1){
      //    parameter_->is_data = 1;
      //  }
      //  else{
      //    parameter_->is_data = 0;
      //  }      

      //  parameter_->p1 = p.p1;
      }
    }
};

class NumberAST : public BaseAST {
  public:
    int number;

    void Dump()const override {} 
    void Dump(parameter_t parameter_) const override {
      parameter_->p1 = number;
    }
};
/*============================================ 3. oprating part end ================================================*/

#endif
