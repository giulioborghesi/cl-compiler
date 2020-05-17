#ifndef COOL_CORE_LOGGER_H
#define COOL_CORE_LOGGER_H

#include <cool/core/log_message.h>

namespace cool {

/// \brief Class defining the interface of a log message writer
class Sink {

public:
  virtual ~Sink() = default;

  /// \brief Log the message
  ///
  /// \param[in] message message to log
  virtual void record(const LogMessage &message) = 0;

protected:
  Sink() = default;
};

class ILogger {

public:
  ILogger() = default;
  ~ILogger() = default;

  /// \brief Log a message
  ///
  /// \param[in] message message to log
  virtual void logMessage(const LogMessage &message) = 0;
};

/// \brief Class that implements the logger interface
class Logger : public ILogger {

public:
  Logger() = delete;
  Logger(Sink *sink, LogMessageSeverity severity);

  ~Logger() = default;

  void logMessage(const LogMessage &logMessage) final override;

private:
  std::unique_ptr<Sink> sink_;
  LogMessageSeverity severity_;
};

/// \brief Specialization for a log writer that writes to stdout
class StdoutSink : public Sink {
public:
  StdoutSink() = default;
  ~StdoutSink() final = default;

  void record(const LogMessage &logMessage) final;
};

} // namespace cool

#endif
