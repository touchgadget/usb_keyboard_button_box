#!/bin/bash
IDEVER="1.8.14"
WORKDIR="/tmp/autobuild_usb_keyboard_button_box"
mkdir -p ${WORKDIR}
# Install Ardino IDE in work directory
if [ -f ~/Downloads/arduino-${IDEVER}-linux64.tar.xz ]
then
    tar xf ~/Downloads/arduino-${IDEVER}-linux64.tar.xz -C ${WORKDIR}
else
    wget -O arduino.tar.xz https://downloads.arduino.cc/arduino-${IDEVER}-linux64.tar.xz
    tar xf arduino.tar.xz -C ${WORKDIR}
    rm arduino.tar.xz
fi
# Create portable sketchbook and library directories
IDEDIR="${WORKDIR}/arduino-${IDEVER}"
LIBDIR="${IDEDIR}/portable/sketchbook/libraries"
mkdir -p "${LIBDIR}"
export PATH="${IDEDIR}:${PATH}"
cd ${IDEDIR}
which arduino
# Install board package
if [ -d ~/Sync/ard_staging ]
then
    ln -s ~/Sync/ard_staging ${IDEDIR}/portable/staging
fi
arduino --pref "compiler.warning_level=default" --save-prefs
arduino --pref "boardsmanager.additional.urls=https://adafruit.github.io/arduino-board-index/package_adafruit_index.json" --save-prefs
arduino --install-boards "adafruit:samd"
BOARD="adafruit:samd:adafruit_qtpy_m0"
arduino --board "${BOARD}" --save-prefs
CC="arduino --verify --board ${BOARD}"
# Button debouncer
arduino --install-library "Bounce2"
#arduino --install-library "Adafruit DotStar"
arduino --install-library "Adafruit NeoPixel"
arduino --install-library "Adafruit BusIO"
arduino --install-library "Adafruit seesaw Library"
ln -s ~/Sync/usb_keyboard_button_box $LIBDIR/..
#git init
#echo -e "*.gz\n*.bz2\n*.tgz\n*.zip" >.gitignore
#git add .
#git commit -m "First draft"
