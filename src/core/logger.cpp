#include <cool/core/logger.h>

#include <iostream>

namespace cool {

Logger::Logger(Sink *sink, LogMessageSeverity severity) : ILogger() {
  sink_ = std::unique_ptr<Sink>(sink);
  severity_ = severity;
}

void Logger::logMessage(const LogMessage &message) {
  if (message.severity() >= severity_) {
    sink_->record(message);
  }
}

void StdoutSink::record(const LogMessage &logMessage) {
  std::cout << logMessage.message() << std::endl;
}

} // namespace cool
