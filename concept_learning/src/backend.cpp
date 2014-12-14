//------------------------------------------------------------------------------
/// @brief SnuPL backend
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/11/28 Bernhard Egger created
/// 2013/06/09 Bernhard Egger adapted to SnuPL/0
///
/// @section license_section License
/// Copyright (c) 2012-2014 Bernhard Egger
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "backend.h"
using namespace std;


//------------------------------------------------------------------------------
// CBackend
//
CBackend::CBackend(ostream &out)
  : _out(out)
{
}

CBackend::~CBackend(void)
{
}

bool CBackend::Emit(CModule *m)
{
  assert(m != NULL);
  _m = m;

  if (!_out.good()) return false;

  bool res = true;

  try {
    EmitHeader();
    EmitCode();
    EmitData();
    EmitFooter();

    res = _out.good();
  } catch (...) {
    res = false;
  }

  return res;
}

void CBackend::EmitHeader(void)
{
}

void CBackend::EmitCode(void)
{
}

void CBackend::EmitData(void)
{
}

void CBackend::EmitFooter(void)
{
}


//------------------------------------------------------------------------------
// CBackendx86
//
CBackendx86::CBackendx86(ostream &out)
  : CBackend(out), _curr_scope(NULL)
{
  _ind = string(4, ' ');
}

CBackendx86::~CBackendx86(void)
{
}

void CBackendx86::EmitHeader(void)
{
  _out << "##################################################" << endl
       << "# " << _m->GetName() << endl
       << "#" << endl
       << endl;
}

