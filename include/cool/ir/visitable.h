#ifndef COOL_IR_VISITABLE_H
#define COOL_IR_VISITABLE_H

#include <cool/analysis/pass.h>
#include <cool/core/status.h>

namespace cool {

class VisitableBase {

public:
  virtual Status visitNode(Context *context, Pass *pass) = 0;
};

/// Class that implements the interface used by the visitor pattern
template <typename Derived> class Visitable : public VisitableBase {
public:
  Status visitNode(Context *context, Pass *pass) final override {
    return pass->visit(context, static_cast<Derived *>(this));
  }
};

} // namespace cool

#endif
