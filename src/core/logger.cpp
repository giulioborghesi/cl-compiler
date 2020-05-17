#include <cool/core/logger.h>

#include <iostream>

namespace cool {

Logger::Logger(Sink *sink, LogMessageSeverity severity) : ILogger() {
  sink_ = std::unique_ptr<Sink>(sink);
  severity_ = severity;
}

void Logger::logMessage(const LogMessage &message) const {
  if (message.severity() >= severity_) {
    sink_->record(message);
  }
}

void StdoutSink::record(const LogMessage &message) {
  std::cout << message.logMessage() << std::endl;
}

} // namespace cool
