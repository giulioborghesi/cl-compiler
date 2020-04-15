#include <cool/frontend/scanner.h>

#include <cassert>
#include <string>

/// Lexer function
extern int yylex();

/// Lexer buffer management functions
extern YY_BUFFER_STATE yy_create_buffer(FILE *, int);
extern void yy_switch_to_buffer(YY_BUFFER_STATE);
extern void yy_delete_buffer(YY_BUFFER_STATE);
extern YY_BUFFER_STATE yy_scan_string(const char *);

namespace cool {

namespace {

const uint32_t BUFFER_SIZE = 4096;

bool IsTokenStringOrID(const TokenIDType &tokenID) {
  if (tokenID == TOKEN_STRING) {
    return true;
  }

  if (tokenID == TOKEN_OBJECT_ID) {
    return true;
  }

  if (tokenID == TOKEN_CLASS_ID) {
    return true;
  }

  return false;
}

} // namespace

Scanner::Scanner()
    : lastToken_(Token(UNINITIALIZED, 0, 0)), inputFile_(nullptr),
      bufferState_(nullptr) {}

Scanner::~Scanner() { resetScanner(); }

const Token &Scanner::currentToken() const {
  assert(lastToken_.tokenID != UNINITIALIZED);
  return lastToken_;
}

const Token &Scanner::nextToken() {
  /// Buffer must be valid and not exhausted
  assert(bufferState_);
  assert(lastToken_.tokenID != TOKEN_EOF);
  assert(lastToken_.tokenID != SCANNER_ERROR_UNTERMINATED_STRING);
  assert(lastToken_.tokenID != SCANNER_ERROR_UNTERMINATED_COMMENT);

  /// Read token and process it
  const auto tokenID = yylex();
  if (IsTokenStringOrID(tokenID)) {
    lastToken_ = Token(tokenID, yylval.string_val, yylval.lloc, yylval.cloc);
    return lastToken_;
  }

  lastToken_ = Token(tokenID, yylval.lloc, yylval.cloc);
  return lastToken_;
}

Status Scanner::setInputFile(const std::string &inputFileName) {
  resetScanner();

  inputFile_ = fopen(inputFileName.c_str(), "r");
  if (!inputFile_) {
    return GenericError("Error: could not open input file");
  }

  bufferState_ = yy_create_buffer(inputFile_, BUFFER_SIZE);
  if (!bufferState_) {
    return GenericError("Error: could not intialize scanner");
  }
  yy_switch_to_buffer(bufferState_);
  return Status::Ok();
}

Status Scanner::setInputString(const std::string &inputString) {
  resetScanner();

  bufferState_ = yy_scan_string(inputString.c_str());
  if (!bufferState_) {
    return GenericError("Error: could not initialize scanner");
  }
  yy_switch_to_buffer(bufferState_);
  return Status::Ok();
}

void Scanner::resetScanner() {
  if (bufferState_) {
    yy_delete_buffer(bufferState_);
    bufferState_ = nullptr;
  }
  if (inputFile_) {
    fclose(inputFile_);
    inputFile_ = nullptr;
  }

  lastToken_.tokenID = UNINITIALIZED;
}

} // namespace cool
