#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "defs.h"
#include "config.h"
#include "StackHost.h"
#include "global.h"
#include "port.h"
#include "net.h"
#include "http.h"
#include "http_parser.h"
#ifdef WIN32
#include "util/pthread.h"
#else
#include <pthread.h>
#endif

static int http_url_callback(http_parser* parser, const char *at, size_t length) {
	net_instance * instance;
	instance = (net_instance *)parser->data;
	char * s = (char *)malloc(length + 1);
	strncpy(s, at, length);
	s[length]=0;
	snprintf(instance->url, NET_INSTANCE_URL_SIZE, "%s", s);
	free(s);
	return 0;
}

static int http_header_field_callback(http_parser* parser, const char *at, size_t length) {
	net_param * param;
	net_instance * instance;
	instance = (net_instance *)parser->data;
	char * s = (char *)malloc(length + 1);
	strncpy(s, at, length);
	s[length]=0;
	//printf("field : %s\n", s);
	//param = net_param_create(s, "", 512);
	if(instance != NULL) {
		strncpy(instance->current_field, s, OWS_MAX_VARIABLE_NAME);
	}
	free(s);
	return 0;
}

static int http_header_value_callback(http_parser* parser, const char *at, size_t length) {
	net_param * param;
	net_instance * instance;
	instance = (net_instance *)parser->data;
	char * s = (char *)malloc(length + 1);
	strncpy(s, at, length);
	s[length]=0;
	//printf("field : %s\n", s);
	if(instance != NULL) {
		param = net_param_create(instance->current_field, s, 128);
		if(instance->request_headers == NULL) instance->request_headers = param;
		else net_param_add(instance->request_headers, param);
	}
	free(s);
	return 0;
}

static int http_body_callback(http_parser* parser, const char *at, size_t length) {
	char * s;
	net_instance * instance;
	instance = (net_instance *)parser->data;
	if(instance != NULL) {
		s = (char *)malloc(length + 1);
		strncpy(s, at, length);
		s[length]=0;
		instance->request_size = length;
		instance->request_payload = s;
	}
	return 0;
}

static int http_complete_callback(http_parser* parser) {
	return 0;
}

uint32 http_parse(net_instance * instance, char * request, uint32 reqlen) {
	uint32 len = 0;
	uint8 state = 0;
	int nparsed;
	http_parser wparser;
	char key[OWS_MAX_VARIABLE_NAME];
	uint32 i = 0,j = 0,k =0;
	http_parser_settings settings;
	memset(&settings, 0, sizeof(http_parser_settings));
	settings.on_url = http_url_callback;
	settings.on_status = http_header_field_callback;
	settings.on_header_field = http_header_field_callback;
	settings.on_header_value = http_header_value_callback;
	settings.on_body = http_body_callback;
	settings.on_message_complete = http_complete_callback;
	/* ... */

	http_parser *parser = (http_parser *)malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_REQUEST);
	parser->data = instance;
	instance->parser = parser;
	nparsed = http_parser_execute(parser, &settings, request, reqlen);
	if (parser->upgrade) {
		/* handle new protocol */
	} else if (nparsed != reqlen) {
		/* Handle error. Usually just close the connection. */
	}
	return 0;
}

/* aaaack but it's fast and const should make it shared text page. */
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

int16 http_url_get_param(char * url, uint16 index, char * param) {
	int start = 0;
	int i = start;
	int16 len = -1;
	int cur_index = -1;
	while(url[i] != 0) {
		if(url[i] == '/') {
			if(cur_index == index) break;
			cur_index++;
			start = i+1;
		}
		i++;
	}
	if(cur_index == index) {
		len = i - start;
		//printf("len : %d\n", len);
		if(param != NULL) memcpy(param, url+start, len);
	}
	return len;
}

uint16 http_base64_length(uint8 * bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);

    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

uint32 http_base64_encode(uint8 * bytes_to_encode, uint32 in_len, uint8 * outbuf) {
  uint32 out_len = 0;
  uint32 i = 0;
  uint32 j = 0;
  uint8 char_array_3[3];
  uint8 char_array_4[4];
	const uint8 base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        outbuf[out_len++] = base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';
	  
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      outbuf[out_len++] = base64_chars[char_array_4[j]];

    while((i++ < 3))
      outbuf[out_len++] = '=';
  }
  outbuf[out_len] = 0;		//EOS
  return out_len;
}

