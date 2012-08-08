
QT += gui xml
TEMPLATE = app
TARGET = PipeDemo
#message(Demo)
#message($$ROOT)
DESTDIR = ../bin
CONFIG += warn_on

INCLUDEPATH += $(OGRE_SOURCE)/OgreMain/include \
    $(OGRE_BUILD)/include \
    include \
    ../QtOgre/include

LIBS *= -L"../lib"
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
    CONFIG += console
    LIBS *= -L"$(OGRE_BUILD)/lib/debug"
    LIBS *= -lOgreMain_d -lQtOgred
}
CONFIG(release, debug|release) {
    LIBS *= -L"$(OGRE_BUILD)/lib/release"
    LIBS *= -lOgreMain -lQtOgre
}

SOURCES += main.cpp
