INC = *.h
SRC := \
	expression_parser.cpp \
	dispatcher.cpp \
	parser_types.cpp \
	lexer.cpp \
	parser.cpp \
	rl.cpp

CXX = g++
# Note: Implicit link rule uses CC (not CXX) to build exe from .o's
CC := $(CXX)
CXXFLAGS = -std=c++2a $(if $(DEBUG),-g)
LDLIBS = -lreadline

all: parser

# Note: As long as program name matches stem of one of the source files,
# implicit link rule can be usd.
parser: $(SRC:cpp=o)

clean:
	rm -f *.o

