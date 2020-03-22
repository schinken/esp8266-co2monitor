# ESP8266-co2monitor

## description

Interface an TFA Dostmann CO2 Monitor with ESP8266 (in our example a WEMOS D1 Mini) to connect it to the "Internet of Things". It publishes the CO2 measurement and temperature to a configured MQTT topic.

It also implements [Home Assistant Auto Discovery](https://www.home-assistant.io/docs/mqtt/discovery/).

## compiling

* Rename settings.h.example to settings.h
* Open up your arduino IDE
* Edit the settings.h to match your setup
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

## using
Since we're using [WiFiManager](https://github.com/tzapu/WiFiManager), after flashing, the EPS8266 will open up a WiFi Hotspot to configure Wireless credentials. Just connect to it using a wifi-capable device and you should be automatically redirected to the configuration page.

The ESP8266 will also default to AP mode if the configured wifi is unavailable.
You might want to set a password for the AP mode in your settings.h

If `USE_HA_AUTODISCOVERY` is set and mqtt is configured correctly,
you should find a new device with CO2 + Temperature entities in your Home Assistant installations devices section.

## dependencies

* Arduino/ESP8266

## notes

Don't use D4 and D3, because it causes power-on issues with wemos d1 mini: https://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
