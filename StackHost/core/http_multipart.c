
#include "http.h"

mp_context* http_mp_new_context(char * boundary) {
	mp_context * ctx = malloc(sizeof(mp_context));
	memset(ctx, 0, sizeof(mp_context));
	strncpy(ctx->boundary, boundary, 96);
	return ctx;
}

void http_mp_cleanup(mp_context * ctx) {
    free(ctx);
}

size_t http_mp_next_linebreak(char* content) {
	size_t offset = 0;
	while (content[offset] != 0) {
		if (content[offset] == '\n') {
			if (offset == 0) return 1;
		}
		offset++;
		if (content[offset] == '\n') {
			//if (offset == 0) return 1;
			break;
		}
	}
	return offset;
}

static int mp_is_boundary(mp_context* ctx, char* line) {
	if (strstr(line, ctx->boundary) != NULL) return 1;
	return 0;
}

char* http_mp_str_next_token(char* content, char delim) {
	size_t offset = 0;
	//printf("content[0] : %d", content[offset]);
	while (content[offset] != 0) {
		offset++;
		if (content[offset] == delim) {
			//if (offset == 0) return 1;
			break;
		}
	}
	if (content[offset] == 0 && offset ==0 ) return NULL;		//delim not found
	return content + offset;
}

static char * to_lowercase (char * buf) {
	int len = 0;
	while(buf[len] != 0) {
		buf[len] = tolower(buf[len]);
		len++;
	}
	return buf;
}

//process content-disposition parameters
void mp_process_disparam(http_param * param, char* str) {
	char name[65];
	char value[256];
	int name_index = 0;
	int value_index = 0;
	int state = 0;
	int len = strlen(str);
	for (int i = 0; i < len; i++) {
		switch (state) {
			case 0:
				if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= '0' && str[i] <= '9') || str[i] == '_') {
					name[name_index++] = str[i];
				}
				else if (str[i] == '=') {
					name[name_index] = 0;
					state = 1;
				}
				break;
			case 1:
				if (str[i] == ' ') break;
				if (str[i] == '\"') {
					state = 2;		//start data state
					break;
				}
				value[value_index++] = str[i];
				break;
			case 2:
				if (str[i] == '\"') {
					state = 3;		//start data state
					break;
				}
				value[value_index++] = str[i];
				break;
			case 3: break;
		}
	}
	value[value_index] = 0;
	//printf("param : %s", name);
	if (strstr(name, "filename")) {
		strncpy(param->filename, value, 256);
	} else if (strstr(name, "name")) {
		strncpy(param->name, value, 64);
	} else if (strstr(name, "boundary")) {
		strncpy(param->boundary, value, 96);
	}
	
}

int http_mp_process_disposition(http_param* param, char* content) {
	char* strbuf;
	char* value;
	char* disposition_type;
	char* disposition_param;
	int ret = 0;
	disposition_type = content;
	//printf("distype : %s\n", disposition_type);

	value = http_mp_str_next_token(to_lowercase(disposition_type), ';');
	if (value != NULL) {
		strbuf = malloc((value - disposition_type) + 1);
		memcpy(strbuf, disposition_type, (value - disposition_type));
		strbuf[(value - disposition_type)] = 0;
		disposition_type = strbuf;
		printf("distype : %s\n", disposition_type);
		if (strstr(disposition_type, "form-data") == NULL) {
			ret = -1;
		}
		//value += 1;
		//parse other params
		while (value != NULL) {
			disposition_param = value + 1;
			value = http_mp_str_next_token(value, ';');
			if (value != NULL) {
				strbuf = malloc((value - disposition_param) + 1);
				memcpy(strbuf, disposition_param, (value - disposition_param));
				strbuf[(value - disposition_param)] = 0;
				disposition_param = strbuf;
				printf("disparam : %s\n", disposition_param);
				mp_process_disparam((http_param*)param, disposition_param);
				//value += 1;
				if (disposition_param != NULL) free(disposition_param);
			}
		}
		if (disposition_type != NULL) free(disposition_type);
	}
	return ret;
}

