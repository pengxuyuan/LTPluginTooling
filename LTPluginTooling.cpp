#include <cstdio>
#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"

//深度优先遍历 AST 和访问节点的类
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "OCCPlusPlusBridge.h"

#include "clang/Rewrite/Core/Rewriter.h"

#include "clang/Sema/Sema.h"

//#include "LTPluginTooling.hpp"

void *GlobleOCTargetObserverCPlusPlus = NULL;
CPlusPlusCallOCFunction GlobleTargetCallFunction = NULL;

using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static void CPlusPlusLog(string logMessage) {
    cout<<"--------- "<<logMessage<<" ---------" <<endl;
}



namespace LTPlugin {
    class LTMatchHandle : public MatchFinder :: MatchCallback {
    private:
        CompilerInstance &CI;
        
        // 是否是用户源码
        bool isUserSourceCode(const string filename) {
//            cout<<"--------- filename:"<<filename<<" ---------"<<endl;
            
            if (filename.empty()) return false;
            
            // 非Xcode中的源码都认为是用户源码
            if (filename.find("/Applications/Xcode.app/") == 0) return false;
            
            cout<<"--------- 用户文件:"<<filename<<" ---------"<<endl;
            
            return true;
        }
        
        // 是否应该使用 Copy 修饰
        bool isShouldUseCopy(const string typeStr) {
            if (typeStr.find("NSString") != string::npos ||
                typeStr.find("NSArray") != string::npos ||
                typeStr.find("NSDictionary") != string::npos) {
                return true;
            }
            
            return false;
        }
        
    public:
        LTMatchHandle(CompilerInstance &CI) : CI(CI){}
        
