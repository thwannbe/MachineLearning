//------------------------------------------------------------------------------
/// @brief SnuPL abstract syntax tree
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2014/09/28 Bernhard Egger assignment 2: AST for SnuPL/-1
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

#include <iostream>
#include <cassert>

#include <typeinfo>

#include "ast.h"
using namespace std;


//------------------------------------------------------------------------------
// CAstNode
//
int CAstNode::_global_id = 0;

CAstNode::CAstNode(CToken token)
  : _token(token), _addr(NULL)
{
  _id = _global_id++;
}

CAstNode::~CAstNode(void)
{
  if (_addr != NULL) delete _addr;
}

int CAstNode::GetID(void) const
{
  return _id;
}

CToken CAstNode::GetToken(void) const
{
  return _token;
}

const CType* CAstNode::GetType(void) const
{
  return CTypeManager::Get()->GetNull();
}

string CAstNode::dotID(void) const
{
  ostringstream out;
  out << "node" << dec << _id;
  return out.str();
}

string CAstNode::dotAttr(void) const
{
  return " [label=\"" + dotID() + "\"]";
}

void CAstNode::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << dotID() << dotAttr() << ";" << endl;
}

CTacAddr* CAstNode::ToTac(CCodeBlock *cb)
{
  return NULL;
}

CTacAddr* CAstNode::GetTacAddr(void) const
{
  return NULL;
}

ostream& operator<<(ostream &out, const CAstNode &t)
{
  return t.print(out);
}

ostream& operator<<(ostream &out, const CAstNode *t)
{
  return t->print(out);
}

//------------------------------------------------------------------------------
// CAstScope
//
CAstScope::CAstScope(CToken t, const string name, CAstScope *parent)
  : CAstNode(t), _name(name), _symtab(NULL), _parent(parent), _statseq(NULL),
    _cb(NULL)
{
  if (_parent != NULL) _parent->AddChild(this);
}

CAstScope::~CAstScope(void)
{
  delete _symtab;
  delete _statseq;
  delete _cb;
}

const string CAstScope::GetName(void) const
{
  return _name;
}

CAstScope* CAstScope::GetParent(void) const
{
  return _parent;
}

size_t CAstScope::GetNumChildren(void) const
{
  return _children.size();
}

CAstScope* CAstScope::GetChild(size_t i) const
{
  assert(i < _children.size());
  return _children[i];
}

CSymtab* CAstScope::GetSymbolTable(void) const
{
  assert(_symtab != NULL);
  return _symtab;
}

void CAstScope::SetStatementSequence(CAstStatement *statseq)
{
  _statseq = statseq;
}

CAstStatement* CAstScope::GetStatementSequence(void) const
{
  return _statseq;
}

bool CAstScope::TypeCheck(CToken *t, string *msg) const
{
  bool result = true;

  try {
    CAstStatement *s = _statseq;
    while (result && (s != NULL)) {
      result = s->TypeCheck(t, msg);
      s = s->GetNext();
    }

    vector<CAstScope*>::const_iterator it = _children.begin();
    while (result && (it != _children.end())) {
      result = (*it)->TypeCheck(t, msg);
      it++;
    }
  } catch (...) {
    result = false;
  }
  
  return result;
}

ostream& CAstScope::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "CAstScope: '" << _name << "'" << endl;
  out << ind << "  symbol table:" << endl;
  _symtab->print(out, indent+4);
  out << ind << "  statement list:" << endl;
  CAstStatement *s = GetStatementSequence();
  if (s != NULL) {
    do {
      s->print(out, indent+4);
      s = s->GetNext();
    } while (s != NULL);
  } else {
    out << ind << "    empty." << endl;
  }

  out << ind << "  nested scopes:" << endl;
  if (_children.size() > 0) {
    for (size_t i=0; i<_children.size(); i++) {
      _children[i]->print(out, indent+4);
    }
  } else {
    out << ind << "    empty." << endl;
  }
  out << ind << endl;

  return out;
}

void CAstScope::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  CAstStatement *s = GetStatementSequence();
  if (s != NULL) {
    string prev = dotID();
    do {
      s->toDot(out, indent);
      out << ind << prev << " -> " << s->dotID() << " [style=dotted];" << endl;
      prev = s->dotID();
      s = s->GetNext();
    } while (s != NULL);
  }

  vector<CAstScope*>::const_iterator it = _children.begin();
  while (it != _children.end()) {
    CAstScope *s = *it++;
    s->toDot(out, indent);
    out << ind << dotID() << " -> " << s->dotID() << ";" << endl;
  }

}

CTacAddr* CAstScope::ToTac(CCodeBlock *cb)
{
  assert(cb != NULL);

  CAstStatement *s = GetStatementSequence();
  // ToTac for all statement sequence.
  while (s != NULL) {
    CTacLabel *next = cb->CreateLabel();
    s->ToTac(cb, next);
    cb->AddInstr(next);
    s = s->GetNext();
  }
  // clean up all unnecessary labels.
  cb->CleanupControlFlow();
  
  return NULL;
}

CCodeBlock* CAstScope::GetCodeBlock(void) const
{
  return _cb;
}

