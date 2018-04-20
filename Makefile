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

# Rules
all: bindir objdir nix

nix: $(SRCS)
	g++ -o $(BINDIR)/$(PROJECT_NAME) $^ 

bindir:
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi

objdir:
	if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi

clean:
	if [ -d $(BINDIR) ] ; then rm -rf $(BINDIR) ; fi
	if [ -d $(OBJDIR) ] ; then rm -rf $(OBJDIR) ; fi
