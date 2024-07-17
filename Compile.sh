#!/bin/bash

PicoCobraDir=/media/drive/picocobra

cd {$PicoCobraDir}/MCUME_pico
export PICO_SDK_PATH={$PicoCobraDir}/pico-sdk
cd build

rm *
rm -R -- */

cmake ..
make

cp {$PicoCobraDir}/MCUME_pico/build/picocobra.uf2 {$PicoCobraDir}/

read -p "Press ENTER key"

