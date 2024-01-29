import socket

def send_string_to_server():
    # Server address and port
    SERVER_ADDRESS = '127.0.0.1'
    SERVER_PORT = 10052

    # Create a TCP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        # Connect to the server
        client_socket.connect((SERVER_ADDRESS, SERVER_PORT))

        # Send the string "ABCD" to the server
        data = "ABCD"
        client_socket.sendall(data.encode())

        print("Data sent successfully")

    except Exception as e:
        print(f"Error: {e}")

    finally:
        # Close the socket
        client_socket.close()


# Call the function to send the string to the server
send_string_to_server()