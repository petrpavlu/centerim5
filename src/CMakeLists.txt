# When you add files here, also add them in po/POTFILES.in
set( centerim_SOURCES
	AccountStatusMenu.cpp
	AccountWindow.cpp
	Accounts.cpp
	BuddyList.cpp
	BuddyListNode.cpp
	CenterIM.cpp
	CenterMain.cpp
	Conversation.cpp
	Conversations.cpp
	Connections.cpp
	GeneralMenu.cpp
	Header.cpp
	Log.cpp
	Notify.cpp
	OptionWindow.cpp
	Request.cpp
	Transfers.cpp
	Utils.cpp
	git-version.cpp
)

set( centerim_HEADERS
	AccountStatusMenu.h
	AccountWindow.h
	Accounts.h
	BuddyList.h
	BuddyListNode.h
	CenterIM.h
	Conversation.h
	Conversations.h
	Connections.h
	GeneralMenu.h
	Header.h
	Log.h
	Notify.h
	OptionWindow.h
	Request.h
	Transfers.h
	Utils.h
	git-version.h.in
)

include_directories( ${centerim_BINARY_DIR}/src/)

add_custom_target( git-version.h ALL
			COMMAND		${centerim_SOURCE_DIR}/misc/git-version-gen ${centerim_SOURCE_DIR}/.version git-version || echo "GIT version updated successfully"
			COMMAND		sed -e s/"@GIT_VERSION@"/`cat git-version`/g < ${CMAKE_CURRENT_SOURCE_DIR}/git-version.h.in > git-version.h
			COMMAND		${CMAKE_COMMAND} -E remove git-version
		)

add_executable( centerim
		${centerim_SOURCES}
		${centerim_HEADERS}
	)

add_dependencies( centerim git-version.h )
add_dependencies( centerim translations )

target_link_libraries( centerim
		termkey
		cppconsui
		${PURPLE_LIBRARIES}
	)

install( TARGETS	centerim
	 DESTINATION	bin/
	 PERMISSIONS	OWNER_READ
	 		OWNER_EXECUTE
			GROUP_READ
			GROUP_EXECUTE
			WORLD_READ
			WORLD_EXECUTE
)
