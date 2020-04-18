find_package(FLEX)
find_package(BISON)

# BISON_TARGET(
#    Parser parser_spec.y 
#    ${CMAKE_CURRENT_BINARY_DIR}/parser_spec.cpp 
#    COMPILE_FLAGS --defines=${PROJECT_SOURCE_DIR}/include/cool/frontend/scanner_spec.h)
FLEX_TARGET(Scanner scanner_spec.l ${CMAKE_CURRENT_BINARY_DIR}/scanner_spec.cpp)
add_library(lib_frontend STATIC scanner_state.cpp ${FLEX_Scanner_OUTPUTS})
