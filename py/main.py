import asyncio
import brotli
import json
import numpy as np
import pygame
import websockets
import serial
from time import perf_counter

async def main():
    pygame.init()
    display = pygame.display.set_mode((38*10, 49*10))
    ser = serial.Serial("COM10", 460800)
    socket = await websockets.connect("ws://localhost:9002")
    print("connected")
    old_time = perf_counter()
    while True:
        try:
            await socket.send(b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}')
            message = await socket.recv()
        except Exception:
            socket = await websockets.connect("ws://localhost:9002")
            continue
        rgb_values = [x & 0b1111_1000 for x in json.loads(message[:-1])["data"][0]]
        values = np.array(rgb_values, dtype=np.uint8).reshape(49, 38, 3)
        image = values.transpose(1, 0, 2)
        uncompressed = bytes()
        for x in range(38):
            for y in range(49):
                uncompressed += bytes([image[x, y, 1], image[x,y,0], image[x,y,2]])
        compressed = brotli.compress(bytes(uncompressed))
        message = len(compressed).to_bytes(2, 'big') + compressed
        print(ser.read(1))
        ser.write(message)
        new_time = perf_counter()
        print(f'{new_time - old_time:.3f}s {1 / (new_time - old_time):.3f}FPS {len(message)/1024/1024/(new_time - old_time)*8:.3f}Mb/s {len(compressed)}B ', end='\t')
        old_time = perf_counter()
        surface = pygame.surfarray.make_surface(image)
        display.blit(pygame.transform.scale(surface, (38*10, 49*10)), (0, 0))
        pygame.display.update()
        pygame.event.pump()

if __name__ == "__main__":
    asyncio.run(main())