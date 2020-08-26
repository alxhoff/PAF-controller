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

*Note: by default the IDF install scripts place the toolchain and openocd
version into the directory `~/.espressif`, I have made the assumption that this
is where the tooling is always located in the CMake.

```
mkdir ~/esp 
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
```

#### Python Packages

I had troubles installing proper versions of certain packages to satisfy the 
IDF's dependencies. Namely the packages `pygdbmi` and `gdbgui`.

To fix such dependencies manually uninstall the packages using `pip`

```
pip uninstall pygdbmi
```

and then install the specific version required (given by IDF script output)

```
pip install -I pygdbmi==0.9.0.2
```

You might need to play with the order in which you install the packages as those with
dependencies might install the others and with a incorrect version.

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

## Debugging Using JLink

```
make debug
```

Additionally you can specify your own OpenOCD version to use using `-DESP_OPENOCD`.
This for instance can be used to use the AUR OpenOCD version, although CMake first
checks if this version of OpenOCD has been installed before searching for the
toolchain version in `~/espressif`.

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
