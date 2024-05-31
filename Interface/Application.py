import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt

# Colors
RED_LIGHT_COLOR = "red"
GREEN_LIGHT_COLOR = "green"
BLUE_LIGHT_COLOR = "blue"
YELLOW_LIGHT_COLOR = "yellow"
ALARM_TEXT_COLOR_ON = "red"
ALARM_TEXT_COLOR_OFF = "white"
ALARM_BG_COLOR_ON = "darkred"
ALARM_BG_COLOR_OFF = "gray"

# Application setup
APPLICATION_NAME = "Internet das Coisas - Grupo 1"

# MQTT setup
MQTT_SERVER = "127.0.0.1"  # Replace with your MQTT server's IP address
MQTT_PORT = 1883
TOPIC = "/ic/Grupo1"

# MQTT client setup
client = mqtt.Client()


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
    handle_message(msg.payload.decode())


def handle_message(message):
    if message.startswith("TEMP:"):
        value = int(message.split(":")[1])
        update_thermometer(value)
    elif message.startswith("NOISE:"):
        state = message.split(":")[1] == "1"
        toggle_noise(state)


client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_SERVER, MQTT_PORT, 60)
client.loop_start()

# Functions
previous_temperature = None
previous_noise_state = False


def update_thermometer(value):
    global previous_temperature
    temperature_label.config(text=f"{value} °C")
    if value < 10:
        light_color = RED_LIGHT_COLOR
    elif 10 <= value <= 30:
        light_color = GREEN_LIGHT_COLOR
    else:
        light_color = BLUE_LIGHT_COLOR
    lights_canvas.itemconfig(multi_light, fill=light_color)

    if previous_temperature != value:
        client.publish(TOPIC, f"TEMP:{value}")
        previous_temperature = value


def send_thermometer(value):
    client.publish(TOPIC, f"TEMP:{value}")


def toggle_noise(state=None):
    global previous_noise_state
    if state is None:
        state = not previous_noise_state

    if state:
        lights_canvas.itemconfig(noise_led, fill=YELLOW_LIGHT_COLOR)
    else:
        lights_canvas.itemconfig(noise_led, fill="black")

    if previous_noise_state != state:
        client.publish(TOPIC, f"NOISE:{1 if state else 0}")
        previous_noise_state = state


blink_counter = 0


def blink_alarm():
    global blink_counter
    if blink_counter < 6:
        current_color = alarm_canvas.itemcget(alarm_message, "fill")
        new_color = ALARM_TEXT_COLOR_OFF if current_color == ALARM_TEXT_COLOR_ON else ALARM_TEXT_COLOR_ON
        new_bg_color = ALARM_BG_COLOR_OFF if current_color == ALARM_TEXT_COLOR_ON else ALARM_BG_COLOR_ON

        alarm_canvas.itemconfig(alarm_message, fill=new_color)
        alarm_canvas.itemconfig(alarm_background, fill=new_bg_color)

        blink_counter += 1
        root.after(500, blink_alarm)
    else:
        alarm_canvas.itemconfig(alarm_message, fill=ALARM_TEXT_COLOR_OFF)
        alarm_canvas.itemconfig(alarm_background, fill=ALARM_BG_COLOR_OFF)
        blink_counter = 0


def ring_buzzer():
    blink_alarm()
    client.publish(TOPIC, "BUZZER_ON")
    root.after(5000, lambda: client.publish(TOPIC, "BUZZER_OFF"))


# Root window setup
root = tk.Tk()
root.title(APPLICATION_NAME)
root.geometry("800x350")

# Lights
lights_frame = tk.Frame(root, width=400, height=120)
lights_frame.grid(row=0, column=0, padx=10, pady=10, sticky="n")

lights_canvas = tk.Canvas(lights_frame, width=400, height=125)
lights_canvas.grid(row=0, column=0, padx=(10, 0))

multi_light = lights_canvas.create_oval(50, 5, 170, 120, fill=RED_LIGHT_COLOR, outline="black", width=1)
noise_led = lights_canvas.create_oval(230, 5, 350, 120, fill="black", outline="black", width=1)

# Buttons
buttons_frame = tk.Frame(root, width=400, height=100)
buttons_frame.grid(row=1, column=0, padx=10, pady=10, sticky="n")

tk.Button(buttons_frame, text="Ring Buzzer", command=ring_buzzer, padx=10, pady=5, bg="red", fg="white").grid(row=0,
                                                                                                              column=0,
                                                                                                              padx=(
                                                                                                              0, 5))
tk.Button(buttons_frame, text="NOISE", command=toggle_noise, padx=10, pady=5, bg="yellow", fg="black").grid(row=0,
                                                                                                            column=1,
                                                                                                            padx=(5, 0))

# Alarm
alarm_frame = tk.Frame(root, width=400, height=100)
alarm_frame.grid(row=2, column=0, padx=10, pady=10, sticky="n")

alarm_canvas = tk.Canvas(alarm_frame, width=400, height=100)
alarm_canvas.grid(row=0, column=0)

alarm_background = alarm_canvas.create_rectangle(80, 20, 320, 70, fill=ALARM_BG_COLOR_OFF)
alarm_message = alarm_canvas.create_text(200, 45, text="ALARM", font=("Digital-7", 25), fill=ALARM_TEXT_COLOR_OFF)
alarm_canvas.tag_lower(alarm_background, alarm_message)

# Temperature
temperature_frame = tk.Frame(root, width=400, height=400)
temperature_frame.grid(row=0, column=1, rowspan=3, padx=10, pady=10, sticky="n")

thermometer_frame = tk.Frame(temperature_frame)
thermometer_frame.grid(row=0, column=0, pady=10)

temperature_label = tk.Label(thermometer_frame, text="25 °C", font=("Digital-7", 48), bg="blue", fg="white", width=5)
temperature_label.grid(row=0, column=0, pady=20)

slider = tk.Scale(temperature_frame, from_=0, to=80, orient=tk.HORIZONTAL, command=send_thermometer, length=300)
slider.grid(row=1, column=0, pady=20)

noise_state = False

root.mainloop()
