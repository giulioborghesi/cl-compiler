#ifndef COOL_ANALYSIS_CLASSES_DEFINITION_PASS_H
#define COOL_ANALYSIS_CLASSES_DEFINITION_PASS_H

#include <cool/analysis/pass.h>

namespace cool {

class ClassesDefinitionPass : public Pass {

public:
  ClassesDefinitionPass() = default;
  ~ClassesDefinitionPass() final override = default;

  Status visit(Context *context, AttributeNode *node) final override;

  Status visit(Context *context, ClassNode *node) final override;

  Status visit(Context *context, ProgramNode *node) final override;
};

} // namespace cool

#endif
