#ifndef COOL_FRONTEND_PARSER_H
#define COOL_FRONTEND_PARSER_H

#include <cool/frontend/scanner_state.h>
#include <cool/ir/fwd.h>

#include <string>

namespace cool {

/// Forward declaration
class LoggerCollection;

/// \brief Class that wraps up a Flex-Bison scanner / parser pair
class Parser {

public:
  Parser() = delete;
  Parser(Parser &&other);

  ~Parser() = default;

  /// \brief Return the last error code seen by the scanner / parser
  ///
  /// \return the last error code seen by the scanner / parser pair
  FrontEndErrorCode lastErrorCode() const;

  /// \brief Factory method to create a Parser object from file
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] filePath path to input file
  /// \return a Parser object
  static Parser MakeFromFile(const std::string &filePath);

  /// \brief Factory method to create a Parser object from a string
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] inputString string to parse
  /// \return a Parser object
  static Parser MakeFromString(const std::string &inputString);

  /// \brief Parse the program
  ///
  /// \note parse should be invoked only once. On successive invocations, parse
  /// will return nullptr
  ///
  /// \return a pointer to the ProgramNode if successful, nullptr otherwise
  ProgramNodePtr parse();

  /// \brief Register a collection of loggers with the scanner / parser
  ///
  /// \note This call will replace the previously registered collection of
  /// loggers with the new one
  ///
  /// \param[in] loggers loggers collection
  void registerLoggers(std::shared_ptr<LoggerCollection> loggers);

private:
  Parser(std::unique_ptr<ScannerState> state);

  std::unique_ptr<ScannerState> state_;
  std::shared_ptr<LoggerCollection> loggers_ = nullptr;
  bool parseComplete_ = false;
};

} // namespace cool

#endif
