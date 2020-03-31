
#include <cool/core/context.h>

namespace cool {

Context::Context() { classRegistry_ = std::make_unique<ClassRegistry>(); }

} // namespace cool