void http_mp_decode(mp_context* ctx, http_param** root_param, char* content) {
	size_t next_line;
	size_t len;
	char* buf = NULL;
	char* header;
	char* value;
	char* disposition_type;
	char* disposition_param;
	int content_length = 0;
	char* content_data = NULL;
	char* strbuf;
	http_param hparam;
	char* start_data = NULL;
	char* end_data = NULL;
	memset(&hparam, 0, sizeof(hparam));		//
	//http_param* root_param = NULL;
	http_param* new_param = NULL;
	//printf("%s\n", sample_text);
next_readline:
	while ((next_line = http_mp_next_linebreak(content + ctx->offset)) != 0) {
		len = next_line;
		buf = malloc(len + 1);
		memcpy(buf, content + ctx->offset, len);
		if(buf[len-1] == '\r') buf[len - 1] = 0;	//skip return carriage
		buf[len] = 0;

		//printf("current state : %d\n", ctx->state);
		//printf("%s (%d) - %d\n", buf, len, ctx->state);
		switch (ctx->state) {
		case MP_STATE_BOUNDARY:		//check for boundary start
			if (mp_is_boundary(ctx, buf)) {
				ctx->state = MP_STATE_HEADER;
			}
			ctx->offset += len +1;
			break;
		case MP_STATE_HEADER:
			//getchar();
			if ((header = strstr(to_lowercase(buf), "content-disposition")) != NULL) {
				//printf("content-dispos\n"); getchar();
				if (http_mp_process_disposition((http_param*)&hparam, header) != 0) {
					ctx->state = MP_STATE_END;		//not form-data, skip operation
				}

			}
			else if ((header = strstr(to_lowercase(buf), "content-length")) != NULL) {
				value = http_mp_str_next_token(header, ':');
				if (value != NULL) {
					value += 1;	//skip delimiter
					content_length = atoi(value);		//
					hparam.size = content_length;
					//printf("content-length: %ld\n", content_length);

				}
			}
			else if ((header = strstr(to_lowercase(buf), "content-type")) != NULL) {
				value = http_mp_str_next_token(header, ':');
				if (value != NULL) {
					value += 1;	//skip delimiter
					strncpy(hparam.type, value, 64);
				}
			}
			else {
				if (strlen(buf) == 0) {
					ctx->state = MP_STATE_DATA_CONTENT;
					if (buf != NULL) free(buf);
					ctx->offset += len;
					start_data = content + ctx->offset+1;
					end_data = content + ctx->offset+1;
					goto next_readline;
				}
			}
			ctx->offset += len +1;
			break;
		case MP_STATE_DATA_CONTENT:
			//if (content_data != NULL) {
			//printf("data : %s\n", buf);
			if (mp_is_boundary(ctx, buf)) {
				ctx->state = MP_STATE_HEADER;
				len = end_data - start_data;
				printf("len : %d\n", len);
				new_param = http_param_create(hparam.name, start_data, len);
				strncpy(new_param->filename, hparam.filename, HTTP_PARAM_FILENAME_SIZE);
				strncpy(new_param->type, hparam.type, HTTP_PARAM_TYPE_SIZE);
				new_param->size = hparam.size;

				//printf("content-data (%d): %s\n", len, new_param->value);
				if (root_param[0] == NULL) root_param[0] = new_param;
				else http_param_add(root_param[0], new_param);


				memset(&hparam, 0, sizeof(hparam));		//
				//ctx->offset += len;
				//break;
				//return;
			}
			end_data += len-1;
			ctx->offset += len;
			break;

		}

		if (buf != NULL) free(buf);
	}
	//last parameter
	if (start_data != NULL) {
		len = end_data - start_data;
		new_param = http_param_create(hparam.name, start_data, len);
		strncpy(new_param->filename, hparam.filename, HTTP_PARAM_FILENAME_SIZE);
		strncpy(new_param->type, hparam.type, HTTP_PARAM_TYPE_SIZE);
		new_param->size = hparam.size;
		if (root_param[0] == NULL) root_param[0] = new_param;
		else http_param_add(root_param[0], new_param);
	}
}