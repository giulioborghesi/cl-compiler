#ifndef COOL_IR_VISITABLE_H
#define COOL_IR_VISITABLE_H

#include <cool/analysis/pass.h>

namespace cool {

/// Class that implements the interface used by the visitor pattern
template <typename Derived> class Visitable {
public:
  bool visitNode(Context *context, Pass *pass) {
    return pass->visit(context, static_cast<Derived *>(this));
  }
};

} // namespace cool

#endif
