#include <cool/frontend/scanner_state.h>

#include <cassert>
#include <cstdio>

namespace {
static constexpr uint32_t MAX_BUFFER_SIZE = 2048;
}

/// Buffer state alias
typedef struct yy_buffer_state *YY_BUFFER_STATE;

/// Flex-defined functions to manage scanner states and buffers
extern YY_BUFFER_STATE yy_create_buffer(FILE *, int, yyscan_t);
extern void yy_delete_buffer(YY_BUFFER_STATE, yyscan_t);
extern int yylex_destroy(yyscan_t);
extern int yylex_init_extra(cool::ExtraState *, yyscan_t *);
extern YY_BUFFER_STATE yy_scan_string(const char *, yyscan_t);
extern void yy_switch_to_buffer(YY_BUFFER_STATE, yyscan_t);

namespace cool {

/// RAII class to store the buffer of a FLEX scanner
class Buffer {

public:
  Buffer() = delete;
  virtual ~Buffer();

  /// \brief Return the buffer
  ///
  /// \return the buffer
  yy_buffer_state *buffer() const { return buffer_; }

protected:
  Buffer(yyscan_t state) : state_(state) {}

  yyscan_t state_ = nullptr;
  yy_buffer_state *buffer_ = nullptr;
};

/// Specialization of buffer class for file objects
class FileBuffer : public Buffer {

public:
  FileBuffer() = delete;
  FileBuffer(yyscan_t state, const std::string &filePath);
  ~FileBuffer() override;

private:
  FILE *file_;
};

/// Specialization of buffer class for string objects
class StringBuffer : public Buffer {

public:
  StringBuffer() = delete;
  StringBuffer(yyscan_t state, const std::string &inputString);
  ~StringBuffer() override = default;

private:
  std::string string_;
};

Buffer::~Buffer() { yy_delete_buffer(buffer_, state_); }

FileBuffer::FileBuffer(yyscan_t state, const std::string &filePath)
    : Buffer(state) {
  /// Open file stream
  file_ = fopen(filePath.c_str(), "r");
  assert(file_);

  /// Create buffer and set it to be the input stream
  buffer_ = yy_create_buffer(file_, MAX_BUFFER_SIZE, state_);
  assert(buffer_);
  yy_switch_to_buffer(buffer_, state_);
}

FileBuffer::~FileBuffer() {
  int status = fclose(file_);
  assert(status == 0);
}

StringBuffer::StringBuffer(yyscan_t state, const std::string &inputString)
    : Buffer(state), string_(inputString) {
  buffer_ = yy_scan_string(string_.c_str(), state_);
  assert(buffer_);
  yy_switch_to_buffer(buffer_, state_);
}

ScannerState::ScannerState() : state_(nullptr), buffer_(nullptr) {
  auto status = yylex_init_extra(&extraState_, &state_);
  assert(status == 0);
}

ScannerState::~ScannerState() {
  buffer_.reset();
  auto status = yylex_destroy(state_);
  assert(status == 0);
}

ScannerState ScannerState::MakeFromFile(const std::string &inputPath) {
  ScannerState state{};
  state.buffer_ =
      std::shared_ptr<Buffer>(new FileBuffer(state.state_, inputPath));
  return state;
}

ScannerState ScannerState::MakeFromString(const std::string &inputString) {
  ScannerState state{};
  state.buffer_ =
      std::shared_ptr<Buffer>(new StringBuffer(state.state_, inputString));
  return state;
}

} // namespace cool
