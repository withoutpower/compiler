#ifndef _VISIT_H_
#define _VISIT_H_

#include <cassert>
#include <iostream>
#include <unordered_map>
#include "koopa.h"

#define ull unsigned long long

int stackoff= 0;
int func_stack_mem;
int now_a = 0;
std::unordered_map<koopa_raw_value_t, int> slice2mem;
// 函数声明略
// ...
void Visit(const koopa_raw_program_t &program); 
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

//visit for different instruction
void Visit(const koopa_raw_return_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_integer_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_global_alloc_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_branch_t &val, const koopa_raw_value_t &value);
void Visit(const koopa_raw_jump_t &val, const koopa_raw_value_t &value);

//get memory needed for this part
int GetMem(const koopa_raw_slice_t &slice);
int GetMem(const koopa_raw_basic_block_t &bb);
int GetMem(const koopa_raw_value_t &value);

void slice_value(koopa_raw_value_t val, int &reg, int now);
void print_lrreg(int lreg, int rreg);

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  std::cout << "  .text" << std::endl;
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ...
  std::cout << "  .globl " << func->name+1 << std::endl;
  std::cout << func->name+1 << ":" << std::endl;
  // 访问所有基本块
  
  stackoff = 0;
  int mem = GetMem(func->bbs);
  int aligned_mem = (mem + 15) / 16 * 16;
  //std::cout << "cur stackoff : " << stackoff << std::endl;
  func_stack_mem = aligned_mem;
  if(func_stack_mem){
    std::cout << "  addi sp, sp, -" << func_stack_mem << std::endl;
  }
  Visit(func->bbs);
  //std::cout << "  addi sp, sp, " << aligned_mem << std::endl;
  //std::cout << "  ret" << std::endl;
  //assert(stackoff == mem);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  std::cout << bb->name+1 << ":" << std::endl;
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret, value);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer, value);
      break;
    case KOOPA_RVT_BINARY:
      // visit binary instruction
      Visit(kind.data.binary, value);
      break;
    case KOOPA_RVT_STORE:
      Visit(kind.data.store, value);
      break;
    case KOOPA_RVT_LOAD:
      Visit(kind.data.load, value);
      break;
    case KOOPA_RVT_ALLOC:
      //Visit(kind.data.store, value);
      break;
    case KOOPA_RVT_BRANCH:
      Visit(kind.data.branch, value);
      break;
    case KOOPA_RVT_JUMP:
      Visit(kind.data.jump, value);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

//visit return value
void Visit(const koopa_raw_return_t &val, const koopa_raw_value_t &value){
//  std::cout << "    li " << "a0 , " << val.value->kind.data.integer.value << std::endl;
 // std::cout << "    ret" << std::endl;

  koopa_raw_value_t return_val = val.value;
  int ret = 0;
  if(return_val->kind.tag == KOOPA_RVT_INTEGER){
   //std::cout << "  li a" << now_a++ << ", " << return_val->kind.data.integer.value << std::endl;
    std::cout << "  li a" << ret++ << ", " << return_val->kind.data.integer.value << std::endl;
  }
  else{
   //std::cout << "  lw a" << now_a++ << ", " << slice2mem[return_val] << "(sp)" << std::endl;
    std::cout << "  lw a" << ret++ << ", " << slice2mem[return_val] << "(sp)" << std::endl;
  }
  if(func_stack_mem){
    std::cout << "  addi sp, sp, " << func_stack_mem << std::endl;
  }
  std::cout << "  ret" << std::endl;
}

void Visit(const koopa_raw_integer_t &val, const koopa_raw_value_t &value){
}

//visit a binary value
void Visit(const koopa_raw_binary_t &val, const koopa_raw_value_t &value){
  int lres = 0;
  int rres = 1;
  slice_value(val.lhs, lres, 0);
  slice_value(val.rhs, rres, 0);
  std::cout << "  ";
  if(val.op == KOOPA_RBO_EQ || val.op == KOOPA_RBO_NOT_EQ){
    std::cout << "xor t0, " ;
    print_lrreg(lres, rres);
  }
  //switch(val.op){
  //  case KOOPA_RBO_NOT_EQ:  std::cout << "xor t" << now << ", "; print_lrreg(lres, rres);break; 
  //  case KOOPA_RBO_EQ:      std::cout << "xor t" << now << ", "; print_lrreg(lres, rres);break; 
  //  case KOOPA_RBO_GT:      break;
  //  case KOOPA_RBO_LT:      break;
  //  case KOOPA_RBO_GE:      break;
  //  case KOOPA_RBO_LE:      break;
  //  case KOOPA_RBO_ADD:     break;
  //  case KOOPA_RBO_SUB:     break;
  //  case KOOPA_RBO_MUL:     break;
  //  case KOOPA_RBO_DIV:     break;
  //  case KOOPA_RBO_MOD:     break;
  //  case KOOPA_RBO_AND:     break;
  //  case KOOPA_RBO_OR:      break;
  //  case KOOPA_RBO_XOR:     break;
  //  case KOOPA_RBO_SHL:     break;
  //  case KOOPA_RBO_SHR:     break;
  //  case KOOPA_RBO_SAR:     break;
  //}
  switch(val.op){
    case KOOPA_RBO_NOT_EQ:  std::cout << "snez ";break;
    case KOOPA_RBO_EQ:      std::cout << "seqz ";break;
    case KOOPA_RBO_GT:      std::cout << "sgt "; break;
    case KOOPA_RBO_LT:      std::cout << "slt "; break;
    case KOOPA_RBO_GE:      std::cout << "slt "; break;
    case KOOPA_RBO_LE:      std::cout << "sgt "; break;
    case KOOPA_RBO_ADD:     std::cout << "add "; break;
    case KOOPA_RBO_SUB:     std::cout << "sub "; break;
    case KOOPA_RBO_MUL:     std::cout << "mul "; break;
    case KOOPA_RBO_DIV:     std::cout << "div "; break;
    case KOOPA_RBO_MOD:     std::cout << "rem "; break;
    case KOOPA_RBO_AND:     std::cout << "and "; break;
    case KOOPA_RBO_OR:      std::cout << "or "; break;
    case KOOPA_RBO_XOR:     std::cout << "xor "; break;
    case KOOPA_RBO_SHL:     std::cout << "sll "; break;
    case KOOPA_RBO_SHR:     std::cout << "srl "; break;
    case KOOPA_RBO_SAR:     std::cout << "sra "; break;
  }
  if( val.op == KOOPA_RBO_EQ || val.op == KOOPA_RBO_NOT_EQ) {
    std::cout << "t0, t0" << std::endl;
  }
  else if( val.op == KOOPA_RBO_GE || val.op == KOOPA_RBO_LE) {
    std::cout << "t0, " ;
    print_lrreg(rres, lres);
  }
  else{
    std::cout << "t0, " ;
    print_lrreg(lres, rres);
  }

  std::cout << "  sw t0, " << stackoff << "(sp)" << std::endl;
  slice2mem[value] = stackoff;
  stackoff += 4;
}

