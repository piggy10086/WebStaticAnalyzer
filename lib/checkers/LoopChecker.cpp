#include "checkers/LoopChecker.h"
#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/Analysis/AnalysisDeclContext.h"

namespace
{

    static inline void printStmt(const Stmt *stmt, const SourceManager &sm, string info = "")
    { //Originate for ZeroChecker
        string begin = stmt->getBeginLoc().printToString(sm);
        cout << begin << endl;
        LangOptions LangOpts;
        LangOpts.CPlusPlus = true;
        stmt->printPretty(outs(), nullptr, LangOpts, 5);
        cout << " " << info;
        cout << endl;
    }

    class LoopVisitor : public RecursiveASTVisitor<LoopVisitor>
    {
    private:
        struct DefectInfo
        {
            Stmt *Statement;
            std::string info;
        };

        vector<DefectInfo> stmts;
        const ASTContext &CTX;
        // const FunctionDecl *funDecl;

        //Source Manager
        const SourceManager &sm;
        clang::LiveVariables * LivenessResult;

        bool check_Expresion(Stmt *stmt)
        {
            Expr *conditionExpr = nullptr;
            VarDecl* CondDecl = nullptr;
            // bool CondLiveness = false;

            if (ForStmt::classof(stmt))
            {
                conditionExpr = reinterpret_cast<ForStmt *>(stmt)->getCond();
                // CondDecl = reinterpret_cast<ForStmt *>(stmt)->getConditionVariable();
            }
            else if (WhileStmt::classof(stmt))
            {
                conditionExpr = reinterpret_cast<WhileStmt *>(stmt)->getCond();
                // CondDecl = reinterpret_cast<WhileStmt *>(stmt)->getConditionVariable();
            }
            else
            {
                std::cout << "Something Wrong with check Expression!" << endl;
                return false;
            }

            // if (CondDecl)
            // {
            //     CondLiveness = LivenessResult->isLive(stmt, CondDecl);
            //     printStmt(stmt,sm,"Successfully get Condition Declaration!");
            // }
            // else
            // {
            //     printStmt(stmt, sm, "Cannot get Condition Declaration!!");
            // }

            string Defect_Description("None");

            if (conditionExpr != nullptr)
            {
                printStmt(conditionExpr, sm);
                llvm::APSInt Result;
                //檢查表達式中是否值恆為常數
                if (conditionExpr->isIntegerConstantExpr(Result, CTX)) //Check wheather the Expression can be fold into a integer
                {
                    if (Result != 0) //if true and the Expression result is always not Zero, then we find an Infinety Loop
                    {
                        Defect_Description = "循環條件恆爲" + Result.toString(10);
                    }
                }
                else //conditionExpr is a Varient
                {
                    printStmt(conditionExpr, sm, "Condition cannot fold into Integer!");
                    // cout<<"Condition Liveness : "<<CondLiveness<<endl;
                }
            }
            else
            { //No Condition Expression in the loop
                Defect_Description = "循環缺乏跳出條件";
                printStmt(stmt, sm, Defect_Description);
                
            }

            if(Defect_Description!="None")
                stmts.push_back({stmt,                 //defect Statement
                                 Defect_Description}); //Defect Info
            
            return true;
        }

    public:
        LoopVisitor(const ASTContext &ctx,const SourceManager &sm,clang::LiveVariables *liveness)
            : CTX(ctx), sm(sm), LivenessResult(liveness){}

        const vector<DefectInfo> &getStmts() const { return stmts; }

        bool VisitWhileStmt(WhileStmt *stmt) //when find while program point enter this function
        {
            bool ExprResult = check_Expresion(stmt);
            // bool CFGResult = check_CFG();
            // return ExprResult && CFGResult;

            // auto CondDel = stmt->getConditionVariableDeclStmt();
            // cout << LivenessResult->isLive(stmt,CondDel) <<endl<<endl;

            return ExprResult;
        }

        bool VisitForStmt(ForStmt *stmt) //when find for program point enter this function
        {
            bool ExprResult = check_Expresion(stmt);
            auto LoopInc = stmt->getInc();
            if(!LoopInc)
            {
                    stmts.push_back({stmt,"缺少控制變量變化條件"});
            }
            // bool CFGResult = check_CFG();
            // return ExprResult && CFGResult;
            // auto CondDel = stmt->getConditionVariableDeclStmt();
            // cout << LivenessResult->isLive(stmt,CondDel) <<endl<<endl;
            return ExprResult;
        }
    };

} //namespace

bool LoopChecker::check_CFG(std::unique_ptr<clang::CFG> &cfg,const ASTContext& ctx,const FunctionDecl *funDecl)
{
    if (!cfg)
        return false;

    // LangOptions LangOpts;
    // LangOpts.CPlusPlus = true;
    // CFG->print(outs(),LangOpts,5 );

    //TODO: Need to check  whether CFGLoopExit Block exsit in CFG
    // for (auto it : cfg)
    // {
    //     it;
    // }

    //TODO:Data FLow Analyze

    //TODO get Loop Control Variables in result
    // result->dumpStmtLiveness(sm);
    // result->isLive()

        //Avalible Expression Analyze ?
    return true;
}

void LoopChecker::getEntryFunc()
{
    std::vector<ASTFunction *> topLevelFuncs = call_graph->getTopLevelFunctions();
    for (auto fun : topLevelFuncs)
    {
        const FunctionDecl *funDecl = manager->getFunctionDecl(fun);
        if (funDecl->isMain())
        {
            entryFunc = fun;
            return;
        }
    }
    entryFunc = nullptr;
    return;
}

std::vector<Defect> LoopChecker::check()
{
    std::vector<ASTFunction *> functions = resource->getFunctions();
    std::vector<Defect> defects;

    for (auto func : functions)
    {
        const FunctionDecl *funDecl = manager->getFunctionDecl(func); //get function declaration
        auto stmts = funDecl->getBody();                               //get body of the function through getbody() -> Clang::Stmt type
        const ASTContext &ASTContext = funDecl->getASTContext();
        auto &sm = ASTContext.getSourceManager();

        auto &CurrentFuncCFG = manager->getCFG(func);


        //Get Liveness Result
        clang::AnalysisDeclContextManager AM(const_cast<clang::ASTContext &>(ASTContext));
        auto AnalysisDeclContext = AM.getContext(funDecl);
        auto result = LiveVariables::computeLiveness(*AnalysisDeclContext, false);

        // result->dumpStmtLiveness(sm);

        //Traverse AST
        LoopVisitor vistor(ASTContext, sm,result);
        vistor.TraverseStmt(stmts);

        // check_CFG(CurrentFuncCFG,ASTContext,funDecl);



        auto DefectList = vistor.getStmts();

        for (auto &&DefectStmt : DefectList)
        {
            Defect d;
            d.location = DefectStmt.Statement->getBeginLoc().printToString(sm);
            d.info = "存在可能的死循環 (" + DefectStmt.info + ")";
            defects.push_back(d);
        }
    }

    return defects;
}
