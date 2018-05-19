LDLIBS = -ltiff -lproj -ljansson
CC = g++

all: cluster_test tiff_loader_test borders_test main

cluster_test.o: cluster_test.cpp clusters.h
clusters.o: clusters.cpp clusters.h

cluster_test: cluster_test.o clusters.o
borders_test: borders_test.o borders.o
borders_test.o: borders.h borders_test.cpp pt.h err.h
borders.o: borders.cpp pt.h err.h

tiff_loader_test.o: tiff_loader_test.cpp tiff_loader.h err.h cache.h corine_proj.h borders.h
tiff_loader_test: tiff_loader_test.o borders.o


main.o: main.cpp tiff_loader.h err.h cache.h corine_proj.h borders.h clusters.h
main: main.o borders.o clusters.o