void CAstScope::SetSymbolTable(CSymtab *st)
{
  if (_symtab != NULL) delete _symtab;
  _symtab = st;
}

void CAstScope::AddChild(CAstScope *child)
{
  _children.push_back(child);
}


//------------------------------------------------------------------------------
// CAstModule
//
CAstModule::CAstModule(CToken t, const string name)
  : CAstScope(t, name, NULL)
{
  SetSymbolTable(new CSymtab());
}

CSymbol* CAstModule::CreateVar(const string ident, const CType *type)
{
  return new CSymGlobal(ident, type);
}

string CAstModule::dotAttr(void) const
{
  return " [label=\"m " + GetName() + "\",shape=box]";
}



//------------------------------------------------------------------------------
// CAstProcedure
//
CAstProcedure::CAstProcedure(CToken t, const string name,
                             CAstScope *parent, CSymProc *symbol)
  : CAstScope(t, name, parent), _symbol(symbol)
{
  assert(GetParent() != NULL);
  SetSymbolTable(new CSymtab(GetParent()->GetSymbolTable()));
  assert(_symbol != NULL);
}

CSymProc* CAstProcedure::GetSymbol(void) const
{
  return _symbol;
}

CSymbol* CAstProcedure::CreateVar(const string ident, const CType *type)
{
  return new CSymLocal(ident, type);
}

const CType* CAstProcedure::GetType(void) const
{
  return GetSymbol()->GetDataType();
}

string CAstProcedure::dotAttr(void) const
{
  return " [label=\"p/f " + GetName() + "\",shape=box]";
}


//------------------------------------------------------------------------------
// CAstType
//
CAstType::CAstType(CToken t, const CType *type)
  : CAstNode(t), _type(type)
{
  assert(type != NULL);
}

const CType* CAstType::GetType(void) const
{
  return _type;
}

ostream& CAstType::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "CAstType (" << _type << ")" << endl;
  return out;
}


//------------------------------------------------------------------------------
// CAstStatement
//
CAstStatement::CAstStatement(CToken token)
  : CAstNode(token), _next(NULL)
{
}

CAstStatement::~CAstStatement(void)
{
  delete _next;
}

void CAstStatement::SetNext(CAstStatement *next)
{
  _next = next;
}

CAstStatement* CAstStatement::GetNext(void) const
{
  return _next;
}

CTacAddr* CAstStatement::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  // generate code for the statement
  // this code should be overriden by its child.

  cb->AddInstr(new CTacInstr(opGoto, next));
  
  return NULL;
}


//------------------------------------------------------------------------------
// CAstStatAssign
//
CAstStatAssign::CAstStatAssign(CToken t,
                               CAstDesignator *lhs, CAstExpression *rhs)
  : CAstStatement(t), _lhs(lhs), _rhs(rhs)
{
  assert(lhs != NULL);
  assert(rhs != NULL);
}

CAstDesignator* CAstStatAssign::GetLHS(void) const
{
  return _lhs;
}

CAstExpression* CAstStatAssign::GetRHS(void) const
{
  return _rhs;
}

bool CAstStatAssign::TypeCheck(CToken *t, string *msg) const
{
  CAstDesignator *id = GetLHS();
  CAstExpression *e = GetRHS();
  
  if (!id->TypeCheck(t, msg)) return false;
  if (!e->TypeCheck(t, msg)) return false;

  if (!id->GetType()->Match(e->GetType())) {
    if (t != NULL) *t = e->GetToken();
    if (msg != NULL) *msg = "assignment type mismatch.";
    return false;
  }

  return true;
}

const CType* CAstStatAssign::GetType(void) const
{
  return _lhs->GetType();
}

ostream& CAstStatAssign::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << ":=" << " ";

  out << endl;

  _lhs->print(out, indent+2);
  _rhs->print(out, indent+2);

  return out;
}

string CAstStatAssign::dotAttr(void) const
{
  return " [label=\":=\",shape=box]";
}

void CAstStatAssign::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  _lhs->toDot(out, indent);
  out << ind << dotID() << "->" << _lhs->dotID() << ";" << endl;
  _rhs->toDot(out, indent);
  out << ind << dotID() << "->" << _rhs->dotID() << ";" << endl;
}

CTacAddr* CAstStatAssign::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(cb != NULL && next != NULL);
  
  CAstDesignator *lhs = GetLHS();
  CAstExpression *rhs = GetRHS();

  CTac *tlhs = lhs->ToTac(cb);
  CTacAddr *trhs = rhs->ToTac(cb);
  
  cb->AddInstr(new CTacInstr(opAssign, tlhs, trhs));
  cb->AddInstr(new CTacInstr(opGoto, next));

  return NULL;
}


//------------------------------------------------------------------------------
// CAstStatCall
//
CAstStatCall::CAstStatCall(CToken t, CAstFunctionCall *call)
  : CAstStatement(t), _call(call)
{
  assert(call != NULL);
}

CAstFunctionCall* CAstStatCall::GetCall(void) const
{
  return _call;
}

bool CAstStatCall::TypeCheck(CToken *t, string *msg) const
{
  return GetCall()->TypeCheck(t, msg);
}

ostream& CAstStatCall::print(ostream &out, int indent) const
{
  _call->print(out, indent);

  return out;
}

