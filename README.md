# Unifi G4 Doorbell chime

Control a chime from the Unifi G4 doorbell. A server receives the doorbell signal (G4 doorbell configured with a mechanical chime), then sends out an event on the network to which any configured chimes listen to and respond. The eventing mechanism is currently using UDP broadcast packets.

The project is divided into the _Controller_ and _Chime_ circuits and utilizes PlatformIO for platform and library management.

## Code

The `controller` and `chime` directories contain the projects for the controller and chimes. The ESP hostname is configured via the c++ preprocessor and requires it to be set. The Makefile needs the following variables:

* `ESP_HOSTNAME` The hostname to be used on the ESP
* `ESP_NETWORKADDRESS` The network address (can be IP address or network name), used for network upload only. Defaults to '`ESP_HOSTNAME`.local'

### Compile
```
ESP_HOSTNAME=xyx make compile
```

### Flash
Connect the ESP to the USB port:
```
ESP_HOSTNAME=xyx make flash
```

### Open serial console to the ESP
Connect the ESP to the USB port:
```
ESP_HOSTNAME=xyx make console
```

### Upload via network
Make sure your computer is on the same subnet:
```
ESP_HOSTNAME=xyx make upload
```
or if you need to override the network address:
```
ESP_HOSTNAME=xyx ESP_NETWORKADDRESS=10.11.12.13 make upload
```


### test

echo -n "ring" | socat - udp-datagram:192.168.101.255:4711,broadcast

set active:
```
curl -X POST http://doorbell-chime-hallway.local/chime/14
```

play:
```
curl http://doorbell-chime-livingroom.local/chime
```
