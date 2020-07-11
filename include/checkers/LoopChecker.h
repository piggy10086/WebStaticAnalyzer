#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <stack>
#include <string>

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Attr.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Analysis/CFG.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>


#include "framework/BasicChecker.h"
#include "clang/Analysis/AnalysisDeclContext.h" //Use AnalysisDeclContext to BuildCFG

using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

class LoopChecker : public BasicChecker {
public:
  LoopChecker(ASTResource *resource, ASTManager *manager,
              CallGraph *call_graph, Config *configure)
      : BasicChecker(resource, manager, call_graph, configure) {};
  void check() override;

private:
  void getEntryFunc();
  ASTFunction *entryFunc;
  // bool check_CFG(std::unique_ptr<clang::CFG> &,const ASTContext&,const FunctionDecl *);
};
