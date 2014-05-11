add_executable(TestCprError TestCprError.cpp)

target_link_libraries(TestCprError cprbase)

add_test(TestCprError TestCprError)
