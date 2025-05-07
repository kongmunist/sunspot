"""
serial_cam_view.py
------------------
Stream-viewer for the ESP32-S3 JPEG-over-serial example.

Usage:
    python serial_cam_view.py --port /dev/tty.usbmodem101 --baud 921600
"""
#!/usr/bin/env python3
"""
serial_cam_view.py  – v1.1
Adds per-frame byte-count to the existing USB-serial viewer.
"""
import argparse, sys, time
from pathlib import Path
import cv2, numpy as np, serial

JPEG_START = b"\xFF\xD8"
JPEG_END   = b"\xFF\xD9"

def parse_args():
    dft_port = "COM3" if sys.platform.startswith("win") else "/dev/ttyACM0"
    p = argparse.ArgumentParser()
    p.add_argument("--port", default=dft_port)
    p.add_argument("--baud", type=int, default=921600)
    p.add_argument("--timeout", type=float, default=0.2)
    p.add_argument("--save-dir", type=Path)
    return p.parse_args()

def main():
    args = parse_args()
    ser = serial.Serial(args.port, args.baud, timeout=args.timeout)
    print(f"Opened {ser.port} @ {ser.baudrate} baud")

    buf = bytearray()
    frame_idx, dropped = 0, 0
    t0 = time.time()

    try:
        while True:
            buf.extend(ser.read(4096))

            # ---- search for complete JPEGs ---------------------------------
            while True:
                start = buf.find(JPEG_START)
                if start == -1:
                    buf.clear()            # discard garbage
                    break
                end = buf.find(JPEG_END, start + 2)
                if end == -1:
                    if start:              # keep only possible opener onward
                        del buf[:start]
                    break

                jpeg = bytes(buf[start:end + 2])
                del buf[:end + 2]          # consume processed bytes

                # ---- NEW: report size --------------------------------------
                print(f"Frame {frame_idx + 1}: {len(jpeg):5d} bytes")

                # ---- decode & display --------------------------------------
                img = cv2.imdecode(np.frombuffer(jpeg, np.uint8),
                                   cv2.IMREAD_COLOR)
                if img is None:
                    dropped += 1
                    continue

                frame_idx += 1
                cv2.imshow("ESP32-Cam", img)
                if cv2.waitKey(1) & 0xFF == 27:  # Esc = quit
                    raise KeyboardInterrupt

                # optional save
                if args.save_dir:
                    args.save_dir.mkdir(parents=True, exist_ok=True)
                    (args.save_dir / f"frame_{frame_idx:06d}.jpg").write_bytes(jpeg)

            # ---- lightweight FPS display every 5 s -------------------------
            if frame_idx and time.time() - t0 > 5:
                fps = frame_idx / (time.time() - t0)
                print(f"{fps:.1f} fps   dropped {dropped}")
                t0, frame_idx = time.time(), 0

    except KeyboardInterrupt:
        print("\nExiting…")
    finally:
        ser.close()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
