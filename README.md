# About

WeatherMonitor is for receiving data from a Lacrosse TX7 transitter using a Raspberry PI.
The data can then be pushed to a remote server for further processing.

Based on the following:
http://forum.hardware.fr/hfr/Discussions/Loisirs/arduino-topic-unique-sujet_93606_29.htm#t32501473
http://www.f6fbb.org/domo/sensors/tx3_th.php

Requires the [wiringpi](https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/) library.

## Usage

### From command line
make
./weatherd -n GPIO_PIN -i EXPECTED_SENSOR_ID

### As service
edit default.weatherd to suite your needs
make
make install
sudo /etc/init.d/weatherd start

