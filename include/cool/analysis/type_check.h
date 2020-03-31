#ifndef COOL_ANALYSIS_TYPE_CHECK_H
#define COOL_ANALYSIS_TYPE_CHECK_H

#include <cool/analysis/pass.h>
#include <cool/ir/fwd.h>

#include <cstdlib>
#include <string>

namespace cool {

/// Forward declarations
class Context;

class TypeCheckPass : public Pass {

public:
  TypeCheckPass() = default;
  ~TypeCheckPass() final override = default;

  Status visit(Context *context, BinaryExprNode *node) final override;

  Status visit(Context *context, BooleanExprNode *node) final override;

  Status visit(Context *context, IfExprNode *node) final override;

  Status visit(Context *context, NewExprNode *node) final override;
};

} // namespace cool

#endif
