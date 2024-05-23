import paho.mqtt.client as mqtt
import time

broker_address = "127.0.0.1"  # Localhost
port = 1883  # Use the port where your broker is running

client = mqtt.Client(client_id="TestPublisher")


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
    else:
        print("Connection failed with code", rc)


client.on_connect = on_connect

client.connect(broker_address, port)

client.loop_start()

try:
    while True:
        client.publish("test/topic", "Hello MQTT")
        print("Message Published")
        time.sleep(5)
except KeyboardInterrupt:
    print("Exiting")
    client.loop_stop()
    client.disconnect()