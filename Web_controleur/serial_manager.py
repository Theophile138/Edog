import serial
import serial.tools.list_ports


class SerialManager:
    def __init__(self):
        self.connection = None
        self.port = None

    def list_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def connect(self, port):
        if self.connection:
            raise Exception(f"Déjà connecté au port {self.port}")
        self.connection = serial.Serial(
            port, baudrate=115200, bytesize=8, timeout=0.01, stopbits=serial.STOPBITS_ONE
        )
        self.port = port

    def disconnect(self):
        if self.connection:
            self.connection.close()
            self.connection = None
            self.port = None

    def is_connected(self):
        return self.connection is not None and self.connection.is_open

    def send(self, message):
        if not self.is_connected():
            raise Exception("Pas de port série connecté")
        self.connection.write((message + "\n").encode())


    def read_available(self):
        if self.is_connected():
            try:
                line = self.connection.readline().decode(errors="ignore").strip()
                return line
            except:
                return ""
        return ""
