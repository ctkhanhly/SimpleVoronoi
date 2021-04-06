

PROGNAME = simple_voronoi

OBJECTS = simple_voronoi.o

CXX      ?= g++
CXXFLAGS ?= -O2 -fpermissive
INSTALL  ?= install
INSTALL_PROGRAM ?= $(INSTALL) 

# https://www.gnu.org/prep/standards/html_node/Directory-Variables.html#Directory-Variables
#https://releases.llvm.org/7.0.0/projects/libcxx/docs/UsingLibcxx.html#using-filesystem-and-libc-fs
# prefix      ?= /usr/local
prefix      ?= /Volumes/lySSD
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin

override CXXFLAGS += -std=c++17 -Wall -fexceptions
override LDFLAGS  += -pthread
# override LDLIBS   += -stdlib=libc++
# override LCEXP 	  += -lc++experimental
# override LDLIBS   += -lstdc++fs

all: $(PROGNAME)

tiv.o: CImg.h

$(PROGNAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS) 

install: all
	$(INSTALL_PROGRAM) $(PROGNAME) $(DESTDIR)$(bindir)/$(PROGNAME)

clean:
	$(RM) -f $(PROGNAME) *.o

.PHONY: all install clean