        void run(const MatchFinder::MatchResult &Result) {
            //C++ 即将调用 OC 代码
//            std::string str1 = "ExampleMethod";
//            void *param = &str1;
//            enum CPlusPlusCallOCFunctionActionType actionType = ActionTypeExampleMethod;
//            GlobleTargetCallFunction(GlobleOCTargetObserverCPlusPlus, actionType, param);
            
            const ObjCPropertyDecl *propertyDecl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("objcPropertyDecl");
            if (propertyDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(propertyDecl->getSourceRange().getBegin()).str())) {
                ObjCPropertyAttribute::Kind attrKind = propertyDecl->getPropertyAttributes();
                string typeStr = propertyDecl->getType().getAsString();
                
                if (propertyDecl->getTypeSourceInfo() &&
                    isShouldUseCopy(typeStr) &&
                    !(attrKind & ObjCPropertyAttribute::kind_copy)) {
                    cout<<"--------- "<<typeStr<<": 不是使用的 copy 修饰--------"<<endl;
                    DiagnosticsEngine &diag = CI.getDiagnostics();
                    diag.Report(propertyDecl->getBeginLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "--------- %0 不是使用的 copy 修饰--------")) << typeStr;
                }
            }
        }
    };

    //继承 RecursiveASTVisitor，用来访问 AST 方法
    class LTClangAutoStatsVisitor : public RecursiveASTVisitor<LTClangAutoStatsVisitor> {
    public:
        clang::Rewriter TheRewriter;
        std::string CodeSnippet;
        
        explicit LTClangAutoStatsVisitor(ASTContext *Ctx) {
            CodeSnippet = "AAAAAA";
        }

        bool VisitObjCImplementationDecl(ObjCImplementationDecl *ID) {
            CPlusPlusLog("LTClangAutoStatsVisitor VisitObjCImplementationDecl 函数调用开始");
            for (auto D : ID->decls()) {
                if (ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
                    handleObjcMethDecl(MD);
                }
            }
            return true;
        }

        bool VisitCXXRecordDecl(CXXRecordDecl *D) {
            CPlusPlusLog("LTClangAutoStatsVisitor VisitCXXRecordDecl 函数调用开始");
            cout<<"--------- VisitCXXRecordDecl 函数名称:"<<D->getNameAsString()<<" ---------"<<endl;
//            std::cout << D->getNameAsString() << std::endl;
            fprintf(stderr, "VisitCXXRecordDecl!\n");
            // 只关心提供了定义的函数（忽略只有声明而没有在源代码中出现定义的函数）
//            if (D->isThisDeclarationADefinition()) {
//                fc++;
//                funcs.push_back(D->getNameAsString());// 获取函数名并保存到 funcs
//                funcs.push_back(<#const_reference __x#>)
//            }

            
            return true;
        }
        
        bool handleObjcMethDecl(ObjCMethodDecl *MD) {
//            return true;
            if (!MD->hasBody()) return true;
            errs() << MD->getNameAsString() << "\n";
            
//            CompoundStmt *cmpdStmt = MD->getCompoundBody();
//            SourceLocation loc = cmpdStmt->getBeginLoc().getLocWithOffset(1);
//            if (loc.isMacroID()) {
////                TheRewriter.getSourceMgr().getImmediateExpansionRange(loc)
////                loc = TheRewriter.getSourceMgr().getImmediateExpansionRange(loc).first;
//            }
//
//            static std::string varName("%__FUNCNAME__%");
//            std::string funcName = MD->getDeclName().getAsString();
//            std::string codes(CodeSnippet);
//            size_t pos = 0;
//            while ((pos = codes.find(varName, pos)) != std::string::npos) {
//                codes.replace(pos, varName.length(), funcName);
//                pos += funcName.length();
//            }
//            TheRewriter.InsertTextBefore(loc, codes);
            
            return true;
        }
    };

    // AST Consumer 提供了访问抽象语法树的接口。
    class LTASTConsumer : public ASTConsumer {
    private:
        MatchFinder matcher;
        LTMatchHandle handler;
        // A RecursiveASTVisitor implementation.
//        FindNamedClassVisitor Visitor;
        
        LTClangAutoStatsVisitor Visitor;
        CompilerInstance *CI;
        
        ////PrintFunctionNames Use
        CompilerInstance &Instance;
        std::set<std::string> ParsedTemplates;
        
    public:
        int fc; // 用于统计函数个数
        std::vector<FunctionDecl*> funcs; // 用于记录已经遍历的函数
        
        //第一次：实现属性 strong copy 检查
//        LTASTConsumer(CompilerInstance &CI) :handler(CI) {
//            matcher.addMatcher(objcPropertyDecl().bind("objcPropertyDecl"), &handler);
//        }
        
//        explicit LTASTConsumer(CompilerInstance &aCI) : Visitor(&(aCI.getASTContext())), CI(aCI), handler(*CI) {
//
//        }
        
        explicit LTASTConsumer(CompilerInstance *aCI) : Visitor(&(aCI->getASTContext())) ,CI(aCI)
                                                                                        ,handler(*aCI)
                                                                                        ,Instance(Instance)
                                                                                        ,ParsedTemplates(ParsedTemplates){
              CPlusPlusLog("LTASTConsumer 构造函数调用");
//            matcher.addMatcher(objcPropertyDecl().bind("objcPropertyDecl"), &handler);
        }
        
        //析构函数
//        ~LTASTConsumer() {
//            std::cout << "I have seen " << fc << " functions. \ They are: " <<endl;
//            for (unsigned i=0; i<funcs.size(); i++)
//            {
//                std::cout << funcs[i] << endl;
//            }
//        }
        
        bool HandleTopLevelDecl(DeclGroupRef DG) override {
            CPlusPlusLog("LTASTConsumer HandleTopLevelDecl 函数调用开始");
            for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i) {
                const Decl *D = *i;
                if (const NamedDecl *ND = dyn_cast<NamedDecl>(D))
                    llvm::errs() << "top-level-decl: \"" << ND->getNameAsString() << "\"\n";
            }
            CPlusPlusLog("LTASTConsumer HandleTopLevelDecl 函数调用完毕");
            cout<<""<<endl;
            return true;
        }
        
        // AST 入口函数：当整个抽象语法树 (AST) 构造完成以后，HandleTranslationUnit 这个函数将会被 Clang 的驱动程序调用。
        //ASTContext是编译实例保存所有AST信息的一种数据结构，主要包括编译期间的符号表和AST原始形式。在遍历AST的时候，需要从该数据结构中提取节点的相关信息
