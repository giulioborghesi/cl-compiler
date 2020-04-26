add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest")

macro(package_add_test_with_libraries TESTNAME FILES LIBRARIES TEST_WORKING_DIRECTORY)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} gtest gtest_main ${LIBRARIES})
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test_with_libraries(test_expr ./ir/test_expr.cpp lib_ir "${PROJECT_DIR}")
package_add_test_with_libraries(test_type_check ./analysis/test_type_check.cpp "lib_analysis;lib_core;lib_ir" "${PROJECT_DIR}")
package_add_test_with_libraries(test_class_registry ./core/test_class_registry.cpp "lib_ir;lib_core" "${PROJECT_DIR}")
package_add_test_with_libraries(test_log_message ./core/test_log_message.cpp "lib_core" "${PROJECT_DIR}")
package_add_test_with_libraries(test_logger_collection ./core/test_logger_collection.cpp "lib_core" "${PROJECT_DIR}")
package_add_test_with_libraries(test_scanner ./frontend/test_scanner.cpp "lib_frontend;lib_core" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/")
package_add_test_with_libraries(test_parser ./frontend/test_parser.cpp "lib_frontend;lib_core;lib_ir" "${CMAKE_CURRENT_SOURCE_DIR}/frontend/")
