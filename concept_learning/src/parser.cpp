//------------------------------------------------------------------------------
/// @brief SnuPL/0 parser
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2014/09/28 Bernhard Egger assignment 2: parser for SnuPL/-1
///
/// @section license_section License
/// Copyright (c) 2012-2014, Bernhard Egger
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

#include <limits.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <exception>

#include "parser.h"
using namespace std;


//------------------------------------------------------------------------------
// CParser
//
CParser::CParser(CScanner *scanner)
{
  _scanner = scanner;
  _module = NULL;
}

CAstNode* CParser::Parse(void)
{
  _abort = false;

  if (_module != NULL) { delete _module; _module = NULL; }

  try {
    if (_scanner != NULL) _module = module();

    if (_module != NULL) {
      CToken t;
      string msg;
      if (!_module->TypeCheck(&t, &msg)) SetError(t, msg);
    }
  } catch (...) {
    _module = NULL;
  }

  return _module;
}

const CToken* CParser::GetErrorToken(void) const
{
  if (_abort) return &_error_token;
  else return NULL;
}

string CParser::GetErrorMessage(void) const
{
  if (_abort) return _message;
  else return "";
}

void CParser::SetError(CToken t, const string message)
{
  _error_token = t;
  _message = message;
  _abort = true;
  throw message;
}

bool CParser::Consume(EToken type, CToken *token)
{
  if (_abort) return false;

  CToken t = _scanner->Get();

  if (t.GetType() != type) {
    SetError(t, "expected '" + CToken::Name(type) + "', got '" +
             t.GetName() + "'");
  }

  if (token != NULL) *token = t;

  return t.GetType() == type;
}

void CParser::InitSymbolTable(CSymtab *s)
{
  CTypeManager *tm = CTypeManager::Get();

  CSymProc *input = new CSymProc("Input", tm->GetInt());

  CSymProc *output = new CSymProc("Output", tm->GetNull());
  output->AddParam(new CSymParam(0, "x", tm->GetInt()));

  s->AddSymbol(input);
  s->AddSymbol(output);
}

CAstModule* CParser::module(void)
{
  //
  // module = "module" ident ";" varDeclaration { subroutineDecl }
  //          "begin" statSequence "end" ident ".".
  //
  // FIRST(module) = { tModule }
  // FOLLOW(module) = { $ }
  //
  CToken mod;
  Consume(tModule, &mod);

  CToken modName;
  Consume(tIdent, &modName);
  CAstModule *m = new CAstModule(mod, modName.GetValue());
  InitSymbolTable(m->GetSymbolTable());
  
  // As module class is constructed, it initialize symbol table too.
  Consume(tSemicolon);

  // global variable is added at symbol table
  varDeclaration(m);
 
  EToken tt = _scanner->Peek().GetType();
  if(tt != tBegin) {  // means subroutineDecl exists
    do {
      switch (tt) {
        case tProcedure:
        case tFunction:
          subroutineDecl(m);
          break;
        
        default:
          SetError(_scanner->Peek(), "subroutineDecl expected.");
          break;
      }
      
      tt = _scanner->Peek().GetType();
      if(tt == tBegin) break;
      
    } while (!_abort);
  }  
  Consume(tBegin);

  CAstStatement *statseq = NULL;
  statseq = statSequence(m);
  
  Consume(tEnd);
  if(_scanner->Peek().GetValue() != modName.GetValue())
    SetError(_scanner->Peek(), "module identifier does not match.");
  Consume(tIdent);
  Consume(tDot);

  m->SetStatementSequence(statseq);
  
  return m;
}

