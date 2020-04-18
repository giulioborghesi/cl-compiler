#ifndef COOL_FRONTEND_SCANNER_STATE_H
#define COOL_FRONTEND_SCANNER_STATE_H

#include <cstdlib>
#include <string>

#include <cool/core/status.h>
#include <cool/frontend/scanner_spec.h>

namespace cool {

/// Forward declaration. Defined in implementation file
class Buffer;

/// RAII class to store the state of a YACC scanner
class ScannerState {

  using BufferState = struct yy_buffer_state *;

public:
  ~ScannerState();

  /// \brief Return the scanner state
  ///
  /// \return the scanner state
  yyscan_t scannerState() const { return state_; }

  /// \brief Factory method to create a ScannerState object from file
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] filePath path to input file
  /// \return a ScannerState object
  static ScannerState MakeFromFile(const std::string &filePath);

  /// \brief Factory method to create a ScannerState object from a string
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] inputString string to parse
  /// \return a ScannerState object
  static ScannerState MakeFromString(const std::string &inputString);

protected:
  /// Use factory method to create ScannerState objects
  ScannerState();

private:
  yyscan_t state_;
  std::shared_ptr<Buffer> buffer_;
};

} // namespace cool

#endif
