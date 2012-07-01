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
HEADERS = src/MyWindow.h \
	src/Supervisor.h \
	src/ModVideochat.h \
	src/MyTcpsocket.h \
	src/ContactHandler.h \
    src/MyUser.h \
    src/MeUser.h \
    src/Chatbox.h \
    src/FuGaModule.h \
    src/FuGaStreamer.h \
    src/FuGaVideo.h \
    src/FugaConfig.h \
    src/FugaHelperFuncs.h
SOURCES = src/main.cpp \
	src/MyWindow.cpp \
	src/Supervisor.cpp \
	src/ModVideochat.cpp \
	src/MyTcpsocket.cpp \
	src/ContactHandler.cpp \
    src/MyUser.cpp \
    src/MeUser.cpp \
    src/Chatbox.cpp \
    src/FuGaModule.cpp \
    src/FuGaStreamer.cpp \
    src/FuGaVideo.cpp \
    src/FugaConfig.cpp \
    src/FugaHelperFuncs.cpp

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