//visit a store instruction
void Visit(const koopa_raw_store_t &val, const koopa_raw_value_t &value){
  //std::cout << "a store inst" << std::endl;
  int reg = 0;
  slice_value(val.value, reg, 0);
  if(reg == -1){
    std::cout << "  sw x0, " << slice2mem[val.dest] << "(sp)" << std::endl;
  }
  else{
    std::cout << "  sw t0, " << slice2mem[val.dest] << "(sp)" << std::endl;
  }
}

void Visit(const koopa_raw_load_t &val, const koopa_raw_value_t &value){
  //std::cout << "a load inst" << std::endl;
  std::cout << "  lw t0, " << slice2mem[val.src] << "(sp)" << std::endl;
  std::cout << "  sw t0, " << stackoff << "(sp)" << std::endl;
  slice2mem[value] = stackoff;
  stackoff += 4;
}
void Visit(const koopa_raw_global_alloc_t &val, const koopa_raw_value_t &value){
  //slice2mem[value] = stackoff;
  //stackoff += 4;
}

void Visit(const koopa_raw_branch_t &val, const koopa_raw_value_t &value){
  //std::cout << "branch true: " << val.true_bb->name << "false: " << val.false_bb->name << std::endl; 
  if(val.cond->kind.tag == KOOPA_RVT_INTEGER){
    std::cout << "  li t0, " << val.cond->kind.data.integer.value << std::endl;
  }
  else{
    std::cout << "  lw t0, " << slice2mem[val.cond] << "(sp)" << std::endl;
  }
  std::cout << "  bnez t0, " << (val.true_bb->name)+1 << std::endl;
  std::cout << "  j " << (val.false_bb->name)+1 << std::endl;

}

void Visit(const koopa_raw_jump_t &val, const koopa_raw_value_t &value){
  std::cout << "  j " << (val.target->name)+1 << std::endl;
}


/*==================== help function ===================*/
int GetMem(const koopa_raw_slice_t &slice){
  int mem = 0, temp;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];

    switch (slice.kind) {
      case KOOPA_RSIK_BASIC_BLOCK:
        temp = GetMem(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        temp = GetMem(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        assert(false);
    }
    mem += temp;
  }
  return mem;
}

int GetMem(const koopa_raw_basic_block_t &bb){
  return GetMem(bb->insts);
}

int GetMem(const koopa_raw_value_t &value) {
  if(value->ty->tag == KOOPA_RTT_UNIT) return 0;
  else{
    if(value->kind.tag == KOOPA_RVT_ALLOC){
      slice2mem[value] = stackoff;
      stackoff += 4;
    }
    return 4;
  }
  //const auto &kind = value->kind;
  //switch (kind.tag) {
  //  case KOOPA_RVT_RETURN: break;
  //  case KOOPA_RVT_INTEGER: return 0;
  //  case KOOPA_RVT_BINARY: return 4;
  //  case KOOPA_RVT_STORE: break;
  //  case KOOPA_RVT_LOAD: return 4;
  //  case KOOPA_RVT_GLOBAL_ALLOC: return 4;
  //}
  //return 0;
}

//function to find the slice value of a statement
void slice_value(const koopa_raw_value_t val, int &reg, int num){
  if(val->kind.tag == KOOPA_RVT_INTEGER){
    if(val->kind.data.integer.value == 0){
      reg = -1;
    }
    else{
      std::cout << "  li t" << reg << ", " << val->kind.data.integer.value << std::endl;
    }
  }
  else{
    std::cout << "  lw t" << reg << ", " << slice2mem[val] << "(sp)" << std::endl;
  }
}

//print the operator reg
void print_lrreg(int lreg, int rreg){
  if(lreg == -1){
    std::cout << "x0";
  }
  else{
    std::cout << "t" << lreg;
  }

  if(rreg == -1){
    std::cout << ", x0" << std::endl;
  }
  else{
    std::cout << ", t" << rreg << std::endl;
  }
}


#endif
