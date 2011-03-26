#######################################
# Basic Makefile with variables

RM 		= rm -f
CC		= g++
LD 		= $(CC)
CFLAGS	= -Wall -O
LDFLAGS =
LIBS 	=
COMPILE = $(CC) $(CFLAGS) -c

###################
TARGET	= hi.exe

SOURCE_FILES = $(wildcard *.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SOURCE_FILES))

###################
# Target Rules
###################
.SUFFIXES : .h .cpp .o
.cpp.o:
	@echo "------------------"
	$(RM) $*.o
	$(COMPILE) $< -o $@

%.o : %.cpp %.h
	$(COMPILE) $< -o $@

%.o : %.cpp
	$(COMPILE) $< -o $@

###################
# All
all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $(TARGET) $(OBJS)

###################
# Clean
#.PHONY, depraved - virtual directory, avoid error when cleano file exists
cleano:
	$(RM) $(OBJS)
	$(RM) *.o
	$(RM) *.s

cleane:
	$(RM) $(TARGET)
	$(RM) *.exe

clean: cleano cleane

###################
#main.o: main.cpp
#	$(COMPILE) -o main.o main.cpp
