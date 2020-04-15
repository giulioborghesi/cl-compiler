#ifndef COOL_FRONTEND_TOKEN_H
#define COOL_FRONTEND_TOKEN_H

#include <cstdlib>
#include <string>

namespace cool {

static constexpr int32_t UNINITIALIZED = -1000;

typedef int32_t TokenIDType;

/// Class that represents a token object in COOL
struct Token {
  Token(TokenIDType inputTokenID, const uint32_t inputLloc,
        const uint32_t inputCloc)
      : tokenID(inputTokenID), lloc(inputLloc), cloc(inputCloc) {}

  Token(TokenIDType inputTokenID, const std::string &inputTokenValue,
        const uint32_t inputLloc, const uint32_t inputCloc)
      : tokenID(inputTokenID), tokenValue(inputTokenValue), lloc(inputLloc),
        cloc(inputCloc) {}

  TokenIDType tokenID = UNINITIALIZED;
  std::string tokenValue;
  uint32_t lloc, cloc;
};

} // namespace cool

#endif