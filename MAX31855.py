"""
This is a library for the octo MAX31855 thermocouple breakout board.

MIT License

Copyright (c) 2020 Mitchell Herbert

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import RPi.GPIO as GPIO
from time import sleep

class MAX31855:
	"""
	Initializes GPIO pins and instance variables.
	CS initializes to high because it is active low.
	SO is an input pin.

	:param SCK: the BCM pin number of the SCK line
	:param CS: the BCM pin number of the CS line
	:param SO: the BCM pin number of the SO line
	:param T0: the BCM pin number of the T0 line
	:param T1: the BCM pin number of the T1 line
	:param T2: the BCM pin number of the T2 line
	"""
	def __init__(self, SCK, CS, SO, T0, T1, T2):
		GPIO.setmode(GPIO.BCM)
		# Setup all of the GPIO pins
		for pin_number in [SCK, T0, T1, T2]:
			GPIO.setup(pin_number, GPIO.OUT)
			GPIO.output(pin_number, 0)
		GPIO.setup(CS, GPIO.OUT)
		GPIO.output(CS, 1)
		GPIO.setup(SO, GPIO.IN)
		# Initialize instance variables
		self.SCK = SCK
		self.CS = CS
		self.SO = SO
		self.T0 = T0
		self.T1 = T1
		self.T2 = T2
		# Initialize the poll data to zero
		self.latest_data = 0b0

	"""
	Communicates with the octo MAX31855 board to retrieve
	temperature and fault data. The data is stored in
	self.latest_data for later reference.

	:param therm_id: id of the thermocouple (0 - 7)
	"""
	def read_data(self, therm_id):
		# Select the thermocouple using multiplexer
		GPIO.output(self.T2, therm_id & 0b100)
		GPIO.output(self.T1, therm_id & 0b10)
		GPIO.output(self.T0, therm_id & 0b1)
		# Wait for the multiplexer to update
		sleep(0.125)
		# Select the chip and record incoming data
		data = 0b0
		GPIO.output(self.CS, 0)
		# Shift in 32 bits of data
		for bitshift in reversed(range(0, 32)):
			GPIO.output(self.SCK, 1)
			data += GPIO.input(self.SO) << bitshift
			GPIO.output(self.SCK, 0)
		GPIO.output(self.CS, 1)
		self.latest_data = data

	"""
	Gets the temperature of the most recently polled
	thermocouple.

	:returns: float representing the temperature in celsius
	"""
	def get_thermocouple_temp(self):
		data = self.latest_data
		# Select appropriate bits
		data = data >> 18
		# Handle twos complement
		if data >= 0x2000:
			data = -((data ^ 0x3fff) + 1)
		# Divide by 4 to handle fractional component
		return data / 4

	"""
	Gets the temperature of the reference junction from
	the most recent poll.

	:returns: float representing the temperature in celsius
	"""
	def get_reference_temp(self):
		data = self.latest_data
		# Select appropriate bits
		data = (data & 0xfff0) >> 4
		# Handle twos complement
		if data & 0x800:
			data = -((data ^ 0xfff) + 1)
		# Divide by 16 to handle fractional component
		return data / 16

	"""
	Returns a value signififying a particular fault in the most
	recent poll.

	0 indicates that no faults exist
	1 indicates an SCV fault (thermocouple is shorted to VCC)
	2 indicates an SCG fault (thermocouple is shorted to GND)
	3 indicates an OC fault (the thermocouple is not connected)

	:returns: an integer representing the fault
	"""
	def get_faults(self):
		data = self.latest_data
		if data & 0x00010000:
			if data & 0b100:
				return 1
			if data & 0b10:
				return 2
			if data & 0b1:
				return 3
		return 0

	"""
	Should be called at the end of program execution to bring
	all GPIO pins to a 'safe' state.
	"""
	def cleanup(self):
		GPIO.cleanup()


	"""
	Returns the value of latest_data

	:returns: the value of latest_data
	"""
	def get_latest_data(self):
		return self.latest_data