string CAstStatCall::dotID(void) const
{
  return _call->dotID();
}

string CAstStatCall::dotAttr(void) const
{
  return _call->dotAttr();
}

void CAstStatCall::toDot(ostream &out, int indent) const
{
  _call->toDot(out, indent);
}

CTacAddr* CAstStatCall::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(cb != NULL && next != NULL);
  
  CAstFunctionCall *call = GetCall();
  
  // Actual call instruction is made in CAstFunctionCall.
  call->ToTac(cb);
  cb->AddInstr(new CTacInstr(opGoto, next));

  return NULL;
}


//------------------------------------------------------------------------------
// CAstStatReturn
//
CAstStatReturn::CAstStatReturn(CToken t, CAstScope *scope, CAstExpression *expr)
  : CAstStatement(t), _scope(scope), _expr(expr)
{
  assert(scope != NULL);
}

CAstScope* CAstStatReturn::GetScope(void) const
{
  return _scope;
}

CAstExpression* CAstStatReturn::GetExpression(void) const
{
  return _expr;
}

bool CAstStatReturn::TypeCheck(CToken *t, string *msg) const
{
  const CType *st = GetScope()->GetType();
  CAstExpression *e = GetExpression();
  
  if (st->Match(CTypeManager::Get()->GetNull())) {
    if (e != NULL) {
      if (t != NULL) *t = e->GetToken();
      if (msg != NULL) *msg = "superfluous expression after return.";
      return false;
    }
  } else {
    if (e == NULL) {
      if (t != NULL) *t = GetToken();
      if (msg != NULL) *msg = "expression expected after return.";
      return false;
    }

    if (!e->TypeCheck(t, msg)) return false;

    if (!st->Match(e->GetType())) {
      if (t != NULL) *t = e->GetToken();
      if (msg != NULL) *msg = "return type mismatch.";
      return false;
    }
  }

  return true;
}

const CType* CAstStatReturn::GetType(void) const
{
  const CType *t = NULL;

  if (GetExpression() != NULL) {
    t = GetExpression()->GetType();
  } else {
    t = CTypeManager::Get()->GetNull();
  }

  return t;
}

ostream& CAstStatReturn::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "return" << " ";

  out << endl;

  if (_expr != NULL) _expr->print(out, indent+2);

  return out;
}

string CAstStatReturn::dotAttr(void) const
{
  return " [label=\"return\",shape=box]";
}

void CAstStatReturn::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  if (_expr != NULL) {
    _expr->toDot(out, indent);
    out << ind << dotID() << "->" << _expr->dotID() << ";" << endl;
  }
}

CTacAddr* CAstStatReturn::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(cb != NULL && next != NULL);
  
  CAstExpression *expr = GetExpression();
  CTacAddr *tExpr = NULL;

  if(expr)  // function case.
    tExpr = expr->ToTac(cb);

  cb->AddInstr(new CTacInstr(opReturn, NULL, tExpr));
  cb->AddInstr(new CTacInstr(opGoto, next));
  
  return NULL;
}


//------------------------------------------------------------------------------
// CAstStatIf
//
CAstStatIf::CAstStatIf(CToken t, CAstExpression *cond,
                       CAstStatement *ifBody, CAstStatement *elseBody)
  : CAstStatement(t), _cond(cond), _ifBody(ifBody), _elseBody(elseBody)
{
  assert(cond != NULL);
}

CAstExpression* CAstStatIf::GetCondition(void) const
{
  return _cond;
}

CAstStatement* CAstStatIf::GetIfBody(void) const
{
  return _ifBody;
}

CAstStatement* CAstStatIf::GetElseBody(void) const
{
  return _elseBody;
}

bool CAstStatIf::TypeCheck(CToken *t, string *msg) const
{
  CAstExpression *cond = GetCondition();
  bool result = true;
  
  if (!cond->TypeCheck(t, msg)) return false;
  if (cond->GetType() != CTypeManager::Get()->GetBool()) {
    if (t != NULL) *t = cond->GetToken();
    if (msg != NULL) *msg = "boolean expression expected.";
    return false;
  }

  try {
    CAstStatement *ifBody = GetIfBody();
    CAstStatement *elseBody = GetElseBody();
    
    while (result && (ifBody != NULL)) {
      result = ifBody->TypeCheck(t, msg);
      ifBody = ifBody->GetNext();
    }
    
    while (result && (elseBody != NULL)) {
      result = elseBody->TypeCheck(t, msg);
      elseBody = elseBody->GetNext();
    }
  } catch (...) {
    result = false;
  }
  
  return result;
}

ostream& CAstStatIf::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "if cond" << endl;
  _cond->print(out, indent+2);
  out << ind << "if-body" << endl;
  if (_ifBody != NULL) {
    CAstStatement *s = _ifBody;
    do {
      s->print(out, indent+2);
      s = s->GetNext();
    } while (s != NULL);
  } else out << ind << "  empty." << endl;
  out << ind << "else-body" << endl;
  if (_elseBody != NULL) {
    CAstStatement *s = _elseBody;
    do {
      s->print(out, indent+2);
      s = s->GetNext();
    } while (s != NULL);
  } else out << ind << "  empty." << endl;

  return out;
}