//        void HandleTranslationUnit(ASTContext &context) {
//            matcher.matchAST(context);
//            fprintf(stderr, "Welcome to BoostCon!\n");
//            TranslationUnitDecl *decl = context.getTranslationUnitDecl();
////            Visitor.TraverseDecl(decl);
//            Visitor.TraverseTranslationUnitDecl(decl);
//
//            fc = 0;
//        }
        
        //打印函数名称
        void HandleTranslationUnit(ASTContext& context) override {
            CPlusPlusLog("LTASTConsumer HandleTranslationUnit 函数调用");
            CPlusPlusLog("转发到 LTClangAutoStatsVisitor 中遍历");
            //转发到 LTClangAutoStatsVisitor 中遍历
            TranslationUnitDecl *decl = context.getTranslationUnitDecl();
            Visitor.TraverseDecl(decl);
//            Visitor.TraverseTranslationUnitDecl(decl);
            return;
            if (!Instance.getLangOpts().DelayedTemplateParsing)
                return;

          // This demonstrates how to force instantiation of some templates in
          // -fdelayed-template-parsing mode. (Note: Doing this unconditionally for
          // all templates is similar to not using -fdelayed-template-parsig in the
          // first place.)
          // The advantage of doing this in HandleTranslationUnit() is that all
          // codegen (when using -add-plugin) is completely finished and this can't
          // affect the compiler output.
          struct Visitor : public RecursiveASTVisitor<Visitor> {
            const std::set<std::string> &ParsedTemplates;
            Visitor(const std::set<std::string> &ParsedTemplates)
                : ParsedTemplates(ParsedTemplates) {}
            bool VisitFunctionDecl(FunctionDecl *FD) {
              if (FD->isLateTemplateParsed() &&
                  ParsedTemplates.count(FD->getNameAsString()))
                LateParsedDecls.insert(FD);
              return true;
            }

            std::set<FunctionDecl*> LateParsedDecls;
          } v(ParsedTemplates);
          v.TraverseDecl(context.getTranslationUnitDecl());
          clang::Sema &sema = Instance.getSema();
          for (const FunctionDecl *FD : v.LateParsedDecls) {
            clang::LateParsedTemplate &LPT =
                *sema.LateParsedTemplateMap.find(FD)->second;
            sema.LateTemplateParser(sema.OpaqueParser, LPT);
            llvm::errs() << "late-parsed-decl: \"" << FD->getNameAsString() << "\"\n";
          }
        }
    
    };

    // 用来为前端工具定义标准化的AST操作流程的
    /*该类与编译实例打交道，词法分析、语法分析等过程都被编译实例隐藏了，编译实例会触发FrontEndAction定义好的方法，并把编译过程中的详细信息都告诉它。如编译哪个文件，编译参数等等。*/
    class LTASTAction: public ASTFrontendAction {
        //PrintFunctionNames Use
        std::set<std::string> ParsedTemplates;
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef iFile) {
            CPlusPlusLog("LTASTAction CreateASTConsumer 函数调用,返回一个 ASTConsumer 对象");
            return unique_ptr<LTASTConsumer> (new LTASTConsumer(&CI));
        }
        
        //猜测：解析入参
        bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> & args) {
            CPlusPlusLog("LTASTAction ParseArgs 函数调用,猜测：解析入参");
            for (unsigned i = 0, e = args.size(); i != e; ++i) {
              llvm::errs() << "PrintFunctionNames arg = " << args[i] << "\n";

              // Example error handling.
              DiagnosticsEngine &D = CI.getDiagnostics();
              if (args[i] == "-an-error") {
                unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error,
                                                    "invalid argument '%0'");
                D.Report(DiagID) << args[i];
                return false;
              } else if (args[i] == "-parse-template") {
                if (i + 1 >= e) {
                  D.Report(D.getCustomDiagID(DiagnosticsEngine::Error,
                                             "missing -parse-template argument"));
                  return false;
                }
                ++i;
                ParsedTemplates.insert(args[i]);
              }
            }
            if (!args.empty() && args[0] == "help")
              PrintHelp(llvm::errs());
            
            return true;
        }
        
        void PrintHelp(llvm::raw_ostream& ros) {
          ros << "Help for PrintFunctionNames plugin goes here\n";
        }
    };


}

// 定义 Clang 插件写法
//static FrontendPluginRegistry::Add<LTPlugin::LTASTAction> X("LTPlugin", "The LTPlugin is my first clang-plugin.");

// 定义 ClangTooling 写法
// 创建了一个参数解析器op，用来处理工具的传入参数，创建所需的CompilationDatabase以及文件列表
static llvm::cl::OptionCategory OptsCategory("LTPlugin");
int main(int argc, const char **argv) {
    CPlusPlusLog("Main 入口函数调用，指定对应的 ASTFrontendAction 对象");
    CommonOptionsParser op(argc, argv, OptsCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    return Tool.run(newFrontendActionFactory<LTPlugin::LTASTAction>().get());
}




