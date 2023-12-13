import eventlet
import json
from flask import Flask, render_template
from flask_mqtt import Mqtt
from flask_socketio import SocketIO, emit
from flask_bootstrap import Bootstrap
import RPi.GPIO as gpio
import Adafruit_DHT as dht
import sqlite3
import time

eventlet.monkey_patch()
# app configuration
app = Flask(__name__)
app.config['SECRET_KEY'] = 'qhdks001!'
app.config['MQTT_BROKER_URL'] = 'broker.hivemq.com'  # use the free broker from HIVEMQ
app.config['MQTT_BROKER_PORT'] = 1883  # default port for non-tls connection
app.config['MQTT_USERNAME'] = '1'  # set the username here if you need authentication for the broker
app.config['MQTT_PASSWORD'] = 'qhdks001!'  # set the password here if the broker demands authentication
app.config['MQTT_KEEPALIVE'] = 5  # set the time interval for sending a ping to the broker to 5 seconds
app.config['MQTT_TLS_ENABLED'] = False  # set TLS to disabled for testing purposes

# socketIO,  MQTT, bootstrap setting
socketio = SocketIO(app)
bootstrap = Bootstrap(app)
#############################################

############# MQTT Setting Start ############

#############################################
#mqtt = Mqtt(app)
mqtt = Mqtt()
def create_app():
	app = Flask(__name__)
	mqtt.init_app(app)

@socketio.on('publish')
def handle_publish(json_str):
    data = json.loads(json_str)
    mqtt.publish(data['topic'], data['message'])

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
	mqtt.subscribe('LED/Person') # Green & Red LED
	mqtt.substribe('LED/AirQuality') # Yellow LED
	mqtt.substribe('DHT') # Temperature and Humidity
	mqtt.substribe('')

@socketio.on('subscribe')
def handle_subscribe(json_str):
	data = json.loads(json_str)
	mqtt.subscribe(data['topic'])


@socketio.on('unsubscribe_all')
def handle_unsubscribe_all():
	mqtt.unsubscribe_all()


@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
	data = dict(
		topic=message.topic,
		payload=message.payload.decode()
	)
	socketio.emit('mqtt_message', data=data)

@mqtt.on_log()
def handle_logging(client, userdata, level, buf):
	print(level, buf)

#############################################

############## MQTT Setting End #############

#############################################








@app.route('/')
def hello():
	return render_template('index2.html')

@socketio.on('my event')
def handle_message(message, methods=['GET', 'POST']):
	print("Received message: "  + str(message))
	socketio.emit("my response", message, callback=messageReceived)

def messageReceived(methods=['GET', 'POST']):
	print('message was received');


def dbcon():
	return sqlite3.connect('smarthomeDB')
def seleft_all():
	ret = list()
	try:
		db = dbcon()
		c = db.cursor()
		c.execute('SELECT * FROM DHT')
		ret = c.fetchall()
	except Exception as e:
		print('db error:',  e)
	finally:
		db.close()
		return ret
@app.route('/room1')
def room1():
	return render_template('Room1.html')

@app.route('/room2')
def room2():
	return render_template('Room2.html')

@app.route('/ems')
def ems():
	return render_template('ems.html')
@app.route('/dht')
def myDHT():
	gpio.setwarnings(False)
	# 아래에서 DHT11 센서의 경우 dht.DHT11, DHT22 센서의 경우 dht.DHT22 
	sensor = dht.DHT22

	PWMpin = 18 # PWM pin
	# 라즈베리 파이의 GPIO 21번 핀이 DHT 센서의 데이터 핀(2번 핀)에 연결된 상태인 경우
	DHTpin = 21
	gpio.setmode(gpio.BCM)
	gpio.setup(DHTpin, gpio.IN)
	gpio.setup(PWMpin, gpio.OUT)

	p = gpio.PWM(PWMpin, 50)
	p.start(0)

	try:
		while True:
			humid, temp = dht.read_retry(sensor, DHTpin)
			p.start(0)
			if humid is not None and temp is not None:
				print("Temp=", round(temp,2), " Humid=", round(humid,2))
				return f"Temp={temp}\nHumid={humid}\n"
				if temp >= 26 and humid >= 40:
					p.ChangeDutyCycle(2.5)
					time.sleep(1)
					p.ChangeDutyCycle(15)
					time.sleep(1)
				else:
					p.ChangeDutyCycle(0)
			else:
				print("Failed to get reading")
				time.sleep(0.5)
	except KeyboardInterrupt:
		print("finished")

if __name__=='__main__':
#	app.debug = True
#	app.run(host="127.0.0.1", port = "3000")
	socketio.run(app, debug=True, host="0.0.0.0", use_reloader=False)

