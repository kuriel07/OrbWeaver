#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "defs.h"
#include "config.h"
#include "StackHost.h"
#include "core/port.h"
#include "core/global.h"
#include "Stack/pkg_encoder.h"
#ifdef WIN32
#include "util/dirent.h"
#else
#include "dirent.h"
#endif
#include "StackVM/midgard.h"
#include <pthread.h>
#include <ctype.h>

int istrncmp(char const *a, char const *b, int max_len)
{
	if(strlen(a) != strlen(b)) return -1;
    for (;max_len>0 && *a !=0; a++, b++, max_len--) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0)
            return d;
    }
	return 0;
}

char* istrstr(char* text, char* pattern) {
	//naive algorithm
	char* index = NULL;
	int s = strlen(text);
	int r = strlen(pattern);
	//1. SET K = 1 and MAX = S - R + 1.
	int k = 0;
	int max = s - r ;
	//2. Repeat Step 3 to 5 while K <= MAX:
	for (; k <= max;) {
		//3.   Repeat for L = 1 to R :
		for (int l = 0; l < r; l++) {
			//If TEXT[K + L - 1] ≠ PAT[L], then : Go to Step 5.
			if ((pattern[l] >= 'a' && pattern[l] <= 'z') || (pattern[l] >= 'A' && pattern[l] <= 'Z')) {
				if (tolower(text[k + l]) != tolower(pattern[l])) goto skip_next;
			} else {
				if (text[k + l] != pattern[l]) goto skip_next;
			}
		}
		//SET INDEX = K, and EXIT.
		index = &text[k]; break;
		skip_next:
		//5.   K = K + 1.
		k += 1;
	}
	//	6. SET INDEX = 0.
	//index = NULL;
	//7. Exit.
	return index;
}

net_param * net_param_create(char * key, char * value, int max_size) {
	int len = ((strlen(value) > max_size)?strlen(value):max_size);
	net_param * param = (net_param *)malloc(sizeof(net_param) + len);
	param->length = len;
	param->next = NULL;
	strncpy(param->key, key, OWS_MAX_VARIABLE_NAME);
	if(value != NULL) {
		strncpy(param->value, value, len);
		param->value[len] = 0;		//null terminated string
	}
	return param;
}

