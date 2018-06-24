/**
* Godot Serial
*   Adding serial port communication for Godot Engine
* Copyright (c) 2018 Rodolfo Ribeiro Gomes
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef GODOT_SERIAL_H
#define GODOT_SERIAL_H

#include <gdnative_api_struct.gen.h>

typedef enum {
	SERIAL_5N1 = 0x501,
	SERIAL_6N1 = 0x601,
	SERIAL_7N1 = 0x701,
	SERIAL_8N1 = 0x801, // default
	SERIAL_5N2 = 0x502,
	SERIAL_6N2 = 0x602,
	SERIAL_7N2 = 0x702,
	SERIAL_8N2 = 0x802,
	SERIAL_5E1 = 0x521,
	SERIAL_6E1 = 0x621,
	SERIAL_7E1 = 0x721,
	SERIAL_8E1 = 0x821,
	SERIAL_5E2 = 0x522,
	SERIAL_6E2 = 0x622,
	SERIAL_7E2 = 0x722,
	SERIAL_8E2 = 0x822,
	SERIAL_5O1 = 0x531,
	SERIAL_6O1 = 0x631,
	SERIAL_7O1 = 0x731,
	SERIAL_8O1 = 0x831,
	SERIAL_5O2 = 0x532,
	SERIAL_6O2 = 0x632,
	SERIAL_7O2 = 0x732,
	SERIAL_8O2 = 0x832,
} godot_serial_config;

#define GODOT_SERIAL_BIT_LENGTH_MASK 0xF00
#define GODOT_SERIAL_PARITY_MASK 0x0F0
#define GODOT_SERIAL_STOP_BIT_MASK 0x00F

typedef struct {
	GDCALLINGCONV void * (*constructor) (godot_object *p_instance, void *p_method_data);
	GDCALLINGCONV void (*destructor) (godot_object *p_instance, void *p_method_data, void *p_user_data);

	GDCALLINGCONV godot_variant (*open)(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*close) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	
	GDCALLINGCONV godot_variant (*is_connected) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	
	GDCALLINGCONV godot_variant (*available_for_read) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*available_for_write) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

	GDCALLINGCONV godot_variant (*flush) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

	GDCALLINGCONV godot_variant (*peek) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*read) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*read_string) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*write) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
} godot_serial_interface;

extern godot_serial_interface godot_serial_implementation;

#endif // GODOT_SERIAL_H
