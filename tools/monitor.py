#!/usr/bin/env python3
"""Монитор последовательного порта. Выход — Ctrl+C."""
import glob, sys, time
import serial

BAUD = 115200

def find_port():
    while True:
        ports = sorted(glob.glob('/dev/cu.usbmodem*'))
        if ports:
            return ports[0]
        print('жду плату...', end='\r', flush=True)
        time.sleep(0.5)

def main():
    while True:
        port = find_port()
        try:
            with serial.Serial(port, BAUD, timeout=1) as s:
                print(f'--- {port} подключён, Ctrl+C для выхода ---')
                while True:
                    line = s.readline()
                    if line:
                        sys.stdout.write(line.decode('utf-8', 'replace'))
                        sys.stdout.flush()
        except serial.SerialException:
            print('\n--- плата отключилась, жду обратно ---')
            time.sleep(1)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\n--- выход ---')
