if(GETTEXT_FOUND)
  file(STRINGS LINGUAS linguas)

  # The default build target (ALL) only creates .gmo files from .po files.
  foreach(lang ${linguas})
    gettext_process_po_files(${lang} ALL PO_FILES ${lang}.po)

    # Use custom install target instead of specifying the INSTALL_DESTINATION
    # option for gettext_process_po_files() because it gets naming of the
    # resulting .mo files wrong.
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo
      DESTINATION "share/locale/${lang}/LC_MESSAGES/" RENAME ${PROJECT_NAME}.mo)
  endforeach()

  # Note: CMake currently (version 3.3) does not have any built-in support for
  # generating .pot files. Additionally, the way FindGettext works does not
  # allow to create targets for generating .po files because both
  # gettext_create_translations() and gettext_process_pot_file() always do
  # msgmerge+msgfmt and the msgfmt command creates a clashing rule with the one
  # produced by gettext_process_po_files().
  #
  # This means update-po functionality provided by the autotools build is not
  # currently available in the CMake build.
endif()
