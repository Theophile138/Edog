import asyncio
import websockets
from websocket_handler import websocket_handler

async def main():
    async with websockets.serve(websocket_handler, "0.0.0.0", 8765):
        print("Serveur WebSocket lancé sur ws://localhost:8765")
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    asyncio.run(main())
