import tkinter as tk
import ttkbootstrap as ttk
import os

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
SCRIPT_PATH = os.path.dirname(__file__)
RESOURCES_DIR = os.path.join(SCRIPT_PATH, "../2/resources")
APPLICATION_NAME = "Internet das Coisas - Grupo 1"

# Functions
def update_thermometer(value):
    temperature_label.config(text=f"{value} °C")
    value = int(value)
    if value < 10:
        light_color = RED_LIGHT_COLOR
    elif 10 <= value <= 30:
        light_color = GREEN_LIGHT_COLOR
    else:
        light_color = BLUE_LIGHT_COLOR
    lights_canvas.itemconfig(multi_light, fill=light_color)

def toggle_noise():
    global noise_state
    noise_state = not noise_state
    if noise_state:
        lights_canvas.itemconfig(noise_led, fill=YELLOW_LIGHT_COLOR)
    else:
        lights_canvas.itemconfig(noise_led, fill="black")

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
        # Reset to initial state after blinking
        alarm_canvas.itemconfig(alarm_message, fill=ALARM_TEXT_COLOR_OFF)
        alarm_canvas.itemconfig(alarm_background, fill=ALARM_BG_COLOR_OFF)
        blink_counter = 0

def ring_buzzer():
    blink_alarm()

# Root window setup
root = ttk.Window(themename="yeti")
root.title(APPLICATION_NAME)
root.geometry("800x350")
root.iconbitmap("")

# Lights
lights_frame = tk.Frame(root, width=400, height=120)
lights_frame.grid(row=0, column=0, padx=10, pady=10, sticky="n")

lights_canvas = ttk.Canvas(lights_frame, width=400, height=125)  # Adjust width to accommodate both ovals
lights_canvas.grid(row=0, column=0, padx=(10, 0))

multi_light = lights_canvas.create_oval(50, 5, 170, 120, fill=RED_LIGHT_COLOR, outline="black", width=1)
noise_led = lights_canvas.create_oval(230, 5, 350, 120, fill="black", outline="black", width=1)  # Adjust coordinates for the Noise LED

# Buttons
buttons_frame = ttk.Frame(root, width=400, height=100)
buttons_frame.grid(row=1, column=0, padx=10, pady=10, sticky="n")

ttk.Button(buttons_frame, text="Ring Buzzer", command=ring_buzzer, padding=10, bootstyle="danger").grid(row=0, column=0, padx=(0, 5))
ttk.Button(buttons_frame, text="NOISE", command=toggle_noise, padding=10, bootstyle="warning").grid(row=0, column=1, padx=(5, 0))

# Alarm
alarm_frame = ttk.Frame(root, width=400, height=100)
alarm_frame.grid(row=2, column=0, padx=10, pady=10, sticky="n")

alarm_canvas = ttk.Canvas(alarm_frame, width=400, height=100)
alarm_canvas.grid(row=0, column=0)

alarm_background = alarm_canvas.create_rectangle(80, 20, 320, 70, fill=ALARM_BG_COLOR_OFF)
alarm_message = alarm_canvas.create_text(200, 45, text="ALARM", font=("Digital-7", 25), fill=ALARM_TEXT_COLOR_OFF)
alarm_canvas.tag_lower(alarm_background, alarm_message)

# Temperature
temperature_frame = ttk.Frame(root, width=400, height=400)
temperature_frame.grid(row=0, column=1, rowspan=3, padx=10, pady=10, sticky="n")

thermometer_frame = ttk.Frame(temperature_frame)
thermometer_frame.grid(row=0, column=0, pady=10)

temperature_label = ttk.Label(thermometer_frame, text="25 °C", font=("Digital-7", 48), bootstyle="info", width=5)
temperature_label.grid(row=0, column=0, pady=20)

# Slider to simulate temperature change
slider = tk.Scale(temperature_frame, from_=0, to=80, orient=tk.HORIZONTAL, command=update_thermometer, length=300)
slider.grid(row=1, column=0, pady=20)

# Initial state of noise
noise_state = False

root.mainloop()
