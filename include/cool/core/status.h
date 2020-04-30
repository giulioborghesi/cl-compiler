#ifndef COOL_CORE_STATUS_H
#define COOL_CORE_STATUS_H

#include <cstdlib>
#include <string>

namespace cool {

/// Class that represents the status of an operation
class Status {

public:
  Status() = default;
  Status(const std::string &errorMsg) : isOk_(false), errorMsg_(errorMsg) {}

  static Status Ok();

  std::string getErrorMessage() const { return errorMsg_; }

  bool isOk() const { return isOk_; }

private:
  bool isOk_ = true;
  std::string errorMsg_;
};

Status EmptyError();
Status GenericError(const std::string &errorMsg);

} // namespace cool

#endif