uint32 http_base64_decode(uint8 * buffer, uint32 size) {
	uint32 nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    //bufin = (const unsigned char *) buffer;
    //while (pr2six[*(bufin++)] <= 63);
    //nprbytes = (bufin - (const unsigned char *) buffer) - 1;
    nprbytes = size;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) buffer;
    bufin = (const unsigned char *) buffer;

    while (nprbytes > 4) {
		*(bufout++) =
			(unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
		*(bufout++) =
			(unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
		*(bufout++) =
			(unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

static uint8 http_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

uint16 http_hex2bin(uint8 * hexstring, uint8 * bytes) {
	uint16 i = 0;
	uint8 c;
	uint16 len=0;
	while(hexstring[i] != 0) {
		if(i & 0x01) {
			c <<= 4;
			c |= http_hex2byte(hexstring[i]);
			bytes[len] = c;
			len++;
		} else {
			c = http_hex2byte(hexstring[i]);
		}
		i++;
	}
	return len;
}

const char bin2hexchar[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
uint16 http_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring) {
	uint16 j;
	for(j=0;j<len;j++) {
		hexstring[j << 1] = bin2hexchar[bytes[j] >> 4];
		hexstring[(j << 1) + 1] = bin2hexchar[bytes[j] & 0x0F];
	}
	hexstring[len << 1] = 0;
	return len << 1;
}

uint16 http_html_escape(uint8 * src, uint8 * dst) {
	uint16 i = 0, j = 0;
	uint8 c;
	while(src[i] != 0) {
		c = src[i];
		if(c >= 'a' && c <= 'z') dst[j++] = c;
		else if(c >= 'A' && c <= 'Z') dst[j++] = c;
		else if(c >= '0' && c <= '9') dst[j++] = c;
		else {
			dst[j++] = '%';
			dst[j++] = bin2hexchar[c >> 4];
			dst[j++] = bin2hexchar[c & 0x0F];
		}
		i++;
	}
	//null string
	dst[j++] = 0;
	return j;
}

uint16 http_html_unescape(uint8 * src, uint8 * dst) {
	//convert to format specified by rfc3986
	uint16 len = strlen((const char *)src);
	uint16 i,j,k;
	uint8 c;
	for(i=0,j=0;i<len;i++) {
		c = src[i];
		if(c== '%') {
			k = 0;
			c = src[++i];
			if(isdigit(c)) k = http_hex2byte(c);
			c = src[++i];
			if(isdigit(c)) { 
				k <<= 4; 
				k += http_hex2byte(c);
			}
			c = k;
		}
		dst[j++]= c;
	}
	dst[j] = 0;
	return j;
}


http_param* http_param_create(char* key, char* value, int max_size) {
	int len = max_size;
	http_param* param = (http_param*)malloc(sizeof(http_param) +  len + 1);
	param->next = NULL;
	strncpy(param->name, key, OWS_MAX_VARIABLE_NAME);
	if (value != NULL) strncpy(param->value, value, max_size);
	param->value[len] = 0;
	//printf("create new param %s : %s\n", key, param->value);
	//printf("====================================\n");
	return param;
}

http_param* http_param_add(http_param* root, http_param* param) {
	http_param* iterator;
	if (root == NULL) return param;
	iterator = root;
	while (iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = param;
	return root;
}


http_param* http_param_get(http_param* root, char* key) {
	http_param* iterator;
	http_param* candidate;
	if (root == NULL) return NULL;
	iterator = root;
	while (iterator != NULL) {
		if (istrncmp(iterator->name, key, OWS_MAX_VARIABLE_NAME) == 0) {
			return iterator;
		}
		iterator = iterator->next;
	}
	return NULL;
}

http_param* http_param_remove(http_param* root, http_param* param) {
	http_param* iterator;
	http_param* prev_iterator = root;
	if (root == NULL) return NULL;
	if (root == param) return NULL;
	iterator = root;
	while (iterator != NULL) {
		if (strncmp(iterator->name, param->name, OWS_MAX_VARIABLE_NAME) == 0) {
			prev_iterator->next = param->next;
			return param;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
	}
	return NULL;
}

http_param* http_param_update(http_param* root, http_param* old, http_param* param) {
	http_param* iterator;
	http_param* prev_iterator = root;
	if (root == NULL) return NULL;
	if (root == param) return NULL;	//cannot update root
	iterator = root;
	while (iterator != NULL) {
		if (strncmp(iterator->name, old->name, OWS_MAX_VARIABLE_NAME) == 0) {
			prev_iterator->next = param;
			param->next = old->next;
			return old;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
	}
	return NULL;
}

http_param* http_param_dump(http_param* root) {
	http_param* iterator;
	http_param* candidate;
	if (root == NULL) return NULL;
	iterator = root;
	while (iterator != NULL) {
		printf("%s : %s\n", iterator->name, iterator->value);
		iterator = iterator->next;
	}
	return NULL;
}

void http_param_clear(http_param* root) {
	http_param* iterator;
	http_param* candidate;
	if (root == NULL) return;
	iterator = root;
	while (iterator != NULL) {
		candidate = iterator;
		iterator = iterator->next;
		free(candidate);
	}
}

void http_param_release(http_param* param) {
	free(param);
}

net_ext_mime g_mime_list[] = {
	{ NET_MIME_BINARY, ".jpg", "image/jpeg"  },
	{ NET_MIME_BINARY, ".jpeg", "image/jpeg"  },
	{ NET_MIME_BINARY, ".png", "image/png"  },
	{ NET_MIME_BINARY, ".gif", "image/gif"  },
	{ NET_MIME_BINARY, ".tif", "image/tiff"  },
	{ NET_MIME_BINARY, ".tiff", "image/tiff"  },
	{ NET_MIME_TEXT, ".css", "text/css"  },
	{ NET_MIME_TEXT, ".js", "text/js"  },
	{ NET_MIME_TEXT, ".xml", "text/xml"  },	
	{ NET_MIME_BINARY, ".zip", "application/zip"  },
	{ NET_MIME_BINARY, ".tar", "application/x-tar"  },
	{ NET_MIME_TEXT, ".txt", "text/plain"  },
	{ NET_MIME_BINARY, ".svg", "image/svg+xml"  },
	{ NET_MIME_BINARY, ".pdf", "application/pdf"  },
	{ NET_MIME_TEXT, ".json", "application/json"  },
	{ NET_MIME_TEXT, ".csv", "text/csv"  },
	{ NET_MIME_BINARY, ".bin", "application/octet-stream"  },
	{ NET_MIME_BINARY, ".ico", "image/vnd.microsoft.icon"  },
	{ NET_MIME_END, ".stk", "image/vnd.microsoft.icon"  },
	{ NET_MIME_END, "", ""  },
};

int http_execute_script(net_instance * instance, net_entry * entry, char * arguments) {
	int error = 0;
	//executed method depend on which method requested
  switch(instance->parser->method) {
	 case HTTP_GET:
		 instance->bytecodes = entry->bytecodes;
		 stack_exec_bytecodes_json(instance, entry->bytecodes, (char *)entry->name, (char *)"init", arguments);
		break;
	 case HTTP_PUT:
	 case HTTP_PATCH:
	 case HTTP_POST:
		 instance->bytecodes = entry->bytecodes;
		 stack_exec_bytecodes_json(instance, entry->bytecodes, (char *)entry->name, (char *)"update", arguments);
		break;
	 case HTTP_DELETE:
		 instance->bytecodes = entry->bytecodes;
		 stack_exec_bytecodes_json(instance, entry->bytecodes, (char *)entry->name, (char *)"delete", arguments);
	    break;
	 case HTTP_HEAD:
		instance->response_size = 0;
		instance->response_payload = NULL;
		instance->bytecodes = entry->bytecodes;
	    break;
	  default: error = HTTP_STATUS_METHOD_NOT_ALLOWED; break;
  }
  return error;
}

void * http_decode(net_instance * instance, net_entry * root, char * request, int recv_size, int * resp_len) {

	char * pch;
	int error = 0;
    pthread_t thread;
	net_entry * iterator;
	net_param * param = NULL;
	char path[4096];
	char file_ext[10];
	int file_ext_len =0 ;
	int offset_arg = 0;
	iterator = root;	//set current iterator bytecodes
	error = 0;		//clear error
	http_parse(instance, request, recv_size);
	//check for root access
	strncpy(path, instance->url, sizeof(path));
	if(strcmp(path, "/") != 0 && strlen(path) != 0) {
		//enumerate uri until first matching class found
		pch = strtok (path, "/");
		while (pch != NULL && iterator != NULL)
		{
			iterator = net_entry_select(iterator, pch);
			if(iterator != NULL) {
				offset_arg += (strlen(pch) + 1);
				if(iterator->type == NET_ENTRY_TYPE_FILE) break;
			} 
			pch = strtok (NULL, "/");
		}
	}
	strncpy(path, instance->url, sizeof(path));
	//directory or class exist
	if(iterator != NULL) {
		  switch(iterator->type) {
			case NET_ENTRY_TYPE_FILE: 
				//try execute class entry, passing arguments
				error = http_execute_script(instance, iterator, path + offset_arg);
				break;
			case NET_ENTRY_TYPE_NAMESPACE: 
				//try execute default class entry
				  iterator = net_entry_select(iterator, "program");
				  if(iterator != NULL) {
					error = http_execute_script(instance, iterator, NULL);
				  }
				  break;
			default: error = HTTP_STATUS_FORBIDDEN;  break;
		  }
	} else {
		file_ext_len = net_url_get_ext(path, file_ext, sizeof(file_ext));
		if(file_ext_len != 0) {
			net_ext_mime * ext_iterator = (net_ext_mime *)&g_mime_list[0];
			while(ext_iterator->type != NET_MIME_END) {
				if(strcmp(ext_iterator->ext, file_ext) == 0) {
					break;
				}
				ext_iterator++;
			}
			//invalid file extension
			if(ext_iterator->type != NET_MIME_END) {
				snprintf(path, sizeof(path), "./www%s", instance->url); 
				FILE * file = fopen(path, "rb");
				int filesize = 0;
				char * filecontent = NULL;
				if(file != NULL) {
					//update content type field
					param = net_param_get(instance->response_headers, "Content-Type");
					if(param == NULL) {
						param = net_param_create("Content-Type", ext_iterator->mime, 64);
						net_param_add(instance->response_headers, param);
					} else {
						snprintf(param->value, param->length, "%s", ext_iterator->mime);
					}
					fseek(file, 0, SEEK_END);
					filesize = ftell(file);
					printf("file size : %d\n", filesize);
					filecontent = (char *)malloc(filesize);
					int n = 0;
					fseek(file, 0, SEEK_SET);
					for(n=0;n<filesize;n++) filecontent[n] = fgetc(file);

					instance->response_size = filesize;
					instance->response_payload = filecontent;
				} else {
					//file cannot be opened/not found
					error = HTTP_STATUS_NOT_FOUND;
				}
			} else {
				//file type not supported
				error = HTTP_STATUS_FORBIDDEN;
			}
		} else {
			//no file extension
			error = HTTP_STATUS_NOT_FOUND;
		}
		//debugging purpose
		//net_param_dump(instance->request_headers);
	}
	

	//parse here
	//response here
	char * response = NULL;
	char * error_content;
	int content_length = 0;
	char * response_content;
	int response_length = 0;
	if(error == 0) {
		content_length = instance->response_size;
		response_content = (char *)instance->response_payload;
	} else {
		error_content = (char * )malloc(512);
		snprintf(error_content, 512, "Error %d", error);
		content_length = strlen(error_content);
		response_content = error_content;
	}
	//update content length field
	param = net_param_get(instance->response_headers, "Content-Length");
	if(param == NULL) {
		param = net_param_create("Content-Length", "0", 64);
		net_param_add(instance->response_headers, param);
	}
	snprintf(param->value, param->length, "%d", content_length);
	  //allocate new response buffer
	response = (char *)malloc(4096 + content_length);
	memset(response, 0, 4096 + content_length);
	snprintf(response, 4096, "HTTP/1.1 200 OK\r\n");
	response_length = strlen(response);
	param = instance->response_headers;
	while(param != NULL) {
		response_length += net_param_print(response + response_length, param);
		param = param->next;
	}
	strcat(response + response_length, "\r\n");	//end of header fields
	response_length += 2;
	memcpy(response + response_length, response_content, content_length);
	response_length += content_length;
	printf(response);
	resp_len[0] = response_length;
	free(response_content);
	return response;
}


int http_read_request(void * conn, char ** buffer) {
	char request[32768];
	char * header;
	char * temp;
	char * value;
	char * buf = NULL;
	unsigned content_length;
	size_t len;
	int consume_len = 0;
	int needed_len = 0;
	//int recv_size = SSL_read((SSL *)ssl, request, sizeof(request));
	int recv_size = net_recv(conn, request, sizeof(request), 0);
	if(recv_size < sizeof(request)) {
		//buf = malloc(recv_size + 1);
		//memcpy(buf, request, recv_size);
	//} else {
		//look ahead operation
		request[recv_size] = 0;
		header = istrstr(request, "Content-Length");		//need to case-insensitive without changing request buffer
		if(header== NULL) {
			//printf("no content-length\n");
			buf = malloc(recv_size + 1);
			memcpy(buf, request, recv_size);
			buf[recv_size] = 0;
			buffer[0] = buf;
			return recv_size;		//unable to read content-length, might be invalid http payload
		}
		//printf("looking for linebreak\n");
		len = http_mp_next_linebreak(header);
		temp = malloc(len + 1);
		if(temp != NULL) {
			memcpy(temp, header, len);
			temp[len] = 0;
			//printf("http_mp_str_next_token\n");
			value = http_mp_str_next_token(temp, ':');
			if(value != NULL) {
				value += 1;		//skip delimiter
				//printf("content length : %s\n", value);
				content_length = atoi(value);
				if(content_length < recv_size) {
					printf("singlepart request %d\n", recv_size);
					buf = malloc(recv_size + 1);
					memcpy(buf, request, recv_size);
					buf[recv_size] = 0;
				} else {
					printf("multipart request %d\n", recv_size);
					needed_len = recv_size + content_length;
					buf = malloc(needed_len + 1);
					memcpy(buf, request, recv_size);
					//if(content_length > recv_size) {
					while(recv_size < needed_len) {
						//recv_size += SSL_read(ssl, buf + recv_size, content_length);
						recv_size += net_recv(conn, buf + recv_size, content_length, 0);
						printf("recv_size : %d\n", recv_size);
						buf[recv_size] = 0;
						if(recv_size >= needed_len) break;
					} 
				}
				//printf("total received : %d\n", recv_size);
			}
			free(temp);
		}
	}
	buffer[0] = buf;
	return recv_size;
}

void * http_thread(void * arg)
{
	net_instance * instance;
	net_entry * root;
	//char request[32768];
	char *request;
	int recv_size;
	char * arguments = NULL;
	char * response;
	int response_length;
	net_exec_arg * thread_arg = (net_exec_arg *)arg;
	int ConnectFD = thread_arg->ConnectFD;
	root = thread_arg->root;
	instance = net_instance_create(ConnectFD, NET_INSTANCE_TYPE_HOST);
	instance->root = thread_arg->root;
	
	instance->client_iptype = thread_arg->client.sa_family;
	switch(thread_arg->client.sa_family) {
		case AF_INET:		//IPv4
			memcpy(instance->client_ip4, thread_arg->client.sa_data + 2, 4);
			instance->port = ((unsigned)thread_arg->client.sa_data[0] * 256) + (unsigned)thread_arg->client.sa_data[1];
			break;
		case AF_INET6:		//IPv6
			memcpy(instance->client_ip6, thread_arg->client.sa_data + 2, 12);
			instance->port = ((unsigned)thread_arg->client.sa_data[0] * 256) + (unsigned)thread_arg->client.sa_data[1];
			break;
		default:
			break;
	}
    recv_size = net_recv(ConnectFD, request, sizeof(request), 0);
	recv_size = http_read_request(ConnectFD, &request);
	if(recv_size > 0 && request != NULL) {
		request[recv_size] = 0;
		//sprintf(response, "HTTP/1.1 200 OK\r\nDate: Thu, 31 Jan 2019 18:36:32 GMT\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %d\r\n\r\n", instance->response_size);
		//if(instance->response_payload) memcpy(response + strlen(response), (const char *)instance->response_payload, instance->response_size);
		response = (char *)http_decode(instance, root, (char *)request, recv_size, &response_length);
		if(response != NULL) {
			net_send(ConnectFD, response, response_length, 0);
			free(response);
		}
		free(request);
	}
	//cleanup
	net_instance_release(instance);
  
    if (net_shutdown(ConnectFD, SHUT_ALL) == -1) {
        perror("shutdown failed");
    }
    net_close(ConnectFD);
	free(arg);
	return response;
}

void * http_task(void * arg) {
    struct sockaddr_in sa;
    pthread_t thread;
	net_exec_arg * thread_arg;
	net_instance * instance;
    struct sockaddr client;
    int     client_len = sizeof(client);
	http_init_arg * init = (http_init_arg *)arg;
    int client_port;
    unsigned char * client_ip;
    int SocketFD = net_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
      perror("http cannot create socket");
      exit(EXIT_FAILURE);
    }
  
    memset(&sa, 0, sizeof sa);
  
    sa.sin_family = AF_INET;
    sa.sin_port = htons(init->port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
    if (net_bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("http bind failed");
      net_close(SocketFD);
      exit(EXIT_FAILURE);
    }
  
    if (net_listen(SocketFD, 10) == -1) {
      perror("http listen failed");
      net_close(SocketFD);
      exit(EXIT_FAILURE);
    }
  
    for (;;) {
      int ConnectFD = net_accept(SocketFD, &client, &client_len);
  
      if (0 > ConnectFD) {
        perror("accept failed");
        net_close(SocketFD);
        exit(EXIT_FAILURE);
      }

	  client_port = ((unsigned char)client.sa_data[0]*256)+(unsigned char)client.sa_data[1];
	  client_ip = client.sa_data + 2;
	  //printf("client(%d) : %ld.%ld.%ld\n", client.sa_family, ((unsigned char)client.sa_data[0]*256)+(unsigned char)client.sa_data[1],(unsigned char)client.sa_data[2],(unsigned char)client.sa_data[3]);
	  //printf("client(%d) : %ld.%ld.%ld.%ld\n", client.sa_family, (unsigned char)client.sa_data[4],(unsigned char)client.sa_data[5],(unsigned char)client.sa_data[6],(unsigned char)client.sa_data[7]);
	  //printf("client(%d) : %ld.%ld.%ld.%ld\n", client.sa_family, (unsigned char)client.sa_data[8],(unsigned char)client.sa_data[9],(unsigned char)client.sa_data[10],(unsigned char)client.sa_data[11]);
	  //printf("client(%d) : %ld.%ld.%ld.%ld\n", client.sa_family, (unsigned char)client.sa_data[12],(unsigned char)client.sa_data[13],(unsigned char)client.sa_data[14],(unsigned char)client.sa_data[15]);
  
      //create a new thread here
	  thread_arg = (net_exec_arg *)malloc(sizeof(net_exec_arg));
	  thread_arg->ConnectFD = ConnectFD;
	  thread_arg->root = (net_entry *)init->root;
	  thread_arg->instance = instance;
	  memcpy(&thread_arg->client, &client, sizeof(struct sockaddr));
	  if(pthread_create(&thread, NULL, http_thread, thread_arg) == 0) {
		//thread executed
	  }
#if defined(WIN32) || defined(WIN64)
	Sleep(200);
#endif
#if defined(__GNUC__)
	usleep(200 * 1000);
#endif
    }

    net_close(SocketFD);
}

int http_init(http_init_arg * arg) {
	int ret = -1;
    pthread_t thread;
	if(pthread_create(&thread, NULL, (void * (*)(void *))http_task, arg) == 0) {
		//thread executed
		  ret = 0;
	}
	return ret;
}
