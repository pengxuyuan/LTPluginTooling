#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "OCCPlusPlusBridge.h"

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
            std::string str1 = "ExampleMethod";
            void *param = &str1;
            enum CPlusPlusCallOCFunctionActionType actionType = ActionTypeExampleMethod;
            GlobleTargetCallFunction(GlobleOCTargetObserverCPlusPlus, actionType, param);
            
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

    class LTASTConsumer : public ASTConsumer {
    private:
        MatchFinder matcher;
        LTMatchHandle handler;
        
    public:
        LTASTConsumer(CompilerInstance &CI) :handler(CI) {
            matcher.addMatcher(objcPropertyDecl().bind("objcPropertyDecl"), &handler);
        }
        
        void HandleTranslationUnit(ASTContext &context) {
            matcher.matchAST(context);
        }
    };

    class LTASTAction: public ASTFrontendAction {
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef iFile) {
            return unique_ptr<LTASTConsumer> (new LTASTConsumer(CI));
        }
        
        bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> & args) {
            return true;
        }
    };
}

//static FrontendPluginRegistry::Add<LTPlugin::LTASTAction> X("LTPlugin", "The LTPlugin is my first clang-plugin.");

static llvm::cl::OptionCategory OptsCategory("LTPlugin");
int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv, OptsCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    return Tool.run(newFrontendActionFactory<LTPlugin::LTASTAction>().get());
}


