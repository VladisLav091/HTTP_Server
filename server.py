import socket

op1 = 0
op2 = 0

def handle_request(client_socket):
    global op1, op2
    request = client_socket.recv(1024).decode('utf-8')
    print(f"Request: {request}")

    request_lines = request.splitlines()

    if len(request_lines) == 0:
        return

    method, path, _ = request_lines[0].split()

    if method == "PUT" and (path == "/op1" or path == "/op2"):
        try:
            content_length = int([line.split(": ")[1] for line in request_lines if line.startswith("Content-Length")][0])
            body = request.split('\r\n\r\n')[1][:content_length].strip()

            if path == "/op1":
                op1 = float(body)
            elif path == "/op2":
                op2 = float(body)

            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"
        except Exception as e:
            response = f"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"

    elif method == "GET" and (path == "/op1" or path == "/op2"):
        if path == "/op1":
            value = str(op1)
        elif path == "/op2":
            value = str(op2)

        response = f"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {len(value)}\r\n\r\n{value}"
    
    elif method == "POST" and path == "/calculate":
        operation = "+"
        for line in request_lines:
            if line.startswith("Operation:"):
                operation = line.split(":")[1].strip()
        
        try:
            if operation == "+":
                result = op1 + op2
            elif operation == "-":
                result = op1 - op2
            elif operation == "*":
                result = op1 * op2
            elif operation == "/":
                result = op1 / op2 if op2 != 0 else "Division by zero"
            else:
                result = "Unknown operation"

            result_str = str(result) + "\n"
            response = f"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: {len(result_str)}\r\n\r\n{result_str}"
        except Exception as e:
            response = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n"
    
    elif path in ["/op1", "/op2", "/calculate"]:
        response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n"
    
    else:
        response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"

    client_socket.sendall(response.encode('utf-8'))

def run_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(('localhost', 10000))
    server_socket.listen(5)
    print("Server is running on port 10000...")

    try:
        while True:
            client_socket, client_address = server_socket.accept()
            print(f"Connection from {client_address}")
            handle_request(client_socket)
            client_socket.close()
    except KeyboardInterrupt:
        print("Shutting down the server...")
        pass
        server_socket.close()

if __name__ == "__main__":
    run_server()
