#include <cool/analysis/inheritance_check.h>

#include <iostream>
#include <string>

namespace cool {

bool InheritanceCheckPass::visit(Context *context, ProgramNode *node) {
  auto &classRegistry = context->getClassRegistry();

  /// Add classes to class registry
  bool success = true;
  for (auto class : node->classes()) {
    const bool addSuccess = classRegistry.addClass(class.get());
    success = success && addSuccess;
  }

  /// Validate inheritance tree
  const bool inheritanceSuccess = registry.validateInheritanceTree();
  return success && inheritanceSuccess;
}

} // namespace cool