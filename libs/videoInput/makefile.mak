!ifndef BINTYPE
BINTYPE = LIB
!endif

DEL = del /Q /F

CC		= cl
CFLAGS	= /nologo /W3 /O2 /Ob2 /MD /GX /D"NDEBUG" /D"WIN32" /c
CFLAGS_INCDIRS = /I"d:\libs\dshow\include"
COMPILER= $(CC) $(CFLAGS) $(CFLAGS_INCDIRS) /Fo

LD		= link

!if "$(BINTYPE)"=="LIB"
LDFLAGS_DLL = -lib /subsystem:windows
EXT = .lib
!else if "$(BINTYPE)"=="DLL"
LDFLAGS_DLL = /subsystem:windows /dll /OPT:REF
EXT = .dll
!endif

LDFLAGS_LIBDIR = /LIBPATH:"d:\libs\dshow\lib"
LDFLAGS_LIBS = strmbase.lib strmiids.lib
#LDFLAGS_LIBS = strmbase.lib strmiids.lib ole32.lib oleaut32.lib
LDFLAGS = /nologo /machine:i386
LINKER  = $(LD) $(LDFLAGS_DLL) $(LDFLAGS) $(LDFLAGS_LIBDIR) $(LDFLAGS_LIBS) /out:

SRCS	= videoInput.cpp
HDRS	= videoInput.h
OBJS	= videoInput.obj

TARGET_NAME = videoInput
TARGET  = $(TARGET_NAME)$(EXT)

$(TARGET) : $(OBJS)
	@echo ======== Linking $@...
	$(LINKER)"$@" $(OBJS)
	
$(OBJS) : $(SRCS) $(HDRS)

.cpp.obj:
	@echo ======== Compiling $@...
	$(COMPILER)"$@" $<

.c.obj:
	@echo ======== Compiling $<...
	$(COMPILER)"$@" $<

clean:
	$(DEL) *.obj *.pdb 
	$(DEL) $(TARGET_NAME).lib $(TARGET_NAME).dll
