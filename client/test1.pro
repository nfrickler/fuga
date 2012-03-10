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
	 src/MyVideo.h \
	src/Supervisor.h \
	src/ModVideochat.h \
	src/TWebcamGrabber.h \
	src/MyTcpsocket.h \
	src/ContactHandler.h \
    src/MyConfig.h \
    src/MyUdpServer.h \
    src/MyModule.h \
    src/MyUser.h \
    src/MeUser.h \
    src/Chatbox.h
SOURCES = src/main.cpp \
	src/MyWindow.cpp \
	src/MyVideo.cpp \
	src/Supervisor.cpp \
	src/ModVideochat.cpp \
	src/TWebcamGrabber.cpp \
	src/MyTcpsocket.cpp \
	src/ContactHandler.cpp \
    src/MyConfig.cpp \
    src/MyUdpServer.cpp \
    src/MyModule.cpp \
    src/MyUser.cpp \
    src/MeUser.cpp \
    src/Chatbox.cpp

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
