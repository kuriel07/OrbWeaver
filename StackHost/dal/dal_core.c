#include "dal.h"
#include "StackVM/midgard.h"
#ifdef HAVE_LIBPQ
#include "libpq-fe.h"
extern dal_interface pg_sql_interface;
#endif

dal_database g_supported_databases[] =  {
#if defined(HAVE_LIBPQ)
	{ "pqsql", &pg_sql_interface },
	{ "pgsql", &pg_sql_interface },
#endif
	{ NULL, NULL },
};


//create database connection
LIB_API dal_conn * dal_create_conn(char * conn_str) {
	char * dbt;
	char prefix_buffer[20];
	int len = 0;
	dal_database * iterator = &g_supported_databases[0];
	dal_conn * conn = NULL;
	dbt = strstr(conn_str, ":");
	memset(prefix_buffer, 0, sizeof(prefix_buffer));
	if(dbt != NULL) {
		len = (dbt - conn_str);
		if(len < 20 && len > 0) {
			memcpy(prefix_buffer, conn_str, len);
			prefix_buffer[len] = 0;
		}
		dbt++;
		while(iterator->prefix != NULL) {
			if(strcmp(iterator->prefix, prefix_buffer) == 0) {
				printf("conn string : %s\n", dbt);
				conn = (dal_conn *)malloc(sizeof(dal_conn));
				memset(conn, 0, sizeof(dal_conn));
				strncpy(conn->connect_str, dbt, 512);
				conn->heap = NULL;
				conn->type = DAL_CONN_PGSQL;
				conn->backend = iterator->backend;
				conn->conn = NULL;
				return conn;
			}
			iterator++;
		}
	}
	return conn;
}

//open connection
LIB_API uint8 dal_open(dal_conn * conn) {
	return conn->backend->open(conn);
}
//close connection
LIB_API void dal_close(dal_conn * conn) {
	dal_conn_clear_statements(conn);		//clear all statements
	dal_conn_clear_results(conn);		//clear all results
	conn->backend->close(conn);
	memset(conn, 0, sizeof(dal_conn));
	free(conn);
}
//query statement
LIB_API dal_result * dal_query(dal_conn * conn, char * statement) {
	return conn->backend->query(conn, statement);
}
//prepare statement
LIB_API dal_prepared_statement * dal_prepare(dal_conn * conn, char * name, char * statement) {
	return conn->backend->prepare(conn, name, statement);
}
//execute statement
LIB_API dal_result * dal_execute(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]) {
	return conn->backend->execute(conn, statement, count, args);
}
//fetch all result
LIB_API dal_array * dal_fetch_all(dal_result * result) {
	return ((dal_node *)result)->conn->backend->fetch(result);
}
//get result count
LIB_API int dal_num_rows(dal_result * result ) {
	return ((dal_node *)result)->conn->backend->count(result);
}
//get first result
LIB_API dal_array * dal_fetch_first(dal_result * result) {
	return ((dal_node *)result)->conn->backend->first(result);
}

//get next result
LIB_API dal_array * dal_fetch_next(dal_result * result) {
	return ((dal_node *)result)->conn->backend->next(result);
}

//get result at/key
LIB_API dal_data * dal_row_at(dal_array * arr, int index) {
	return NULL;
}

LIB_API dal_data * dal_row_key(dal_array * arr, char * name) {
	return NULL;
}

