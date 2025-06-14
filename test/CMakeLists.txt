add_subdirectory(catch)

function(ADD_IGOR_TESTCASE arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE igor Catch2::Catch2WithMain)
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${IGOR_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${IGOR_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${IGOR_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${IGOR_CXX_FLAGS_RELEASE}>"
  )
  set_property(TARGET ${arg1} PROPERTY CXX_STANDARD 20)
  set_property(TARGET ${arg1} PROPERTY CXX_STANDARD_REQUIRED YES)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)
  add_test(${arg1} ${arg1})
endfunction()

ADD_IGOR_TESTCASE(basic)
