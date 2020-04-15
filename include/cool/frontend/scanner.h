#ifndef COOL_FRONTEND_SCANNER_H
#define COOL_FRONTEND_SCANNER_H

#include <cool/core/status.h>
#include <cool/frontend/scanner_spec.h>
#include <cool/frontend/token.h>

#include <cstdio>

/// Forward declaration
typedef struct yy_buffer_state *YY_BUFFER_STATE;

namespace cool {

/// Class that implements a flex-based scanner
class Scanner {

public:
  Scanner();
  ~Scanner();

  /// \brief Return the last scanned token
  ///
  /// \warning This method requires that one token in the current input stream
  /// has been read. An assertion will be triggered if this precondition is not
  /// satisfied.
  ///
  /// \return last token in input stream
  const Token &currentToken() const;

  /// \brief Parse next token
  ///
  /// \warning This method requires that the next token exist. This is the case
  /// if the last token was not EOF. An assertion will be triggered if this
  /// precondition is not satisfied
  ///
  /// \return next token in input stream
  const Token &nextToken();

  /// \brief Set scanner input to file
  ///
  /// \param[in] inputFile path to file to scan
  /// \return Status::Ok() on success
  Status setInputFile(const std::string &inputFile);

  /// \brief Set scanner input to string
  ///
  /// \param[in] inputString string to scan
  /// \return Status::Ok() on success
  Status setInputString(const std::string &inputString);

private:
  /// Reset the scanner
  void resetScanner();

  Token lastToken_;
  FILE *inputFile_ = nullptr;
  YY_BUFFER_STATE bufferState_ = nullptr;
};

} // namespace cool

#endif
