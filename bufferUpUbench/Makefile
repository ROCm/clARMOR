
HERE := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CC),cc)
CC=gcc
endif

CSOURCES=$(shell find ${CURDIR} -name "*.c" -type f)
CPPSOURCES=$(shell find ${CURDIR} -name "*.cpp" -type f)

LIB_DIR=$(AMDAPPSDKROOT)/lib/x86_64
INC_DIR=$(AMDAPPSDKROOT)/include

INCLUDE_FLAGS=-I$(AMDAPPSDKROOT)/include

COBJECTS=$(CSOURCES:.c=.o) 
CPPOBJECTS=$(CPPSOURCES:.cpp=.o)

SOURCES=$(CSOURCES) $(CPPSOURCES)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

CXXFLAGS+=-lstdc++ -Wl,--no-as-needed -rdynamic -g -pthread -O3 -Wall -Werror
#CXXFLAGS+=$(INCLUDE_FLAGS) -L$(AMDAPPSDKROOT)/lib/x86_64 -lOpenCL

.PHONY: all
all: $(SOURCES) bufferUp.exe

.PHONY: clean
clean:
	$(RM) -f *.exe *.o *~ *.out


#bufferUp.exe: $(CPPSOURCES) $(CSOURCES)
#	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -o $@ $<
bufferUp.exe: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL $^ -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@

%.o : %.c
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@

#%.cpp %.c %.o:
#	$(CC) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@

#$(OBJECTS): $(SOURCES)
#	$(CC) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@
#
#$(CPPOBJECTS): $(CPPSOURCES)
#	$(CC) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@

#$(COBJECTS): $(CSOURCES)
#	$(CC) $(CXXFLAGS) -I$(INC_DIR) -L$(LIB_DIR) -lOpenCL -c $< -o $@




