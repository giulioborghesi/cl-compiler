#ifndef COOL_FRONTEND_SCANNER_STATE_H
#define COOL_FRONTEND_SCANNER_STATE_H

#include <cool/core/status.h>
#include <cool/frontend/error_codes.h>
#include <cool/frontend/scanner_extra.h>
#include <cool/frontend/scanner_spec.h>

#include <cstdlib>
#include <memory>
#include <string>

namespace cool {

/// Forward declaration. Defined in implementation file
class Buffer;

/// RAII class to store the state of a FLEX scanner
class ScannerState {

public:
  ~ScannerState();

  /// \brief Return the scanner state
  ///
  /// \return the scanner state
  yyscan_t scannerState() const { return state_; }

  /// \brief Return the last error code seen by the scanner, or 0 if no error
  /// occurred
  ///
  /// \return the last error code seen by the scanner
  FrontEndErrorCode lastErrorCode() const;

  /// \brief Factory method to create a ScannerState object from file
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] filePath path to input file
  /// \return a unique pointer to a ScannerState object
  static std::unique_ptr<ScannerState>
  MakeFromFile(const std::string &filePath);

  /// \brief Factory method to create a ScannerState object from a string
  ///
  /// \warning This method will throw an assertion should an error occur
  ///
  /// \param[in] inputString string to parse
  /// \return a unique pointer to a ScannerState object
  static std::unique_ptr<ScannerState>
  MakeFromString(const std::string &inputString);

  /// \brief Reset the error code
  void resetErrorCode();

protected:
  /// Use factory method to create ScannerState objects
  ScannerState();

private:
  yyscan_t state_;
  ExtraState extraState_;
  std::unique_ptr<Buffer> buffer_;
};

} // namespace cool

#endif
