#!/bin/bash
echo "Virtual Cane Fresh Install"
echo "Raspberry PI must be connected to the internet"
echo
echo
#Project Github link
GITURL='https://github.com/sxp9646/projectDaredevil'

#Code Clone directory
GCLONE_ADDR='/home/pi/Documents/git1'

#file addresses
#MOD='modules'
#CONFIG='raspi-blacklist.conf'

echo "IMU libraries, Software, and system initialization Starting..."
#Cloning project from git
mkdir $GCLONE_ADDR
cd $GCLONE_ADDR
git init
git clone $GITURL

#IMU supporting SW
sudo apt-get install -y putty

#IMU libraries and dependencies
#add mirror for libi2c-dev it's not native any more
cd /etc/apt
sudo sed -i '/#deb http://http.us.debian.org/debian stretch main/c\deb http://http.us.debian.org/debian stretch main' sources.list
sudo grep -q -F 'deb http://http.us.debian.org/debian stretch main' sources.list || sudo echo 'deb http://http.us.debian.org/debian stretch main' >> sources.list
sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install -y i2c-tools libi2c-dev python-smbus

#IMU system config
sudo sed -i '/blacklist i2c-bcm2708/c\#blacklist i2c-bcm2708/' '/etc/modprobe.d/raspi-blacklist.conf'
##sudo sed -e '\|"test bitch"|h; ${x;s/test//;{g;t};a\' -e 'test bitch' -e '}' test


#modules
cd /etc
sudo sed -i '/#i2c-dev/c\i2c-dev' modules
sudo grep -q -F 'i2c-dev' modules || sudo echo 'i2c-dev' >> modules

sudo sed -i '/#i2c-bcm2708/c\i2c-bcm2708' modules
sudo grep -q -F 'i2c-bcm2708' modules || sudo echo 'i2c-bcm2708' >> modules

#config.txt
cd /boot
sudo sed -i '/#dtparam=i2c_arm=on/c\dtparam=i2c_arm=on' config.txt
sudo grep -q -F 'dtparam=i2c_arm=on' config.txt || sudo echo 'dtparam=i2c_arm=on' >> config.txt

sudo sed -i '/#dtparam=i2s=on/c\dtparam=i2s=on' config.txt
sudo grep -q -F 'dtparam=i2s=on' config.txt || sudo echo 'dtparam=i2s=on' >> config.txt
##should do a check if address is detected.

#Bluetooth config
#should check if the right version of bluez and pulse to report later
#BT config.txt
#hdmi_force_hotplug=1
sudo sed -i '/#hdmi_force_hotplug=1/c\hdmi_force_hotplug=1' config.txt
sudo grep -q -F 'hdmi_force_hotplug=1' config.txt || sudo echo 'hdmi_force_hotplug=1' >> config.txt

#dtoverlay=pi3-disable-bt
sudo sed -i '/#dtoverlay=pi3-disable-bt/c\dtoverlay=pi3-disable-bt' config.txt
sudo grep -q -F 'dtoverlay=pi3-disable-bt' config.txt || sudo echo 'dtoverlay=pi3-disable-bt' >> config.txt

# in sudo nano /etc/modprobe.d/raspi-blacklist.conf
cd /etc/modprobe.d/
sudo sed -i '/#blacklist btbcm/c\blacklist btbcm' CONFIG
sudo grep -q -F 'blacklist btbcm' CONFIG || sudo echo 'blacklist btbcm' >> CONFIG

sudo sed -i '/#blacklist hci_uart/c\blacklist hci_uart' CONFIG
sudo grep -q -F 'blacklist hci_uart' CONFIG || sudo echo 'blacklist hci_uart' >> CONFIG
#blacklist btbcm
#blacklist hci_uart

sudo apt-get install pulseaudio pulseaudio-module-bluetooth
#Direct to report dpkg -l pulseaudio pulseaudio-module-bluetooth

#Stop bluealsa to make sure the sw of the built in bluetooth is disabled
sudo killall bluealsa
sudo pulseaudio --start
