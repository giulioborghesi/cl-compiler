find_package(FLEX)
FLEX_TARGET(Scanner scanner_spec.lex ${CMAKE_CURRENT_BINARY_DIR}/scanner_spec.cpp)
add_library(lib_frontend STATIC scanner.cpp ${FLEX_Scanner_OUTPUTS})
