QT = gui core opengl network
CONFIG += qt \
 warn_on \
 console \
 opengl \
 debug_and_release \
 build_all \
 thread \
 lib_bundle
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
HEADERS = \
    src/Chatbox.h \
    src/FuGaModule.h \
    src/FugaConfig.h \
    src/FugaHelperFuncs.h \
    src/Fuga.h \
    src/FugaWindow.h \
    src/FugaContacts.h \
    src/FugaContact.h \
    src/FugaDns.h \
    src/MFugaVideochat.h \
    src/FugaVideo.h \
    src/FugaStreamer.h
SOURCES = src/main.cpp \
    src/Chatbox.cpp \
    src/FuGaModule.cpp \
    src/FugaConfig.cpp \
    src/FugaHelperFuncs.cpp \
    src/Fuga.cpp \
    src/FugaWindow.cpp \
    src/FugaContacts.cpp \
    src/FugaContact.cpp \
    src/FugaDns.cpp \
    src/MFugaVideochat.cpp \
    src/FugaStreamer.cpp \
    src/FugaVideo.cpp

# gstreamer
CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-0.10
INCLUDEPATH += /usr/include/glib-2.0 \
	/usr/lib/glib-2.0/include \
	/usr/include/libxml2
LIBS += -pthread \
	-lgstreamer-0.10 \
	-lgobject-2.0 \
	-lgmodule-2.0 \
	-lgthread-2.0 \
	-lrt \
	-lxml2 \
	-lglib-2.0 \
	-lgstapp-0.10 \
	-lgstinterfaces-0.10
