#!/usr/bin/env python3
import socket
import serial
import time
import sys

RIGCTL_HOST = "127.0.0.1"
RIGCTL_PORT = 19090
SERIAL_PORT = "/dev/lcd_controller"
SERIAL_BAUD = 9600
POLL_INTERVAL = 0.5

def cat_command(sock, cmd):
    try:
        sock.sendall(cmd.encode())
        time.sleep(0.05)
        response = b""
        while True:
            chunk = sock.recv(1024)
            if not chunk:
                break
            response += chunk
            if b";" in chunk:
                break
        return response.decode().strip()
    except:
        return None

def parse_response(resp, prefix):
    if resp and resp.startswith(prefix):
        value = resp[len(prefix):].rstrip(";")
        return value
    return None

def main():
    serial_port = SERIAL_PORT
    if len(sys.argv) > 1:
        serial_port = sys.argv[1]

    print(f"LCD Bridge starting")
    print(f"CAT server: {RIGCTL_HOST}:{RIGCTL_PORT}")
    print(f"Serial port: {serial_port}")

    ser = None
    sock = None
    last_freq = ""
    last_mode = ""
    last_smeter = ""
    waiting_for_pihpsdr = False
    waiting_for_arduino = False

    while True:
        try:
            if sock is None:
                try:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(5)
                    sock.connect((RIGCTL_HOST, RIGCTL_PORT))
                    sock.setblocking(False)
                    print("Connected to pihpsdr")
                    waiting_for_pihpsdr = False
                except socket.error:
                    if not waiting_for_pihpsdr:
                        print("Waiting for pihpsdr...")
                        waiting_for_pihpsdr = True
                    sock = None
                    time.sleep(2)
                    continue

            if ser is None:
                try:
                    ser = serial.Serial(serial_port, SERIAL_BAUD, timeout=1)
                    time.sleep(2)
                    print("Connected to Arduino")
                    waiting_for_arduino = False
                except serial.SerialException:
                    if not waiting_for_arduino:
                        print("Waiting for Arduino...")
                        waiting_for_arduino = True
                    ser = None
                    time.sleep(2)
                    continue

            sock.setblocking(True)
            sock.settimeout(2)

            freq_resp = cat_command(sock, "FA;")
            freq = parse_response(freq_resp, "FA")

            mode_resp = cat_command(sock, "MD;")
            mode = parse_response(mode_resp, "MD")

            smeter_resp = cat_command(sock, "SM0;")
            smeter = parse_response(smeter_resp, "SM")

            if freq and freq != last_freq:
                ser.write(f"FA{freq};\n".encode())
                last_freq = freq
                print(f"Freq: {freq}")

            if mode and mode != last_mode:
                ser.write(f"MD{mode};\n".encode())
                last_mode = mode
                print(f"Mode: {mode}")

            if smeter and smeter != last_smeter:
                ser.write(f"SM{smeter};\n".encode())
                last_smeter = smeter

            time.sleep(POLL_INTERVAL)

        except socket.error:
            if sock:
                sock.close()
            sock = None
            last_freq = ""
            last_mode = ""
            last_smeter = ""

        except serial.SerialException:
            if ser:
                ser.close()
            ser = None

        except KeyboardInterrupt:
            print("\nExiting...")
            break

    if ser:
        ser.close()
    if sock:
        sock.close()

if __name__ == "__main__":
    main()
