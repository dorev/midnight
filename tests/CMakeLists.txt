project(tests)

add_executable(${PROJECT_NAME}
    main.cpp
    tests.cpp
)

target_link_libraries(${PROJECT_NAME} Loom)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${GTEST_ROOT}/googletest
    PUBLIC ${GTEST_ROOT}/googletest/include
    PUBLIC ${GTEST_ROOT}/googlemock
    PUBLIC ${GTEST_ROOT}/googlemock/include
)
