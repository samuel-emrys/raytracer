function(enable_sanitizers project_name)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" ON)

    if(ENABLE_COVERAGE)
      set(COVERAGE_FLAGS --coverage -O0 -g)
      set(OPTIMIZATION_FLAGS -O0 -g)
      target_compile_options(${project_name} INTERFACE $<$<CONFIG:DEBUG>:${COVERAGE_FLAGS} ${OPTIMIZATION_FLAGS}>)
      target_link_libraries(${project_name} INTERFACE $<$<CONFIG:DEBUG>:${COVERAGE_FLAGS}>)
    endif()

    set(SANITIZERS "")

    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ON)
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
    endif()

    option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" ON)
    if(ENABLE_SANITIZER_LEAK)
      list(APPEND SANITIZERS "leak")
    endif()

    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" ON)
    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
    endif()

    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    if(ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "thread")
      endif()
    endif()

    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(WARNING "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented otherwise it reports false positives")
      if("address" IN_LIST SANITIZERS
         OR "thread" IN_LIST SANITIZERS
         OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "memory")
      endif()
    endif()

    list(
      JOIN
      SANITIZERS
      ","
      LIST_OF_SANITIZERS)
    message("List of sanitizers: ${LIST_OF_SANITIZERS}")

  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT
       "${LIST_OF_SANITIZERS}"
       STREQUAL
       "")
      target_compile_options(${project_name} INTERFACE $<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>)
      target_link_options(${project_name} INTERFACE $<$<CONFIG:DEBUG>:-fsanitize=${LIST_OF_SANITIZERS}>)
    endif()
  endif()

endfunction()
