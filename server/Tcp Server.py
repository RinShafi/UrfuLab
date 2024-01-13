import socket
import time

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = '127.0.0.1'
port = 8000

server_socket.bind((host, port))

server_socket.listen(1)
print("Ожидание подключения клиента...")
client_socket, addr = server_socket.accept()
print('Подключение от', addr)
time.sleep(1)
message = 'Пингую!'
client_socket.send(message.encode('utf-8'))
data = client_socket.recv(1024)
print('Получено:', data.decode('utf-8'))

client_socket.close()
server_socket.close()