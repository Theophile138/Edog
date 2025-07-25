python -m venv env
source env/bin/activate
pip install websockets
pip install aiohttp
pip install pyserial

cd.. 
cd dvb/Edog/EdogController
source env/bin/activate
python3 main.py

cd Edog/EdogController
source env/bin/activate
python3 main.py