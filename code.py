from adafruit_display_text.label import Label
import adafruit_display_text.scrolling_label
import adafruit_imageload
import board
import busio
import displayio
import framebufferio
import random
import rgbmatrix
import terminalio
import sys
import time
import re

def p1scroll(line):
    line.y = line.y - 1
    line_width = line.bounding_box[2]
    if line.y < -(len(line.text) * 6):
        line.y = display.width

def p2scroll(line):
    line.y = line.y + 1
    line_width = line.bounding_box[2]
    if line.y >= len(line.text) * 6:
        #line.y = display.width
        line.y = -line_width


displayio.release_displays()
matrix = rgbmatrix.RGBMatrix(
    width=32,
    bit_depth=4,
    rgb_pins=[
        board.MTX_R1,
        board.MTX_G1,
        board.MTX_B1,
        board.MTX_R2,
        board.MTX_G2,
        board.MTX_B2,
    ],
    addr_pins=[board.MTX_ADDRA, board.MTX_ADDRB, board.MTX_ADDRC, board.MTX_ADDRD],
    clock_pin=board.MTX_CLK,
    latch_pin=board.MTX_LAT,
    output_enable_pin=board.MTX_OE,
)
display = framebufferio.FramebufferDisplay(matrix, rotation=0, auto_refresh=False)

splash = displayio.Group()

filename = "running-monty-mole.bmp"
odb = displayio.OnDiskBitmap(filename)
face = displayio.TileGrid(odb, pixel_shader=odb.pixel_shader)
splash.append(face)

class Splash():
    def __init__(self, x):
        self.x = x

    def update(self):
        face.x -= 1
        if face.x < -55:
            face.x = 32

splashObj = Splash(face.x)

player_group = displayio.Group()

p1_label = adafruit_display_text.label.Label(
    text="Whack A Mole Attack      ",
    font=terminalio.FONT,
    color=0x33FF55,
    line_spacing=0.7,
    scale=1,
    label_direction="DWR",
)
p1_label.x = 8
p1_label.y = -4
player_group.append(p1_label)

p2_label = adafruit_display_text.label.Label(
    text="Whack A Mole Attack      ",
    font=terminalio.FONT,
    color=0xFF99FF,
    line_spacing=0.7,
    scale=1,
    label_direction="UPR",
)
p2_label.x = 24
p2_label.y = display.width
player_group.append(p2_label)

do_scroll = True


uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0)

patt = re.compile(b"#p1:([0-9]+)#p2:([0-9]+)#loop:([0-9]+)")

try:
    f = open("highscore.txt", "r")
    lines = f.readlines()
    highscore = lines[0]
except:
    highscore = 0

scroll_increment = 125
loop_idx = 0

display.root_group = player_group
showSplash = False
wasShowSplash = False
while True:

    if showSplash:
        wasShowSplash = True
        display.root_group = splash
        splashObj.update()
    if wasShowSplash and not showSplash:
        wasShowSplash = False
        display.root_group = player_group

    display.refresh(minimum_frames_per_second=0)

    data = uart.read(32)

    # We've received some serial data, parse it and figure out what to display
    if data is not None:

        for line in data.splitlines():
            print(line)
            match = patt.search(data)
            if match is not None:
                do_scroll = False

                p1score = int(match.group(1))
                p2score = int(match.group(2))
                loop = int(match.group(3))

                print(
                    "Received Score P1:{} P2:{} Loop:{}".format(p1score, p2score, loop)
                )
                p1_label.text = "{:>6d}".format(p1score)
                p2_label.text = "{:>6d}".format(p2score)

            else:
                if data.startswith(b"#CONREADY"):
                    # Initialize the serial connection between us and the controller
                    uart.write("DSPREADY")
                    showSplash = False
                elif data.startswith(b'#SHOWSPLASH'):
                    do_scroll = False
                    showSplash = True
                elif data.startswith(b'#SHOWPRESSSTART'):
                    do_scroll = True
                    p1_label.text = "Press Start      "
                    p2_label.text = "Press Start      "
                    showSplash = False
                elif data.startswith(b"#SHOWHIGHSCORE"):
                    do_scroll = True
                    p1_label.text = "Highscore: {}      ".format(highscore)
                    p2_label.text = "Highscore: {}      ".format(highscore)
                    showSplash = False
                elif data.startswith(b"#NEWGAME"):
                    do_scroll = True
                    p1_label.text = "New Game       "
                    p2_label.text = "New Game       "
                    showSplash = False
                elif data.startswith(b"#GAMEREADY:READY"):
                    do_scroll = False
                    p1_label.text = " READY "
                    p2_label.text = " READY "
                elif data.startswith(b"#ISP2READY"):
                    do_scroll = True
                    p1_label.text = "Waiting          "
                    showSplash = False
                elif data.startswith(b"#ISP1READY"):
                    do_scroll = True
                    p2_label.text = "Waiting          "
                    showSplash = False
                elif data.startswith(b"#GAMEREADY:SET"):
                    do_scroll = False
                    p1_label.text = "  SET "
                    p2_label.text = "  SET  "
                elif data.startswith(b"#GAMEREADY:GO"):
                    do_scroll = False
                    p1_label.text = "  GO! "
                    p2_label.text = "  GO!  "
                elif data.startswith(b"#PLAYER1WIN"):
                    do_scroll = True
                    score = int(data.split(b":")[1])
                    p2_label.text = "Press Start      "
                    if score > highscore:
                        p1_label.text = "WINNER! NEW HIGHSCORE! {}     ".format(score)
                        highscore = score
                    else:
                        p1_label.text = "YOU WIN {}      ".format(score)
                elif data.startswith(b"#PLAYER2WIN"):
                    do_scroll = True
                    score = int(data.split(b":")[1])
                    p1_label.text = "Press Start      "
                    if score > highscore:
                        p2_label.text = "WINNER! NEW HIGHSCORE! {}     ".format(score)
                        highscore = score
                    else:
                        p2_label.text = "YOU WIN {}      ".format(score)
                elif data.startswith(b"#TIEGAME:"):
                    do_scroll = True
                    score = int(data.split(b":")[1])
                    p1_label.text = "TIE GAME {}      ".format(score)
                    p2_label.text = "TIE GAME {}      ".format(score)

                else:
                    do_scroll = True
                    print("unknown {}".format(data))
                    p1_label.text = "unknown: {}     ".format(data)
                    p2_label.text = "unknown: {}     ".format(data)

    loop_idx += 1
    if do_scroll:
        if loop_idx >= scroll_increment:
            p1scroll(p1_label)
            p2scroll(p2_label)
            loop_idx = 0
    else:
        p1_label.x = 8
        p1_label.y = -4
        p2_label.x = 24
        p2_label.y = 36

    display.refresh()
