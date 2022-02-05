import serial

usb_port = '/dev/cu.usbmodem1481101'
usb_port_speed = 19200

measurements = []

with serial.Serial(port=usb_port, baudrate=usb_port_speed) as s:
    data = ""

    while True:
        if s.in_waiting:
            data = s.readline()
            d = data.decode('utf-8').replace('\r\n','').strip()

            if d == 'done':
                print("Done received, stopping loop")
                break

            el = d.split(',')
            value = float(el[0])
            unit = int(el[1])
            counter = int(el[2])

            if int(unit)==1:
                raise ValueError('inches received')

            if abs(value)> 50:
                raise ValueError('measured result out of bounds')

            measurements.append([counter, value])
            print(counter, value)


# save the file
print('Writing file...')
with open('results.csv', x) as f:
    for row in measurements:
        f.write(F'{row[0]},{row[1]}\n')
print("...done")

# finish up
print("Finished")