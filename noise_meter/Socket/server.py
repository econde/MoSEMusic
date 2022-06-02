import socket
import json
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
from datetime import datetime

def recvall(sock):
    BUFF_SIZE = 9999 
    data = ''
    while True:
        part = sock.recv(BUFF_SIZE).decode()
        data += part
        if len(part) < BUFF_SIZE:
            # either 0 or end of data
            break
    return data

def firebase(data):
    # print(data)
    today_date = datetime.now().strftime("%Y-%m-%d")
    for key in data:
        evento = key
    
    ref = db.reference("/" + today_date + "/" + evento)
    ref.update(data[evento])

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 8000        # Port to listen on (non-privileged ports are > 1023)
cred = credentials.Certificate('key.json')#fichero com os credenciais
firebase_admin.initialize_app(cred, {
    'databaseURL': ##Passar url da base de dados
})
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        conn, addr = s.accept()
        with conn:
            print('Connected by', addr)
            while True:
                data = recvall(conn)#receção por blocos
                if not data:
                    break
                newdata=json.loads(data)
                firebase(newdata)
                
