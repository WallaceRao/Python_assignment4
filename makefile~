# Makefile for 159.735 Assignment 3
#

CPP = g++
NVCC = nvcc

# Use this for your CUDA programs
NVCC = nvcc

# FLAGS for Linux
CFLAGS = -w -O3 -D_FORCE_INLINES

# Locally compiled modules
OBJS = fitsfile.o lenses.o

# Link to CFITSIO libraries - modify these accordingly
LIBP = -L/home/s16093645/Downloads/cfitsio/lib
INCP = -I/home/s16093645/Downloads/cfitsio/include


LIBS = -lcfitsio -lm

MODS = $(INCP) $(LIBP) $(LIBS) $(OBJS) 
MODSCU = $(INCP) $(LIBP) $(LIBS) fitsfile.o

BINS = lens_demo

all : $(BINS)

clean :
	rm -f $(BINS)
	rm -f *.o

# Demo program. Add more programs by making entries similar to this


lensing : lensing.cu fitsfile.o
	${NVCC} $(CFLAGS) -o lensing lensing.cu $(MODSCU)



# Modules compiled and linked separately
fitsfile.o : fitsfile.cpp fitsfile.h
	${CPP} $(CFLAGS) $(INCP) -c fitsfile.cpp



