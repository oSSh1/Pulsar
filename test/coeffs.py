import numpy as np
from scipy import signal

# Исходные данные
fs = 100.0  # Частота дискретизации, Гц
f_low = 0.7  # Нижняя граница, Гц
f_high = 5.0  # Верхняя граница, Гц

numtaps = 401

b = signal.firwin(numtaps, [f_low, f_high], pass_zero=False, fs=fs)


for i, val in enumerate(b):
    # Выводим элемент с запятой и пробелом, не переходя на новую строку
    print(f"{val},", end=" ")
    # Каждые 5 элементов (или на самом последнем элементе массива) делаем перенос строки
    if (i + 1) % 5 == 0 or (i + 1) == len(b):
        print()


