import socket
import binascii

HOST = '127.0.0.1'
PORT = 5000

channel_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

channel_socket.bind((HOST, PORT))

channel_socket.listen(1)  

print(f"Listening on {HOST}:{PORT}...")

while True:
    conn_socket, client_address = channel_socket.accept()
    print(f"Accepted connection from {client_address}")

    try:
        data = conn_socket.recv(1024)
        if not data:
            break
        
        hex_data = binascii.hexlify(data).decode('utf-8')

        print("Received data:", hex_data)

    except Exception as e:
        print("Error:", e)
    finally:
        conn_socket.close()

channel_socket.close()
print("Closed socket")