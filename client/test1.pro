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
	src/TStreamListener.h \
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
	src/TStreamListener.cpp \
	src/MyTcpsocket.cpp \
	src/ContactHandler.cpp \
    src/MyConfig.cpp \
    src/MyUdpServer.cpp \
    src/MyModule.cpp \
    src/MyUser.cpp \
    src/MeUser.cpp \
    src/Chatbox.cpp

INCLUDEPATH += "\usr\include\opencv\cxcore\include" \
	"\usr\include\opencv\cv\include" \
	"\usr\include\opencv\cvaux\include" \
	"\usr\include\otherlibs\highgui" \
	"\usr\include\opencv\cxcore\include"

LIBS +=  -lcv \
  -lml \
  -lcvaux \
  -lhighgui \
  -lcxcore

TEMPLATE =  app
