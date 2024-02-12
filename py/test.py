import base64
import random
import serial
import brotli
from time import perf_counter

test_string_uncompressed = bytes([65] * 49 * 38 * 3)

with serial.Serial("COM10", 115200) as ser:
    old_time = perf_counter()
    while True:
#        test_string_uncompressed = bytes(random.getrandbits(8) for _ in range(49*38*3))
        test_string = brotli.compress(test_string_uncompressed)
        test_string = len(test_string).to_bytes(2, 'big') + test_string
#        print(base64.b64encode(test_string[2:]).decode())
        print(len(test_string) - 2, int.from_bytes(test_string[:2], 'big'), len(brotli.decompress(test_string[2:])))
        ser.write(test_string)
        print(ser.read(1))
        new_time = perf_counter()
        print(f'{new_time - old_time:.3f}s {1 / (new_time - old_time):.3f}FPS {len(test_string)/1024/1024/(new_time - old_time)*8:.3f}Mb/s')
        old_time = perf_counter()