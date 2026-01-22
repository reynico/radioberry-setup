#!/usr/bin/env python3
import socket
import serial
import time
import sys

RIGCTL_HOST = "127.0.0.1"
RIGCTL_PORT = 19090
SERIAL_PORT = "/dev/lcd_controller"
SERIAL_BAUD = 9600
POLL_INTERVAL = 0.3

def cat_command(sock, cmd):
    try:
        sock.sendall(cmd.encode())
        response = b""
        sock.settimeout(1)
        while True:
            chunk = sock.recv(1024)
            if not chunk:
                break
            response += chunk
            if b";" in chunk:
                break
        return response.decode().strip()
    except Exception as e:
        print(f"ERROR cmd={cmd}: {e}")
        return None

def main():
    serial_port = SERIAL_PORT
    if len(sys.argv) > 1:
        serial_port = sys.argv[1]

    print(f"LCD Bridge starting")
    print(f"CAT server: {RIGCTL_HOST}:{RIGCTL_PORT}")
    print(f"Serial: {serial_port}")

    ser = None
    sock = None
    last_freq = ""
    last_mode = ""
    last_vfo = ""
    last_tx = ""
    last_sm = None
    waiting_for_pihpsdr = False
    waiting_for_arduino = False

    while True:
        try:
            if sock is None:
                try:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(5)
                    sock.connect((RIGCTL_HOST, RIGCTL_PORT))
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

            if_resp = cat_command(sock, "IF;")

            freq = None
            mode = None
            tx = None

            if if_resp and if_resp.startswith("IF") and len(if_resp) >= 38:
                freq = if_resp[2:13].lstrip("0") or "0"
                tx = if_resp[28]
                mode = if_resp[29]

            fr_resp = cat_command(sock, "FR;")
            vfo = None
            if fr_resp and fr_resp.startswith("FR"):
                vfo = fr_resp[2:3]

            sm_resp = cat_command(sock, "SM0;")
            sm = None
            sm_db = None
            if sm_resp and sm_resp.startswith("SM"):
                try:
                    sm_db = int(sm_resp[2:].rstrip(";"))
                    sm_min = -115
                    sm_max = -53
                    sm_clamped = max(sm_min, min(sm_max, sm_db))
                    sm = int((sm_clamped - sm_min) * 255 / (sm_max - sm_min))
                except ValueError:
                    pass

            if freq and freq != last_freq:
                ser.write(f"FA{freq};\n".encode())
                last_freq = freq

            if mode and mode != last_mode:
                ser.write(f"MD{mode};\n".encode())
                last_mode = mode

            if vfo and vfo != last_vfo:
                ser.write(f"FR{vfo};\n".encode())
                last_vfo = vfo

            if tx and tx != last_tx:
                ser.write(f"TX{tx};\n".encode())
                last_tx = tx

            if sm is not None and sm != last_sm:
                ser.write(f"SM{sm};\n".encode())
                last_sm = sm

            time.sleep(POLL_INTERVAL)

        except socket.error:
            if sock:
                sock.close()
            sock = None
            last_freq = ""
            last_mode = ""
            last_vfo = ""
            last_tx = ""
            last_sm = None

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
