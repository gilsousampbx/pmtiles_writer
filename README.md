# PMTiles Writer
Experiment to convert to the `Writer` class from [PMTiles project](https://github.com/protomaps/PMTiles/blob/main/python/pmtiles/writer.py)  into a C++ class. The create program to test the functionality is based on the example [create_raster_example.py](https://github.com/protomaps/PMTiles/blob/main/python/examples/create_raster_example.py)

When the program is run it should create a file called `stamen_toner_maxzoom3.pmtiles` in the directory it was run.

## Build

Create a `build` folder
```
mkdir build
```

Initialize cmake
```
cmake -B ./build -S .
```

Got into the build dir and run make

```
cd build
make
```
