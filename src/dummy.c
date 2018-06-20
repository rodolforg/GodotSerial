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

#include "godot_serial.h"
#include "serial_interface.h"

typedef struct {
	bool is_open;
	godot_serial_config config;
	godot_string port;
} data_struct;

static GDCALLINGCONV void * constructor(godot_object *p_instance, void *p_method_data) {
	data_struct *data = api->godot_alloc(sizeof(data_struct));
	data->is_open = false;
	data->config = SERIAL_8N1;
/*	data->port = NULL;*/

	return data;
}

static GDCALLINGCONV void destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
	data_struct *data = (data_struct *) p_user_data;
	api->godot_string_destroy(&data->port);
	api->godot_free(p_user_data);
}

static GDCALLINGCONV godot_variant open(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_string data;
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	bool success = false;
	
	if (!user_data->is_open) {
		user_data->is_open = true;
		user_data->config = SERIAL_8N1;
		api->godot_string_new(&user_data->port);
		api->godot_string_parse_utf8(&user_data->port, "COM2");
		
		success = true;
	}

	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static GDCALLINGCONV godot_variant close(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_string data;
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (user_data->is_open) {
		// do close
		user_data->is_open = false;
	}
	
	api->godot_variant_new_bool(&ret, true);
	return ret;
}

static GDCALLINGCONV godot_variant is_connected(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_string data;
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	api->godot_variant_new_bool(&ret, user_data->is_open);
	return ret;
}

static GDCALLINGCONV godot_variant available_for_read(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
static GDCALLINGCONV godot_variant available_for_write(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

static GDCALLINGCONV godot_variant flush(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

static GDCALLINGCONV godot_variant peek(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
static GDCALLINGCONV godot_variant read(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
static GDCALLINGCONV godot_variant read_string(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
static GDCALLINGCONV godot_variant write(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

godot_serial_interface godot_serial_implementation = {constructor, destructor, open, close, is_connected};
