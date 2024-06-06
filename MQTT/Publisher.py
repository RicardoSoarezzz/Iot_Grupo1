import paho.mqtt.client as mqtt
import random
import time

broker_address = "127.0.0.1"
port = 1883  # Use the port where your broker is running

client = mqtt.Client(client_id="Grupo1")


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
        temp = random.randint(1, 80)
        client.publish("/ic/Grupo1", f"TEMP:{temp}")
        time.sleep(0.5)
        client.publish("/ic/Grupo1", "ALARM:1")
        time.sleep(0.5)
        client.publish("/ic/Grupo1", "NOISE:1")
        time.sleep(0.5)
        client.publish("/ic/Grupo1", "NOISE:0")
        time.sleep(0.5)
        client.publish("/ic/Grupo1", "ALARM:0")
        time.sleep(0.5)
        print("Message Published")

except KeyboardInterrupt:
    print("Exiting")
    client.loop_stop()
    client.disconnect()