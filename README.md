# Windmap

Dense and fast windmap

## Omegalib module

Tested with Omegalib 13.1

Compile:
```
cd windmap
mkdir build
cd build
cmake ..
make
```

Run with python script:
```

```

## Standalone app

Tested with MacOS 10.13.1

```
cd windmap/app
mkdir build
cd build
cmake .. -DSTANDALONE_APP=On [-G Xcode]
make (or run build in XCode)
```