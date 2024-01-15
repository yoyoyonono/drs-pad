import asyncio
import json
import numpy as np
import pygame
import websockets

async def main():
    pygame.init()
    display = pygame.display.set_mode((38*10, 49*10))
    async with websockets.connect("ws://localhost:9002") as socket:
        print("connected")
        while True:
            await socket.send(b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}')
            message = await socket.recv()
            rgb_values = [x & 0xFE for x in json.loads(message[:-1])["data"][0]]
            image = np.array(rgb_values, dtype=np.uint8).reshape(49, 38, 3).transpose(1, 0, 2)
            surface = pygame.surfarray.make_surface(image)
            display.blit(pygame.transform.scale(surface, (38*10, 49*10)), (0, 0))
            pygame.display.update()
            pygame.event.pump()

if __name__ == "__main__":
    asyncio.run(main())