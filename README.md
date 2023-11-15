# Leaf Operating System

a lightweight multitasking-graphical operating system designed for small embedded system, it features :

1. multitasking capability, pre-emptive with memory protection
2. integrated Graphical User Interface
3. integrated image decoder based on libpng and libjpg
4. support sslv3, tls1.1 and tls1.2 based on woflSSL library
5. OrbWeaver virtual machine to run user application on top of operating system without MMU support
6. Inter Process Communication between task (enabling hybcrid kernel, instead of monolithic)
7. ARM Cortex M4 (STM32F407, STM32F413), ARM Cortex M7 (STM32F746 - Discovery Board, STM32F765) support
8. integrated file system based on FAT32 file system (sd card access)
9. Wi-FI (ESP32 Module), ISO7816, Global Platform, ISO14443, GSM (SIM800 compatible), USB Device, SD CArd, I2C, 8080 LCD (FSMC)

Multitasking+GUI preview
[![Watch the video](https://img.youtube.com/vi/jQXdla5kHUI/hqdefault.jpg)](https://www.youtube.com/embed/jQXdla5kHUI)

FAT32 File System access on SD Card
[![Watch the video](https://img.youtube.com/vi/PyfjOimzPGQ/hqdefault.jpg)](https://www.youtube.com/embed/PyfjOimzPGQ)

Signal Analyzer with Fast Fourier Transform 
[![Watch the video](https://img.youtube.com/vi/O3enWRYGT20/hqdefault.jpg)](https://www.youtube.com/embed/O3enWRYGT20)

USB Microphone (implementing isochronous packet transfer for audio streaming)
[![Watch the video](https://img.youtube.com/vi/GIT_wHkBR8U/hqdefault.jpg)](https://www.youtube.com/embed/GIT_wHkBR8U)

due to the nature of library being used licensed under GPLv2, as copyleft product then this project also licensed under GPLv2


