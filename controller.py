#!/usr/bin/env python3

import math
from typing import Optional, Tuple

import cv2
import numpy as np

try:
    import serial
except Exception:
    serial = None


WINDOW_NAME = "Gimbal Host Controller"
CANVAS_WIDTH = 640
CANVAS_HEIGHT = 480
CENTER_X = CANVAS_WIDTH // 2
CENTER_Y = CANVAS_HEIGHT // 2

PIXEL_TO_STEP_RATIO = 1
KEY_STEP_INCREMENT = 64
ANIMATION_STEP_PER_FRAME = 20.0

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200
INVERT_X_AXIS = False
INVERT_Y_AXIS = False

BG_COLOR = (40, 40, 40)
CROSSHAIR_COLOR = (0, 220, 0)
TARGET_DOT_COLOR = (0, 255, 0)
CURRENT_DOT_COLOR = (0, 0, 255)
TEXT_COLOR = (230, 230, 230)

serial_conn: Optional["serial.Serial"] = None
serial_available = False

target_step_x = 0
target_step_y = 0
current_step_x = 0.0
current_step_y = 0.0


def init_serial() -> None:
    global serial_conn, serial_available
    if serial is None:
        print("[Serial] pyserial is not installed, fallback to print-only mode.")
        serial_available = False
        return

    try:
        serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.05)
        serial_available = True
        print(f"[Serial] Connected: {SERIAL_PORT} @ {BAUD_RATE}")
    except Exception as exc:
        serial_conn = None
        serial_available = False
        print(f"[Serial] Init failed ({exc}), fallback to print-only mode.")


def pixel_to_steps(px: int, py: int) -> Tuple[int, int]:
    cart_x = px - CENTER_X
    cart_y = CENTER_Y - py
    step_x = round(cart_x * PIXEL_TO_STEP_RATIO)
    step_y = round(cart_y * PIXEL_TO_STEP_RATIO)
    return step_x, step_y


def steps_to_pixel(step_x: float, step_y: float) -> Tuple[int, int]:
    px = round(CENTER_X + step_x / PIXEL_TO_STEP_RATIO)
    py = round(CENTER_Y - step_y / PIXEL_TO_STEP_RATIO)
    px = max(0, min(CANVAS_WIDTH - 1, px))
    py = max(0, min(CANVAS_HEIGHT - 1, py))
    return px, py


def send_target_steps(step_x: int, step_y: int) -> None:
    global serial_available, serial_conn
    tx_step_x = -step_x if INVERT_X_AXIS else step_x
    tx_step_y = -step_y if INVERT_Y_AXIS else step_y
    payload = f"<{tx_step_x},{tx_step_y}>\n"

    if serial_available and serial_conn is not None:
        try:
            serial_conn.write(payload.encode("ascii"))
            print(payload.strip())
            return
        except Exception as exc:
            print(f"[Serial] Write failed ({exc}), switching to print-only mode.")
            serial_available = False
            try:
                serial_conn.close()
            except Exception:
                pass
            serial_conn = None

    print(payload.strip())


def send_laser_cmd(cmd: int) -> None:
    global serial_available, serial_conn
    payload = f"<L,{cmd}>\n"

    if serial_available and serial_conn is not None:
        try:
            serial_conn.write(payload.encode("ascii"))
            print(f"[Serial] Laser Trigger Cmd: {cmd} sent.")
            return
        except Exception as exc:
            print(f"[Serial] Write failed ({exc}), switching to print-only mode.")
            serial_available = False
            try:
                serial_conn.close()
            except Exception:
                pass
            serial_conn = None

    print(payload.strip())


def update_target_from_steps(step_x: int, step_y: int) -> None:
    global target_step_x, target_step_y
    target_step_x = step_x
    target_step_y = step_y
    send_target_steps(target_step_x, target_step_y)


def on_mouse(event: int, x: int, y: int, flags: int, param: object) -> None:
    del flags, param
    if event == cv2.EVENT_LBUTTONDOWN:
        step_x, step_y = pixel_to_steps(x, y)
        update_target_from_steps(step_x, step_y)


