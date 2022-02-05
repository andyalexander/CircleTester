import math

RADIUS = 100
N_STEPS = 50
n_lines = 0

WAIT_PORT = 2
WAIT_PIN = 10
WAIT_TIME = 50            # milliseconds to wait for the reading request to be read

MOVE_FEED_RATE = 2000

rad_per_step = 2*math.pi / N_STEPS

output_file = open("/Users/andrew/Dropbox/Misc/temp/cirlce_tester_output.tap", "w")

def outputLine(line):
    global n_lines
    n_lines += 1
    s = F"N{n_lines:03d} {line}"

    print(s)
    output_file.write(s + "\n")

def writeIntro():
    outputLine("M206")                  # disable THC
    outputLine("F2000")                 # set feed rate
    outputLine("G91")                   # set relative co-ordinates
    outputLine("G92 X0 Y0 Z0")          # set current position to be 0,0,0
    outputLine(F"G01 X{RADIUS}")        # move to right most part
    outputLine("G90")                   # move to absolute co-ordinates

def writeWaitInput():
    outputLine(F"M66 P{WAIT_PIN} E{WAIT_PORT} L4 Q1")       #wait for falling edge, timeout 1 second


writeIntro()

# current position
curr_x = RADIUS
curr_y = 0.0

dir_type = 2                            # 2 is clockwise, 3 anti

# we compute the exact position we expect to avoid any cumulative errors in the maths

for step_no in range(N_STEPS):
    d = 1 if dir_type==3 else -1        # get the direction multiplier
    angle = d * (step_no+1) * rad_per_step

    new_x = RADIUS * math.cos(angle)
    new_y = RADIUS * math.sin(angle)

    s = F"G0{dir_type:01d} X{new_x:.3f} Y{new_y:.3f} R{RADIUS} F{MOVE_FEED_RATE}"

    outputLine(s)
    outputLine("M07")           # signal to read
    outputLine(f"G04 P{WAIT_TIME}")
    outputLine("M09")
    writeWaitInput()

outputLine("M08")               # signify end
outputLine(f"G04 P{WAIT_TIME}")
outputLine("M09")

outputLine("M30")               # end


output_file.close()