# thanks oreilly https://www.oreilly.com/library/view/managing-projects-with/0596006101/ch03.html
ESP_HOSTNAME ?= $(if $1,$$ESP_HOSTNAME,$(error You must set 'ESP_HOSTNAME'))
ESP_NETWORKADDRESS ?= $(if $1,ESP_NETWORKADDRESS,$$ESP_HOSTNAME.local)
PIO_CLI = ESP_HOSTNAME=$(ESP_HOSTNAME) ESP_NETWORKADDRESS=$(ESP_NETWORKADDRESS) pio
CHIME_VOLUME = /Volumes/NO\ NAME

env:
	@echo ESP_HOSTNAME=$(ESP_HOSTNAME)
	@echo ESP_NETWORKADDRESS=$(ESP_NETWORKADDRESS)

compile: env
	$(PIO_CLI) run

flash: env
	$(PIO_CLI) run -t upload

upload: env
	$(PIO_CLI) run -t upload -e ota

console: env
	$(PIO_CLI) device monitor

copychimes:
	rm $(CHIME_VOLUME)/*
	cp chimes/* $(CHIME_VOLUME)/
	dot_clean -v -m $(CHIME_VOLUME)
	diskutil eject $(CHIME_VOLUME)
