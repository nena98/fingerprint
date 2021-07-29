import time
import serial

# configure the serial connections
ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=9600,
    parity='N',
    stopbits=1,
    bytesize=8)

ser.isOpen()

while 1 :
	# get keyboard input
	user_input = input(">> ")
	if user_input == '5':
		# send the character to the device
		ser.write(user_input.encode())
		out = ''
		# let's wait one second before reading output (let's give device time to answer)
		time.sleep(1)
		while ser.inWaiting() > 0:
			out += (ser.read(1)).decode('ascii')

		if out != '':
			print(">>" + out)
		
		#wait for user to put fingerprint
		time.sleep(10)
		while ser.inWaiting() > 0:
			out1 = ser.read(1)
			print(out1)


