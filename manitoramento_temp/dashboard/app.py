import time
import json
import threading
import serial
import serial.tools.list_ports
from flask import Flask, render_template
from flask_socketio import SocketIO

# Configuração do App
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# Variáveis Globais
arduino_port = None
baud_rate = 9600
serial_conn = None
latest_data = {"temperatura": 0, "umidade": 0}

def find_arduino():
    """Tenta encontrar o Arduino automaticamente"""
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if "Arduino" in p.description or "CH340" in p.description or "USB Serial" in p.description:
            return p.device
    return None

def read_serial():
    """Lê dados da porta serial em background"""
    global serial_conn, latest_data
    
    while True:
        if serial_conn is None:
            port = find_arduino()
            if port:
                try:
                    serial_conn = serial.Serial(port, baud_rate, timeout=1)
                    print(f"✅ Arduino conectado em {port}")
                    time.sleep(2) # Aguarda reset do Arduino
                except Exception as e:
                    print(f"❌ Erro ao conectar: {e}")
            else:
                # print("⚠️ Arduino não detectado. Tentando novamente...")
                socketio.emit('update', latest_data) # Envia dados zerados ou anteriores
                time.sleep(3)
                continue

        try:
            if serial_conn.in_waiting > 0:
                line = serial_conn.readline().decode('utf-8').strip()
                if line.startswith('{') and line.endswith('}'):
                    try:
                        data = json.loads(line)
                        latest_data = data
                        socketio.emit('update', data)
                        # print(f"Dados recebidos: {data}")
                    except json.JSONDecodeError:
                        pass
        except Exception as e:
            print(f"❌ Conexão perdida: {e}")
            serial_conn.close()
            serial_conn = None

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    # Inicia a thread de leitura serial
    t = threading.Thread(target=read_serial)
    t.daemon = True
    t.start()

    print("🚀 Servidor rodando! Acesse: http://localhost:5050")
    print("📲 Na rede local, use o IP do seu PC:5050")
    socketio.run(app, host='0.0.0.0', port=5050, allow_unsafe_werkzeug=True)
