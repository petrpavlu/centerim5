# When you add files here, also add them in po/POTFILES.in.
set(centerim5_SOURCES
  AccountStatusMenu.cpp
  AccountWindow.cpp
  Accounts.cpp
  BuddyList.cpp
  BuddyListNode.cpp
  CenterIM.cpp
  Connections.cpp
  Conversation.cpp
  ConversationRoomList.cpp
  Conversations.cpp
  Footer.cpp
  GeneralMenu.cpp
  Header.cpp
  Log.cpp
  Notify.cpp
  OptionWindow.cpp
  PluginWindow.cpp
  Request.cpp
  Transfers.cpp
  Utils.cpp
  git-version.cpp)

add_executable(centerim5 ${centerim5_SOURCES})

set_property(TARGET centerim5
  PROPERTY INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
target_include_directories(centerim5 PRIVATE
  ${Intl_INCLUDE_DIRS})
target_compile_options(centerim5 PRIVATE
  ${PURPLE_CFLAGS} ${GLIB2_CFLAGS} ${SIGC_CFLAGS})
target_link_libraries(centerim5 PRIVATE
  ${Intl_LIBRARIES} ${PURPLE_LDFLAGS} ${GLIB2_LDFLAGS} ${SIGC_LDFLAGS}
  cppconsui)

install(TARGETS centerim5 DESTINATION bin)