string CAstStatIf::dotAttr(void) const
{
  return " [label=\"if\",shape=box]";
}

void CAstStatIf::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  _cond->toDot(out, indent);
  out << ind << dotID() << "->" << _cond->dotID() << ";" << endl;

  if (_ifBody != NULL) {
    CAstStatement *s = _ifBody;
    if (s != NULL) {
      string prev = dotID();
      do {
        s->toDot(out, indent);
        out << ind << prev << " -> " << s->dotID() << " [style=dotted];"
            << endl;
        prev = s->dotID();
        s = s->GetNext();
      } while (s != NULL);
    }
  }

  if (_elseBody != NULL) {
    CAstStatement *s = _elseBody;
    if (s != NULL) {
      string prev = dotID();
      do {
        s->toDot(out, indent);
        out << ind << prev << " -> " << s->dotID() << " [style=dotted];" 
            << endl;
        prev = s->dotID();
        s = s->GetNext();
      } while (s != NULL);
    }
  }
}

CTacAddr* CAstStatIf::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(cb != NULL && next != NULL);
    
  CAstExpression *cond = GetCondition();
  CAstStatement *ifBody = GetIfBody();
  CAstStatement *elseBody = GetElseBody();
  
  CTacLabel *c_true = cb->CreateLabel("if_true");
  CTacLabel *c_false = cb->CreateLabel("if_false");
  
  // starting with translate condition
  CTacAddr *tCond = cond->ToTac(cb, c_true, c_false);
  if(dynamic_cast<CAstDesignator*>(cond)) { // if cond is a single boolean
    cb->AddInstr(new CTacInstr(opEqual, c_true, tCond, new CTacConst(1)));
    cb->AddInstr(new CTacInstr(opGoto, c_false)); 
  }

  // if_body starts..
  cb->AddInstr(c_true);
  while(ifBody) {
    CTacLabel *if_next = cb->CreateLabel("if_next");
    ifBody->ToTac(cb, if_next);
    cb->AddInstr(if_next);
    ifBody = ifBody->GetNext();
  }
  cb->AddInstr(new CTacInstr(opGoto, next));

  // if else_body exists, else_body starts..
  cb->AddInstr(c_false);
  if(elseBody) {
    while(elseBody) {
      CTacLabel *else_next = cb->CreateLabel("else_next");
      elseBody->ToTac(cb, else_next);
      cb->AddInstr(else_next);
      elseBody = elseBody->GetNext();
    }
    cb->AddInstr(new CTacInstr(opGoto, next));
  }
  
  return NULL;
}


//------------------------------------------------------------------------------
// CAstStatWhile
//
CAstStatWhile::CAstStatWhile(CToken t,
                             CAstExpression *cond, CAstStatement *body)
  : CAstStatement(t), _cond(cond), _body(body)
{
  assert(cond != NULL);
}

CAstExpression* CAstStatWhile::GetCondition(void) const
{
  return _cond;
}

CAstStatement* CAstStatWhile::GetBody(void) const
{
  return _body;
}

bool CAstStatWhile::TypeCheck(CToken *t, string *msg) const
{
  CAstExpression *cond = GetCondition();
  bool result = true;

  if (!cond->TypeCheck(t, msg)) return false;
  if (cond->GetType() != CTypeManager::Get()->GetBool()) {
    if (t != NULL) *t = cond->GetToken();
    if (msg != NULL) *msg = "boolean expression expected.";
    return false;
  }
  
  try {
    CAstStatement *body = GetBody();
    while (result && (body != NULL)) {
      result = body->TypeCheck(t, msg);
      body = body->GetNext();
    }
  } catch (...) {
    result = false;
  }
  
  return result;
}

ostream& CAstStatWhile::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "while cond" << endl;
  _cond->print(out, indent+2);
  out << ind << "while-body" << endl;
  if (_body != NULL) {
    CAstStatement *s = _body;
    do {
      s->print(out, indent+2);
      s = s->GetNext();
    } while (s != NULL);
  }
  else out << ind << "  empty." << endl;

  return out;
}

string CAstStatWhile::dotAttr(void) const
{
  return " [label=\"while\",shape=box]";
}

void CAstStatWhile::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  _cond->toDot(out, indent);
  out << ind << dotID() << "->" << _cond->dotID() << ";" << endl;

  if (_body != NULL) {
    CAstStatement *s = _body;
    if (s != NULL) {
      string prev = dotID();
      do {
        s->toDot(out, indent);
        out << ind << prev << " -> " << s->dotID() << " [style=dotted];"
            << endl;
        prev = s->dotID();
        s = s->GetNext();
      } while (s != NULL);
    }
  }
}

