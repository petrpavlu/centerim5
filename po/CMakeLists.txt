# TODO: read in from LINGUAS?  I think that may be an autotool specific thing
set(langs
  cs
  de
  nl
  ru
  sv
)

# can't use this and gettext_process_po_files, disable for now
## Generate .po file list
#set(po_files )
#foreach(lang ${langs})
#  list(APPEND po_files ${lang}.po)
#endforeach()
#
## Creates "translations" target, that will update po files
#gettext_create_translations(centerim5.pot ${po_files})

# You would think that cmake Gettext would provide a target to generate .gmo
# files without regenerating .po files, but it doesn't.  Loop manually.
foreach(lang ${langs})
  gettext_process_po_files(${lang} ALL PO_FILES ${lang}.po)
  # Use this install target instead of INSTALL_DESTINATION to get the correct naming of the resulting .mo files
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION "share/locale/${lang}/LC_MESSAGES/" RENAME centerim5.mo)
endforeach()
