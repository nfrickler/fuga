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
	src/ModVideochat.h \
    src/Chatbox.h \
    src/FuGaModule.h \
    src/FuGaStreamer.h \
    src/FuGaVideo.h \
    src/FugaConfig.h \
    src/FugaHelperFuncs.h \
    src/Fuga.h \
    src/FugaAuth.h \
    src/FugaWindow.h \
    src/FugaContacts.h \
    src/FugaContact.h \
    src/FugaDns.h
SOURCES = src/main.cpp \
	src/ModVideochat.cpp \
    src/Chatbox.cpp \
    src/FuGaModule.cpp \
    src/FuGaStreamer.cpp \
    src/FuGaVideo.cpp \
    src/FugaConfig.cpp \
    src/FugaHelperFuncs.cpp \
    src/Fuga.cpp \
    src/FugaAuth.cpp \
    src/FugaWindow.cpp \
    src/FugaContacts.cpp \
    src/FugaContact.cpp \
    src/FugaDns.cpp

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
