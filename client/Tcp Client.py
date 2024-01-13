import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host = '127.0.0.1'
port = 8000
client_socket.connect((host, port))

data = client_socket.recv(1024)
print('Получено:', data.decode('utf-8'))

message = 'Отправка запроса серверу'

client_socket.send(message.encode('utf-8'))
client_socket.close()