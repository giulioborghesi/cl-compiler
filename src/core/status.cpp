#include <cool/core/status.h>

namespace cool {

Status Status::Ok() { return Status{}; }

Status Status::Error() { return Status{false}; }

Status GenericError(const std::string &errorMsg) {
  return cool::Status{errorMsg};
}

} // namespace cool