BAUD = 115200
PIO = pio
HOSTNAME = doorbell-controller.local

compile: 
	$(PIO) run

flash: 
	$(PIO) run -t upload

upload:
	$(PIO) run -t upload --upload-port $(HOSTNAME)

console:
	$(PIO) device monitor -b $(BAUD)
