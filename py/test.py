import base64
import random
import serial
import brotli
from time import perf_counter

#test_string_uncompressed = bytes([65] * 49 * 38 * 3)
i = 0
with serial.Serial("COM10", 460800) as ser:
    old_time = perf_counter()
    while True:
        test_string_uncompressed = bytes(i for _ in range(49*38*3))
        i += 1
        if i > 255:
            i = 0
        compressed = brotli.compress(test_string_uncompressed)
        test_string = len(compressed).to_bytes(2, 'big') + compressed
#        print(base64.b64encode(test_string[2:]).decode())
        print(ser.read(1))
        ser.write(test_string)
        new_time = perf_counter()
        print(f'{new_time - old_time:.3f}s {1 / (new_time - old_time):.3f}FPS {len(test_string)/1024/1024/(new_time - old_time)*8:.3f}Mb/s')
        old_time = perf_counter()