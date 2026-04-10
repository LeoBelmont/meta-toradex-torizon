#!/bin/sh

# WARNING: This script does NOT yet work with the SL2619 board

echo "Flashing board..."

sudo ./bin/linux/x86_64/astra-boot astra-usbboot-images/sl1680_suboot/

sudo ./bin/linux/x86_64/uuu ./bin/linux/x86_64/uuu.auto
