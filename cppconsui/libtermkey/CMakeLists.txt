# when you add files here, also add them in po/POTFILES.in
set( termkey_SOURCES
	driver-csi.c
	driver-ti.c
	termkey.c
)

set( termkey_HEADERS
	termkey-internal.h
	termkey.h
)

add_library(	termkey STATIC
		${termkey_SOURCES}
		${termkey_HEADERS}
	)
