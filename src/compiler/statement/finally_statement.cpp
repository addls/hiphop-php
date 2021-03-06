/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <compiler/statement/finally_statement.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/function_scope.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// constructors/destructors

FinallyStatement::FinallyStatement
(STATEMENT_CONSTRUCTOR_PARAMETERS,
 StatementPtr finallyStmt)
  : Statement(STATEMENT_CONSTRUCTOR_PARAMETER_VALUES(FinallyStatement)),
    m_stmt(finallyStmt) {
}

StatementPtr FinallyStatement::clone() {
  FinallyStatementPtr stmt(new FinallyStatement(*this));
  stmt->m_stmt = Clone(m_stmt);
  return stmt;
}

int FinallyStatement::getRecursiveCount() const {
  return (m_stmt ? m_stmt->getRecursiveCount() : 0);
}

///////////////////////////////////////////////////////////////////////////////
// parser functions

///////////////////////////////////////////////////////////////////////////////
// static analysis functions

void FinallyStatement::analyzeProgram(AnalysisResultPtr ar) {
  if (m_stmt) m_stmt->analyzeProgram(ar);
  if (ar->getPhase() == AnalysisResult::AnalyzeAll) {
    FunctionScopeRawPtr fs = getFunctionScope();
    if (fs) fs->setHasTry();
  }
}

bool FinallyStatement::hasDecl() const {
  return (m_stmt && m_stmt->hasDecl());
}

bool FinallyStatement::hasRetExp() const {
  return (m_stmt && m_stmt->hasRetExp());
}

ConstructPtr FinallyStatement::getNthKid(int n) const {
  switch (n) {
    case 0:
      return m_stmt;
    default:
      ASSERT(false);
      break;
  }
  return ConstructPtr();
}

int FinallyStatement::getKidCount() const {
  return 1;
}

void FinallyStatement::setNthKid(int n, ConstructPtr cp) {
  switch (n) {
    case 0:
      m_stmt = boost::dynamic_pointer_cast<Statement>(cp);
      break;
    default:
      ASSERT(false);
      break;
  }
}

void FinallyStatement::inferTypes(AnalysisResultPtr ar) {
  if (m_stmt) m_stmt->inferTypes(ar);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

void FinallyStatement::outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_indentBegin("/*finally here we go {\n");
  if (m_stmt) m_stmt->outputPHP(cg, ar);
  cg_indentEnd("}");
  cg_printf("\n*/");
}

void FinallyStatement::outputCPPImpl(CodeGenerator &cg, AnalysisResultPtr ar) {
  cg_indentBegin("try {\n");
  if (m_stmt) {
    m_stmt->outputCPP(cg, ar);
  }
  cg_indentEnd("}");
  cg_indentBegin(" catch (Object e) {\n");
  cg_indentBegin("if (has_exception) {\n");
  cg_printf("static_cast<c_Exception*>(e.get())->t_setpreviouschain(exception);\n");
  cg_indentEnd("}\n");
  cg_printf("exception = e;\n");
  cg_printf("has_exception = true;\n");
  cg_indentEnd("}\n");
}