void CParser::varDeclaration(CAstScope *s)
{
  // 
  // varDeclaration = [ "var" { ident { "," ident } ":" type ";" } ].
  //
  // FIRST(varDeclaration) = { tVarDecl, e }
  // FOLLOW(varDeclaration) = { tProcedure, tFunction, tBegin }
  //
  EToken tt = _scanner->Peek().GetType();
  if(tt == tVarDecl) {
    Consume(tVarDecl);
    do {
      CToken t;
      vector<CSymbol*> _gl_var;
      tt = _scanner->Peek().GetType();
      if(tt == tProcedure || tt == tFunction || tt == tBegin) break;
      
      Consume(tIdent, &t);
      if(s->GetSymbolTable()->FindSymbol(t.GetValue(), sLocal))
        SetError(_scanner->Peek(), "Symbol [" + t.GetValue() + "] is already declared.");
      _gl_var.push_back(s->CreateVar(t.GetValue(), CTypeManager::Get()->GetNull()));
      
      do {
        tt = _scanner->Peek().GetType();
        if (tt != tComma) break;

        Consume(tComma);
        Consume(tIdent, &t);
        if(s->GetSymbolTable()->FindSymbol(t.GetValue(), sLocal))
          SetError(_scanner->Peek(), "Symbol [" + t.GetValue() + "] is already declared.");
        _gl_var.push_back(s->CreateVar(t.GetValue(), CTypeManager::Get()->GetNull()));
      } while (!_abort);

      Consume(tColon);

      tt = _scanner->Peek().GetType();
      switch(tt) {
        case tInteger:
          Consume(tInteger);
          for(int i = 0; i < _gl_var.size(); i++)
            _gl_var[i]->SetDataType(CTypeManager::Get()->GetInt());
          break;

        case tBoolean:
          Consume(tBoolean);
          for(int i = 0; i < _gl_var.size(); i++)
            _gl_var[i]->SetDataType(CTypeManager::Get()->GetBool());
          break;

        default:
          SetError(_scanner->Peek(), "type expected.");
          break;
      }
      Consume(tSemicolon);
      
      for(int i = 0; i < _gl_var.size(); i++)
        s->GetSymbolTable()->AddSymbol(_gl_var[i]);

    } while (!_abort);
  }
}

void CParser::subroutineDecl(CAstScope *s)
{
  // 
  // subroutineDecl = (procedureDecl | functionDecl) subroutineBody ident ";".
  //
  // FIRST(subroutineDecl) = { tProcedure, tFunction }
  // FOLLOW(subroutineDecl) = { tBegin }
  //
  CAstProcedure *subroutine = NULL;
  EToken tt = _scanner->Peek().GetType();
  switch(tt) {
    case tProcedure:
      subroutine = procedureDecl(s);
      break;

    case tFunction:
      subroutine = functionDecl(s);
      break;

    default:
      SetError(_scanner->Peek(), "\"procedure\" or \"function\" expected.");
      break;
  }
  assert(subroutine != NULL);

  s->GetSymbolTable()->AddSymbol(subroutine->GetSymbol());

  subroutineBody(subroutine);
  
  if(_scanner->Peek().GetValue() != subroutine->GetName())
    SetError(_scanner->Peek(), "procedure or function identifier does not match.");

  Consume(tIdent);
  Consume(tSemicolon);
}

void CParser::subroutineBody(CAstScope *s)
{
  //
  // subroutineBody = varDeclaration "begin" statSequence "end".
  //
  // FIRST(subroutineBody) = { tVarDecl, tBegin }
  // FOLLOW(subroutineBody) = { tIdent }
  //
  varDeclaration(s);

  Consume(tBegin);
  CAstStatement *statseq = NULL;
  statseq = statSequence(s);

  Consume(tEnd);

  s->SetStatementSequence(statseq);
}

CAstProcedure* CParser::procedureDecl(CAstScope *s)
{
  //
  // procedureDecl = "procedure" ident [ formalParam ] ";".
  //
  // FIRST(procedureDecl) = { tProcedure }
  // FOLLOW(procedureDecl) = { tVarDecl, tBegin }
  //
  CToken t;
  
  Consume(tProcedure);
  Consume(tIdent, &t);
  
  CSymProc *sr_sym = new CSymProc(t.GetValue(), CTypeManager::Get()->GetNull());
  CAstProcedure *subroutine = new CAstProcedure(t, t.GetValue(), s, sr_sym);
  
  EToken tt = _scanner->Peek().GetType();
  if(tt == tLBrak)
    formalParam(subroutine);
  else if (tt == tColon)
    SetError(_scanner->Peek(), "procedure has no return type.");
  else if (tt != tSemicolon)
    SetError(_scanner->Peek(), "parameter list expected.");

  Consume(tSemicolon);
  
  return subroutine;
}

CAstProcedure* CParser::functionDecl(CAstScope *s)
{
  //
  // functionDecl = "function" ident [formalParam ] ":" type ";".
  //
  // FIRST(functionDecl) = { tFunction }
  // FOLLOW(functionDecl) = { tVarDecl, tBegin }
  //
  CToken t;

  Consume(tFunction);
  Consume(tIdent, &t);

  CSymProc *sr_sym = new CSymProc(t.GetValue(), CTypeManager::Get()->GetNull());
  CAstProcedure *subroutine = new CAstProcedure(t, t.GetValue(), s, sr_sym);

  EToken tt = _scanner->Peek().GetType();
  if(tt != tColon)
    formalParam(subroutine);

  Consume(tColon);

  tt = _scanner->Peek().GetType();
  switch(tt) {
    case tInteger:
      Consume(tInteger);
      subroutine->GetSymbol()->SetDataType(CTypeManager::Get()->GetInt());
      break;

    case tBoolean:
      Consume(tBoolean);
      subroutine->GetSymbol()->SetDataType(CTypeManager::Get()->GetBool());
      break;

    default:
      SetError(_scanner->Peek(), "type expected.");
      break;
  }
  
  Consume(tSemicolon);

  return subroutine;
}

