CC      = g++
LD      = g++
MKDIR_P = mkdir -p
RM_F    = rm -f
RMDIR   = rmdir

SRCDIR   = ./src
BUILDDIR = ./build

CFLAGS  = --std=c++14 -Wall -Werror $(shell sdl-config --cflags) 
LDFLAGS = $(shell sdl-config --libs) 
SRC     = $(wildcard src/*.cpp)
OBJ     = $(patsubst src/%.cpp,build/%.o,$(SRC))
PRODUCT = conways

vpath %.cpp $(SRCDIR)

all: $(BUILDDIR) $(BUILDDIR)/$(PRODUCT)

$(BUILDDIR):
	$(MKDIR_P) $(BUILDDIR)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(PRODUCT): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	$(RM_F) $(BUILDDIR)/$(PRODUCT) $(OBJ)
	$(RMDIR) $(BUILDDIR)

