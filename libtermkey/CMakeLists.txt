set (	termkey_SOURCES
		driver-csi.c
		driver-ti.c
		termkey.c
	)

set (	termkey_HEADERS
		termkey-internal.h
		termkey.h
	)

add_library (	termkey SHARED
			${termkey_HEADERS}
			${termkey_SOURCES}
	)

target_link_libraries ( termkey
		${CURSES_LIBRARIES}
	)

set_target_properties( termkey PROPERTIES
		COMPILE_FLAGS "-std=c99"
	)