CTacAddr* CAstStatWhile::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(cb != NULL && next != NULL);
    
  CAstExpression *cond = GetCondition();
  CAstStatement *body = GetBody();
  
  CTacLabel *while_cond = cb->CreateLabel("while_cond");
  CTacLabel *c_true = cb->CreateLabel("while_body");

  // while condition begins
  cb->AddInstr(while_cond);
  CTacAddr *tCond = cond->ToTac(cb, c_true, next);
  if(dynamic_cast<CAstDesignator*>(cond)) { // if cond is a single boolean
    cb->AddInstr(new CTacInstr(opEqual, c_true, tCond, new CTacConst(1)));
    cb->AddInstr(new CTacInstr(opGoto, next));
  }

  // while body starts..
  cb->AddInstr(c_true);
  while(body) {
    CTacLabel *while_next = cb->CreateLabel("while_next");
    body->ToTac(cb, while_next);
    cb->AddInstr(while_next);
    body = body->GetNext();
  }

  cb->AddInstr(new CTacInstr(opGoto, while_cond));
  
  return NULL;
}


//------------------------------------------------------------------------------
// CAstExpression
//
CAstExpression::CAstExpression(CToken t)
  : CAstNode(t)
{
}

CTacAddr* CAstExpression::ToTac(CCodeBlock *cb,
                                CTacLabel *lfalse, CTacLabel *ltrue)
{
  // generate jumping code for boolean expression

  return NULL;
}

//------------------------------------------------------------------------------
// CAstOperation
//
CAstOperation::CAstOperation(CToken t, EOperation oper)
  : CAstExpression(t), _oper(oper)
{
}

EOperation CAstOperation::GetOperation(void) const
{
  return _oper;
}


//------------------------------------------------------------------------------
// CAstBinaryOp
//
CAstBinaryOp::CAstBinaryOp(CToken t, EOperation oper,
                           CAstExpression *l,CAstExpression *r)
  : CAstOperation(t, oper), _left(l), _right(r)
{
  // these are the only binary operation we support for now
  assert((oper == opAdd)        || (oper == opSub)         ||
         (oper == opMul)        || (oper == opDiv)         ||
         (oper == opAnd)        || (oper == opOr)          ||
         (oper == opEqual)      || (oper == opNotEqual)    ||
         (oper == opLessThan)   || (oper == opLessEqual)   ||
         (oper == opBiggerThan) || (oper == opBiggerEqual)
        );
  assert(l != NULL);
  assert(r != NULL);
}

CAstExpression* CAstBinaryOp::GetLeft(void) const
{
  return _left;
}

CAstExpression* CAstBinaryOp::GetRight(void) const
{
  return _right;
}

bool CAstBinaryOp::TypeCheck(CToken *t, string *msg) const
{
  EOperation oper = GetOperation();
  CAstExpression *left = GetLeft();
  CAstExpression *right = GetRight();

  if(!left->TypeCheck(t, msg)) return false;
  if(!right->TypeCheck(t, msg)) return false;

  switch(oper) {
    case opAdd:
    case opSub:
    case opMul:
    case opDiv:
      if(left->GetType() != CTypeManager::Get()->GetInt() ||
        right->GetType() != CTypeManager::Get()->GetInt()) {
        if (t != NULL) *t = GetToken();
        if (msg != NULL) *msg = "Operand types should be integer for '+','-','*',or '/'.";
        return false;
      }
      break;
    case opAnd:
    case opOr:
      if(left->GetType() != CTypeManager::Get()->GetBool() ||
        right->GetType() != CTypeManager::Get()->GetBool()) {
        if (t != NULL) *t = GetToken();
        if (msg != NULL) *msg = "Operand types should be boolean for '&&',or '||'.";
        return false;
      }
      break;
    case opEqual:
    case opNotEqual:
      if(left->GetType() != right->GetType()) {
        if (t != NULL) *t = GetToken();
        if (msg != NULL) *msg = "Operand types should be same for '=',or '#'.";
        return false;
      } else if(left->GetType() == CTypeManager::Get()->GetNull()) {
        // left and right has same type, so we only check left type
        if (t != NULL) *t = GetToken();
        if (msg != NULL) *msg = "Operand types should not be NULL.";
        return false;
      }
      break;
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      if(left->GetType() != CTypeManager::Get()->GetInt() ||
        right->GetType() != CTypeManager::Get()->GetInt()) {
        if (t != NULL) *t = GetToken();
        if (msg != NULL) *msg = "Operand types should be integer for '<','<=','>',or '>='.";
        return false;
      }
      break;
  }
  
  return true;
}

const CType* CAstBinaryOp::GetType(void) const
{
  EOperation oper = GetOperation();
  const CType *ret = NULL;
  
  switch(oper) {
    case opAdd:
    case opSub:
    case opMul:
    case opDiv:
      ret = CTypeManager::Get()->GetInt(); break;
    case opAnd:
    case opOr:
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      ret = CTypeManager::Get()->GetBool(); break;
  }

  return ret;
}

ostream& CAstBinaryOp::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << GetOperation() << " ";

  out << endl;

  _left->print(out, indent+2);
  _right->print(out, indent+2);

  return out;
}

string CAstBinaryOp::dotAttr(void) const
{
  ostringstream out;
  out << " [label=\"" << GetOperation() << "\",shape=box]";
  return out.str();
}

void CAstBinaryOp::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  _left->toDot(out, indent);
  out << ind << dotID() << "->" << _left->dotID() << ";" << endl;
  _right->toDot(out, indent);
  out << ind << dotID() << "->" << _right->dotID() << ";" << endl;
}

