#!/usr/bin/env python3

import cv2
import numpy as np

WINDOW_NAME = "Keypad Input Tester"
WIDTH = 800
HEIGHT = 600

UP_KEYS_LOW8 = {ord("w"), ord("W"), ord("8")}
DOWN_KEYS_LOW8 = {ord("s"), ord("S"), ord("2")}
LEFT_KEYS_LOW8 = {ord("a"), ord("A"), ord("4")}
RIGHT_KEYS_LOW8 = {ord("d"), ord("D"), ord("6")}

UP_KEYS_LOW16 = {0xFF52}
DOWN_KEYS_LOW16 = {0xFF54}
LEFT_KEYS_LOW16 = {0xFF51}
RIGHT_KEYS_LOW16 = {0xFF53}


def normalize_key_code(key_code: int) -> tuple[int, int]:
    return key_code & 0xFF, key_code & 0xFFFF


def classify_key(key_code: int) -> str:
    low8, low16 = normalize_key_code(key_code)
    char_code = low16 if low16 <= 0xFF else None

    if low16 in UP_KEYS_LOW16:
        return "UP"
    if low16 in DOWN_KEYS_LOW16:
        return "DOWN"
    if low16 in LEFT_KEYS_LOW16:
        return "LEFT"
    if low16 in RIGHT_KEYS_LOW16:
        return "RIGHT"

    if char_code in UP_KEYS_LOW8:
        return "UP"
    if char_code in DOWN_KEYS_LOW8:
        return "DOWN"
    if char_code in LEFT_KEYS_LOW8:
        return "LEFT"
    if char_code in RIGHT_KEYS_LOW8:
        return "RIGHT"
    if char_code in (ord("q"), ord("Q")):
        return "QUIT"
    return "UNMAPPED"


def draw_screen(last_code: int, last_name: str, history: list[str]) -> np.ndarray:
    frame = np.full((HEIGHT, WIDTH, 3), (40, 40, 40), dtype=np.uint8)
    low8, low16 = normalize_key_code(last_code) if last_code >= 0 else (-1, -1)

    lines = [
        "Keypad Test (OpenCV waitKeyEx)",
        "Press keypad 8/2/4/6 and WASD, press q to quit.",
        "Focused window is required for key capture.",
        f"Last key code: {last_code} (hex: 0x{last_code:08X})",
        f"Normalized low8/low16: 0x{low8:02X} / 0x{low16:04X}",
        f"Classified as: {last_name}",
        "Recent events:",
    ]

    y = 34
    for text in lines:
        cv2.putText(frame, text, (16, y), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (220, 220, 220), 2, cv2.LINE_AA)
        y += 34

    hist_y = y
    for item in history[-6:]:
        cv2.putText(frame, item, (30, hist_y), cv2.FONT_HERSHEY_SIMPLEX, 0.62, (0, 220, 255), 2, cv2.LINE_AA)
        hist_y += 30

    return frame


def main() -> None:
    cv2.namedWindow(WINDOW_NAME)

    last_code = -1
    last_name = "NONE"
    history: list[str] = []

    while True:
        frame = draw_screen(last_code, last_name, history)
        cv2.imshow(WINDOW_NAME, frame)

        key = cv2.waitKeyEx(20)
        if key < 0:
            continue

        last_code = key
        last_name = classify_key(key)
        low8, low16 = normalize_key_code(key)
        event = f"code={key} hex=0x{key:08X} low8=0x{low8:02X} low16=0x{low16:04X} => {last_name}"
        history.append(event)
        print(event)

        if last_name == "QUIT":
            break

    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()