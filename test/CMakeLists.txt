if(TEST)

enable_testing()

aux_source_directory(. SRCS)

foreach(TEST_SRC IN LISTS SRCS)
    get_filename_component(TEST_TARGET ${TEST_SRC} NAME_WE)
    add_executable(${TEST_TARGET} ${TEST_SRC})
    target_link_libraries(${TEST_TARGET} hgargparse)
    link_directories(${LIBRARY_OUTPUT_PATH})
    add_test(NAME test_${TEST_TARGET} COMMAND ${TEST_TARGET})
endforeach()

unset(TEST_SRC)
unset(SRCS)

endif()
