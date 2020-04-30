#ifndef COOL_ANALYSIS_CLASSES_IMPLEMENTATION_H
#define COOL_ANALYSIS_CLASSES_IMPLEMENTATION_H

#include <cool/analysis/pass.h>

namespace cool {

class ClassesImplementationPass : public Pass {

public:
  ClassesImplementationPass() = default;
  ~ClassesImplementationPass() final override = default;

  Status visit(Context *context, AttributeNode *node) final override;

  Status visit(Context *context, ClassNode *node) final override;

  Status visit(Context *context, MethodNode *node) final override;

  Status visit(Context *context, ProgramNode *node) final override;
};

} // namespace cool

#endif
