
QT += gui xml

TEMPLATE = lib
TARGET = QtOgre
#VERSION = 0.1.0
CONFIG += shared create_prl
DESTDIR = ../lib
DLLDESTDIR = ../bin
DEFINES += QtOgre_Export

DEPENDPATH += . include resources src ui
INCLUDEPATH += $(OGRE_SOURCE)/OgreMain/include \
    $(OGRE_BUILD)/include \
    include

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
    LIBS *= -L"$(OGRE_BUILD)/lib/debug"
    LIBS *= -lOgreMain_d
}

CONFIG(release, debug|release) {
    LIBS *= -L"$(OGRE_BUILD)/lib/release"
    LIBS *= -lOgreMain
}

# Input
HEADERS += include/Application.h \
           include/BasicEventHandler.h \
           include/EventHandler.h \
           include/HandlerManager.h \
           include/OgreWidget.h \
           include/Prerequisites.h

SOURCES += src/Application.cpp \
           src/BasicEventHandler.cpp \
           src/EventHandler.cpp \
           src/HandlerManager.cpp \
           src/OgreWidget.cpp

FORMS += ui/FPSDialog.ui
RESOURCES += resources/resources.qrc
