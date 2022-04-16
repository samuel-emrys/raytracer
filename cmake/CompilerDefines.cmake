function(set_project_defines project_name)

  set(UNIX_COMMON_DEFS)
  set(UNIX_DEBUG_DEFS)# _GLIBCXX_DEBUG) # Suppressed until issue is fixed in upstream xtensor library: see xtensor#2296 and xtensor#2471
  set(UNIX_RELEASE_DEFS)
  set(MSVC_COMMON_DEFS)
  set(MSVC_DEBUG_DEFS)
  set(MSVC_RELEASE_DEFS)

  if(MSVC)
    set(PROJECT_DEFINES ${MSVC_COMMON_DEFS} $<$<CONFIG:DEBUG>:${MSVC_DEBUG_DEFS}>
                        $<$<CONFIG:RELEASE>:${MSVC_RELEASE_DEFS}>)
  else()
    set(PROJECT_DEFINES ${UNIX_COMMON_DEFS} $<$<CONFIG:DEBUG>:${UNIX_DEBUG_DEFS}>
                        $<$<CONFIG:RELEASE>:${UNIX_RELEASE_DEFS}>)
  endif()
  target_compile_definitions(${project_name} INTERFACE ${PROJECT_DEFINES})

endfunction()
