import asyncio
import brotli
import json
import numpy as np
import pygame
import websockets
import serial
from time import perf_counter, sleep
import threading
import math

response_data = [0] * 38 * 49 * 3

reverse_bit = lambda x : int('{:08b}'.format(x)[::-1], 2)

def clamp_led(x):
    return math.floor(x * (255 / 215))

async def socket_handler():
    global response_data
    async for socket in websockets.connect("ws://localhost:9002"):
        try:
            print("connected")
            while True:
                await socket.send(b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}')
                message = await socket.recv()
                response_data = [clamp_led(x) & 0b1111_1110 for x in json.loads(message[:-1])["data"][0]]
        except Exception:
            continue

def serial_send():
    with serial.Serial('COM5', 460800) as ser:
        sleep(1)
        ser.write(b'g')
        while True:
            old_time = perf_counter()
            rgb_values = [reverse_bit(x) for x in response_data]
            image = np.array(rgb_values, dtype=np.uint8).reshape(49, 38, 3).transpose(1, 0, 2)
            uncompressed = bytes()
            for x in range(38):
                for y in range(49):
                    if (x % 5) % 2 == 0:
                        uncompressed += bytes([image[x, y, 1], image[x,y,0], image[x,y,2]])
                    else:
                        uncompressed += bytes([image[x, 48-y, 1], image[x,48-y,0], image[x,48-y,2]])
            compressed = brotli.compress(bytes(uncompressed))
            message = len(compressed).to_bytes(2, 'big') + compressed
            print(ser.read(1))
            ser.write(message)
            new_time = perf_counter()
            print(f'{new_time - old_time:.3f}s {1 / (new_time - old_time):.3f}FPS {len(message)/1024/1024/(new_time - old_time)*8:.3f}Mb/s {len(compressed)}B ', end='\t')

def display():
    pygame.init()
    display = pygame.display.set_mode((38*10, 49*10))
    clock = pygame.time.Clock()
    while True:
        image_values = np.array(response_data).reshape(49, 38, 3).transpose(1, 0, 2)
        surface = pygame.surfarray.make_surface(image_values)
        display.blit(pygame.transform.scale(surface, (38*10, 49*10)), (0, 0))
        pygame.display.update()
        pygame.event.pump()
        clock.tick(60)

if __name__ == "__main__":
    threading.Thread(target=display).start()
    threading.Thread(target=serial_send).start()
    asyncio.get_event_loop().run_until_complete(socket_handler())