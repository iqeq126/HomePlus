import paho.mqtt.client as mqtt
import RPi.GPIO as gpio
import time
gpio.setwarnings(False)
pin = 18

gpio.setmode(gpio.BCM)
gpio.setup(pin, gpio.OUT)
pwm = gpio.PWM(pin, 50)  # Pulse Width Modulation
pwm.start(0)
# motor's flag
global p_past
p_past = ""

def on_connect(client, userdata, flags, rc):
	print("connected with result code " + str(rc))
	client.subscribe("Window/Servo")

def on_message(client, userdata, msg):
	global p_past
	p = msg.payload.decode("utf-8")
	if "Open" in p and "Open" != p_past:
		pwm.ChangeDutyCycle(7.5)
		time.sleep(0.5)
		p_past = "Open"
		print("Topic : " + msg.topic + " | Message : " + msg.payload.decode("utf-8"))
	if "Close" in p and "Close" != p_past:
		pwm.ChangeDutyCycle(2.5)
		time.sleep(0.5)
		p_past = "Close"
		print("Topic : " + msg.topic + " | Message : " + msg.payload.decode("utf-8"))
	
pwmClient = mqtt.Client()
pwmClient.on_connect = on_connect
pwmClient.on_message = on_message
pwmClient.connect("192.168.8.177", 1883, 60)


try:
	pwmClient.loop_forever()

except KeyboardInterrupt:
	print("Finished!!")
	gpio.cleanup()
	pwmClient.unsubscribe("Window/Servo")
	pwmClient.disconnect()

