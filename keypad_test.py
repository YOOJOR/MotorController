#!/usr/bin/env python3

import cv2
import numpy as np

try:
    import serial
except Exception:
    serial = None

WINDOW_NAME = "Click-to-Move Controller"
CANVAS_WIDTH = 400
CANVAS_HEIGHT = 200

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

BG_COLOR = (40, 40, 40)
TEXT_COLOR = (230, 230, 230)
ACTIVE_TEXT_COLOR = (0, 255, 0)

serial_conn = None
serial_available = False

# 记录当前状态，防止重复发送
current_tx_x = 0
current_tx_y = 0


def init_serial() -> None:
    global serial_conn, serial_available
    if serial is None:
        print("[Serial] pyserial 库未安装，退化为仅打印模式。")
        serial_available = False
        return

    try:
        serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.05)
        serial_available = True
        print(f"[Serial] 已连接: {SERIAL_PORT} @ {BAUD_RATE}")
    except Exception as exc:
        serial_conn = None
        serial_available = False
        print(f"[Serial] 初始化失败 ({exc})，退化为仅打印模式。")


def send_cmd(tx_x: int, tx_y: int) -> None:
    global serial_available, serial_conn
    payload = f"<{tx_x},{tx_y}>\n"

    if serial_available and serial_conn is not None:
        try:
            serial_conn.write(payload.encode("ascii"))
            print(f"发送至串口: {payload.strip()}")
            return
        except Exception as exc:
            print(f"[Serial] 写入失败 ({exc})，切换为仅打印模式。")
            serial_available = False
            try:
                serial_conn.close()
            except Exception:
                pass
            serial_conn = None

    print(f"模拟发送: {payload.strip()}")


def main() -> None:
    global current_tx_x, current_tx_y
    init_serial()

    cv2.namedWindow(WINDOW_NAME)
    
    running = True
    while running:
        frame = np.full((CANVAS_HEIGHT, CANVAS_WIDTH, 3), BG_COLOR, dtype=np.uint8)

        # 监听键盘，等待 16 毫秒
        key = cv2.waitKeyEx(16)
        
        # 预设目标状态为当前状态
        target_tx_x = current_tx_x
        target_tx_y = current_tx_y

        if key != -1:
            low16 = key & 0xFFFF
            char_code = key & 0xFF
            
            # 1. 触发动作 (方向键上 或 W)
            if low16 == 0xFF52 or char_code in (ord("w"), ord("W")):
                target_tx_x, target_tx_y = 0, 1
            elif low16 == 0xFF54 or char_code in (ord("s"), ord("S")): # 下
                target_tx_x, target_tx_y = 0, -1
            elif low16 == 0xFF51 or char_code in (ord("a"), ord("A")): # 左
                target_tx_x, target_tx_y = -1, 0
            elif low16 == 0xFF53 or char_code in (ord("d"), ord("D")): # 右
                target_tx_x, target_tx_y = 1, 0
                
            # 2. 归位动作 (主键盘的 0 或 小键盘的 0)
            # Linux X11 下小键盘 0 通常是 0xFFB0，Windows 下通常是 0x60 (96)
            elif char_code == ord("0") or low16 == 0xFFB0 or low16 == 96:
                target_tx_x, target_tx_y = 0, 0
                
            # 3. 退出 (Esc)
            elif char_code == 27:
                running = False
                break

        # 如果状态发生了改变，才发送串口指令
        if target_tx_x != current_tx_x or target_tx_y != current_tx_y:
            current_tx_x = target_tx_x
            current_tx_y = target_tx_y
            send_cmd(current_tx_x, current_tx_y)

        # UI 绘制
        status_text = f"Current Output: <{current_tx_x},{current_tx_y}>"
        color = ACTIVE_TEXT_COLOR if (current_tx_x != 0 or current_tx_y != 0) else TEXT_COLOR
        
        cv2.putText(frame, "Press Arrows to Move. Press '0' to Stop.", 
                    (15, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, TEXT_COLOR, 1, cv2.LINE_AA)
        cv2.putText(frame, status_text, 
                    (15, 100), cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2, cv2.LINE_AA)

        cv2.imshow(WINDOW_NAME, frame)
        
        # 窗口被关闭时退出
        if cv2.getWindowProperty(WINDOW_NAME, cv2.WND_PROP_VISIBLE) < 1:
            running = False

    # 退出清理
    cv2.destroyAllWindows()
    if serial_conn is not None:
        send_cmd(0, 0)
        try:
            serial_conn.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()