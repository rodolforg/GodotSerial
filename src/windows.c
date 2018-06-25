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
#include <windows.h>

typedef struct {
	bool is_open;
	godot_serial_config config;
	godot_string port;
	
	HANDLE hComm;

	char read_buffer[256];
	unsigned char read_ptr;
	unsigned char write_ptr;
} data_struct;

static GDCALLINGCONV void * constructor(godot_object *p_instance, void *p_method_data) {
	data_struct *data = api->godot_alloc(sizeof(data_struct));
	data->is_open = false;
	data->config = SERIAL_8N1;
	api->godot_string_new(&data->port);
	
	data->hComm = INVALID_HANDLE_VALUE;
	
	data->read_ptr = 0;
	data->write_ptr = 0;

	return data;
}

static GDCALLINGCONV void destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
	data_struct *data = (data_struct *) p_user_data;
	api->godot_string_destroy(&data->port);
	api->godot_free(p_user_data);
}

static bool _open(const char* port_name, godot_serial_config config) {
	HANDLE hComm;
	hComm = CreateFile(
	                  port_name, //Nome da porta.
	                  GENERIC_READ | GENERIC_WRITE, //Para leitura e escrita.
	                  0, //(Zero) Nenhuma outra abertura será permitida.
	                  NULL, //Atributos de segurança. (NULL) padrão.
	                  OPEN_EXISTING, //Criação ou abertura.
	                  0, //Entrada e saída sem overlapped.
	                  NULL //Atributos e Flags. Deve ser NULL para COM.
	);

	if (hComm == INVALID_HANDLE_VALUE)
		return false; //Erro ao tentar abrir a porta especificada.	

	DCB dcb; //Estrutura utilizada para definir todos os parâmetros da comunicação.
	if( !GetCommState(hComm, &dcb))
		return false; // Erro na leitura de DCB.
	
	const int bitlength = config & GODOT_SERIAL_BIT_LENGTH_MASK;
	const int parity = config & GODOT_SERIAL_STOP_BIT_MASK;
	const int stopbits = config & GODOT_SERIAL_STOP_BIT_MASK;
	
	dcb.BaudRate = CBR_19200;
	dcb.ByteSize = bitlength;
	dcb.Parity = parity == 0 ? NOPARITY : parity == 1 ? ODDPARITY : EVENPARITY;
	dcb.StopBits = stopbits == 1 ? ONESTOPBIT : stopbits == 2 ? TWOSTOPBITS : ONE5STOPBITS;
	//Define novo estado.
	if( SetCommState(hComm, &dcb) == 0 )
		return false; //Erro.
	
	return true;
}