def draw_crosshair(frame) -> None:
    half = 14
    cv2.line(frame, (CENTER_X - half, CENTER_Y), (CENTER_X + half, CENTER_Y), CROSSHAIR_COLOR, 2)
    cv2.line(frame, (CENTER_X, CENTER_Y - half), (CENTER_X, CENTER_Y + half), CROSSHAIR_COLOR, 2)


def move_current_towards_target() -> None:
    global current_step_x, current_step_y

    dx = float(target_step_x) - current_step_x
    dy = float(target_step_y) - current_step_y
    dist = math.hypot(dx, dy)
    if dist <= 1e-9:
        return

    if dist <= ANIMATION_STEP_PER_FRAME:
        current_step_x = float(target_step_x)
        current_step_y = float(target_step_y)
        return

    ratio = ANIMATION_STEP_PER_FRAME / dist
    current_step_x += dx * ratio
    current_step_y += dy * ratio


def normalize_key_code(key_code: int) -> tuple[int, int]:
    # On Linux/X11, waitKeyEx often prefixes extra bits (e.g. 0x00100077 for 'w').
    return key_code & 0xFF, key_code & 0xFFFF


def handle_key(key_code: int) -> bool:
    if key_code < 0:
        return True

    low8, low16 = normalize_key_code(key_code)
    char_code = low16 if low16 <= 0xFF else None

    if low16 == 0xFF52:
        update_target_from_steps(target_step_x, target_step_y + KEY_STEP_INCREMENT)
        return True
    if low16 == 0xFF54:
        update_target_from_steps(target_step_x, target_step_y - KEY_STEP_INCREMENT)
        return True
    if low16 == 0xFF51:
        update_target_from_steps(target_step_x - KEY_STEP_INCREMENT, target_step_y)
        return True
    if low16 == 0xFF53:
        update_target_from_steps(target_step_x + KEY_STEP_INCREMENT, target_step_y)
        return True

    if char_code == 27: # Esc
        return False

    if char_code in (ord("q"), ord("Q")):
        send_laser_cmd(1)
        return True
    if char_code in (ord("w"), ord("W")):
        send_laser_cmd(2)
        return True
    if char_code in (ord("e"), ord("E")):
        send_laser_cmd(3)
        return True

    dx = 0
    dy = 0

    up_keys_low8 = {ord("i"), ord("I"), ord("8")}
    down_keys_low8 = {ord("k"), ord("K"), ord("2")}
    left_keys_low8 = {ord("j"), ord("J"), ord("4")}
    right_keys_low8 = {ord("l"), ord("L"), ord("6")}

    if char_code in up_keys_low8:
        dy = KEY_STEP_INCREMENT
    elif char_code in down_keys_low8:
        dy = -KEY_STEP_INCREMENT
    elif char_code in left_keys_low8:
        dx = -KEY_STEP_INCREMENT
    elif char_code in right_keys_low8:
        dx = KEY_STEP_INCREMENT
    else:
        return True

    update_target_from_steps(target_step_x + dx, target_step_y + dy)
    return True


def main() -> None:
    init_serial()

    cv2.namedWindow(WINDOW_NAME)
    cv2.setMouseCallback(WINDOW_NAME, on_mouse)

    running = True
    while running:
        frame = np.full((CANVAS_HEIGHT, CANVAS_WIDTH, 3), BG_COLOR, dtype=np.uint8)

        draw_crosshair(frame)

        target_px, target_py = steps_to_pixel(target_step_x, target_step_y)
        move_current_towards_target()
        current_px, current_py = steps_to_pixel(current_step_x, current_step_y)

        cv2.circle(frame, (target_px, target_py), 6, TARGET_DOT_COLOR, -1)
        cv2.circle(frame, (current_px, current_py), 6, CURRENT_DOT_COLOR, -1)

        text = f"Target Steps: X={target_step_x}, Y={target_step_y}"
        cv2.putText(frame, text, (12, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.65, TEXT_COLOR, 2, cv2.LINE_AA)

        cv2.imshow(WINDOW_NAME, frame)
        key = cv2.waitKeyEx(16)
        running = handle_key(key)

    cv2.destroyAllWindows()
    if serial_conn is not None:
        try:
            serial_conn.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()