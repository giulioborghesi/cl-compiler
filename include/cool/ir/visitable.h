#ifndef COOL_IR_VISITABLE_H
#define COOL_IR_VISITABLE_H

#include <cool/analysis/pass.h>
#include <cool/codegen/codegen_base.h>
#include <cool/core/status.h>

#include <iostream>
#include <utility>

namespace cool {

/// Class that implements the interface used by the visitor pattern
template <typename Base, typename Derived> class Visitable : public Base {
public:
  virtual ~Visitable() = default;

  Status visitNode(AnalysisContext *context, Pass *pass) final override {
    return pass->visit(context, static_cast<Derived *>(this));
  }

  Status generateCode(CodegenContext *context, CodegenBasePass *pass,
                      std::iostream *ios) final override {
    return pass->codegen(context, static_cast<Derived *>(this), ios);
  }

protected:
  template <typename... Args>
  Visitable(Args &&... args) : Base(std::forward<Args>(args)...) {}
};

} // namespace cool

#endif