void CParser::formalParam(CAstScope *s)
{
  //
  // formalParam = "(" [ ident { "," ident } ] ")"
  // 
  // FIRST(formalParam) = { tLBrak }
  // FOLLOW(formalParam) = { tSemicolon, tColon }
  //
  CToken t;
  Consume(tLBrak);
  int index = 0;

  do {
    EToken tt = _scanner->Peek().GetType();
    if (tt == tRBrak) break;

    Consume(tIdent, &t);
    if(s->GetSymbolTable()->FindSymbol(t.GetValue(), sLocal))
      SetError(_scanner->Peek(), "Symbol [" + t.GetValue() + "] is already declared.");
    CSymParam *param = new CSymParam(index++, t.GetValue(), CTypeManager::Get()->GetInt());
    dynamic_cast<CAstProcedure*>(s)->GetSymbol()->AddParam(param);
    s->GetSymbolTable()->AddSymbol(param);  

    tt = _scanner->Peek().GetType();
    if (tt != tComma) break;

    Consume(tComma);
  } while (!_abort);

  Consume(tRBrak);
}

CAstStatement* CParser::statSequence(CAstScope *s)
{
  //
  // statSequence = [ statement { ";" statement } ].
  // statement    = assignment | subroutineCall | ifStatement |
  //                whileStatement | returnStatement.
  //
  // FIRST(statSequence) = { tIdent, tIf, tWhile, tReturn, e }
  // FOLLOW(statSequence) = { tEnd, tElse }
  //
  CAstStatement *head = NULL;

  EToken tt = _scanner->Peek().GetType();
  if (tt != tEnd && tt != tElse) {
    CAstStatement *tail = NULL;

    do {
      CToken t;
      tt = _scanner->Peek().GetType();
      CAstStatement *st = NULL;
      CAstDesignator *id = NULL;

      switch (tt) {
        case tIdent:  // assignment or subroutineCall
          id = ident(s);
          assert(id != NULL);
          tt = _scanner->Peek().GetType();
          if (tt == tAssign) {
            if(id->GetSymbol()->GetSymbolType() == stProcedure)
              SetError(_scanner->Peek(), "subroutine symbol can't be assigned.");
            st = assignment(s, id);
          } else if (tt == tLBrak) {
            st = subroutineCall(s, id);
          } else {
            SetError(_scanner->Peek(), "assignment or subroutinecall expected.");
          }
          break;

        case tIf:
          st = ifStatement(s);
          break;

        case tWhile:
          st = whileStatement(s);
          break;

        case tReturn:
          st = returnStatement(s);
          break;

        default:
          SetError(_scanner->Peek(), "statement expected.");
          break;
      }
      assert(st != NULL);
      if (head == NULL) head = st;
      else tail->SetNext(st);
      tail = st;

      tt = _scanner->Peek().GetType();
      if (tt == tEnd || tt == tElse) break;

      Consume(tSemicolon);
    } while (!_abort);
  }

  return head;
}

CAstStatAssign* CParser::assignment(CAstScope *s, CAstDesignator* id)
{
  //
  // assignment = ident ":=" expression.
  //
  // FIRST(assignment) = { tIdent }
  // FOLLOW(assignment) = { tEnd, tElse, tSemicolon }
  //
  // NOTE :: arg 'id' is already consumed at statSequence,
  //         i.e. actual starting point is from ":=".
  //
  CToken t;

  Consume(tAssign, &t);

  CAstExpression *rhs = expression(s);

  return new CAstStatAssign(t, id, rhs);
}

CAstStatCall* CParser::subroutineCall(CAstScope *s, CAstDesignator* id)
{
  //
  // subroutineCall = ident "(" [ expression { "," expression } ] ")".
  //
  // FIRST(subroutineCall) = { tIdent }
  // FOLLOW(subroutineCall) = { tMulDiv, tAnd, tPlusMinus, tOr, tRelOp, tComma,
  //                            tRBrak, tSemicolon, tEnd, tElse }
  //
  // NOTE :: arg 'id' is already consumed at statSequence or factor,
  //         i.e. actual starting point is from "(".

  CToken t;

  Consume(tLBrak, &t);

  CAstFunctionCall *call = NULL;
  if(id->GetSymbol()->GetSymbolType() == stProcedure) {
    const CSymProc *subroutine = dynamic_cast<const CSymProc*>(id->GetSymbol());
    call = new CAstFunctionCall(t, subroutine);
  } else {
    SetError(_scanner->Peek(), "subroutine name expected.");
  }
  assert(call != NULL);

  do {
    EToken tt = _scanner->Peek().GetType();
    if(tt == tRBrak) break;

    CAstExpression *arg = expression(s);
    call->AddArg(arg);

    tt = _scanner->Peek().GetType();
    if(tt == tComma)
      Consume(tComma);
      
  } while(!_abort);

  Consume(tRBrak);

  return new CAstStatCall(t, call);
}

