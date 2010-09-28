FIND_PATH( LIBSIGC_INCLUDE_DIR
		sigc++/sigc++.h
		/usr/include/sigc++-2.0/
		/usr/local/include/sigc++-2.0
		/opt/gnome/include/sigc++-2.0
	)

FIND_PATH( LIBSIGC_LIB_INCLUDE_DIR
		sigc++config.h
		/usr/lib/sigc++-2.0/include
		/usr/lib64/sigc++-2.0/include
		/usr/local/lib/sigc++-2.0/include
		/usr/local/lib64/sigc++-2.0/include
		/opt/gnome/lib/sigc++-2.0/include
		/opt/gnome/lib64/sigc++-2.0/include
	)

FIND_LIBRARY( LIBSIGC_LIBRARY
		NAMES	sigc-2.0
		PATHS	/usr/lib
			/usr/lib64
			/usr/local/lib
			/usr/local/lib64
			/opt/gnome/lib
			/opt/gnome/lib64
)

SET( LIBSIGC_INCLUDE_DIRS
		${LIBSIGC_INCLUDE_DIR}
		${LIBSIGC_LIB_INCLUDE_DIR}
)

IF( LIBSIGC_INCLUDE_DIRS AND LIBSIGC_LIBRARY )
   SET( LIBSIGC_FOUND TRUE )
ENDIF( LIBSIGC_INCLUDE_DIRS AND LIBSIGC_LIBRARY )


IF( LIBSIGC_FOUND )
   IF( NOT libsigc_FIND_QUIETLY )
      MESSAGE( STATUS "Found libsigc: ${LIBSIGC_LIBRARY}" )
   ENDIF( NOT libsigc_FIND_QUIETLY )
ELSE( LIBSIGC_FOUND )
   IF( libsigc_FIND_REQUIRED )
      MESSAGE( FATAL_ERROR "Could not find libsigc" )
   ENDIF( libsigc_FIND_REQUIRED )
ENDIF( LIBSIGC_FOUND )
