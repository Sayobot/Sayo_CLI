# Saybot Keyboard CLI  

## Linux  
### Using system libraries  
- required system libraries:  
	- ```jsoncpp```  
	- ```hidapi```  
	- For Ubuntu you can install these using `sudo apt-get install libjsoncpp-dev libhidapi-dev`
- clone repository: ```git clone git@github.com:Sayobot/Sayo_CLI.git```  
- install udev rules: copy ```98-saybot.rules``` to ```/etc/udev/rules.d```  
- reload udev rules: ```sudo udevadm control --reload-rules```  
- build: run ```./build_linux_system_libs.sh```  
- run configurator: ```./Sayo_CLI_Linux```  

### Using cloned libraries  
- clone repository: ```git clone --recurse-submodules -j8 git@github.com:Sayobot/Sayo_CLI.git```  
- install udev rules: copy ```98-saybot.rules``` to ```/etc/udev/rules.d```  
- reload udev rules: ```sudo udevadm control --reload-rules```  
- build: run ```./build_linux.sh```  
- run configurator: ```./Sayo_CLI_Linux```  

## MacOS  
- clone repository: ```git clone --recursive-submodules -j8 git@github.com:Sayobot/Sayo_CLI.git```  
- run: ```./build_Mac.sh```  
- run configurator: ```./Sayo_CLI_Mac```  

