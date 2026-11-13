#!/bin/bash
# Собрать, залить, смотреть вывод. Выход из монитора — Ctrl+C.
cd "$(dirname "$0")"

# 1. освободить порт, если его кто-то держит
pkill -f "screen /dev/cu.usbmodem" 2>/dev/null
pkill -f "monitor.py"             2>/dev/null
sleep 0.3

# 2. собрать
./build.sh || exit 1

# 3. залить; при осечке с серийником — повторить, плата уже в режиме прошивки
UF2=$(ls build/*.uf2 | head -1)
picotool load -x -f "$UF2" 2>/dev/null || {
    echo "повтор заливки..."
    sleep 1
    picotool load -x "$UF2" || { echo "ЗАЛИТЬ НЕ УДАЛОСЬ"; exit 1; }
}

# 4. смотреть вывод
echo
exec python3 tools/monitor.py