LFLAGS = -O3 -lm `pkg-config opencv4 --cflags --libs`

.SUFFIXES:

all: camera cameras

camera: camera.c
	g++ -o $@ camera.cpp $(LFLAGS)

cameras: cameras.cpp
	g++ -o $@ cameras.cpp $(LFLAGS)

one: camera
	./camera

two: cameras
	./cameras

clean:
	rm -f camera cameras
