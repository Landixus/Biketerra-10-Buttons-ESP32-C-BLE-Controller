# Biketerra-10-Buttons-ESP32-C-BLE-Controller
Biketerra 10 Buttons ESP32-C BLE Controller for Handlebar custom Mount

An updated Version with Wemos32D1 Mini with 2 5way navigation modules are here: 
https://github.com/Landixus/oneZlickAway/blob/main/wemos32d1mini.ino

This Sketch is for ESP32C Dev Module, i use a geekable: https://de.aliexpress.com/item/1005007104801519.html
And the 2. Sketch for ESP32C XIAO from seed.

Settings in Arduino IDE:
Board ESP32C3 DEV Modul or ESP32C XIAO
Downgrade with Boardmanager to esp32 2.0.17 for Espressif *this is important*
Upload with DIO | 115200 | CDC Enabled on boot

Make sure you have "ESP32_BLE_Keyboard" library without other "ESP32_XX_BLE_Keyboard_XX" libraries

My Modul is really trick and instable, mayBe use a nodemcu32 with BLE.

The buttons are arranged for BikeTerra, feel free to arrange for your needs.

Wiring Screenshots come later.

The device go to sleep after 5 minutes, and can be waked up with left Button, if still tired, just reset the modul.
Dont forget to add the device in your Bluetooth manager as Lenkrad Pro / Or what ever you named it in the sketch:)
If you load a new Sketch to the device, kick it out of your windows BT Manager before you try to upload the new sketch.
![Alt text](https://github.com/Landixus/Biketerra-10-Buttons-ESP32-C-BLE-Controller/blob/main/20260130_162737.jpg "Image")

