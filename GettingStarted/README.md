[![LoraType](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType.png "LoraType")](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType.png "LoraType")


### Additional features for the community

[![LoraTypeForDiy](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType_ForDIY.png "LoraTypeForDiy")](https://raw.githubusercontent.com/AutomationArt/LoraType/main/Image/LoraType_ForDIY.png "LoraTypeForDiy")

For fast and convenient programming, we suggest using clamp *(you can also use a soldering RXD_2 | TXD_2 | GND )*
[![LoraTypeForDiy](https://raw.githubusercontent.com/AutomationArt/LoraType/main/GettingStarted/img/LoraType_start1.png "LoraTypeForDiy")](https://raw.githubusercontent.com/AutomationArt/LoraType/main/GettingStarted/img/LoraType_start1.png "LoraTypeForDiy")

For full internal debugging of [JTAG](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/index.html "JTAG") debugging, you will need the official [ESP-PROG](https://espressif-docs.readthedocs-hosted.com/projects/espressif-esp-iot-solution/en/latest/hw-reference/ESP-Prog_guide.html "ESP-PROG") debugger  *(pay attention to JP1-JP4 and the general scheme)*

[![LoraTypeForDiy](https://raw.githubusercontent.com/AutomationArt/LoraType/main/GettingStarted/img/LoraType_start2.png "LoraTypeForDiy")](https://raw.githubusercontent.com/AutomationArt/LoraType/main/GettingStarted/img/LoraType_start2.png "LoraTypeForDiy")


**DIY Opportunities**
- Fast firmware (UART) - ***TXD_2 | RXD_2 | GND***
- Adding your own sensors to the device (via I2C bus, ***SCL | SDA | GND | 3.3V***) 
- JTAG debugging (***TCK | TDO | TMS | TDI | GND | 3.3V***)
- Li-po battery change (***Bat + Bat-***)
- Using integrated antenna (need to add 2 components ***La | Ca***)
- Connection of external power generators (Energy harvesters and etc,***Solar input + -***) 
- Pin **P4** can be used to control devices or be used as a touch button (esp32 built-in function)


[Documentation for all components](https://github.com/AutomationArt/LoraType/tree/main/Documents "Documentation for all components") :arrow_left:   

------------

### How to Firmware 
The basic sample firmware is created in [Platformio](https://platformio.org "Platformio"), but all libraries with examples are available in the [Lib folder](https://github.com/AutomationArt/LoraType/tree/main/Software/lib "Lib folder") and can be run and compiled in the [Arduino IDE](https://www.arduino.cc/en/software "Arduino IDE")

- Pio Home - Quick Project - Open Project 
- Choose a folder LoraType/Software/
- Change file Platformio.io
- Change the code to suit your needs
- PlatformIO: Build -> Upload