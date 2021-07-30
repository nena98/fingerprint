import time
import serial
import PIL.Image as Image

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
		time.sleep(5)
		out1 = bytearray(b'')
		while ser.inWaiting() > 0:
			out1.extend(ser.read(1))
			time.sleep(0.001)
		start_index = out1.find(b'<I>')
		end_index = out1.find(b'</I>')
		print(start_index)
		print(end_index)
		out1 = out1[(start_index+3):end_index]
		
		img = Image.new('RGB', (176, 176))
		img.putdata(out1)
		img.save('fingerprint.png')

