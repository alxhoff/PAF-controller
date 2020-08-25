# PAF-controller

Controller for the Personal Air Filter (PAF) system being developed at TUM EDA/RCS

## Setup 

*Note: this is the setup steps on an Arch Linux machine*

### Prereqs

From [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#get-started-get-prerequisites):

```
sudo pacman -S --needed gcc git make flex bison gperf python-pip cmake ninja ccache dfu-util
```

Set python3 as default

```
sudo ln -sf /usr/bin/python3.7 /usr/bin/python
```

### AUR toolchain

OpenOCD and toolchain from AUR

```
yaourt gcc-xtensa-esp32-elf-bin
yaourt openocd-esp32
```

### ESP-IDF

```
mkdir ~/esp 
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
```

### Environment Variables 

```
. $HOME/esp/esp-idf/export.sh
```

Make sure you're using bash.

## Test Build

```
cd $HOME/esp/esp-idf/examples/wifi/scan
make menuconfig #Configure SDK options
make flash monitor #With device plugged in it should appear as /dev/ttyUSB0)
```

## Building Project

```
mkdir build
cd build
cmake ..
make
```

## Flashing

```
make flash
```

## Additional Targets

### Astyle Format

```
cmake -DENABLE_ASTYLE=ON ..
make format
```
