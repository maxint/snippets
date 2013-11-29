NDK_BASE = /home/NDK/android-ndk-r7
TCDIR = $(NDK_BASE)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86
PREF = $(NDK_BASE)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-


CC = $(PREF)gcc
CPP = $(PREF)g++

INCS = -I$(NDK_BASE)/platforms/android-9/arch-arm/usr/include \
	   -I inc

LIBS = -L$(NDK_BASE)/platforms/android-9/arch-arm/usr/lib \
		-lc -lm -llog -lgcc \
		lib/android/libbar.so

CFLAGS = -nostdlib -fPIC -Wall -O2
CPPFLAGS =
LDFLAGS = -nostdlib -Wl,-shared,-Bsymbolic -Wl,--no-undefined

foo.so : foo.o
foo.o : foo.cpp

%.o : %.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS)$(INCS) -c -o $@ $<

%.so : %.o
	$(CPP) $(LDFLAGS) $^ $(LIBFLAGS) $(LIBS) -o $@

clean:
	@rm -f *.so
	@rm -f *.o
