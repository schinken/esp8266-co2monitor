# ESP8266-co2monitor

## description

Interface an TFA Dostmann CO2 Monitor with ESP8266 (in our example a WEMOS D1 Mini) to connect it to the "Internet of Things". It publishes the CO2 measurement and temperature to a configures MQTT topic.

## compiling

* Rename settings.h.example to settings.h
* Open up your arduino IDE
* Configure your OTA, Wifi, MQTT Topics in the settings.h file
* Upload the compiled result to your Wemos D1 Mini

## wiring

Add a pin header to the existing PCB. Left to right: GND, Clock, Data, 5V:
<br>
<a href="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/pinheader.jpg?raw=true">
    <img alt="PIN Header" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/pinheader-thumb.jpg?raw=true">
</a>

Wire up your Wemos with 4 wires, connected to 5V, G, D2 and D1:
<br>
<a href="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wemos-wiring.jpg?raw=true">
    <img alt="Wemos Wiring" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wemos-wiring-thumb.jpg?raw=true">
</a>

Now connect your Wemos D1 mini to your co2 monitor:
<br>
<a href="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wiring.jpg?raw=true">
    <img alt="Final Wiring" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wiring-thumb.jpg?raw=true">
</a>

## dependencies

* PubSubClient
* Arduino/ESP8266

## notes

Don't use D4 and D3, because it causes power-on issues with wemos d1 mini: https://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
