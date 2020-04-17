
INCFLAG		+= -I./include
DEBUGFLAG	= -g -O0 -Wall
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG)
LDFLAGS	    	+= -lseekware `pkg-config --libs libusb-1.0`
CXXFLAGS	+= $(CFLAGS)
CPPFLAGS	+= $(CFLAGS)

TARGET	= seekware-fbdev
OBJECTS	= objs/seekware-fbdev.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LDFLAGS)

objs/%.o:	src/%.cpp
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

objs/%.o:	src/%.c
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean:
	@rm -rf core *~  $(TARGET) $(OBJECTS) $(LIB) $(LIBOBJ) src/*~ include/*~

