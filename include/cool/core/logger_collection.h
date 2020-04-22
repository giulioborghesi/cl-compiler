#ifndef COOL_CORE_LOGGER_COLLECTION_H
#define COOL_CORE_LOGGER_COLLECTION_H

#include <cool/core/status.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace cool {

/// Forward declarations
class Logger;
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
  /// \return the specified logger object
  std::shared_ptr<Logger> logger(const std::string &loggerName) const;

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
                        std::shared_ptr<Logger> logger);

  /// \brief Remove a logger from the collection
  ///
  /// \param[in] loggerName name of logger to remove
  /// \return Status::Ok() if successfull, an error message otherwise
  Status removeLogger(const std::string &loggerName);

private:
  std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
};

} // namespace cool

#endif