void CBackendx86::EmitCode(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# text section" << endl
       << _ind << "#" << endl
       << _ind << ".text" << endl
       << _ind << ".align 4" << endl
       << endl
       << _ind << "# entry point and pre-defined functions" << endl
       << _ind << ".global main" << endl
       << _ind << ".extern Input" << endl
       << _ind << ".extern Output" << endl
       << endl;

  // Get children from module scope
  vector<CScope*> children = _m->GetSubscopes();
  for(int i = 0; i < children.size(); i++)
    EmitScope(children[i]); // emit scope for each child
  
  // emit scope for main module
  EmitScope(_m);

  _out << _ind << "# end of text section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendx86::EmitData(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# global data section" << endl
       << _ind << "#" << endl
       << _ind << ".data" << endl
       << _ind << ".align 4" << endl
       << endl;

  EmitGlobalData(_m);

  _out << _ind << "# end of global data section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendx86::EmitFooter(void)
{
  _out << _ind << ".end" << endl
       << "##################################################" << endl;
}

void CBackendx86::SetScope(CScope *scope)
{
  _curr_scope = scope;
}

CScope* CBackendx86::GetScope(void) const
{
  return _curr_scope;
}

void CBackendx86::EmitScope(CScope *scope)
{
  assert(scope != NULL);

  string label;

  if (scope->GetParent() == NULL) label = "main";
  else label = scope->GetName();

  // label
  _out << _ind << "# scope " << scope->GetName() << endl
       << label << ":" << endl;

  SetScope(scope);
  // param_offs = 8, local_offs = -12 (details on ComputeStackOffsets's comment)
  size_t locSize = ComputeStackOffsets(scope->GetSymbolTable(), 8, -12);
  
  // Subroutine Prologue
  //  1. save ebp by pushing it onto the stack
  //  2. set ebp to esp
  //  3. save callee-saved registers
  //  4. generated space on the stack for locals and spilled variables by adjusting
  //     the stack pointer
  
  EmitInstruction("pushl", "%ebp");
  EmitInstruction("movl" , "%esp, %ebp");
  EmitInstruction("pushl", "%ebx");
  EmitInstruction("pushl", "%esi");
  EmitInstruction("pushl", "%edi");
  EmitInstruction("subl" , Imm(locSize) + string(", %esp"),
                  "make room for locals");
  _out << endl;
  
  EmitCodeBlock(scope->GetCodeBlock(), scope->GetSymbolTable());
  _out << endl;

  // Subroutine Epilogue
  //  1. remove space on stack for locals and spilled variables by setting
  //     the stack pointer immediately below the callee-saved registers
  //  2. restore callee-saved registers
  //  3. restore ebp
  //  4. issue the ret instruction
  
  label = Label("exit"); 
  _out << label << ":" << endl;
  EmitInstruction("addl", Imm(locSize) + string(", %esp"),
                  "remove locals");
  EmitInstruction("popl", "%edi");
  EmitInstruction("popl", "%esi");
  EmitInstruction("popl", "%ebx");
  EmitInstruction("popl", "%ebp");
  EmitInstruction("ret");

  _out << endl;
}

void CBackendx86::EmitGlobalData(CScope *scope)
{
  assert(scope != NULL);

  // forall globals g in scope's symbol table do
  //   emit g respecting its alignment restrictions
  CSymtab* symtab = scope->GetSymbolTable();
  vector <CSymbol*> symVec = symtab->GetSymbols();
  bool first = true;
  for(int i = 0; i < symVec.size(); i++) {
    if(symVec[i]->GetSymbolType() == stGlobal) {
      if(first) { // just print scope first time, if any globals
        _out << _ind << "# scope " << scope->GetName() << endl;
        first = false;
      }
      
      // find out global symbol data type
      size_t size;
      string type;
      if(symVec[i]->GetDataType() == CTypeManager::Get()->GetInt()) {
        size = 4;
        type = string("<int>");
      } else if(symVec[i]->GetDataType() == CTypeManager::Get()->GetBool()) {
        size = 4; // for alignment
        type = string("<bool>");
      }

      ostringstream o;
      o << symVec[i]->GetName() << ':';
      _out << left
           << setw(8) << o.str()
           << setw(9) << ".skip " << dec << setw(18) << size;
      _out << " # " << type;
      _out << endl;
    }
  }
  _out << endl;

  // emit globals in subscopes (necessary if we support static local variables)
  vector<CScope*>::const_iterator sit = scope->GetSubscopes().begin();
  while (sit != scope->GetSubscopes().end()) EmitGlobalData(*sit++);
}

void CBackendx86::EmitCodeBlock(CCodeBlock *cb, CSymtab *symtab)
{
  assert(cb != NULL);
  assert(symtab != NULL);

  const list<CTacInstr*> &instr = cb->GetInstr();
  list<CTacInstr*>::const_iterator it = instr.begin();
  
  while (it != instr.end()) EmitInstruction(*it++, symtab);
}

void CBackendx86::EmitInstruction(CTacInstr *i, CSymtab *symtab)
{
  assert(i != NULL);
  assert(symtab != NULL);

  ostringstream cmt;
  cmt << i;
  
  EOperation op = i->GetOperation();
  CTac *src1 = i->GetSrc(1);  // can be null
  CTac *src2 = i->GetSrc(2);  // can be null
  CTac *dst  = i->GetDest();  // can be null

  // for opCall
  CTacName *callee = NULL;
  const CSymProc *calleeSym = NULL;

  switch (op) {
    // binary operators
    case opAdd:
    case opSub:
    case opAnd:
    case opOr:
      EmitInstruction("movl", Operand(src1) + string(", %eax"),
                      cmt.str());
      EmitInstruction(Operation(op), Operand(src2) + string(", %eax"));
      EmitInstruction("movl", string("%eax, ") + Operand(dst));
      break;
        
    case opMul:
    case opDiv:
      // Multiply and Divide uses fixed registers %eax, %edx in X86 arch
      EmitInstruction("movl", Operand(src1) + string(", %eax"),
                      cmt.str());
      // %edx register should be empty before division
      if (op == opDiv)
        EmitInstruction("cdq"); // sign extension %eax => [%edx:%eax]
      // Multiply and Divide should move second operand to %ebx, and operate
      EmitInstruction("movl", Operand(src2) + string(", %ebx"));
      EmitInstruction(Operation(op), "%ebx");
      EmitInstruction("movl", string("%eax, ") + Operand(dst));
      break;

    // unary operators
    case opNot: // opNot is not used at all
    case opNeg:
      EmitInstruction("movl", Operand(src1) + string(", %eax"),
                      cmt.str());
      EmitInstruction(Operation(op), "%eax");
      EmitInstruction("movl", string("%eax, ") + Operand(dst));
      break;

    case opPos:
    // memory operations
    case opAssign:
      // Pos unary op just do copy like Assgin op
      if(dynamic_cast<CTacName*>(dst)->GetSymbol()->GetDataType() == 
        CTypeManager::Get()->GetBool()) {
        EmitInstruction("movb", Operand(src1) + string(", %al"), cmt.str());
        EmitInstruction("movb", string("%al, ") + Operand(dst));
      }
      else {// int
        EmitInstruction("movl", Operand(src1) + string(", %eax"), cmt.str());
        EmitInstruction("movl", string("%eax, ") + Operand(dst));
      }
      break;

    // unconditional branching
    case opGoto:
      EmitInstruction(Operation(op), Label(dynamic_cast<CTacLabel*>(dst)),
                      cmt.str());
      break;

    // conditional branching
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      EmitInstruction("movl", Operand(src1) + string(", %eax"),
                      cmt.str());
      EmitInstruction("cmpl", Operand(src2) + string(", %eax"));
      EmitInstruction(Operation(op), Label(dynamic_cast<CTacLabel*>(dst)));
      break;

    // function call-related operations
    case opParam:
      EmitInstruction("movl", Operand(src1) + string(", %eax"),
                      cmt.str());
      EmitInstruction("pushl", "%eax");
      break;

    case opCall:
      callee = dynamic_cast<CTacName*>(src1);
      calleeSym = dynamic_cast<const CSymProc*>(callee->GetSymbol());
      EmitInstruction(Operation(op), Operand(src1), cmt.str());
      // each param has 4 byte (int type)
      if(calleeSym->GetNParams()) // not 0 param
        EmitInstruction("addl", Imm(calleeSym->GetNParams() * 4) + string(", %esp"));
      if(dst)
        EmitInstruction("movl", string("%eax, ") + Operand(dst));
      break;
    
    // return
    case opReturn:
      if(src1)
        EmitInstruction("movl", Operand(src1) + string(", %eax"), cmt.str());
      EmitInstruction("jmp", Label("exit")); 
      break;

    // special
    case opLabel:
      _out << Label(dynamic_cast<CTacLabel*>(i)) << ":" << endl;
      break;

    case opNop:
      EmitInstruction("nop", "", cmt.str());
      break;

    default:
      EmitInstruction("# ???", "not implemented", cmt.str());
  }
}

void CBackendx86::EmitInstruction(string mnemonic, string args, string comment)
{
  _out << left
       << _ind
       << setw(7) << mnemonic << " "
       << setw(23) << args;
  if (comment != "") _out << " # " << comment;
  _out << endl;
}

string CBackendx86::Operation(EOperation op) const
{
  string operation;
  
  // return appropriate assembly operation
  switch (op) {
    // binary operators
    case opAdd:         operation = "addl";   break;
    case opSub:         operation = "subl";   break;
    case opMul:         operation = "imull";  break;
    case opDiv:         operation = "idivl";  break;
    case opAnd:         operation = "andl";   break;
    case opOr:          operation = "orl";    break;
    
    // unary operators
    case opNot:         operation = "notl";   break; // actually not used
    case opNeg:         operation = "negl";   break;
    case opPos:         break; // Pos Unary Op do just copy !
    
    // memory operations
    case opAssign:      break; // Assign Op do just copy like Pos !
    
    // unconditional branching
    case opGoto:        operation = "jmp";    break;
    
    // conditional branching
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      operation = "j" + Condition(op); break;
    
    // function call-related operations
    case opParam:       break; // Param Op do just push param !
    case opCall:        operation = "call";   break;
    
    // return
    case opReturn:      break;
    
    default: operation = "???"; // error
  }
  return operation;
}

string CBackendx86::Operand(CTac *op) const
{
  string operand;

  if(CTacConst* cons = dynamic_cast<CTacConst*>(op)) {
    // Immediate value
    operand = Imm(cons->GetValue());
  } 
  else if(CTacName* name = dynamic_cast<CTacName*>(op)) {
    // Designator
    const CSymbol* sym = name->GetSymbol();
    if(sym->GetSymbolType() == stLocal || 
        sym->GetSymbolType() == stParam) {
      // can be access by %ebp and offset
      ostringstream o;
      
      o << dec << sym->GetOffset() << '(' <<
        sym->GetBaseRegister() << ')';
      
      operand = o.str();
    } 
    else if(sym->GetSymbolType() == stGlobal ||
              sym->GetSymbolType() == stProcedure) {
      // access by label
      operand = sym->GetName();
    }
  }

  return operand;
}

string CBackendx86::Imm(int value) const
{
  ostringstream o;
  o << "$" << dec << value;
  return o.str();
}

string CBackendx86::Label(CTacLabel* label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  ostringstream o;
  o << "l_" << cs->GetName() << "_" << label->GetLabel();
  return o.str();
  return "l_" + cs->GetName() + "_" + label->GetLabel();
}

string CBackendx86::Label(string label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  return "l_" + cs->GetName() + "_" + label;
}

string CBackendx86::Condition(EOperation cond) const
{
  switch (cond) {
    case opEqual:       return "e";
    case opNotEqual:    return "ne";
    case opLessThan:    return "l";
    case opLessEqual:   return "le";
    case opBiggerThan:  return "g";
    case opBiggerEqual: return "ge";
    default:            assert(false); break;
  }
}


size_t CBackendx86::ComputeStackOffsets(CSymtab *symtab,
                                        int param_ofs,int local_ofs)
{
  assert(symtab != NULL);
  vector<CSymbol*> slist = symtab->GetSymbols();

  // Procedure / function activation frame
  // 
  // mem addr   |   container
  // ====== caller area ======
  // ...        |   ...
  // %ebp + 16  |   param 3
  // %ebp + 12  |   param 2
  // %ebp +  8  |   param 1
  // %ebp +  4  |   return addr
  // %ebp       |   saved ebp
  // ====== callee area ======
  // %ebp - 12  |   callee-saved reg
  // %ebp - n   |   local var
  // ...        |   ...
  
  size_t size = 0;  // initialized by 0
  int curParamOff = param_ofs;
  int curLocalOff = local_ofs;

  for(int i = 0; i < slist.size(); i++) {
    ESymbolType st = slist[i]->GetSymbolType();
    if(st == stLocal) {
      if (slist[i]->GetDataType() == CTypeManager::Get()->GetInt()) {
        int padding = (4 - (size % 4)) % 4; // for alignment
        curLocalOff -= 4 + padding; // int size
        size += 4 + padding;
      } else if (slist[i]->GetDataType() == CTypeManager::Get()->GetBool()) {
        curLocalOff -= 1; // bool size
        size += 1;
      }
      
      slist[i]->SetBaseRegister("%ebp");
      slist[i]->SetOffset(curLocalOff);
    } 
    else if (st == stParam) {
      slist[i]->SetBaseRegister("%ebp");
      slist[i]->SetOffset(curParamOff);
      curParamOff += 4;
    }
  }

  size += (4 - (size % 4)) % 4; // for alignment
  
  return size;
}
