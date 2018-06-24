extends Node

# load the SIMPLE library
onready var Serial = preload("res://addons/serial/libserial.gdns").new()

func _ready():
	print(Serial.open())
	print(Serial.is_connected())
	print(Serial.close())
	print(Serial.is_connected())
	
	print(Serial.open())
	print(Serial.available())
	print(Serial.available_for_write())
	print(Serial.write("oi"))
	print(Serial.available())
	print(Serial.available_for_write())
	print(char(Serial.peek()))
	print(Serial.read_string())
	print(Serial.available())
	print(Serial.available_for_write())
	print(Serial.close())
	
	print(preload("res://addons/serial/libserial.gdns").new().peek())