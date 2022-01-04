# Zork I: The Great Underground Empire
# (c) 1980 by INFOCOM, Inc.
# C port and parser (c) 2021 by Donnie Russell II

# This source code is provided for personal, educational use only.
# You are welcome to use this source code to develop your own works,
# but the story-related content belongs to the original authors of Zork.


CC = gcc
CFLAGS = -O3
LDFLAGS = -s


default: Zork1C

Zork1C: _text.o _vocab.o _game.o _parser.o _villains.o utility.o
	$(CC) $(LDFLAGS) _text.o _vocab.o _game.o _parser.o _villains.o utility.o -o Zork1C


_text.o: _text.cpp def.h
	$(CC) $(CFLAGS) -c _text.cpp -o $@

_vocab.o: _vocab.cpp def.h
	$(CC) $(CFLAGS) -c _vocab.cpp -o $@

_game.o: _game.cpp def.h _enum.h
	$(CC) $(CFLAGS) -c _game.cpp -o $@

_parser.o: _parser.cpp def.h _enum.h
	$(CC) $(CFLAGS) -c _parser.cpp -o $@

_villains.o: _villains.cpp def.h _enum.h
	$(CC) $(CFLAGS) -c _villains.cpp -o $@

utility.o: utility.cpp def.h _enum.h
	$(CC) $(CFLAGS) -c utility.cpp -o $@


_enum.h: gen_enum_h
	./gen_enum_h


_game.cpp: _strings.txt

_parser.cpp: _strings.txt

_text.cpp: _strings.txt

_villains.cpp: _strings.txt

_strings.txt: gen_text_cpp
	./gen_text_cpp


_vocab.cpp: gen_vocab_cpp
	./gen_vocab_cpp


gen_enum_h: descriptions.o gen_enum_h.o
	$(CC) $(LDFLAGS) descriptions.o gen_enum_h.o -o $@

gen_text_cpp: descriptions.o gen_text_cpp.o
	$(CC) $(LDFLAGS) descriptions.o gen_text_cpp.o -o $@

gen_vocab_cpp: gen_vocab_cpp.o
	$(CC) $(LDFLAGS) gen_vocab_cpp.o -o $@


descriptions.o: descriptions.cxx def.h
	$(CC) $(CFLAGS) -c descriptions.cxx -o $@

gen_enum_h.o: gen_enum_h.cxx def.h
	$(CC) $(CFLAGS) -c gen_enum_h.cxx -o $@

gen_text_cpp.o: gen_text_cpp.cxx def.h
	$(CC) $(CFLAGS) -c gen_text_cpp.cxx -o $@

gen_vocab_cpp.o: gen_vocab_cpp.cxx def.h _enum.h
	$(CC) $(CFLAGS) -c gen_vocab_cpp.cxx -o $@
