#include <cool/analysis/analysis_context.h>
#include <cool/analysis/classes_definition.h>
#include <cool/analysis/classes_implementation.h>
#include <cool/analysis/type_check.h>
#include <cool/codegen/codegen_code.h>
#include <cool/codegen/codegen_constants.h>
#include <cool/codegen/codegen_context.h>
#include <cool/codegen/codegen_tables.h>
#include <cool/core/class_registry.h>
#include <cool/core/logger.h>
#include <cool/frontend/parser.h>
#include <cool/ir/class.h>

#include <experimental/filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

using namespace cool;

namespace {

/// Error codes
constexpr static const int32_t INVALID_NUMBER_OF_PARAMETERS = -1;
constexpr static const int32_t INPUT_FILE_DOES_NOT_EXIST = -2;
constexpr static const int32_t PARSER_ERROR = -3;
constexpr static const int32_t SEMANTIC_ANALYSIS_ERROR = -4;

/// \brief Helper function to create a logger to stdout
///
/// \return a logger that logs messages to stdout
std::shared_ptr<Logger> CreateStdoutLogger() {
  auto kSeverity = LogMessageSeverity::WARNING;
  return std::make_shared<Logger>(new StdoutSink(), kSeverity);
}

void DoCodegen(ProgramNodePtr node, std::shared_ptr<ClassRegistry> registry) {
  /// Create a codegen context
  auto context = std::make_unique<CodegenContext>(registry);

  /// Initialize passes
  std::vector<std::shared_ptr<CodegenBasePass>> passes = {
      std::make_shared<CodegenConstantsPass>(),
      std::make_shared<CodegenTablesPass>(),
      std::make_shared<CodegenObjectsInitPass>()};

  /// Run passes
  for (auto pass : passes) {
    auto status = pass->codegen(context.get(), node.get(), &std::cout);
    assert(status.isOk());
  }
}

/// \brief Helper function to run the semantic analysis phase
///
/// \param[in] node program node
/// \param[in] registry class registry
/// \param[in] loggers loggers collection
/// \return Status::Ok() is successful, an error message otherwise
Status DoSemanticAnalysis(ProgramNodePtr node,
                          std::shared_ptr<ClassRegistry> registry,
                          std::shared_ptr<LoggerCollection> loggers) {
  /// Create an analysis context
  auto context = std::make_unique<AnalysisContext>(registry, loggers);

  /// Initialize passes
  std::vector<std::shared_ptr<Pass>> passes = {
      std::make_shared<ClassesDefinitionPass>(),
      std::make_shared<ClassesImplementationPass>(),
      std::make_shared<TypeCheckPass>()};

  /// Run passes
  for (auto pass : passes) {
    auto status = pass->visit(context.get(), node.get());
    if (!status.isOk()) {
      std::cout << status.getErrorMessage() << std::endl;
      return status;
    }
  }
  return Status::Ok();
}

} // namespace

int main(int argc, char *argv[]) {
  /// Program expects exactly one argument
  if (argc != 2) {
    std::cerr << "Error: program takes exactly one parameter (filename)"
              << std::endl;
    return INVALID_NUMBER_OF_PARAMETERS;
  }

  /// Store filename into string variable. Ensure file exists
  const std::string fileName = argv[1];
  if (!std::experimental::filesystem::exists(fileName)) {
    std::cerr << "Error: file not found" << std::endl;
    return INPUT_FILE_DOES_NOT_EXIST;
  }

  /// Create loggers
  auto loggers = std::make_shared<LoggerCollection>();
  loggers->registerLogger("default", CreateStdoutLogger());

  /// Create scanner / parser and parse program
  auto parser = Parser::MakeFromFile(fileName);
  parser.registerLoggers(loggers);
  auto programNode = parser.parse();
  if (parser.lastErrorCode() != FrontEndErrorCode::NO_ERROR) {
    std::cerr << "Error: parsing did not succeed" << std::endl;
    return PARSER_ERROR;
  }

  /// Set the program file name and create the class registry
  programNode->setFileName(fileName);
  auto registry = std::make_shared<ClassRegistry>();

  /// Perform semantic analysis
  auto semanticStatus = DoSemanticAnalysis(programNode, registry, loggers);
  if (!semanticStatus.isOk()) {
    std::cerr << "Error: semantic analysis failed" << std::endl;
    return SEMANTIC_ANALYSIS_ERROR;
  }

  /// Generate code
  DoCodegen(programNode, registry);
  return 0;
}
