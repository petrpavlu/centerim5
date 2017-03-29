# Find include directory.
find_path(Iconv_INCLUDE_DIR NAMES "iconv.h" DOC "iconv include directory")
mark_as_advanced(Iconv_INCLUDE_DIR)

# Find all iconv libraries.
find_library(Iconv_LIBRARY "iconv"
  DOC "libiconv libraries (if not in the C library)")
mark_as_advanced(Iconv_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Iconv DEFAULT_MSG Iconv_INCLUDE_DIR)

if(Iconv_FOUND)
  set(Iconv_INCLUDE_DIRS "${Iconv_INCLUDE_DIR}")
  if(Iconv_LIBRARY)
    set(Iconv_LIBRARIES "${Iconv_LIBRARY}")
  else()
    unset(Iconv_LIBRARIES)
  endif()
endif()
