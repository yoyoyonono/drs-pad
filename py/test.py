import serial
from time import perf_counter

test_string = b' ' * 49 * 38 * 3

with serial.Serial("COM5", 921600) as ser:
    while True:
        t1_start = perf_counter()
        ser.write(test_string)
        if ser.read(1) != b'a':
            break
        t1_stop = perf_counter()
        print(f'{t1_stop - t1_start:.3f}s {1 / (t1_stop - t1_start):.3f}FPS')