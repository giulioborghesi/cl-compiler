#ifndef COOL_ANALYSIS_INHERITANCE_CHECK_H
#define COOL_ANALYSIS_INHERITANCE_CHECK_H

#include <cool/analysis/pass.h>

namespace cool {

class InheritanceCheckPass : public Pass {

public:
  ~InheritanceCheckPass() final override = default;
  bool visit(Context *context, ProgramNode *node) final override;
};

} // namespace cool

#endif
