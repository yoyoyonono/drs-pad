import brotli
import json
import websockets

# get frames from the websocket and compress them and print the length comparison


async def main():
    socket = await websockets.connect("ws://localhost:9002")
    print("connected")
    while True:
        try:
            await socket.send(
                b'{"id":0,"module":"drs","function":"tapeled_get","params":[]}'
            )
            message = await socket.recv()
        except:
            socket = await websockets.connect("ws://localhost:9002")
            continue
        serial_message = bytes(x & 0xFE for x in json.loads(message[:-1])["data"][0])
        print(
            f"{len(serial_message)} -> {len(brotli.compress(serial_message))}"
        )


if __name__ == "__main__":
    import asyncio

    asyncio.run(main())