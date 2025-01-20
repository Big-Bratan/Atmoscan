<a id="readme-top"></a>
<div align="center">
  <h1>Atmoscan</h1>
  <p>
    Air Quality Control Monitor.
  </p>


<!-- Badges -->
<p>
  <a href="https://github.com/Big-Bratan/Atmoscan/graphs/contributors">
    <img src="https://img.shields.io/github/contributors/Big-Bratan/Atmoscan" alt="contributors" />
  </a>
  <a href="">
    <img src="https://img.shields.io/github/last-commit/Big-Bratan/Atmoscan" alt="last update" />
  </a>
  <a href="https://github.com/Big-Bratan/Atmoscan/network/members">
    <img src="https://img.shields.io/github/forks/Big-Bratan/Atmoscan" alt="forks" />
  </a>
  <a href="https://github.com/Big-Bratan/Atmoscan/stargazers">
    <img src="https://img.shields.io/github/stars/Big-Bratan/Atmoscan" alt="stars" />
  </a>
  <a href="https://github.com/Big-Bratan/Atmoscan/issues/">
    <img src="https://img.shields.io/github/issues/Big-Bratan/Atmoscan" alt="open issues" />
  </a>
  <a href="https://github.com/Big-Bratan/Atmoscan/blob/master/LICENSE">
    <img src="https://img.shields.io/github/license/Big-Bratan/Atmoscan.svg" alt="license" />
  </a>
</p>

</div>

<br />

<!-- Table of Contents -->
<details open>
  <summary>Table of Contents</summary>

- [About](#about)
- [Features](#dart-features)
- [Components](#components)
- [Wiring](#wiring)
- [Installation](#installation)
- [Usage](#usage)
- [License](#license)

</details>

<!-- About the Project -->

## About

<div align="center">
  <img src="https://placehold.co/600x400?text=Screenshot+here" alt="screenshot" />
</div>

Atmoscan is an Arduino-based air quality monitoring system designed to measure and display various environmental
parameters in real-time. The device provides continuous monitoring of air quality indicators and alerts users when air
quality deteriorates to unhealthy levels.

<!-- Features -->

### :dart: Features

- Real-time monitoring of multiple air quality parameters
- OLED display showing current readings
- Visual and auditory alerts for poor air quality
- Historical data logging capabilities

<!-- Components -->

## Components

### Required Hardware

- Arduino Uno
- DHT11 Temperature & Humidity Sensor
- MH-Z19 CO2 Sensor
- PMS5003 Particule Sensor
- 128*64 OLED Display (I2C)
- LEDs and Buzzer for alerts
- Micro SD SPI Reader

<!-- Libraries -->

### Libraries Required

This project uses Yarn as package manager

```cpp
  cpp
#include
<Wire.h>
#include
<SoftwareSerial.h>
#include
"MHZ19.h"
#include
<dht.h>
```

<!-- Wiring -->

## Wiring

[wiring diagrams will go here]

### Pin Connections

- DHT11 → Pin
- MH-Z19 → Pins 12 (RX), 13 (TX)
- PMS5003 → Analog Pin
- OLED Display → I2C (SDA: A4, SCL: A5)
- SPI reader → Pins

<!-- Installation -->

## Installation

1. Clone the repository

```bash
  git clone https://github.com/Big-Bratan/Atmoscan.git
```

2. Install required libraries in Arduino IDE
3. Connect components according to the wiring diagram
4. Upload the code to your Arduino

<!-- Usage -->

## Usage

1. Power up the device
2. The OLED display will show the startup logo followed by current readings
3. Monitor displays:

- CO2 levels (ppm)
- Temperature (°C)
- Humidity (%)
- Particulate Matter (PM2.5)
- Air Quality Index

<!-- Acknowledgments -->
<!--
## :gem: Project resources

Use this section to mention useful resources and libraries that you have used in your projects.

- [Emoji Cheat Sheet](https://github.com/ikatyang/emoji-cheat-sheet/blob/master/README.md#travel--places)
- [Readme Template](https://github.com/othneildrew/Best-README-Template)

## License

Distributed under the MIT License. See `LICENSE` for more information.
-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>
