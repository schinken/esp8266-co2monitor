# ESP8266-co2monitor

## description

Interface an TFA Dostmann CO2 Monitor with ESP8266 (in our example a WEMOS D1 Mini) to connect it to the "Internet of Things". It publishes the CO2 measurement and temperature to a configures MQTT topic.

## compiling

* Rename settings.h.example to settings.h
* Open up your arduino IDE
* Configure your OTA, Wifi, MQTT Topics in the settings.h file
* Upload the compiled result to your Wemos W1 Mini

## wiring

Add a pin header to the existing PCB. Left to right: GND, Block, Data, 5V:
<br>
<img alt="PIN Header" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/pinheader.jpg?raw=true" height="250">

Wire up your WEMOS with 4 wires, connected to 5V, G, D4 and D3:
<br>
<img alt="Wemos Wiring" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wemos-wiring.jpg?raw=true" height="250">

Now connect your Wemos D1 mini to your co2 monitor:
<br>
<img alt="Result" src="https://github.com/b4ckspace/esp8266-co2monitor/blob/master/doc/images/wiring.jpg?raw=true" height="250">

## dependencies

* PubSubClient
* Arduino/ESP8266
