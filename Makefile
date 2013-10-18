CC = gcc
CCFLAGS = -W -Wall -g -DDEBUG

SOURCES_DIR = Sources
OBJECTS_DIR = Objects
BINARIES_DIR = Binaries

DEPENDENCIES_SHARED = $(SOURCES_DIR)/Elliptic_Curves.h $(SOURCES_DIR)/Point.h $(SOURCES_DIR)/Network.h

OBJECTS_SHARED = $(OBJECTS_DIR)/Elliptic_Curves.o $(OBJECTS_DIR)/Point.o $(OBJECTS_DIR)/Network.o
OBJECTS_TESTS = $(OBJECTS_DIR)/Tests.o
OBJECTS_DIFFIE_HELLMAN = $(OBJECTS_DIR)/Diffie_Hellman.o
LIBRARIES = -lgmp

all: $(OBJECTS_SHARED) $(OBJECTS_TESTS) $(OBJECTS_DIFFIE_HELLMAN)
	@# Compile tests
	$(CC) $(CCFLAGS) $(OBJECTS_SHARED) $(OBJECTS_TESTS) -o $(BINARIES_DIR)/Tests $(LIBRARIES)
	@# Compile classic Diffie-Hellman algorithm
	$(CC) $(CCFLAGS) $(OBJECTS_SHARED) $(OBJECTS_DIFFIE_HELLMAN) -o $(BINARIES_DIR)/Diffie_Hellman $(LIBRARIES)

release: CCFLAGS = -W -Wall -O3 -fexpensive-optimizations -ffast-math -Wl,--strip-all
release: all

#---------------------------------------------------------------------------------------------------------------------------------------------------
# Base objects used by all programs
#---------------------------------------------------------------------------------------------------------------------------------------------------
$(OBJECTS_DIR)/Elliptic_Curves.o: $(SOURCES_DIR)/Elliptic_Curves.c $(SOURCES_DIR)/Elliptic_Curves.h $(SOURCES_DIR)/Point.h
	$(CC) $(CCFLAGS) -c $(SOURCES_DIR)/Elliptic_Curves.c -o $(OBJECTS_DIR)/Elliptic_Curves.o

$(OBJECTS_DIR)/Point.o: $(SOURCES_DIR)/Point.c $(SOURCES_DIR)/Point.h
	$(CC) $(CCFLAGS) -c $(SOURCES_DIR)/Point.c -o $(OBJECTS_DIR)/Point.o

$(OBJECTS_DIR)/Network.o: $(SOURCES_DIR)/Network.c $(SOURCES_DIR)/Network.h
	$(CC) $(CCFLAGS) -c $(SOURCES_DIR)/Network.c -o $(OBJECTS_DIR)/Network.o

#---------------------------------------------------------------------------------------------------------------------------------------------------
# Generic tests
#---------------------------------------------------------------------------------------------------------------------------------------------------
$(OBJECTS_DIR)/Tests.o: $(SOURCES_DIR)/Tests.c $(DEPENDENCIES_SHARED)
	$(CC) $(CCFLAGS) -c $(SOURCES_DIR)/Tests.c -o $(OBJECTS_DIR)/Tests.o

#---------------------------------------------------------------------------------------------------------------------------------------------------
# Diffie-Hellman key exchanging
#---------------------------------------------------------------------------------------------------------------------------------------------------
$(OBJECTS_DIR)/Diffie_Hellman.o: $(SOURCES_DIR)/Diffie_Hellman.c $(DEPENDENCIES_SHARED)
	$(CC) $(CCFLAGS) -c $(SOURCES_DIR)/Diffie_Hellman.c -o $(OBJECTS_DIR)/Diffie_Hellman.o

clean:
	rm -f $(OBJECTS_DIR)/* $(BINARIES_DIR)/*