static GDCALLINGCONV godot_variant open(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	bool success = false;
	if (p_num_args >= 1) {
		godot_string port_name_str = api->godot_variant_as_string(p_args[0]);
		godot_char_string port_name_ascii_str = api->godot_string_ascii(&port_name_str);
		
		godot_serial_config port_config = SERIAL_8N1;
		if (p_num_args >= 2) {
			if (api->godot_variant_get_type(p_args[1]) == GODOT_VARIANT_TYPE_INT) {
				port_config = api->godot_variant_as_int(p_args[1]);
			} else if (api->godot_variant_get_type(p_args[1]) == GODOT_VARIANT_TYPE_STRING) {
				godot_string port_config_str = api->godot_variant_as_string(p_args[1]);
				godot_char_string port_config_ascii_str = api->godot_string_ascii(&port_config_str);
				if (api->godot_char_string_length(&port_config_ascii_str) != 3) {
					port_config = 0;
				} else {
					const char *ascii_data = api->godot_char_string_get_data(&port_config_ascii_str);
					port_config  =  (ascii_data[2] - '0')         & 0x000f;
					port_config |= ((ascii_data[0] - '0') << 16) && 0x0f00;
					if (ascii_data[1] == 'O' || ascii_data[1] == 'o')
						port_config |= 0x030;
					else if (ascii_data[1] == 'E' || ascii_data[1] == 'e')
						port_config |= 0x020;
					else if (ascii_data[1] != 'N' && ascii_data[1] != 'n') // last valid option
						port_config = 0;
				}
				api->godot_char_string_destroy(&port_config_ascii_str);
				api->godot_string_destroy(&port_config_str);
			} else {
				port_config = 0;
			}
		}
		
		if (port_config != 0 && api->godot_char_string_length(&port_name_ascii_str) > 0 && !user_data->is_open) {
			const char *port_name_ascii_str_buffer = api->godot_char_string_get_data(&port_name_ascii_str);
			if (_open(port_name_ascii_str_buffer, port_config)) {
				user_data->is_open = true;
				user_data->config = SERIAL_8N1;
				api->godot_string_new_copy(&user_data->port, &port_name_str);
				
				success = true;
			}
		}
		api->godot_char_string_destroy(&port_name_ascii_str);
		api->godot_string_destroy(&port_name_str);
	}

	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static GDCALLINGCONV godot_variant close(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (user_data->is_open) {
		// do close
		CloseHandle(user_data->hComm);
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
	
	api->godot_variant_new_int(&ret, 256);
	return ret;
}


static GDCALLINGCONV godot_variant flush(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	FlushFileBuffers(user_data->hComm);

	api->godot_variant_new_nil(&ret);
	return ret;
}

static int _read_and_buffer(data_struct * user_data) {
	int val;
	DWORD dwRead1 = 0;
	DWORD dwRead2 = 0;

	if (256 - _available_for_read(user_data) <= 0) { // buffer is full
		return -1;
	}
	if (user_data->write_ptr >= user_data->read_ptr) {
		int first_try_length = 256 - user_data->write_ptr;
		if (FALSE == ReadFile(user_data->hComm, &user_data->read_buffer[user_data->write_ptr], first_try_length, &dwRead1, NULL)) {
			return -1;
		} else {
			user_data->write_ptr += dwRead1;
			if (dwRead1 < first_try_length)
				return dwRead1;
			int second_try_length = user_data->read_ptr;
			if (FALSE == ReadFile(user_data->hComm, &user_data->read_buffer[0], second_try_length, &dwRead2, NULL)) {
				return dwRead1;
			} else {
				user_data->write_ptr += dwRead2;
				return dwRead1+dwRead2;
			}
		}
	} else {
		int single_try_length = user_data->read_ptr - user_data->write_ptr;
		if (FALSE == ReadFile(user_data->hComm, &user_data->read_buffer[user_data->write_ptr], single_try_length, &dwRead1, NULL)) {
			return -1;
		} else {
			user_data->write_ptr += dwRead1;
			return dwRead1;
		}
	}
}

static GDCALLINGCONV godot_variant peek(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	int val;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (_available_for_read(user_data) < 1)
		_read_and_buffer(user_data);

	if (_available_for_read(user_data) > 0)
		val = user_data->read_buffer[user_data->read_ptr];
	else
		val = -1;
	
	api->godot_variant_new_int(&ret, val);
	return ret;
}

static GDCALLINGCONV godot_variant read(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	int val;
	data_struct * user_data = (data_struct *) p_user_data;

	if (_available_for_read(user_data) < 1)
		_read_and_buffer(user_data);

	if (_available_for_read(user_data) > 0) {
		val = user_data->read_buffer[user_data->read_ptr];
		user_data->read_ptr++;
	} else
		val = -1;

	api->godot_variant_new_int(&ret, val);
	return ret;
}

static GDCALLINGCONV godot_variant read_string(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	godot_string string;
	data_struct * user_data = (data_struct *) p_user_data;
	
	char str[256];
	if (_available_for_read(user_data) < 1)
		_read_and_buffer(user_data);
	int max_length = _available_for_read(user_data);
	if (max_length == 0) {

		api->godot_variant_new_nil(&ret);
		return ret;
	}
	
	if (user_data->write_ptr > user_data->read_ptr) {
		memcpy(str, user_data->read_buffer + user_data->read_ptr, max_length);
	} else {
		memcpy(str, user_data->read_buffer + user_data->read_ptr, 256-user_data->read_ptr);
		if (user_data->write_ptr > 0)
			memcpy(str + 256-user_data->read_ptr, user_data->read_buffer, user_data->write_ptr);
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
			num_errors++;
			continue;
		}
		
		DWORD dwBytesWritten = 0;
		BOOL bErrorFlag = FALSE;
		bErrorFlag = WriteFile( 
		                       user_data->hComm,  // open file handle
		                       data_to_write, // start of data to write
		                       ptr,               // number of bytes to write
		                       &dwBytesWritten,   // number of bytes that were written
		                       NULL);             // no overlapped structure

		if (FALSE == bErrorFlag)
		{
		    num_errors++;
		}
		else
		{
		    if (dwBytesWritten != ptr)
		    {
		        num_errors++;
		    }
		    else
		    {
		        // success
		    }
		}

	}

	api->godot_variant_new_int(&ret, num_errors);
	return ret;
}


godot_serial_interface godot_serial_implementation = {constructor, destructor,
                                                      open, close, is_connected,
                                                      available_for_read, available_for_write,
                                                      flush, peek, read, read_string, write};
