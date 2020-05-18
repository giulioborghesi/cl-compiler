#ifndef COOL_CORE_STATUS_H
#define COOL_CORE_STATUS_H

#include <cstdlib>
#include <string>

namespace cool {

/// Class that represents the status of an operation
class Status {

public:
  Status() = default;
  Status(bool isOk) : isOk_(isOk) {}
  Status(const std::string &errorMsg) : isOk_(false), errorMsg_(errorMsg) {}

  /// Create a successfull status
  ///
  /// \return a Status object corresponding to success
  static Status Ok();

  /// Create an error status
  ///
  /// \return a Status object corresponding to an error with no message
  static Status Error();

  /// Get the error message associated with the Status object
  std::string getErrorMessage() const { return errorMsg_; }

  /// Get the operation status
  ///
  /// \return true if the operation was successfull, false otherwise
  bool isOk() const { return isOk_; }

private:
  bool isOk_ = true;
  std::string errorMsg_;
};

Status GenericError(const std::string &errorMsg);

} // namespace cool

#endif
