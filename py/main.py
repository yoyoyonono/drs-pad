import asyncio
import dataclasses
import json
import numpy as np
import pygame
import websockets

@dataclasses.dataclass
class LED:
    r: int
    g: int
    b: int

async def main():
    pygame.init()
    display = pygame.display.set_mode((38*10, 49*10))
    async with websockets.connect("ws://localhost:9002") as socket:
        print("connected")
        while True:
            await socket.send(b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}')
            message = await socket.recv()
            rgb_values = [x & 0xFE for x in json.loads(message[:-1])["data"][0]]
            values = np.array(rgb_values, dtype=np.uint8).reshape(49, 38, 3)
            image = values.transpose(1, 0, 2)
            leds = np.array([LED(*x) for x in values.reshape(49*38, 3)]).reshape(49, 38)
            for y in range(49):
                for x in range(38):
                    # TODO: write the thing to dump the thing
                    pass
            surface = pygame.surfarray.make_surface(image)
            display.blit(pygame.transform.scale(surface, (38*10, 49*10)), (0, 0))
            pygame.display.update()
            pygame.event.pump()

if __name__ == "__main__":
    asyncio.run(main())