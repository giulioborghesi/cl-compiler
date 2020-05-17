#include <cool/core/logger.h>
#include <cool/core/logger_collection.h>

namespace cool {

ILogger *LoggerCollection::logger(const std::string &loggerName) const {
  if (!loggers_.count(loggerName)) {
    return nullptr;
  }
  return loggers_.find(loggerName)->second.get();
}

void LoggerCollection::logMessage(const LogMessage &message) const {
  for (auto &logger : loggers_) {
    logger.second->logMessage(message);
  }
}

Status LoggerCollection::registerLogger(const std::string &loggerName,
                                        std::shared_ptr<ILogger> logger) {
  if (loggers_.count(loggerName)) {
    return GenericError("Error: logger is already defined");
  }

  loggers_.insert({loggerName, logger});
  return Status::Ok();
}

Status LoggerCollection::removeLogger(const std::string &loggerName) {
  if (!loggers_.count(loggerName)) {
    return GenericError("Error: logger does not exist");
  }

  loggers_.erase(loggers_.find(loggerName));
  return Status::Ok();
}

} // namespace cool
