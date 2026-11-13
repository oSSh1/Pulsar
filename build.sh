#!/bin/bash
# Сборка прошивки. Запуск: ./build.sh
set -e
cd "$(dirname "$0")"

export PICO_SDK_PATH=~/.pico-sdk/sdk/2.3.1
export PICO_TOOLCHAIN_PATH=~/.pico-sdk/toolchain/14_2_Rel1
CMAKE=~/.pico-sdk/cmake/v3.28.6/bin/cmake     # 3.28: SDK не дружит с 3.31

[ -d build ] || "$CMAKE" -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja -C build

echo
ls -lh build/*.uf2 | awk '{print "готово:", $9, $5}'
echo "залить: BOOTSEL + перетащить, либо  picotool load -x -f build/pulsar_onboard.uf2"
