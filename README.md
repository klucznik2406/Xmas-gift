# Xmas-gift

### Source code dependencies (libraries needed to be installed):
* [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit_NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix)
* [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
* OneWire
* DallasTemperature

### Project pinout:
<ul>
<li>DS1302 - clock module</li>
<ul>
<li>I/O - PIN 6</li>
<li>SCLk - PIN 4 </li>
<li>CE - PIN A2 </li>
</ul>
<li>Set/Modify Button - PIN A0 </li>
<li>Menu Button - PIN 2 </li>
<li>Dallas Temperature DS18B20 - PIN 9 </li>
<li>Photoresistor connection</li>
<ul>
<li>Power Enable - PIN 7</li>
<li>Analog Read - PIN A3 </li>
</ul>
<li>Christmas mode pins</li>
<ul>
<li>In - PIN 11</li>
<li>Out - PIN 12</li>
</ul>
</ul>

### Power mode switch
* Position 0 - Device is fully powered only from external power source - via USB cable
* Position I - Device is powered from internal battery (Blue LED ON inside device). **!! Warrning, for safetly while working in this position USB cable should be disconnected !!**
* Position II - Device is fully powered only from external power source - via USB cable - also in meantime devices changes battery used for Position I working - if inside red LED is ON then battery is still charging, if ther is no RED led light, but very weak Blue LED light - battery fully charged 

### Time setting:
1. Press Menu button.
2. Wait for a couple of seconds, if display screen will not change into time setting mode (2 raw digts displayed in blue color), then go to step 1.
3. Adjust minutes with Modify button.
4. Acknowledge minutes setting by presing Menu button (on Menu button press time minutes are set to desired value and seconds are cleared to 0)
5. Adjust hours with Modify button (24h mode only).
6. Acknoeledge hours setting by pressing Menu button.
7. After a couple of seconds clock should return back to it's previous program.
