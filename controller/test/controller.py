import socket

def main():
    port = 4711

    #interfaces = socket.getaddrinfo(host=socket.gethostname(), port=None, family=socket.AF_INET)
    interfaces = socket.getaddrinfo(host="localhost", port=None, family=socket.AF_INET)
    allips = [ip[-1][0] for ip in interfaces]

    msg = b'ring'

    for ip in allips:
        print(f'Broadcasting UDP packet on {ip}')
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.bind((ip,0))
        sock.sendto(msg, (ip, port))
        sock.close()

main()
