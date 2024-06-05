import paho.mqtt.client as mqtt

broker_address = "192.168.0.101"  # Localhost
port = 1883  # Use the port where your broker is running

client = mqtt.Client(client_id="Grupo1Subscriber")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
        client.subscribe("/ic/Grupo1")
    else:
        print("Connection failed with code", rc)

def on_message(client, userdata, msg):
    print(f"Received message: {msg.payload.decode()} on topic {msg.topic}")
    print(msg)

client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_address, port)

client.loop_forever()