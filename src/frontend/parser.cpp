#include <cool/core/logger_collection.h>
#include <cool/frontend/parser.h>

#include <iostream>

namespace cool {

Parser::Parser(std::unique_ptr<ScannerState> state)
    : state_(std::move(state)) {}

Parser::Parser(Parser &&other) {
  this->state_ = std::move(other.state_);
  this->parseComplete_ = other.parseComplete_;
}

FrontEndErrorCode Parser::lastErrorCode() const {
  return state_->lastErrorCode();
}

ProgramNodePtr Parser::parse() {
  /// Cannot parse twice
  if (parseComplete_) {
    return nullptr;
  }

  /// Parse program
  parseComplete_ = true;
  ProgramNodePtr parseResult;
  auto status = yyparse(loggers_.get(), state_->scannerState(), &parseResult);

  /// Return nullptr if parsing failed, otherwise the parsed program node
  if (status != 0) {
    return nullptr;
  }
  return parseResult;
}

void Parser::registerLoggers(std::shared_ptr<LoggerCollection> loggers) {
  loggers_ = loggers;
}

Parser Parser::MakeFromFile(const std::string &filePath) {
  auto state = ScannerState::MakeFromFile(filePath);
  return Parser(std::move(state));
}

Parser Parser::MakeFromString(const std::string &inputString) {
  auto state = ScannerState::MakeFromString(inputString);
  return Parser(std::move(state));
}

} // namespace cool
