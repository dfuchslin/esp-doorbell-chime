BAUD = 115200
PIO = pio
HOSTNAME1 = doorbell-chime-hallway.local
#HOSTNAME2 = doorbell-chime-livingroom.local
HOSTNAME2 = 192.168.101.153

compile: 
	$(PIO) run

flash: 
	$(PIO) run -t upload

upload1:
	$(PIO) run -t upload -e tinypico_ota --upload-port $(HOSTNAME1)

upload2:
	$(PIO) run -t upload -e tinypico_ota --upload-port $(HOSTNAME2)

console:
	$(PIO) device monitor -b $(BAUD)

copychimes:
	cp chimes/* /Volumes/NO\ NAME/
	dot_clean -v -m /Volumes/NO\ NAME