CTacAddr* CAstBinaryOp::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  assert(cb != NULL);

  CAstExpression *left = GetLeft();
  CAstExpression *right = GetRight();
  CTacAddr *ret = NULL;
  CTacAddr *tLeft = NULL;
  CTacAddr *tRight = NULL;
  
  // depends on operation type, ToTac differently.
  EOperation oper = GetOperation();
  
  if(IsRelOp(oper) || oper == opOr || oper == opAnd) { // related with boolean
    // if no given label for true & false, create them.
    CTacLabel *l_true = (ltrue) ? ltrue : cb->CreateLabel();
    CTacLabel *l_false = (lfalse) ? lfalse : cb->CreateLabel();
    CTacLabel *l_next = NULL;
    CTacLabel *test_next = cb->CreateLabel();
    
    if(!ltrue || !lfalse) { // it is most out binary ops
      // always return type is boolean.
      ret = cb->CreateTemp(CTypeManager::Get()->GetBool());
      // l_next is needed.
      l_next = cb->CreateLabel();
    }
    switch(oper) {
      case opOr:
        tLeft = left->ToTac(cb, l_true, test_next);
        // left term is boolean constant.
        if(CAstConstant* boolConst = dynamic_cast<CAstConstant*>(left)) {
          if(boolConst->GetValue() == 1) // true
            cb->AddInstr(new CTacInstr(opGoto, l_true));
          else // false
            cb->AddInstr(new CTacInstr(opGoto, test_next));
        }
        //else if(dynamic_cast<CAstDesignator*>(left)) { // left term is id.
        else if(tLeft) { // if left term is id or func call, then tLeft is not null
          cb->AddInstr(new CTacInstr(opEqual, l_true, tLeft, new CTacConst(1)));
          cb->AddInstr(new CTacInstr(opGoto, test_next));
        }
        cb->AddInstr(test_next);
        tRight = right->ToTac(cb, l_true, l_false);
        // right term is boolean constant.
        if(CAstConstant* boolConst = dynamic_cast<CAstConstant*>(right)) {
          if(boolConst->GetValue() == 1) // true
            cb->AddInstr(new CTacInstr(opGoto, l_true));
          else // false
            cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        //else if(dynamic_cast<CAstDesignator*>(right)) { // right term is id.
        else if(tRight) { // if right term is id or func call, then tRight is not null
          cb->AddInstr(new CTacInstr(opEqual, l_true, tRight, new CTacConst(1)));
          cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        break;
      case opAnd:
        tLeft = left->ToTac(cb, test_next, l_false);
        // left term is boolean constant.
        if(CAstConstant* boolConst = dynamic_cast<CAstConstant*>(left)) {
          if(boolConst->GetValue() == 1) // true
            cb->AddInstr(new CTacInstr(opGoto, test_next));
          else // false
            cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        //else if(dynamic_cast<CAstDesignator*>(left)) { // left term is id.
        else if(tLeft) { // if left term is id or func call, then tLeft is not null
          cb->AddInstr(new CTacInstr(opEqual, test_next, tLeft, new CTacConst(1)));
          cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        cb->AddInstr(test_next);
        tRight = right->ToTac(cb, l_true, l_false);
        // right term is boolean constant.
        if(CAstConstant* boolConst = dynamic_cast<CAstConstant*>(right)) {
          if(boolConst->GetValue() == 1) // true
            cb->AddInstr(new CTacInstr(opGoto, l_true));
          else // false
            cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        //else if(dynamic_cast<CAstDesignator*>(right)) { // right term is id.
        else if(tRight) { // if right term is id or func call, then tRight is not null
          cb->AddInstr(new CTacInstr(opEqual, l_true, tRight, new CTacConst(1)));
          cb->AddInstr(new CTacInstr(opGoto, l_false));
        }
        break;
      // rel Ops
      case opEqual:
      case opNotEqual:
      case opLessThan:
      case opLessEqual:
      case opBiggerThan:
      case opBiggerEqual:
        tLeft = left->ToTac(cb, l_true, l_false);
        tRight = right->ToTac(cb, l_true, l_false);
        cb->AddInstr(new CTacInstr(oper, l_true, tLeft, tRight));
        cb->AddInstr(new CTacInstr(opGoto, l_false));
        break;
    }
    if(!ltrue)  { // assign temp as true
      cb->AddInstr(l_true);
      cb->AddInstr(new CTacInstr(opAssign, ret, new CTacConst(1)));
      cb->AddInstr(new CTacInstr(opGoto, l_next));
    }
    if(!lfalse) { // assign temp as false
      cb->AddInstr(l_false);
      cb->AddInstr(new CTacInstr(opAssign, ret, new CTacConst(0)));
      cb->AddInstr(new CTacInstr(opGoto, l_next));
    }
    if(!ltrue || !lfalse)
      cb->AddInstr(l_next);
  }
  else { // related with integer
    // no need for label of true & false
    tLeft = left->ToTac(cb);
    tRight = right->ToTac(cb);

    ret = cb->CreateTemp(left->GetType()); // left & right same type.
    cb->AddInstr(new CTacInstr(oper, ret, tLeft, tRight));
  }
  
  return ret;
}


//------------------------------------------------------------------------------
// CAstUnaryOp
//
CAstUnaryOp::CAstUnaryOp(CToken t, EOperation oper, CAstExpression *e)
  : CAstOperation(t, oper), _operand(e)
{
  assert((oper == opPos) || (oper == opNeg) || (oper == opNot));
  assert(e != NULL);
}

CAstExpression* CAstUnaryOp::GetOperand(void) const
{
  return _operand;
}

bool CAstUnaryOp::TypeCheck(CToken *t, string *msg) const
{
  CAstExpression *e = GetOperand();
  
  if (!e->TypeCheck(t, msg)) return false;
  
  if( ((GetOperation() == opPos || GetOperation() == opNeg) && 
      (e->GetType() != CTypeManager::Get()->GetInt())) ||
      ((GetOperation() == opNot) && 
      (e->GetType() != CTypeManager::Get()->GetBool())) ) {
    if (t != NULL) *t = GetToken();
    if (msg != NULL) *msg = "unary operation type mismatch.";
    return false;
  }
  
  return true;
}

const CType* CAstUnaryOp::GetType(void) const
{
  return _operand->GetType();
}

ostream& CAstUnaryOp::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << GetOperation() << " ";

  out << endl;

  _operand->print(out, indent+2);

  return out;
}

string CAstUnaryOp::dotAttr(void) const
{
  ostringstream out;
  out << " [label=\"" << GetOperation() << "\",shape=box]";
  return out.str();
}

void CAstUnaryOp::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  _operand->toDot(out, indent);
  out << ind << dotID() << "->" << _operand->dotID() << ";" << endl;
}

CTacAddr* CAstUnaryOp::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  assert(cb != NULL);

  CTacAddr *ret = NULL;
  EOperation oper = GetOperation();
  CAstExpression *operand = GetOperand();
  CTacAddr *tOperand = NULL;

  if(oper == opNot) { // boolean
    CTacLabel *c_true = ltrue;
    CTacLabel *c_false = lfalse;
    if(!c_true || !c_false) {
      c_true = cb->CreateLabel();
      c_false = cb->CreateLabel();
    }

    tOperand = operand->ToTac(cb, c_false, c_true); // swap
    // last sequence 'not' unary op
    if(CAstConstant* boolConst = dynamic_cast<CAstConstant*>(operand)) {
      // operand is bool const
      if(boolConst->GetValue() == 1) // true
        cb->AddInstr(new CTacInstr(opGoto, c_false)); // !true = false
      else // false
        cb->AddInstr(new CTacInstr(opGoto, c_true)); // !false = true
    }
    else if(tOperand) { // if operand is id or func call, then tOperand returns not null
      // operand is id
      cb->AddInstr(new CTacInstr(opEqual, c_false, tOperand, new CTacConst(1)));
      cb->AddInstr(new CTacInstr(opGoto, c_true));
    }


    if(ltrue || lfalse) { // not first of sequence 'not' unary op
      ret = NULL;
    }
    else { // first of sequence 'not' unary op
      CTacLabel *next = cb->CreateLabel();
      cb->AddInstr(c_true);
      ret = cb->CreateTemp(operand->GetType());    
      cb->AddInstr(new CTacInstr(opAssign, ret, new CTacConst(1)));
      cb->AddInstr(new CTacInstr(opGoto, next));
      cb->AddInstr(c_false);
      cb->AddInstr(new CTacInstr(opAssign, ret, new CTacConst(0)));
      cb->AddInstr(new CTacInstr(opGoto, next));
      cb->AddInstr(next);
    }
  }
  else { // opPos || opNeg - integer
    tOperand = operand->ToTac(cb);
    ret = cb->CreateTemp(operand->GetType());    
    cb->AddInstr(new CTacInstr(oper, ret, tOperand));
  }
  return ret;
}


//------------------------------------------------------------------------------
// CAstFunctionCall
//
CAstFunctionCall::CAstFunctionCall(CToken t, const CSymProc *symbol)
  : CAstExpression(t), _symbol(symbol)
{
  assert(symbol != NULL);
}

const CSymProc* CAstFunctionCall::GetSymbol(void) const
{
  return _symbol;
}

void CAstFunctionCall::AddArg(CAstExpression *arg)
{
  _arg.push_back(arg);
}

int CAstFunctionCall::GetNArgs(void) const
{
  return (int)_arg.size();
}

CAstExpression* CAstFunctionCall::GetArg(int index) const
{
  assert((index >= 0) && (index < _arg.size()));
  return _arg[index];
}

bool CAstFunctionCall::TypeCheck(CToken *t, string *msg) const
{
  const CSymProc *symbol = GetSymbol();
  
  // check the number of procedure/function arguments.
  if (GetNArgs() != symbol->GetNParams()) {
    if (t != NULL) *t = GetToken();
    if (msg != NULL) *msg = "the number of arguments mismatch.";
    return false;
  }

  // first, type check for expression of arguments.
  // then, check the types of procedure/function arguments.
  // in this project, arguments type is only integer.
  for (int i = 0; i < GetNArgs(); i++) {
    CAstExpression *arg = GetArg(i);
    if (!arg->TypeCheck(t, msg)) return false;

    if (arg->GetType() != symbol->GetParam(i)->GetDataType()) {
      if (t != NULL) *t = arg->GetToken();
      if (msg != NULL) *msg = "the type of arguments mismatch.";
      return false;
    }
  }

  return true;
}

const CType* CAstFunctionCall::GetType(void) const
{
  return GetSymbol()->GetDataType();
}

ostream& CAstFunctionCall::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "call " << _symbol << " ";
  
  out << endl;

  for (size_t i=0; i<_arg.size(); i++) {
    _arg[i]->print(out, indent+2);
  }

  return out;
}

