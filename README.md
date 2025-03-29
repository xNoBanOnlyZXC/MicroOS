<div align="center">
    <h1>MicroOS</h1>
    Small OS for ESP-like microcontroller.
    <p>Made by <bold>~$ sudo++</bold></p>
    <img alt="code size" src="https://img.shields.io/github/languages/code-size/xnobanonlyzxc/microos?style=for-the-badge">
    <img alt="repo stars" src="https://img.shields.io/github/stars/xnobanonlyzxc/microos?style=for-the-badge">
    <img alt="repo stars" src="https://img.shields.io/github/commit-activity/w/xnobanonlyzxc/microos?style=for-the-badge">
</div>

---
### How to initialize?

1. Install [Visual Studio Code](https://code.visualstudio.com/download) & [Python (For example 3.11)](https://www.python.org/downloads/release/python-3119/)
2. Install `PlatformIO` extension
3. Download source `.zip` file, unzip it
4. Initialize PlatformIO core, go to home > open project > choose folder
5. Platformio automatically install all dependencies, just wait

---
## How to flash?

### From PlatformIO
1. Connect your ESP to PC, make sure that the computer has detected the ESP correctly
2. Click `→` mark on bottom panel
3. **FOR SOME ESP MODELS:** Hold `BOOT` button on ESP until flash begins

### From console
1. Download `.bin` file from [releases page](https://github.com/xNoBanOnlyZXC/MicroOS/releases)
2. Make sure you have installed Python & esptool (see below)
3. Use esptool to flash ESP:
```bash
esptool.py --chip esp32 --port <PORT> --baud <BAUD_RATE> write_flash <ADDRESS> <FILE.bin>
```
Example (windows):
```bash
esptool.py --chip esp32 --port COM3 --baud 921600 write_flash 0x1000 firmware.bin
```
Example (linux):
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash 0x1000 firmware.bin
```

---
## How to install esptool?
1. Make sure you have installed Python
2. Run this in console:
```bash
pip install -U esptool
```

---
## Port definition
### On windows
Open Device Manager and find the Ports (COM and LPT) section. The port will be listed as COMx, where x is the port number

### On Linux
Use terminal command:
```bash
ls /dev/tty.*
```
Typically, the port will look like /dev/ttyUSB0 or /dev/tty. SLAB_USBtoUART

---
## How to use MicroOS?

idk lmao check sources
