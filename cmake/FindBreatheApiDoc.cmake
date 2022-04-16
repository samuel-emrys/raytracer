#Look for an executable called breathe-apidoc
find_program(BREATHE_APIDOC_EXECUTABLE
             NAMES breathe-apidoc
             DOC "Path to breathe-apidoc executable")

include(FindPackageHandleStandardArgs)

#Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(BreatheApiDoc
                                  "Failed to find breathe-apidoc executable"
                                  BREATHE_APIDOC_EXECUTABLE)
