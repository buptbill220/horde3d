GXX = gcc
#GCC_FLAGS = -g -O2 -Wreorder -D_DEBUG
GCC_FLAGS = -g -O2 -Wreorder
SUFFIX = c cpp cc cxx

BINDIR = bin
TARGET = $(BINDIR)/ColladaConv
SRCS = $(wildcard *.cpp)
OBJDIR = objs
DIR = $(notdir $(SRCS))
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(DIR))

INC = -I ../Shared/
LIB = -lpthread -ldl -lm -lstdc++


all: $(TARGET)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(OBJDIR)
	$(GXX) $(GCC_FLAGS) $(INC) -c $< -o $@

$(TARGET): $(OBJS)
	mkdir -p $(BINDIR)
	$(GXX) $(GCC_FLAGS) $(OBJS) -o $@ $(LIB)

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)
