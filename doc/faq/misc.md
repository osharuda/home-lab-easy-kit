# <p align="center">Miscaleneous notes</p>
<p align="center"><img src="../images/hlek.svg"></p>

# 1. Enable wifi
Edit `wpa_supplicant.conf` configuration file and add your wifi settings
```
sudo vim /etc/wpa_supplicant/wpa_supplicant.conf
```

Add the following block for your wi-fi access point.
```
network={
	ssid="<SSID>"
   key_mgmt=WPA-PSK
	psk="<PASSWORD>"
}
```


Also, wireless inteface may be blocked by `rfkill`:

To list restrictions:
```
rfkill list
```

To unblock interface:
```
rfkill unblock <id>
```

# 2. Reset DHCP lease
```
sudo su
echo -n "" >/var/lib/dhcp/dhclient.leases
dhclient -v eth0
exit
```


# 3. Set Raspberry PI I2C to 400KHz (Fast Mode)
Add the following parameter to the ```/boot/firmware/config.txt```
```
dtparam=i2c_arm_baudrate=400000
```
and reboot.
