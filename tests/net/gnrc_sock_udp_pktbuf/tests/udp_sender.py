import argparse
import signal
import socket
import sys
import time

_exit = False

def _socket():
    return socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)

def _sendto(udp_socket, target_ip, target_port, message):
    try:
        udp_socket.sendto(message.encode(), (target_ip, target_port))
    except Exception as e:
        print(f"Error: {e}")

def signal_handler(sig, frame):
    _exit = True

def main():
    parser = argparse.ArgumentParser(description="UDP Sender with IPv6 support")
    parser.add_argument("target_ip", help="Target IPv6 address")
    parser.add_argument("target_port", type=int, help="Target port number")
    args = parser.parse_args()

    target_ip = args.target_ip
    target_port = args.target_port

    udp_socket = _socket()

    counter = 0
    while _exit == False:
        message = str(counter) + bytearray([0x23 for i in range(0, 511)]).decode()
        _sendto(udp_socket, target_ip, target_port, message[0:512])
        counter = counter + 1
        time.sleep(0.1)

    udp_socket.close()

if __name__ == "__main__":
    main()
