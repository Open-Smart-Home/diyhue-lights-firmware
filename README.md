
# Firmware for diyHue custom lights
A unified firmware to create your smart-light (LED bulb or LED strip) compatible with the [diyHue](https://github.com/diyhue) project.

You can build your light in two way:
  - build it from scratch using an ESP8266 and the basic schematics you can find here
  - using an existing LED bulb/strip based on ESP8266 and modify it with the tutorial in this repo ([read Here]())

If you chose the second way I added a list of suggested LED bulbs/strip that has been tested

 [LIST OF SUGGESTED LIGHTS][]
 
 
## How to compile firmware (short version)
The firmware is structured to be very easy to configure, compiling and upload for entry-level users too. By the way if you need advanced information you can find them [here][]

  - Download this repo using GIT clone or the 'download' button, and save into a folder on your PC
  - Download Arduino IDE from [here](https://www.arduino.cc/en/main/software)
  - When the IDE has been installed, open it, then
		File > Preferences 
	add this URL into the "Additional Boards Manager URLs"
		http://arduino.esp8266.com/stable/package_esp8266com_index.json
	then press "OK"
  - Open Boards Manager (Tool > Board.. > Boards Manager), then search and install "esp8266" by ESP8266 Community
  - Now open this project (File > Open > folder_where_you_saved_project > diyHueLight.ino )
  - When project has been opened, you will see a series of tabs on the top of code-area, click on the tab named "config_light.h"
  - That file is the configuration file you have to use to "tell" to the firmware the kind of light you will use (if you have any problem to identify the light model you can read the [advanced section]()). To select a light type simple remove the '#' before it's name.
  - Now the firmware is configured, you have to connect your ESP8266 board (click [here][] if you don't know how to do that) and flash the firmware using the button with a right-handed arrow at the top of the Arduino IDE windows.
  - The light is ready to work. To use it you have to use the [diyHue](https://github.com/diyhue) project.
  - ENJOY!!






Credits:
This project is inspired by the project by [@mariusmotea](https://github.com/mariusmotea) with some improvements to achieve a better readable and maintenable code. Other credits from the original project:
  - [@J3n50m4t](https://github.com/J3n50m4t) - Yeelight integration
  - Martin Cerný ([@mcer12](https://github.com/mcer12)) - Yeelight color bulb
  - probonopd https://github.com/probonopd/ESP8266HueEmulator
  - sidoh https://github.com/sidoh/esp8266_milight_hub
  - StefanBruens https://github.com/StefanBruens/ESP8266_new_pwm
  - Cédric @ticed35 for linkbutton implementation
  - [@Mevel](https://github.com/Mevel) - 433Mhz devices
  - [@Nikfinn99](https://github.com/Nikfinn99) - PCB designs