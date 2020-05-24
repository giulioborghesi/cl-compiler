#ifndef COOL_TESTS_UTILS_H
#define COOL_TESTS_UTILS_H

#include <cool/core/log_message.h>
#include <cool/core/logger.h>

#include <cassert>
#include <string>
#include <vector>

namespace cool {

class StringLogger : public ILogger {

public:
  StringLogger() = default;
  ~StringLogger() = default;

  void logMessage(const LogMessage &message) { logs_.push_back(message); }

  /// Return the number of logged messages
  size_t loggedMessageCount() const { return logs_.size(); }

  /// Return the logged message with the given index
  ///
  /// \param[in] idx index of logged message to fetch
  /// \return the logged message
  const LogMessage &loggedMessage(const size_t idx) const {
    assert(idx < logs_.size());
    return logs_[idx];
  }

  /// Reset the logged messages
  void reset() { logs_.clear(); }

private:
  std::vector<LogMessage> logs_;
};

} // namespace cool

#endif
