LDLIBS = $(`pkg-config --libs curl`) -ltiff
CC = g++

all: cluster_test tiff_loader_test

cluster_test.o: cluster_test.cpp clusters.h
clusters.o: clusters.cpp clusters.h

cluster_test: cluster_test.o clusters.o


tiff_loader_test.o: tiff_loader_test.cpp tiff_loader.h err.h cache.h
