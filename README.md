#rover

## /firmware
source for avr/arduino based firmware used in the project

### Using ino build environment for arduino

- ino.ini: arduino board setup file
- lib/: arduino libraries in c++
- src/: arduino source files

#### Build firmware
```
ino build
```
#### Upload Firmware
```
ino upload
```
#### Serial monitor (picocom)
start: ```ino serial```, stop: ```C-a C-x```

## /software
source for the high level software running on raspberry-pi

## /hardware
information about hardware configuration and hardware documents
