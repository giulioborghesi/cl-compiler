#ifndef COOL_IR_VISITABLE_H
#define COOL_IR_VISITABLE_H

#include <cool/analysis/pass.h>
#include <cool/core/status.h>

#include <utility>

namespace cool {

/// Class that implements the interface used by the visitor pattern
template <typename Base, typename Derived> class Visitable : public Base {
public:
  virtual ~Visitable() = default;

  Status visitNode(Context *context, Pass *pass) final override {
    return pass->visit(context, static_cast<Derived *>(this));
  }

protected:
  template <typename... Args>
  Visitable(Args &&... args) : Base(std::forward<Args>(args)...) {}
};

} // namespace cool

#endif
