find_package(FLEX)
FLEX_TARGET(Scanner cool_flex.lex ${CMAKE_CURRENT_BINARY_DIR}/cool_flex.cpp)
add_library(lib_scanner STATIC ${FLEX_Scanner_OUTPUTS})
