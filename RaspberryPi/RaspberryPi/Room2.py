import paho.mqtt.client as mqtt
import RPi.GPIO as gpio
import time
gpio.setwarnings(False)
pin = [16, 21]
LED = [0,1]
rgb = ['Red', 'Green'] 

global led
led = 0
gpio.setmode(gpio.BCM)
gpio.setup(pin, gpio.OUT)

def on_connect(client, userdata, flags, rc):
	print("connected with result code " + str(rc))
	client.subscribe("Room2/entry")

def on_message(client, userdata, msg):
	global led
	led = msg.payload.decode("utf-8")
	if led == "Absence":
		for i in LED:
			gpio.output(pin[i], gpio.LOW)
		print("All LED is Low")
	if led == "Existance":
		for i in LED:
			gpio.output(pin[i], gpio.LOW)
		gpio.output(pin[1], gpio.HIGH)
	if led == "Do not disturb":
		for i in LED:
			gpio.output(pin[1], gpio.LOW)
		gpio.output(pin[0], gpio.HIGH)
	print("Topic : " + msg.topic + " | Message : " + msg.payload.decode("utf-8"))

ledClient = mqtt.Client()
ledClient.on_connect = on_connect
ledClient.on_message = on_message
ledClient.connect("192.168.8.177", 1883, 60)


try:
	ledClient.loop_forever()

except KeyboardInterrupt:
	print("Finished!!")
	gpio.cleanup()
	ledClient.unsubscribe("Room2/entry")
	ledClient.disconnect()

