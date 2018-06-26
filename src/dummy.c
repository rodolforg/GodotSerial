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
#include <string.h>

typedef struct {
	bool is_open;
	godot_serial_config config;
	godot_string port;
	int timeout;

	char buffer[256];
	unsigned char read_ptr;
	unsigned char write_ptr;
} data_struct;

static GDCALLINGCONV void * constructor(godot_object *p_instance, void *p_method_data) {
	data_struct *data = api->godot_alloc(sizeof(data_struct));
	data->is_open = false;
	data->config = SERIAL_8N1;
	api->godot_string_new(&data->port);
	
	data->read_ptr = 0;
	data->write_ptr = 0;

	return data;
}

static GDCALLINGCONV void destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
	data_struct *data = (data_struct *) p_user_data;
	api->godot_string_destroy(&data->port);
	api->godot_free(p_user_data);
}

static GDCALLINGCONV godot_variant open(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
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
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	api->godot_variant_new_bool(&ret, user_data->is_open);
	return ret;
}

static int _available_for_read(const data_struct * user_data) {
	int val;
	if (user_data->write_ptr >= user_data->read_ptr)
		val = user_data->write_ptr - user_data->read_ptr;
	else
		val = 256 - user_data->read_ptr + user_data->write_ptr;
	return val;
}

static GDCALLINGCONV godot_variant available_for_read(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;

	int val = _available_for_read(user_data);
	
	api->godot_variant_new_int(&ret, val);
	return ret;
}

static GDCALLINGCONV godot_variant available_for_write(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	int val;
	data_struct * user_data = (data_struct *) p_user_data;

	if (user_data->write_ptr >= user_data->read_ptr)
		val = 256 - user_data->write_ptr + user_data->read_ptr;
	else
		val = user_data->read_ptr - user_data->write_ptr;
	
	api->godot_variant_new_int(&ret, val);
	return ret;
}


static GDCALLINGCONV godot_variant flush(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	api->godot_variant_new_nil(&ret);
	return ret;
}

static GDCALLINGCONV godot_variant peek(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	int val;
	data_struct * user_data = (data_struct *) p_user_data;

	val = user_data->buffer[user_data->read_ptr];
	
	api->godot_variant_new_int(&ret, val);
	return ret;
}

static GDCALLINGCONV godot_variant read(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	int val;
	data_struct * user_data = (data_struct *) p_user_data;

	val = user_data->buffer[user_data->read_ptr];
	user_data->read_ptr++;

	api->godot_variant_new_int(&ret, val);
	return ret;
}

static GDCALLINGCONV godot_variant read_string(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	godot_string string;
	data_struct * user_data = (data_struct *) p_user_data;
	
	char str[256];
	int max_length = _available_for_read(user_data);
	if (max_length == 0) {

		api->godot_variant_new_nil(&ret);
		return ret;
	}
	
	if (user_data->write_ptr > user_data->read_ptr) {
		memcpy(str, user_data->buffer + user_data->read_ptr, max_length);
	} else {
		memcpy(str, user_data->buffer + user_data->read_ptr, 256-user_data->read_ptr);
		if (user_data->write_ptr > 0)
			memcpy(str + 256-user_data->read_ptr, user_data->buffer, user_data->write_ptr);
	}
	
	api->godot_string_new(&string);
	godot_bool successful_parsing = GODOT_FALSE;
	
	while (max_length > 0 && successful_parsing == GODOT_FALSE) {
		successful_parsing = ! api->godot_string_parse_utf8_with_len(&string, str, max_length);
		if (successful_parsing == GODOT_FALSE)
			max_length--;
	}

	if (max_length <= 0) {
		api->godot_variant_new_nil(&ret);
	} else {
		api->godot_variant_new_string(&ret, &string);
		user_data->read_ptr += max_length;
	}
	
	api->godot_string_destroy(&string);

	return ret;
}

static GDCALLINGCONV godot_variant write(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;

	char data_to_write[256];
	int ptr;
	int num_errors = 0;
	for (int n_arg = 0; n_arg < p_num_args; n_arg++) {
		ptr = 0;
		bool valid_arg = true;
		switch (api->godot_variant_get_type(p_args[n_arg])) {
		case GODOT_VARIANT_TYPE_BOOL: {
			godot_bool val = api->godot_variant_as_bool(p_args[n_arg]);
			if (val == GODOT_FALSE) {
				memcpy(data_to_write, "false", 5);
				ptr = 5;
			} else {
				memcpy(data_to_write, "true", 4);
				ptr = 4;
			}
			break;
		}
/*		case GODOT_VARIANT_TYPE_INT:*/
/*		case GODOT_VARIANT_TYPE_REAL:*/
/*			break;*/
		case GODOT_VARIANT_TYPE_STRING: {
			godot_string str = api->godot_variant_as_string(p_args[n_arg]);
			godot_char_string cstr = api->godot_string_utf8(&str);
			int length = api->godot_char_string_length(&cstr);
			const char * val = api->godot_char_string_get_data(&cstr);
			memcpy(data_to_write, val, length);
			ptr = length;
			api->godot_char_string_destroy(&cstr);
			api->godot_string_destroy(&str);
			break;
		}
		default:
			valid_arg = false;
		}
		
		if (!valid_arg) {
			num_errors ++;
			continue;
		}
		for (int i = 0; i < ptr; i++) {
			user_data->buffer[user_data->write_ptr] = data_to_write[i];
			user_data->write_ptr++;
		}
	}

	api->godot_variant_new_int(&ret, num_errors);
	return ret;
}

static GDCALLINGCONV godot_variant set_timeout(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	bool success = false;
	
	if (p_num_args > 0 && api->godot_variant_get_type(p_args[0]) == GODOT_VARIANT_TYPE_INT) {
		user_data->timeout = api->godot_variant_as_int(p_args[0]);
		success = true;
	}
	
	api->godot_variant_new_bool(&ret, success);
	return ret;
}

godot_serial_interface godot_serial_implementation = {constructor, destructor,
                                                      open, close, is_connected,
                                                      available_for_read, available_for_write,
                                                      flush, peek, read, read_string, write,
                                                      set_timeout};
