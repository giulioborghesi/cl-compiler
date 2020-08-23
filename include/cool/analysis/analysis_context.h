#ifndef COOL_ANALYSIS_CONTEXT_H
#define COOL_ANALYSIS_CONTEXT_H

#include <cool/analysis/method_record.h>
#include <cool/core/context.h>
#include <cool/core/symbol_table.h>

namespace cool {

class AnalysisContext : public Context<SymbolTable<std::string, ExprType>,
                                       SymbolTable<std::string, MethodRecord>> {

public:
  AnalysisContext() = delete;
  explicit AnalysisContext(ClassRegistry *classRegistry)
      : Context(classRegistry) {}

  AnalysisContext(ClassRegistry *classRegistry,
                  std::shared_ptr<LoggerCollection> logger)
      : Context(classRegistry, logger) {}
};

} // namespace cool

#endif
