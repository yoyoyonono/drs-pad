import asyncio
import dataclasses
import json
import numpy as np
import pygame
import websockets
import serial
from time import perf_counter

async def main():
    pygame.init()
    display = pygame.display.set_mode((38*10, 49*10))
    ser = serial.Serial("COM5", 921600)
    socket = await websockets.connect("ws://localhost:9002")
    print("connected")
    while True:
        try:
            await socket.send(b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}')
            message = await socket.recv()
        except:
            socket = await websockets.connect("ws://localhost:9002")
            continue
        rgb_values = [x & 0xFE for x in json.loads(message[:-1])["data"][0]]
        values = np.array(rgb_values, dtype=np.uint8).reshape(49, 38, 3)
        image = values.transpose(1, 0, 2)
        t1_start = perf_counter()
        message = bytes(rgb_values)
        ser.write(message)
        t1_stop = perf_counter()
        print(f'{t1_stop - t1_start:.3f}s {1 / (t1_stop - t1_start):.3f}FPS {len(message)/1024/1024/(t1_stop - t1_start)*8:.3f}Mb/s')
        surface = pygame.surfarray.make_surface(image)
        display.blit(pygame.transform.scale(surface, (38*10, 49*10)), (0, 0))
        pygame.display.update()
        pygame.event.pump()

if __name__ == "__main__":
    asyncio.run(main())