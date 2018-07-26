# talking-fiber
Firmware for optic fiber activity detection. Based on M3 MCU

# Introduction

This project covers the development of a firmware for an embedded device of Sinapse for the detection of activity in an optic fiber. The device is powered by a battery, so the FW should reach very low consumption.

The firmware it is based in an already working code for M3 MCUs with an API Engine solving the MQTT communications based in M95

# Resume of tasks to be performed

1. Adapt existing M3 code to new talking fiber board. Normally it is only a GPIO modification. The board connections are specified in the HW-FW document available in the project
2. Development of SB function in order to read status of the fiber
3. To create a new main that provides the required algorith. Basically read SB and publish status. TODO: Flow diagram 
4. Optimize the behaviour to consume the minimum power as possible