CAstStatIf* CParser::ifStatement(CAstScope *s)
{
  // 
  // ifStatement = "if" "(" expression ")" "then" statSequence
  //               [ "else" statSequence ] "end".
  // 
  // FIRST(ifStatement) = { tIf }
  // FOLLOW(ifStatement) = { tEnd, tElse, tSemicolon }
  //
  CToken t;

  Consume(tIf, &t);
  Consume(tLBrak);

  CAstExpression *cond = expression(s);
  
  Consume(tRBrak);
  Consume(tThen);

  CAstStatement *ifBody = statSequence(s);
  
  CAstStatement *elseBody = NULL;
  EToken tt = _scanner->Peek().GetType();
  if (tt == tElse) {
    Consume(tElse);
    elseBody = statSequence(s);
  }
  
  Consume(tEnd);

  return new CAstStatIf(t, cond, ifBody, elseBody);  
}

CAstStatWhile* CParser::whileStatement(CAstScope *s)
{
  //
  // whileStatement = "while" "(" expression ")" "do" statSequence "end".
  //
  // FIRST(whileStatement) = { tWhile }
  // FOLLOW(whileStatement) = { tEnd, tElse, tSemicolon }
  //
  CToken t;

  Consume(tWhile, &t);
  Consume(tLBrak);

  CAstExpression *cond = expression(s);

  Consume(tRBrak);
  Consume(tDo);

  CAstStatement *body = statSequence(s);
  
  Consume(tEnd);

  return new CAstStatWhile(t, cond, body);
}

CAstStatReturn* CParser::returnStatement(CAstScope *s)
{
  //
  // returnStatement = "return" [ expression ].
  //
  // FIRST(returnStatement) = { tReturn }
  // FOLLOW(returnStatement) = { tEnd, tElse, tSemicolon }
  //
  CToken t;

  Consume(tReturn, &t);
  
  EToken tt = _scanner->Peek().GetType();

  CAstExpression *expr = NULL;
  if(tt != tEnd && tt != tElse && tt != tSemicolon) {
    expr = expression(s);
  }
  
  return new CAstStatReturn(t, s, expr);
}

