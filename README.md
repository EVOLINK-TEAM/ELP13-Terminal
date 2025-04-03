<h1 align="center">Terminal</h1>

<div style="display: flex; justify-content: flex-end; align-items: center;">
    <img src="images/evolink.png" alt="Logo" style="height: 2em;margin-right:1.5em">
    <span style="vertical-align:middle">ELP-13 FROM OXTRE..</span>
</div>

## Overview

A oled smart watch

<p align="center">
 <img border="1px" width="80%" src="./images/main.jpg" alt="main.jpg">
</p>

<p align="center">
 <img border="1px" width="80%" src="./images/show1.gif" alt="show1.gif">
</p>

## Features

This project aims to use a similar `CLI` interface on embedded devices that programmers love

- control borad
  1. RGB : ws2812b
  2. POWER : TP4059 + TPS78233
  3. MCU : ESP32-PICO-D4
  4. RTC : PCF8563T
  5. ACCE : LSM6DS3TR
  6. key : Wheel Switch
  7. BAT : 303030 280ma

- screen borad
  1. OLED : 1.3 ssd1306
  2. SENSORS : BH1750+BME280+QMC5883L
  3. BUZZER : MLT-5020
  4. GPS : GP-02
  5. Infrared : IR12+PT12

## Program

Serial debugging and programming of ESP32 using `typec` interface (built-in CP2102)

Some libraries may not be found on the Arduino library manager, you can get these libraries through the following methods:  

[Evlk-Terminal-Library](https://github.com/EVOLINK-TEAM/Evlk-Terminal-Library)  
[Evlk-Shell-Library](https://github.com/EVOLINK-TEAM/Evlk-Shell-Library)  

## Use

Press the button or shake your arm to display the time

## And More?

<p align="center">
 <img border="1px" width="30%" src="./images/pcb4.png" alt="pcb4.png">
 <img border="1px" width="30%" src="./images/pcb5.png" alt="pcb5.png">
 <img border="1px" width="30%" src="./images/pcb6.png" alt="pcb6.png">
</p>

<p align="center">
 <img border="1px" width="30%" src="./images/pcb.png" alt="pcb.png">
 <img border="1px" width="30%" src="./images/pcb2.png" alt="pcb2.png">
 <img border="1px" width="30%" src="./images/pcb3.png" alt="pcb3.png">
</p>
