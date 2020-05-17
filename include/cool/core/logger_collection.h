#ifndef COOL_CORE_LOGGER_COLLECTION_H
#define COOL_CORE_LOGGER_COLLECTION_H

#include <cool/core/log_message.h>
#include <cool/core/status.h>

#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace cool {

/// Forward declarations
class ILogger;
class LogMessage;

/// \brief Class that maintains a collection of logger objects
class LoggerCollection {

public:
  LoggerCollection() = default;

  /// \brief Return a logger given its name
  ///
  /// \warning This function will return nullptr if the logger does not exist
  ///
  /// \param[in] loggerName name of logger to return
  /// \return a pointer to the logger object
  ILogger *logger(const std::string &loggerName) const;

  /// \brief Log a message
  ///
  /// \param[in] message message to log
  void logMessage(const LogMessage &message) const;

  /// \brief Add a logger to the collection
  ///
  /// \param[in] loggerName name of logger to add
  /// \param[in] logger shared pointer to logger object
  /// \return Status::Ok() if successfull, an error message otherwise
  Status registerLogger(const std::string &loggerName,
                        std::shared_ptr<ILogger> logger);

  /// \brief Remove a logger from the collection
  ///
  /// \param[in] loggerName name of logger to remove
  /// \return Status::Ok() if successfull, an error message otherwise
  Status removeLogger(const std::string &loggerName);

private:
  std::unordered_map<std::string, std::shared_ptr<ILogger>> loggers_;
};

#define LOG_MESSAGE_WITH_LOCATION(logger, token, severity, ...)                \
  {                                                                            \
    std::ostringstream sHeader;                                                \
    sHeader << "Error: line " << token->lineLoc() << ", column "               \
            << token->charLoc() << ". ";                                       \
                                                                               \
    char buffer[2048];                                                         \
    snprintf(buffer, 2048, __VA_ARGS__);                                       \
                                                                               \
    std::string message = sHeader.str();                                       \
    message.append(std::string(buffer));                                       \
    LogMessage logMessage(message, LogMessageSeverity::severity);              \
    logger->logMessage(logMessage);                                            \
  }

#define LOG_MESSAGE(logger, severity, ...)                                     \
  {                                                                            \
    std::ostringstream sHeader;                                                \
    sHeader << "Generic error. ";                                              \
                                                                               \
    char buffer[2048];                                                         \
    snprintf(buffer, 2048, __VA_ARGS__);                                       \
                                                                               \
    std::string message = sHeader.str();                                       \
    message.append(std::string(buffer));                                       \
    LogMessage logMessage(message, LogMessageSeverity::severity);              \
    logger->logMessage(logMessage);                                            \
  }

#define LOG_ERROR_MESSAGE_WITH_LOCATION(logger, token, ...)                    \
  if (logger) {                                                                \
    LOG_MESSAGE_WITH_LOCATION(logger, token, ERROR, __VA_ARGS__)               \
  }

#define LOG_DEBUG_MESSAGE_WITH_LOCATION(logger, token, ...)                    \
  if (logger) {                                                                \
    LOG_MESSAGE_WITH_LOCATION(logger, token, DEBUG, __VA_ARGS__)               \
  }

#define LOG_ERROR_MESSAGE(logger, ...)                                         \
  if (logger) {                                                                \
    LOG_MESSAGE(logger, ERROR, __VA_ARGS__);                                   \
  }

#define LOG_DEBUG_MESSAGE(logger, ...)                                         \
  if (logger) {                                                                \
    LOG_MESSAGE(logger, DEBUG, __VA_ARGS__);                                   \
  }

} // namespace cool

#endif
