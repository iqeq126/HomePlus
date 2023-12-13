import paho.mqtt.client as mqtt
import RPi.GPIO as gpio
import time
gpio.setwarnings(False)
pin = [19, 20]
LED = [0,1]
Room = ['Room1', 'Room2']

global led
led = 0
gpio.setmode(gpio.BCM)
gpio.setup(pin, gpio.OUT)

def on_connect(client, userdata, flags, rc):
	print("connected with result code " + str(rc))
	client.subscribe("House/Air")

def on_message(client, userdata, msg):
	global led
	led = msg.payload.decode("utf-8")
	if "B" in led:
		for i in LED:
			gpio.output(pin[i], gpio.HIGH)
		print("Air quality is bad!")
	if "G" in led:
		for i in LED:
			gpio.output(pin[i], gpio.LOW)
		print("Air quality is good!")
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
	ledClient.unsubscribe("House/Air")
	ledClient.disconnect()

