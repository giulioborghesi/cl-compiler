#ifndef COOL_FRONTEND_SCANNER_EXTRA_H
#define COOL_FRONTEND_SCANNER_EXTRA_H

#include <cstdlib>
#include <string>

namespace cool {

struct ExtraState {
  uint32_t currentLine = 1;
  uint32_t currentColumn = 1;

  uint32_t firstLine = 1;
  uint32_t firstColumn = 1;

  uint32_t openComments = 0;
  std::string stringText;
};

} // namespace cool

#endif
