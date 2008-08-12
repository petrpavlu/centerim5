cmake_minimum_required( VERSION 2.4.7 )
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

if( COMMAND cmake_policy )
    cmake_policy( VERSION 2.6 )
endif()

set( CENTERIM_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/centerimConfig.cmake )

# Check if the centerim cmake configuration file exists.
if( NOT EXISTS ${CENTERIM_CONFIG_FILE} )
    message( FATAL_ERROR "Configuration file not found." )
else()
    include( ${CENTERIM_CONFIG_FILE} )
endif()

#Set the centerim cppconsui sources
set( CENTERIM_CPPCONSUI_SRCS ${CENTERIM_CPPCONSUI_DIR}/AbstractListBox.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Border.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Button.cpp
    ${CENTERIM_CPPCONSUI_DIR}/ColorScheme.cpp
    ${CENTERIM_CPPCONSUI_DIR}/ComboBox.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Container.cpp
    ${CENTERIM_CPPCONSUI_DIR}/CppConsUI.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Dialog.cpp
    ${CENTERIM_CPPCONSUI_DIR}/HorizontalLine.cpp
    ${CENTERIM_CPPCONSUI_DIR}/HorizontalListBox.cpp
    ${CENTERIM_CPPCONSUI_DIR}/InputDialog.cpp
    ${CENTERIM_CPPCONSUI_DIR}/InputProcessor.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Keys.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Label.cpp
    ${CENTERIM_CPPCONSUI_DIR}/LineStyle.cpp
    ${CENTERIM_CPPCONSUI_DIR}/ListBox.cpp
    ${CENTERIM_CPPCONSUI_DIR}/MenuWindow.cpp
    ${CENTERIM_CPPCONSUI_DIR}/MessageDialog.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Panel.cpp
    ${CENTERIM_CPPCONSUI_DIR}/ScrollPane.cpp
    ${CENTERIM_CPPCONSUI_DIR}/TextBrowser.cpp
    ${CENTERIM_CPPCONSUI_DIR}/TextEntry.cpp
    ${CENTERIM_CPPCONSUI_DIR}/TextInput.cpp
    ${CENTERIM_CPPCONSUI_DIR}/TextWindow.cpp
    ${CENTERIM_CPPCONSUI_DIR}/TreeView.cpp
    ${CENTERIM_CPPCONSUI_DIR}/VerticalLine.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Widget.cpp
    ${CENTERIM_CPPCONSUI_DIR}/Window.cpp
    ${CENTERIM_CPPCONSUI_DIR}/WindowManager.cpp
    )

# Build the cppconsui library
add_library( ${CENTERIM_CPPCONSUI_PROJECT_NAME} ${CENTERIM_CPPCONSUI_SRCS} )

# Set the centerim sources
set( CENTERIM_SRCS ${CENTERIM_SRCS_DIR}/AccountStatusMenu.cpp
   ${CENTERIM_SRCS_DIR}/AccountWindow.cpp
   ${CENTERIM_SRCS_DIR}/Accounts.cpp
   ${CENTERIM_SRCS_DIR}/BuddyList.cpp
   ${CENTERIM_SRCS_DIR}/BuddyListNode.cpp
   ${CENTERIM_SRCS_DIR}/CenterIM.cpp
   ${CENTERIM_SRCS_DIR}/CenterMain.cpp
   ${CENTERIM_SRCS_DIR}/Conf.cpp
   ${CENTERIM_SRCS_DIR}/Connections.cpp
   ${CENTERIM_SRCS_DIR}/Conversation.cpp
   ${CENTERIM_SRCS_DIR}/Conversations.cpp
   ${CENTERIM_SRCS_DIR}/GeneralMenu.cpp
   ${CENTERIM_SRCS_DIR}/Log.cpp
   ${CENTERIM_SRCS_DIR}/Transfers.cpp
   ${CENTERIM_SRCS_DIR}/git-version.cpp
   )

# Build the centerim executable
add_executable( ${CENTERIM_PROJECT_NAME} ${CENTERIM_SRCS} )

# Add centerim dependency with cppconsui library
add_dependencies( ${CENTERIM_PROJECT_NAME} ${CENTERIM_CPPCONSUI_PROJECT_NAME} )

# Link centerim with the required modules
target_link_libraries( ${CENTERIM_PROJECT_NAME} ${CENTERIM_REQUIRED_LIBRARIES} 
    ${CENTERIM_CPPCONSUI_PROJECT_NAME}
    ${LIB_NCURSES}
    )

