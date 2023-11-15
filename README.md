# Leaf Operating System

a lightweight multitasking-graphical operating system designed for small embedded system, it features :

1. multitasking capability, pre-emptive with memory protection
2. integrated Graphical User Interface
3. integrated image decoder based on libpng and libjpg
4. support sslv3, tls1.1 and tls1.2 based on woflSSL library
5. OrbWeaver virtual machine to run user application on top of operating system without MMU support
6. Inter Process Communication between task (enabling hybcrid kernel, instead of monolithic)
7. ARM Cortex M4 (STM32F407, STM32F413), ARM Cortex M7 (STM32F746 - Discovery Board, STM32F765) support
8. support for STM32 graphical hardware accelerator (ChromART)
9. integrated file system based on FAT32 file system (sd card access)
10. Wi-FI (ESP32 Module), ISO7816, Global Platform, ISO14443, GSM (SIM800 compatible), USB Device, SD CArd, I2C, 8080 LCD (FSMC)

### Multitasking+GUI preview

demonstrate a multi-window graphical user interface running OrbWeaver application and system application simultaneously, windows switcher used for switching windows can be accessed from topbar

[![Watch the video](https://img.youtube.com/vi/jQXdla5kHUI/hqdefault.jpg)](https://www.youtube.com/embed/jQXdla5kHUI)

### FAT32 File System access on SD Card

this application shows how to browse sd card file system using eFAT file system library on LeafOS

[![Watch the video](https://img.youtube.com/vi/PyfjOimzPGQ/hqdefault.jpg)](https://www.youtube.com/embed/PyfjOimzPGQ)

### Signal Analyzer with Fast Fourier Transform 

an example of Fast Fourier Transform based on CMSIS-DSP library running on LeafOS, transformation done in realtime using audio signal from microphone

[![Watch the video](https://img.youtube.com/vi/O3enWRYGT20/hqdefault.jpg)](https://www.youtube.com/embed/O3enWRYGT20)

### USB Microphone (implementing isochronous packet transfer for audio streaming)

this application shows LeafOS acted as USB Device, capable of transmitting audio signal to PC through USB interface based on isochronous packet

[![Watch the video](https://img.youtube.com/vi/GIT_wHkBR8U/hqdefault.jpg)](https://www.youtube.com/embed/GIT_wHkBR8U)

due to the nature of the library being used, licensed under GPLv2, as copyleft product then this project also licensed under GPLv2

## Requirements

1. Keil IDE v4 or v5
2. CMSIS v4.3 and CMSIS-DSP v1.4.6 (haven't tried with lower version, using STM32 Standard Peripheral Library v1 might need some changes)
3. JTAG ULINK/JLINK or STLink for debugging purpose


## Build the project

1. open MDK ARM IDE Project (.uvproj) using Keil IDE
2. configure the device and CMSIS support (include CMSIS-DSP library for FFT)
3. check config.h, defs.h if you need to activate some features such as switching LCD display

## Porting

all files Users/Interfaces with prefix if_xxxx and APIS with the same prefix provides Hardware Abstraction Layer for operating system, in order to port APIs to different hardware, developer must provide APIs with the same input paramters and return value, check ili932x.c, if_gui.c and if_gui_fmc.c to see how it works  