string CAstFunctionCall::dotAttr(void) const
{
  ostringstream out;
  out << " [label=\"call " << _symbol->GetName() << "\",shape=box]";
  return out.str();
}

void CAstFunctionCall::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  for (size_t i=0; i<_arg.size(); i++) {
    _arg[i]->toDot(out, indent);
    out << ind << dotID() << "->" << _arg[i]->dotID() << ";" << endl;
  }
}

CTacAddr* CAstFunctionCall::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  assert(cb != NULL);

  CTac *tDest = NULL;
  
  // parameters instructions
  for(int index = GetNArgs()-1; index >= 0; index--) {
    CAstExpression *arg = GetArg(index);
    CTacAddr *tArg = arg->ToTac(cb);
    CTac *tIndex = new CTacConst(index);
    cb->AddInstr(new CTacInstr(opParam, tIndex, tArg));
  }
  
  // check if procedure or function
  if(GetType() != CTypeManager::Get()->GetNull())
    tDest = cb->CreateTemp(GetType());

  cb->AddInstr(new CTacInstr(opCall, tDest, new CTacName(GetSymbol())));
  
  return dynamic_cast<CTacAddr*>(tDest);
}


//------------------------------------------------------------------------------
// CAstOperand
//
CAstOperand::CAstOperand(CToken t)
  : CAstExpression(t)
{
}


