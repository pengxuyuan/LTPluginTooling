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

//#include "LTPluginTooling.hpp"

void *GlobleOCTargetObserverCPlusPlus = NULL;
CPlusPlusCallOCFunction GlobleTargetCallFunction = NULL;

using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;
using namespace clang::tooling;

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
            for (auto D : ID->decls()) {
                if (ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
                    handleObjcMethDecl(MD);
                }
            }
            return true;
        }

        bool handleObjcMethDecl(ObjCMethodDecl *MD) {
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
    
        bool VisitCXXRecordDecl(CXXRecordDecl *D) {
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
        
        explicit LTASTConsumer(CompilerInstance *aCI) : Visitor(&(aCI->getASTContext())), CI(aCI), handler(*aCI) {
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
        
        // AST 入口函数：当整个抽象语法树 (AST) 构造完成以后，HandleTranslationUnit 这个函数将会被 Clang 的驱动程序调用。
        void HandleTranslationUnit(ASTContext &context) {
            matcher.matchAST(context);
            fprintf(stderr, "Welcome to BoostCon!\n");
            TranslationUnitDecl *decl = context.getTranslationUnitDecl();
//            Visitor.TraverseDecl(decl);
            Visitor.TraverseTranslationUnitDecl(decl);
            
            fc = 0;
        }
    
    };

    // 用来为前端工具定义标准化的AST操作流程的
    class LTASTAction: public ASTFrontendAction {
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef iFile) {
            return unique_ptr<LTASTConsumer> (new LTASTConsumer(&CI));
        }
        
        bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> & args) {
            return true;
        }
        
//        void EndSourceFileAction() override {
//          size_t pos = filePath.find_last_of(".");
//          if (pos != std::string::npos) {
//              ClasFilePath = filePath + ".clas";
//          }
//          std::ofstream clasFile(ClasFilePath);
//          assert(clasFile.is_open());
//          FileID fid = getCompilerInstance().getSourceManager(). getMainFileID();
//          RewriteBuffer &buffer = LogRewriter.getEditBuffer(fid);
//          RewriteBuffer::iterator I = buffer.begin();
//          RewriteBuffer::iterator E = buffer.end();
//          for (; I != E; I.MoveToNextPiece()) {
//              (clasFile << I.piece().str());
//          }
//          clasFile.flush();
//          clasFile.close();
//        }
        
    };


}

//static FrontendPluginRegistry::Add<LTPlugin::LTASTAction> X("LTPlugin", "The LTPlugin is my first clang-plugin.");

// 创建了一个参数解析器op，用来处理工具的传入参数，创建所需的CompilationDatabase以及文件列表
static llvm::cl::OptionCategory OptsCategory("LTPlugin");
int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv, OptsCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    return Tool.run(newFrontendActionFactory<LTPlugin::LTASTAction>().get());
}


