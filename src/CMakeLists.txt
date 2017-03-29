# When you add files here, also add them in po/POTFILES.in
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

set(centerim5_HEADERS
  AccountStatusMenu.h
  AccountWindow.h
  Accounts.h
  BuddyList.h
  BuddyListNode.h
  CenterIM.h
  Connections.h
  Conversation.h
  ConversationRoomList.h
  Conversations.h
  Footer.h
  GeneralMenu.h
  Header.h
  Log.h
  Notify.h
  OptionWindow.h
  PluginWindow.h
  Request.h
  Transfers.h
  Utils.h
  git-version.h.in)

include_directories("${centerim5_BINARY_DIR}/src")

add_custom_target(git-version.h ALL VERBATIM
  COMMAND sh -c "cd \"${centerim5_SOURCE_DIR}\"; version=`misc/git-version-gen .tarball-version`; cd \"${CMAKE_CURRENT_BINARY_DIR}\"; if test \"$version\" != \"`cat .version 2> /dev/null`\"; then printf '%s' \"$version\" > .version; sed -e \"s/@GIT_VERSION@/`cat .version`/\" < \"${CMAKE_CURRENT_SOURCE_DIR}/git-version.h.in\" > git-version.h; fi")

add_executable(centerim5
  ${centerim5_SOURCES}
  ${centerim5_HEADERS})

add_dependencies(centerim5 git-version.h)

set_target_properties(centerim5
  PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

target_link_libraries(centerim5
  cppconsui
  ${PURPLE_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

install(TARGETS centerim5 DESTINATION bin)
