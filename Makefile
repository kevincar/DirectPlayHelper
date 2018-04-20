# make file for DPServer
# Author: kevincar

# Working Directory
SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.cpp)

OBJDIR := obj
OBJS :=

BINDIR := bin

# Project Settings
PROJECT_NAME := DPServer

CFLAGS :=
CXXFLAGS :=
CPPFLAGS :=
LDFLAGS :=

# Cross Compile Settings
mingw := /usr/local/bin/i686-w64-mingw32-g++
minCPPFLAGS := -static -static-libstdc++ -static-libgcc

# Rules
all: bindir objdir nix

nix: $(SRCS)
	g++ -o $(BINDIR)/$(PROJECT_NAME) $^ 

winnix: $(SRCS)
	$(mingw) $(minCPPFLAGS) -o $(BINDIR)/$(PROJECT_NAME).exe $^

bindir:
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi

objdir:
	if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi

clean:
	if [ -d $(BINDIR) ] ; then rm -rf $(BINDIR) ; fi
	if [ -d $(OBJDIR) ] ; then rm -rf $(OBJDIR) ; fi
