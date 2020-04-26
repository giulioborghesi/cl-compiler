#ifndef COOL_FRONTEND_SCANNER_EXTRA_H
#define COOL_FRONTEND_SCANNER_EXTRA_H

#include <cool/core/logger_collection.h>
#include <cool/frontend/error_codes.h>

#include <cstdlib>
#include <string>

namespace cool {

struct ExtraState {
  uint32_t currentLine = 1;
  uint32_t currentColumn = 1;

  uint32_t openComments = 0;
  FrontEndErrorCode lastErrorCode = FrontEndErrorCode::NO_ERROR;

  std::string stringText;
};

} // namespace cool

#endif
