import paho.mqtt.client as mqtt

broker_address = "127.0.0.1"  # Localhost
port = 1883  # Use the port where your broker is running

client = mqtt.Client(client_id="TestSubscriber")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
        client.subscribe("test/topic")
    else:
        print("Connection failed with code", rc)

def on_message(client, userdata, msg):
    print(f"Received message: {msg.payload.decode()} on topic {msg.topic}")

client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_address, port)

client.loop_forever()