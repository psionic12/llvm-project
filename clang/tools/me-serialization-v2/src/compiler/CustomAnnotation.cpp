#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/Attributes.h"

#include "DiagMsg.h"

namespace clang {
namespace s11n {
struct S11nAttrInfo : public ParsedAttrInfo {
  S11nAttrInfo() {
    // variadic parameters
    OptArgs = 15;
    static constexpr Spelling S[] = {
        {ParsedAttr::AS_GNU, "s11n"},
        {ParsedAttr::AS_CXX11, "s11n"},
    };
    Spellings = S;
  }

  bool diagAppertainsToDecl(Sema &S, const ParsedAttr &Attr,
                            const Decl *D) const override {
    auto &Diags = S.getDiagnostics();
    // This attribute appertains to fields only.
    const auto *FD = dyn_cast_or_null<const FieldDecl>(D);
    if (!FD) {
      S.Diag(Attr.getLoc(),
             Diags.getCustomDiagID(DiagnosticsEngine::Error,
                                   s11n::warn_attribute_wrong_decl_type_str))
          << "must attach to fields";
      return false;
    }
    return true;
  }

  AttrHandling handleDeclAttribute(Sema &S, Decl *D,
                                   const ParsedAttr &Attr) const override {
    return AttributeApplied;
  }
};
static ParsedAttrInfoRegistry::Add<S11nAttrInfo> X("s11n_attr", "attribute used for serialization");
} // namespace s11n
} // namespace clang