//------------------------------------------------------------------------------
// CAstDesignator
//
CAstDesignator::CAstDesignator(CToken t, const CSymbol *symbol,
                               CAstExpression *offset)
  : CAstOperand(t), _symbol(symbol), _offset(offset)
{
  assert(symbol != NULL);
}

const CSymbol* CAstDesignator::GetSymbol(void) const
{
  return _symbol;
}

bool CAstDesignator::TypeCheck(CToken *t, string *msg) const
{
  return true;
}

const CType* CAstDesignator::GetType(void) const
{
  const CType *t = GetSymbol()->GetDataType();

  if (_offset != NULL) {
    if (t->IsArray() && (_offset->GetType()->IsScalar())) {
      const CArrayType *at = dynamic_cast<const CArrayType*>(t);
      assert(at != NULL);
      t = at->GetBaseType();
    } else {
      t = NULL;
    }
  }

  return t;
}

ostream& CAstDesignator::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << _symbol << " ";

  out << endl;

  if (_offset != NULL) _offset->print(out, indent+2);

  return out;
}

string CAstDesignator::dotAttr(void) const
{
  ostringstream out;
  out << " [label=\"" << _symbol->GetName();
  if (_offset != NULL) out << "[]";
  out << "\",shape=ellipse]";
  return out.str();
}

void CAstDesignator::toDot(ostream &out, int indent) const
{
  string ind(indent, ' ');

  CAstNode::toDot(out, indent);

  if (_offset != NULL) {
    _offset->toDot(out, indent);
    out << ind << dotID() << "->" << _offset->dotID() << ";" << endl;
  }
}

CTacAddr* CAstDesignator::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  // just find symbol and return.
  return new CTacName(GetSymbol());
}


//------------------------------------------------------------------------------
// CAstConstant
//
CAstConstant::CAstConstant(CToken t, const CType *type, long long value)
  : CAstOperand(t), _type(type), _value(value)
{
}

void CAstConstant::SetValue(long long value)
{
  _value = value;
}

long long CAstConstant::GetValue(void) const
{
  return _value;
}

string CAstConstant::GetValueStr(void) const
{
  ostringstream out;

  if (GetType() == CTypeManager::Get()->GetBool()) {
    out << (_value == 0 ? "false" : "true");
  } else {
    out << dec << _value;
  }

  return out.str();
}

bool CAstConstant::TypeCheck(CToken *t, string *msg) const
{
  // Integer type can get value range as (- 2 ^ 31) ~ (2 ^ 31 - 1)
  if (GetType() == CTypeManager::Get()->GetInt()) {
    long long value = GetValue();
    if (value < -2147483648 || value > 2147483647) {
      if (t != NULL) *t = GetToken();
      if (msg != NULL) *msg = "out of integer value range.";
      return false;
    }
  }
  
  return true;
}

const CType* CAstConstant::GetType(void) const
{
  return _type;
}

ostream& CAstConstant::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << GetValueStr() << " ";

  out << endl;

  return out;
}

string CAstConstant::dotAttr(void) const
{
  ostringstream out;
  out << " [label=\"" << GetValueStr() << "\",shape=ellipse]";
  return out.str();
}

CTacAddr* CAstConstant::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  // just find value and return. 
  return new CTacConst(GetValue());
}

