import paho.mqtt.client as mqtt
import RPi.GPIO as gpio
import time
gpio.setwarnings(False)
pin = [16, 20, 21]
LED = [1,2,3]
rgb = ['Red', 'Yellow', 'Green'] 

global led
led = 0
gpio.setmode(gpio.BCM)
gpio.setup(pin, gpio.OUT)

def on_connect(client, userdata, flags, rc):
	print("connected with result code " + str(rc))
	client.subscribe("Window/Servo1")

def on_message(client, userdata, msg):
	print("Topic : " + msg.topic + " | Message : " + msg.payload.decode("utf-8"))

ledClient = mqtt.Client()
ledClient.on_connect = on_connect
ledClient.on_message = on_message
ledClient.connect("localhost", 1883, 60)


try:
	ledClient.loop_forever()

except KeyboardInterrupt:
	print("Finished!!")
	gpio.cleanup()
	ledClient.unsubscribe("Window/Servo1")
	ledClient.disconnect()

