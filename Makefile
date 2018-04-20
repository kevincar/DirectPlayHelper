# make file for DPServer
# Author: kevincar

# Working Directory
SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.cpp)

OBJDIR :=
OBJS :=

BINDIR := bin

# Project Settings
PROJECT_NAME := "DPServer"

CFLAGS :=
CXXFLAGS :=
CPPFLAGS :=
LDFLAGS :=

# Rules
all:
	echo "Nothing yet..."

nix: $(SRCS)
	g++ -o $(BINDIR)/$(PROJECT_NAME) @^