CAstExpression* CParser::expression(CAstScope* s)
{
  //
  // expression = simpleexpr [ relOp simpleexpr ].
  //
  // FIRST(expression) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(expression) = { tEnd, tElse, tSemicolon, tComma, tRBrak }
  //
  CToken t;
  EOperation relop;
  CAstExpression *left = NULL, *right = NULL;

  left = simpleexpr(s);

  if (_scanner->Peek().GetType() == tRelOp) {
    Consume(tRelOp, &t);
    right = simpleexpr(s);
   
    if (t.GetValue() == "=") relop = opEqual;
    else if (t.GetValue() == "#") relop = opNotEqual;
    else if (t.GetValue() == "<") relop = opLessThan;
    else if (t.GetValue() == "<=") relop = opLessEqual;
    else if (t.GetValue() == ">") relop = opBiggerThan;
    else if (t.GetValue() == ">=") relop = opBiggerEqual;
    else SetError(t, "invalid relation.");
    
    return new CAstBinaryOp(t, relop, left, right);
  } else {
    return left;
  }
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr ::= ["+"|"-"] term { termOp term }.
  //
  // FIRST(simpleexpr) = { tIdent, tNumber, tBoolConst, tLBrak, tNot, tPlusMinus }
  // FOLLOW(simpleexpr) = { tRelOp, tComma, tRBrak, tSemicolon, tEnd, tElse }
  //
  CAstExpression *n = NULL;
  bool skip_n = false;
  
  CToken t;
  EToken tt = _scanner->Peek().GetType();
  if(tt == tPlusMinus) {
    Consume(tPlusMinus, &t);
    if(_scanner->Peek().GetType() == tNumber) {
      if(t.GetValue() == "-")
        n = number(true);
      else
        n = number();
      skip_n = true;
    }
  }
  
  if(!skip_n) {
    n = term(s);

    if (t.GetType() == tPlusMinus) {
      CAstExpression *e = n;
      EOperation oper = (t.GetValue() == "+") ? opPos : opNeg;
      n = new CAstUnaryOp(t, oper, e);
    }
  }

  tt = _scanner->Peek().GetType();
  while (tt == tPlusMinus || tt == tOr) {
    CAstExpression *l = n, *r;
    EOperation oper;

    Consume(tt, &t);
    if(t.GetValue() == "+") oper = opAdd;
    else if(t.GetValue() == "-") oper = opSub;
    else if(t.GetValue() == "||") oper = opOr;
    else SetError(t, "invalid termOp.");

    r = term(s);

    n = new CAstBinaryOp(t, oper, l, r);
    tt = _scanner->Peek().GetType();
  }

  return n;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term = factor { factOp factor }.
  //
  // FIRST(term) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(term) = { tPlusMinus, tOr, tRelOp, tComma, tRBrak, tSemicolon,
  //                  tEnd, tElse }
  //
  CAstExpression *n = NULL;

  n = factor(s);

  EToken tt = _scanner->Peek().GetType();

  while (tt == tMulDiv || tt == tAnd) {
    CToken t;
    CAstExpression *l = n, *r;
    EOperation oper;

    Consume(tt, &t);
    if(t.GetValue() == "*") oper = opMul;
    else if(t.GetValue() == "/") oper = opDiv;
    else if(t.GetValue() == "&&") oper = opAnd;
    else SetError(t, "invalid factOp.");

    r = factor(s);

    n = new CAstBinaryOp(t, oper, l, r);
    tt = _scanner->Peek().GetType();
  }

  return n;
}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor = ident | number | boolean | "(" expression ")" |
  //          subroutineCall | "!" factor.
  //
  // FIRST(factor) = { tIdent, tNumber, tBoolConst, tLBrak, tNot }
  // FOLLOW(factor) = { tMulDiv, tAnd, tPlusMinus, tOr, tRelOp, tComma, tRBrak,
  //                    tSemicolon, tEnd, tElse }
  //
  CToken t;
  CAstExpression *n = NULL;

  CAstExpression *e = NULL; // for factor ::= "!" factor
  CAstStatCall *call = NULL; // for subroutineCall

  EToken tt = _scanner->Peek().GetType();

  CAstDesignator *id = NULL;
  switch (tt) {
    // factor ::= ident | subroutineCall
    case tIdent:
      id = ident(s);
      tt = _scanner->Peek().GetType();
      if(tt == tLBrak) {
        call = subroutineCall(s, id);
        n = call->GetCall();
      } else {
        n = id;
      }
      break;
      
    // factor ::= number
    case tNumber:
      n = number();
      break;
    
    // factor ::= boolean
    case tBoolConst:
      n = boolean();
      break;

    // factor ::= "(" expression ")"
    case tLBrak:
      Consume(tLBrak);
      n = expression(s);
      Consume(tRBrak);
      break;
    
    // factor ::= "!" factor
    case tNot:
      Consume(tNot, &t);
      e = factor(s);
      n = new CAstUnaryOp(t, opNot, e);
      break;

    default:
      SetError(_scanner->Peek(), "factor expected.");
      break;
  }

  return n;
}

CAstConstant* CParser::number(bool minus)
{
  //
  // number = digit { digit }.
  //
  // "digit { digit }" is scanned as one token (tNumber)
  //

  CToken t;

  Consume(tNumber, &t);

  errno = 0;
  long long v = strtoll(t.GetValue().c_str(), NULL, 10);
  if (errno != 0) SetError(t, "invalid number.");
  if (minus) v *= -1;

  return new CAstConstant(t, CTypeManager::Get()->GetInt(), v);
}

CAstConstant* CParser::boolean(void)
{
  //
  // boolean = "true" | "false" 
  //

  CToken t;

  Consume(tBoolConst, &t);

  long long v = (t.GetValue() == "true") ? 1 : 0;

  return new CAstConstant(t, CTypeManager::Get()->GetBool(), v);
}

CAstDesignator* CParser::ident(CAstScope *s)
{
  //
  // ident = letter { letter | digit }.
  //
  // "letter { letter | digit }" is scanned as one token (tIdent)
  //

  CToken t;
  
  Consume(tIdent, &t);
  
  const CSymbol* sym = s->GetSymbolTable()->FindSymbol(t.GetValue());
  if (!sym) // symbol not exists
    SetError(_scanner->Peek(), "Symbol [" + t.GetValue() + "] is not in symtab.");

  return new CAstDesignator(t, sym);
}
