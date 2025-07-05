# Edog

Pour windows : 
Open up a Windows Powershell terminal (Resist the urge to run Powershell as administrator, that will break things)

choco install make

cd bldc

make arm_sdk_install
make <-- Pick out the name of your target device from the supported boards list. For instance, I have a Trampa VESC 100/250, so my target is 100_250
