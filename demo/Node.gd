extends Node

# load the SIMPLE library
onready var Serial = preload("res://addons/serial/libserial.gdns").new()

func _ready():
	print(Serial.open())
	print(Serial.is_connected())
	print(Serial.close())
	print(Serial.is_connected())