//internal APIs
LIB_API dal_node * dal_node_add(dal_node * root, dal_node * node) {
	dal_node * iterator = root;
	if(iterator == NULL) return NULL;
	while(iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = node;
	return node;
}

LIB_API dal_node * dal_node_remove(dal_node * root, dal_node * node ) {
	dal_node * iterator = root;
	dal_node * prev = NULL;
	if(iterator == NULL) return NULL;
	if(root == node) return NULL;
	while(iterator != NULL ){
		if(iterator == node) {
			if(prev == NULL) return NULL;
			prev->next = node->next;
			return node;
		}
		iterator = iterator->next;
	}
	return NULL;
}

LIB_API void dal_node_clear(dal_node * root) {
	dal_node * iterator = root;
	dal_node * candidate;
	if(iterator == NULL) return ;
	while(iterator != NULL ){
		candidate = iterator;
		iterator = iterator->next;
		free(candidate);
	}
}

LIB_API dal_node * dal_record_add(dal_node * root, dal_node * node) {
	dal_node * iterator = root;
	if(iterator == NULL) return NULL;
	while(iterator->next_node != NULL) {
		iterator = iterator->next_node;
	}
	iterator->next_node = node;
	return node;
}

LIB_API dal_node * dal_record_remove(dal_node * root, dal_node * node ) {
	dal_node * iterator = root;
	dal_node * prev = NULL;
	if(iterator == NULL) return NULL;
	if(root == node) return NULL;
	while(iterator != NULL ){
		if(iterator == node) {
			if(prev == NULL) return NULL;
			prev->next_node = node->next_node;
			return node;
		}
		iterator = iterator->next_node;
	}
	return NULL;
}

LIB_API void dal_record_clear(dal_node * root) {
	dal_node * iterator = root;
	dal_node * candidate;
	if(iterator == NULL) return;
	while(iterator != NULL ){
		candidate = iterator;
		iterator = iterator->next_node;
		dal_node_clear(candidate->next);
		free(candidate);
	}
}

dal_key * dal_create_column(dal_conn * conn, int index, char * name) {
	dal_key * column = (dal_key *)malloc(sizeof(dal_key) + strlen(name) + 1);
	if(column != NULL) {
		((dal_node *)column)->next = NULL;
		((dal_node *)column)->next_node = NULL;
		((dal_node *)column)->conn = conn;
		((dal_node *)column)->type = DAL_TYPE_COLUMN;
		column->index = index;
		strcpy((char *)column->name, (char *)name);
	}
	return column;
}


dal_data * dal_create_data(dal_conn * conn, uint16 type, int length, char * data) {
	dal_data * record = (dal_data *)malloc(sizeof(dal_data) + length + 1);
	if(record != NULL) {
		((dal_node *)record)->next = NULL;
		((dal_node *)record)->next_node = NULL;
		((dal_node *)record)->conn = conn;
		((dal_node *)record)->type = type;
		memcpy(record->data, data, length);
		record->length = length;
		record->data[length] = 0;	//null terminated string
	}
	return record;
}

void dal_result_add_column(dal_result * result, dal_key * column) {
	if(result->columns == NULL) {
		result->columns = column;
	} else {
		dal_node_add((dal_node *)result->columns, (dal_node *)column);
	}
}

void dal_result_remove_column(dal_result * result, dal_key * column) {
	if(result->columns == column) {
		result->columns = (dal_column *)((dal_node *)column)->next;
	} else {
		dal_node_remove((dal_node *)result->columns, (dal_node *)column);
	}
}

dal_key * dal_result_find_column(dal_result * result, char * name) {
	dal_key * iterator = result->columns;
	while(iterator != NULL) {
		if(strcmp((const char *)iterator->name, name) == 0) {
			return iterator;
		}
		iterator = (dal_key *)((dal_node *)iterator)->next;
	}
	return iterator;
}

dal_result * dal_conn_create_result (dal_conn * conn, void * result) {
	dal_result * res = (dal_result *)malloc(sizeof(dal_result));
	if(res != NULL) {
		memset(res, 0, sizeof(dal_result));
		res->conn = conn;
		res->next = NULL;
		res->result = result;
		res->records = NULL;
		res->enumerator = NULL;
		dal_conn_add_result(conn, res);
	}
	return res;
}

dal_conn_add_result(dal_conn * conn, dal_result * result) {
	dal_result * iterator;
	if(conn == NULL) return;
	if(result == NULL) return;
	if(conn->results == NULL) {
		conn->results = result;
	} else {
		iterator = conn->results;
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = result;
	}
}

static void dal_release_result_safe(dal_result * result) {
	if(result == NULL) return;
	dal_node_clear((dal_node *)result->columns);		//release columns
	dal_record_clear((dal_node *)result->records);		//release fields
	free(result);

}

void dal_release_result(dal_result * result) {
	dal_result * candidate;
	if(result == NULL) return;
	candidate = dal_conn_remove_result(result->conn, result);
	if(result == candidate) {
		dal_release_result_safe(result);
	}
}

dal_result * dal_conn_remove_result(dal_conn * conn, dal_result * result) {
	dal_result * iterator = NULL;
	dal_result * prev = NULL;
	if(conn == NULL) return iterator;
	if(conn->results == NULL) return iterator;
	iterator = conn->results;
	while(iterator != NULL) {
		if(iterator == result) {
			if(prev == NULL) conn->results = iterator->next;
			else prev->next = iterator->next;
			return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

void dal_conn_clear_results(dal_conn * conn) {
	dal_result * iterator;
	dal_result * candidate;
	if(conn == NULL) return;
	if(conn->results == NULL) return;
	iterator = conn->results;
	while(iterator != NULL) {
		candidate = iterator;
		iterator = iterator->next;
		dal_release_result_safe(candidate);
	}
	conn->results = NULL;
}

dal_prepared_statement * dal_conn_create_statement(dal_conn * conn, char * name, char * query) {
	dal_prepared_statement * pstatement = (dal_prepared_statement *)malloc(sizeof(dal_prepared_statement) + strlen(query));
	if(pstatement != NULL) {
		pstatement->conn = conn;
		pstatement->next = NULL;
		strncpy(pstatement->name, name, 256);
		strcpy(pstatement->query, query);
		dal_conn_add_statement(conn, pstatement);
	}
	return pstatement;
}

void dal_conn_add_statement(dal_conn * conn, dal_prepared_statement * stmt) {
	dal_prepared_statement * iterator;
	if(conn == NULL) return;
	if(stmt == NULL) return;
	if(conn->statements == NULL) {
		conn->statements = stmt;
	} else {
		iterator = conn->statements;
		while(iterator->next != NULL) {
			iterator = iterator->next;
		}
		iterator->next = stmt;
	}
}

static void dal_release_statement_safe(dal_prepared_statement * stmt) {
	if(stmt == NULL) return;
	free(stmt);

}

void dal_release_statement(dal_prepared_statement * stmt) {
	dal_prepared_statement * candidate;
	if(stmt == NULL) return;
	candidate = dal_conn_remove_statement(stmt->conn, stmt);
	if(stmt == candidate) {
		dal_release_statement_safe(stmt);
	}
}

dal_prepared_statement * dal_conn_remove_statement(dal_conn * conn, dal_prepared_statement * stmt) {
	dal_prepared_statement * iterator = NULL;
	dal_prepared_statement * prev = NULL;
	if(conn == NULL) return iterator;
	if(conn->statements == NULL) return iterator;
	iterator = conn->statements;
	while(iterator != NULL) {
		if(iterator == stmt) {
			if(prev == NULL) conn->statements = iterator->next;
			else prev->next = iterator->next;
			return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

dal_prepared_statement * dal_conn_find_statement(dal_conn * conn, char * name) {
	dal_prepared_statement * iterator = NULL;
	if(conn == NULL) return iterator;
	if(conn->statements == NULL) return iterator;
	iterator = conn->statements;
	while(iterator != NULL) {
		if(strcmp(iterator->name, name) == 0) {
			return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

void dal_conn_clear_statements(dal_conn * conn) {
	dal_prepared_statement * iterator;
	dal_prepared_statement * candidate;
	if(conn == NULL) return;
	if(conn->statements == NULL) return;
	iterator = conn->statements;
	while(iterator != NULL) {
		candidate = iterator;
		iterator = iterator->next;
		dal_release_statement_safe(candidate);
	}
	conn->statements = NULL;
}

void dal_dump_result(dal_result * result) {
	dal_array * arr;
	dal_column * column_enum;
	dal_record * row_iterator;
	dal_data * data_iterator;
	arr = (dal_array *)result->records;
	if(arr == NULL) { 
		arr = dal_fetch_all(result);		//must fetch all
	}
	if(arr != NULL) {
		column_enum = result->columns;
		while(column_enum != NULL) {
			printf("%s|", column_enum->name);
			column_enum = (dal_column *)((dal_node *)column_enum)->next;
		}
		printf("\n");
		row_iterator = arr;
		while(row_iterator != NULL) {
			data_iterator = (dal_data *)row_iterator;
			while(data_iterator != NULL) {
				printf("%s|", data_iterator->data, data_iterator->length);
				data_iterator = (dal_data *)((dal_node *)data_iterator)->next;
			}
			printf("\n");
			row_iterator = (dal_record *)((dal_node *)row_iterator)->next_node;
		}
	}
}
