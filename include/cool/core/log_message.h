#ifndef COOL_CORE_LOG_MESSAGE_H
#define COOL_CORE_LOG_MESSAGE_H

#include <cstdio>
#include <string>
#include <utility>

namespace cool {

enum class LogMessageSeverity { DEBUG = 0, WARNING = 1, ERROR = 2, FATAL = 3 };

/// \brief Class that represents a log message
class LogMessage {

public:
  LogMessage(const char *logMessage, LogMessageSeverity severity)
      : logMessage_(logMessage), severity_(severity) {}
  LogMessage(const std::string &logMessage, LogMessageSeverity severity)
      : logMessage_(logMessage), severity_(severity) {}

  /// \brief Get the log message
  ///
  /// \return the log message
  const std::string &logMessage() const { return logMessage_; }

  /// \brief Create a debug message
  ///
  /// \warning For simplicity of implementation, this method assumes that the
  /// message format and the format arguments are correct. It is the
  /// responsibility of the calling code to ensure that these preconditions are
  /// satisfied
  ///
  /// \param[in] format debug message format
  /// \param[in] args format arguments
  /// \return a debug message
  template <typename... Args>
  static LogMessage MakeDebugMessage(const std::string &format, Args... args);

  /// \brief Create an error message
  ///
  /// \warning For simplicity of implementation, this method assumes that the
  /// message format and the format arguments are correct. It is the
  /// responsibility of the calling code to ensure that these preconditions are
  /// satisfied
  ///
  /// \param[in] format error message format
  /// \param[in] args format arguments
  /// \return an error message
  template <typename... Args>
  static LogMessage MakeErrorMessage(const std::string &format, Args... args);

  /// \brief Get the log message severity
  ///
  /// \return the log message severity
  LogMessageSeverity severity() const { return severity_; }

private:
  /// \brief Create a message
  ///
  /// \warning For simplicity of implementation, this method assumes that the
  /// message format and the format arguments are correct. It is the
  /// responsibility of the calling code to ensure that these preconditions are
  /// satisfied
  ///
  /// \param[in] format message format
  /// \param[in] severity message severity
  /// \param[in] args format arguments
  /// \return a message of specified severity
  template <typename... Args>
  static LogMessage MakeMessage(const std::string &format,
                                LogMessageSeverity severity, Args... args);

  const std::string logMessage_;
  LogMessageSeverity severity_;
};

namespace {

/// \brief Helper to remove const and reference qualifiers from a type
template <typename T>
using BaseType = std::remove_const_t<std::remove_reference_t<T>>;

/// \brief Metafunction to extract a format argument
template <typename T> struct ArgExtractor {
  static T GetArg(T arg) { return arg; }
};

/// \brief Specialization of above metafunction for strings
template <> struct ArgExtractor<std::string> {
  static const char *GetArg(const std::string &arg) { return arg.c_str(); }
};

} // namespace

template <typename... Args>
LogMessage LogMessage::MakeDebugMessage(const std::string &format,
                                        Args... args) {
  return LogMessage::MakeMessage(format, LogMessageSeverity::DEBUG,
                                 std::forward<Args>(args)...);
}

template <typename... Args>
LogMessage LogMessage::MakeErrorMessage(const std::string &format,
                                        Args... args) {
  return LogMessage::MakeMessage(format, LogMessageSeverity::ERROR,
                                 std::forward<Args>(args)...);
}

template <typename... Args>
LogMessage LogMessage::MakeMessage(const std::string &format,
                                   LogMessageSeverity severity, Args... args) {
  static constexpr uint32_t MAX_BUFFER_SIZE = 1024;

  char message[MAX_BUFFER_SIZE];
  snprintf(message, MAX_BUFFER_SIZE, format.c_str(),
           ArgExtractor<BaseType<Args>>::GetArg(args)...);
  return LogMessage(message, severity);
}

} // namespace cool

#endif