net_param * net_param_add(net_param * root, net_param * param) {
	net_param * iterator;
	if(root == NULL) return param;
	iterator = root;
	while(iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = param;
}

net_param * net_request_header_add(net_instance *instance, net_param * param) {
	if(instance->request_headers == NULL)instance->request_headers = param;
	else net_param_add(instance->request_headers, param);
	return param;
}

net_param * net_response_header_add(net_instance * instance, net_param * param) {
	if(instance->response_headers == NULL)instance->response_headers = param;
	else net_param_add(instance->response_headers, param);
	return param;
}

net_param * net_request_header_remove(net_instance * instance, net_param * param) {
	if(instance->response_headers == param) instance->request_headers = param->next;
	else return net_param_remove(instance->request_headers, param);
	return param;
}

net_param * net_response_header_remove(net_instance * instance, net_param * param) {
	if(instance->response_headers == param) instance->response_headers = param->next;
	else return net_param_remove(instance->response_headers, param);
	return param;
}

net_param * net_request_header_set(net_instance * instance, net_param * old, net_param * param) {
	if(instance->response_headers == old) {
		instance->request_headers = param;
		param->next = old->next;
	}
	else return net_param_update(instance->request_headers, old, param);
	return old;
}

net_param * net_response_header_set(net_instance * instance, net_param * old, net_param * param) {
	if(instance->response_headers == old) {
		instance->response_headers = param;
		param->next = old->next;
	}
	else return net_param_update(instance->response_headers, old, param);
	return old;
}

net_param * net_param_get(net_param * root, char * key) {
	net_param * iterator;
	net_param * candidate;
	if(root == NULL) return NULL;
	iterator = root;
	while(iterator != NULL) {
		if(strncmp(iterator->key, key, OWS_MAX_VARIABLE_NAME) == 0) {
			return iterator;
		}
		iterator = iterator->next;
	}
	return NULL;
}

net_param * net_param_remove(net_param * root, net_param * param) {
	net_param * iterator;
	net_param * prev_iterator = root;
	if(root == NULL) return NULL;
	if(root == param) return NULL;
	iterator = root;
	while(iterator != NULL) {
		if(strncmp(iterator->key, param->key, OWS_MAX_VARIABLE_NAME) == 0) {
			prev_iterator->next = param->next;
			return param;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
	}
	return NULL;
}

net_param * net_param_update(net_param * root, net_param * old, net_param * param) {
	net_param * iterator;
	net_param * prev_iterator = root;
	if(root == NULL) return NULL;
	if(root == param) return NULL;	//cannot update root
	iterator = root;
	while(iterator != NULL) {
		if(strncmp(iterator->key, old->key, OWS_MAX_VARIABLE_NAME) == 0) {
			prev_iterator->next = param;
			param->next = old->next;
			return old;
		}
		prev_iterator = iterator;
		iterator = iterator->next;
	}
	return NULL;
}

net_param * net_param_dump(net_param * root) {
	net_param * iterator;
	net_param * candidate;
	if(root == NULL) return NULL;
	iterator = root;
	while(iterator != NULL) {
		printf("%s : %s\n", iterator->key, iterator->value);
		iterator = iterator->next;
	}
	return NULL;
}

int net_param_print(char * buffer, net_param * param) {
	if(param == NULL) return 0;
	sprintf(buffer, "%s: %s\r\n", param->key, param->value);
	printf(buffer);
	return strlen(buffer);
}

void net_param_clear(net_param * root) {
	net_param * iterator;
	net_param * candidate;
	if(root == NULL) return;
	iterator = root;
	while(iterator != NULL) {
		candidate = iterator;
		iterator = iterator->next;
		free(candidate);
	}
}

void net_param_release(net_param * param) {
	free(param);
}

const char * g_str_days[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

const char * g_str_months[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

net_instance * net_instance_create(int conn, int type) {
	net_param * param;
	tm *ltm;
	char datetime_buffer[64];
	net_instance * instance = (net_instance *)malloc(sizeof(net_instance));
	time_t now = time(0);
	// convert now to string form
	//char* dt = ctime(&now);
	memset(instance, 0, sizeof(net_instance));
	instance->type = type;
	instance->conn = conn;
	instance->status = 200;
	instance->message = "OK";
	param = net_param_create("Date", "", 64);
	ltm = localtime(&now);
	sprintf(datetime_buffer, "%s, %d %s %04d %02d:%02d:%02d GMT", g_str_days[ltm->tm_wday], ltm->tm_mday, g_str_months[ltm->tm_mon],
		1970 + ltm->tm_year, ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
	sprintf(param->value, "%s", datetime_buffer);
	net_response_header_add(instance, param);
	net_response_header_add(instance, net_param_create("Server", hostname, 64));
	net_response_header_add(instance, net_param_create("Content-Type", "text/html; charset=utf-8", 64));
	net_response_header_add(instance, net_param_create("Content-Length", "0", 64));
	return instance;
}

void net_instance_release(net_instance * instance) {
	if(instance->request_headers != NULL) net_param_clear(instance->request_headers);
	if(instance->response_headers != NULL) net_param_clear(instance->response_headers);
	if(instance->parser != NULL) free(instance->parser);
	if(instance->request_payload != NULL) free(instance->request_payload);
	free(instance);
}

net_entry * net_entry_create (uint16 type, char * name) {
	net_entry * entry = (net_entry *)malloc(sizeof(net_entry));
	memset(entry, 0, sizeof(net_entry));
	entry->type= type;
	strncpy((char *)entry->name, name, 256);
	return entry;
}

net_entry * net_entry_select(net_entry * parent, char * name) {
	net_entry * iterator;
	if(parent == NULL) return NULL;
	iterator = parent->child;
	while(iterator != NULL) {
		if(strncmp(iterator->name, name, 255) == 0) {
			return iterator;
		}
		iterator = iterator->sibling;
	}
	return NULL;
}

net_entry * net_entry_add(net_entry * parent, net_entry * entry) {
	net_entry * iterator;
	if(parent == NULL) return entry;
	iterator = parent->child;
	if(iterator == NULL) {
		parent->child = entry;
	} else {
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = entry;
	}
	entry->parent = parent;
	return entry;
}

net_entry * net_entry_remove(net_entry * entry) {
	net_entry * parent;
	net_entry * iterator;
	net_entry * prev = NULL;
	if(entry == NULL) return entry;
	if(entry->parent == NULL) return entry;
	parent = entry->parent;
	iterator = parent->child;
	while(iterator != entry && iterator != NULL) {
		prev = iterator;
		iterator = iterator->sibling;
	}
	if(iterator == entry) {
		if(prev != NULL) {
			prev->sibling = entry->sibling;
		} else {
			parent->child = entry->sibling;
		}
	}
	return entry;
}

net_callback * net_callback_create(char * name, int16 arg_count, uint32 offset) {
	net_callback * callback = (net_callback *)malloc(sizeof(net_callback));
	memset(callback, 0, sizeof(net_callback));
	strncpy(callback->name, name,  256);
	((vm_function *)callback)->arg_count = arg_count;
	((vm_function *)callback)->offset = offset;
	return callback;
}

net_callback * net_bytecodes_add_callback(net_bytecodes * bytecodes, net_callback * callback) {
	net_callback * iterator = NULL;
	if(bytecodes == NULL) return NULL;
	iterator = (net_callback *)bytecodes->callback;
	if(iterator == NULL) bytecodes->callback = callback;
	else {
		while(iterator->next != NULL) {
			iterator= iterator->next;
		}
		iterator->next = callback;
	}
	return callback;
}

net_callback * net_bytecodes_find_callback(net_bytecodes * bytecodes, char * name) {
	net_callback * iterator = NULL;
	if(bytecodes == NULL) return NULL;
	iterator = (net_callback *)bytecodes->callback;
	if(iterator == NULL) return NULL;
	while(iterator != NULL) {
		if(strncmp(iterator->name, name, 256) == 0) {
			return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

void net_bytecodes_remove_callback(net_bytecodes * bytecodes, net_callback * callback) {
	net_callback * iterator = NULL;
	net_callback * prev = NULL;
	if(bytecodes == NULL) return;
	iterator = (net_callback *)bytecodes->callback;
	if(iterator == NULL) return;
	while(iterator != NULL) {
		if(iterator == callback) {
			if(prev == NULL) bytecodes->callback = iterator->next;
			else {
				prev->next = iterator->next;
			}
			free(iterator);
			return;
		}
		iterator = iterator->next;
	}
}

int net_url_get_ext(char * url, char * ext, int maxlen) {
	char c;
	int state = 0;
	int len = 0;
	while((c = *url++) != 0) {
		if(c == '.') {				//find first .
			state = 1;
		}
		if(state == 1) {
			if(c == '/' || c == '?') break;
			ext[len++] = c;
		}
		if(len == (maxlen - 1)) break;
	}
	ext[len] = 0;
	return len;
}

typedef struct {
	net_instance * instance;
	net_entry * entry;
}net_init_arg ;

void * net_init_task(void * arg) {
	char * response_content;
	net_instance * instance = ((net_init_arg *)arg)->instance;
	net_entry * entry = ((net_init_arg *)arg)->entry;
	stack_exec_bytecodes_json(instance, entry->bytecodes, (char *)entry->name, (char *)"setup", NULL);
	if((char *)instance->response_payload != NULL) {
		response_content = (char *)malloc(instance->response_size + 1);
		memcpy(response_content, (char *)instance->response_payload, instance->response_size);
		response_content[instance->response_size] = 0;
		free((char *)instance->response_payload);
		printf(response_content);
		free(response_content);
	}
	//cleanup
	net_instance_release(instance);
	//end setup execution
	free(arg);
	return 0;
}

net_entry * net_enumerate_directory(
    char *dirname, int is_root, net_entry * parent)
{
    DIR *dir;
	net_bytecodes * bytecodes;
	vm_instance vcit;
	char current_path[512];
	char reconstructed_method[512];
	char path[512];
	char full_path[512];
	char * response_content;
    struct dirent *ent;
	pk_object * iterator;
	net_entry * entry;
	char file_ext[32];
	int file_ext_len;
	net_init_arg * init_arg ;
	pthread_t ptid;
	net_instance * instance;
	current_path[0] = 0;
	strcpy(current_path, dirname);
    /* Open directory stream */
    dir = opendir (dirname);
    if (dir != NULL) {

        //iterate subdirectory first
        while ((ent = readdir (dir)) != NULL) {
            switch (ent->d_type) {
				case DT_DIR:
					//printf ("%s/\n", ent->d_name);
					if(ent->d_name[0] != '.') {
						sprintf(path, "%s/%s", current_path, ent->d_name);
						//printf("%s\n", path);
						net_entry_add(parent, 
							net_enumerate_directory(path, 0, net_entry_create(NET_ENTRY_TYPE_NAMESPACE, ent->d_name)
							));
					}
					break;

				case DT_LNK:
					//printf ("%s@\n", ent->d_name);
					break;

				case DT_REG:
				default:
					//printf ("%s*\n", ent->d_name);
					break;
            }
        }

		dir = opendir (dirname);
		//load all files in this directory
        while ((ent = readdir (dir)) != NULL) {
            switch (ent->d_type) {
				case DT_REG:
					sprintf(path, "%s/%s", current_path, ent->d_name);
					sprintf(full_path, "%s/%s", dirname, ent->d_name);	//actual path
					//printf("%s\n", ent->d_name);
					file_ext_len = net_url_get_ext(ent->d_name, file_ext, sizeof(file_ext));
					if(file_ext_len != 0 && strcmp(file_ext, ".stk") == 0) {
						pk_object * last_root = pk_get_last_root(pk_get_root());
						bytecodes = stack_load_file(full_path, (strcmp(file_ext, ".stk") == 0));
						if(bytecodes != NULL) {
							strncpy(bytecodes->path, path, 512);
							pk_object * iterator;
							if(last_root != NULL) iterator = (pk_object *)last_root->next;	//(pk_object *)bytecodes->root;
							else iterator = (pk_object *)bytecodes->root;
							if(iterator == NULL) break;	//not valid package
							while(iterator != NULL) {
								entry = net_entry_create(NET_ENTRY_TYPE_FILE, _RECAST(char *, ((pk_class *)iterator)->name));
								if(entry != NULL) {
									net_entry_add(parent, entry);
									entry->bytecodes = bytecodes;
									//run setup
									instance = net_instance_create(0, NET_INSTANCE_TYPE_APP);
									instance->root = parent;
									instance->bytecodes = entry->bytecodes;
#if 0
									init_arg = (net_init_arg *)malloc(sizeof(net_init_arg));
									init_arg->instance = instance;
									init_arg->entry = entry;
									pthread_create(&ptid, NULL, net_init_task, init_arg);
#else
									stack_exec_bytecodes_json(instance, entry->bytecodes, (char *)entry->name, (char *)"setup", NULL);
									if((char *)instance->response_payload != NULL) {
										response_content = (char *)malloc(instance->response_size + 1);
										memcpy(response_content, (char *)instance->response_payload, instance->response_size);
										response_content[instance->response_size] = 0;
										free((char *)instance->response_payload);
										printf(response_content);
										free(response_content);
									}
									//cleanup
									net_instance_release(instance);
									//end setup execution
#endif
								}
								iterator = (pk_object *)iterator->next;
							}
						}
					}
					break;

				case DT_DIR: break;

				case DT_LNK: break;

				default: break;
            }
        }
        closedir (dir);
    }
	return parent;
}



