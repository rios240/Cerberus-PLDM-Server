import socket
import threading
import signal
import sys

# Define the server address and port
server_address = ('0.0.0.0', 5555)

# Create a socket to listen for incoming connections
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(server_address)
server_socket.listen(2)  # Allow up to 2 connections, one for client and one for server

# Function to handle the client connection
def client_handler(client_socket, server_socket):
    while True:
        data = client_socket.recv(1024)
        if not data:
            break
        hex_data = data.hex()  # Convert data to hexadecimal
        print(f"Received from client: {hex_data}")
        server_socket.send(data)

# Function to handle the server connection
def server_handler(client_socket, server_socket):
    server_socket.settimeout(5)  # Set a 5-second timeout (adjust as needed)
    while True:
        try:
            data = server_socket.recv(1024)
            if not data:
                break
            hex_data = data.hex()  # Convert data to hexadecimal
            print(f"Received from server: {hex_data}")
            client_socket.send(data)
        except socket.timeout:
            print("Timeout: No data received from the server.")
            break
        
# Lists to store client and server sockets
client_sockets = []
server_sockets = []

# Handle Ctrl+C to gracefully close the sockets
def signal_handler(sig, frame):
    print("Ctrl+C received. Closing sockets...")
    for client_socket in client_sockets:
        client_socket.close()
    for server_socket in server_sockets:
        server_socket.close()
    sys.exit(0)

# Register the signal handler for Ctrl+C
signal.signal(signal.SIGINT, signal_handler)

while True:
    print("Waiting for connections...")
    client_socket, client_address = server_socket.accept()
    print(f"Accepted connection from {client_address}")

    server_socket, server_address = server_socket.accept()
    print(f"Accepted connection from {server_address}")
    
    client_sockets.append(client_socket)
    server_sockets.append(server_socket)

    # Create two separate threads to handle data forwarding between client and server
    client_thread = threading.Thread(target=client_handler, args=(client_socket, server_socket))
    server_thread = threading.Thread(target=server_handler, args=(client_socket, server_socket))

    client_thread.start()
    server_thread.start()
