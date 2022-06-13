#include "defs.h"
#include "config.h"
#include <stdarg.h>
#ifndef DAL__H

#ifdef HAVE_LIBPQ
#include "libpq-fe.h"
#endif

#define DAL_TYPE_ARRAY		0x4000
#define DAL_TYPE_RECORD		0x4000
#define DAL_TYPE_ROWS		0x8000
#define DAL_TYPE_COLUMN		0x6000
#define DAL_TYPE_INTEGER	0x0001
#define DAL_TYPE_REAL		0x0002
#define DAL_TYPE_STRING		0x0010
#define DAL_TYPE_BYTES		0x0080
#define DAL_TYPE_BIGINT		0x0004

#define DAL_CONN_PGSQL		0x01

typedef struct dal_database dal_database;
typedef struct dal_conn dal_conn;
typedef struct dal_prepared_statement dal_prepared_statement;
typedef struct dal_result dal_result;
typedef struct dal_node dal_node;
typedef struct dal_array dal_array;
typedef struct dal_key dal_key;
typedef struct dal_key dal_column;
typedef struct dal_array dal_record;
typedef struct dal_data dal_data;
typedef struct dal_data dal_cell;
typedef struct dal_interface dal_interface;

struct dal_database {
	char * prefix;
	dal_interface * backend;
};

struct dal_interface {
	//open connection
	uint8 (*open)(dal_conn * conn);
	//close connection
	void (*close)(dal_conn * conn);
	//query statement
	dal_result * (*query)(dal_conn * conn, char * statement);
	//prepare statement
	dal_prepared_statement * (*prepare)(dal_conn * conn, char * name, char * statement);
	//execute statement
	dal_result * (*execute)(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]);
	//fetch all result
	dal_array * (*fetch)(dal_result *);
	//get result count
	int (*count)(dal_result *);
	//get first result
	dal_array * (*first)(dal_result *);
	//get next result
	dal_array * (*next)(dal_result *);
};

struct dal_conn {
	uint8 type;
	dal_interface * backend;
	void * conn;
	char connect_str[512];
	void * heap;
	dal_prepared_statement * statements;
	dal_result * results;		//keep track of all result
};

struct dal_prepared_statement {
	dal_conn * conn;
	dal_prepared_statement * next;
	char name[256];
	void * prepared_statement;
	char query[1];
};

struct dal_result {
	dal_conn * conn;
	dal_result * next;
	dal_column * columns;
	void * enumerator;
	void * records;
	void * result;
};

struct dal_node {
	dal_conn * conn;
	uint16 type;
	dal_node * next;
	dal_node * next_node;
};

struct dal_key {
	dal_node base;
	int index;
	uint8 name[1];
};

struct dal_array {
	dal_node base;
	dal_column * columns;
};

struct dal_data {
	dal_node base;
	uint32 length;
	uint8 data[1];
};

#ifdef __cplusplus
extern "C" {
#endif
//APIs
//create database connection
LIB_API dal_conn * dal_create_conn(char * conn_str);
//open connection
LIB_API uint8 dal_open(dal_conn * conn);
//close connection
LIB_API void dal_close(dal_conn * conn);
//query statement
LIB_API dal_result * dal_query(dal_conn * conn, char * statement);
//prepare statement
LIB_API dal_prepared_statement * dal_prepare(dal_conn * conn, char * name, char * statement);
//execute statement
LIB_API dal_result * dal_execute(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]);
//fetch all result
LIB_API dal_array * dal_fetch_all(dal_result *);
//get result count
LIB_API int dal_num_rows(dal_result *);
//get first result
LIB_API dal_array * dal_fetch_first(dal_result *);
//get next result
LIB_API dal_array * dal_fetch_next(dal_result *);
//get result at/key
LIB_API dal_data * dal_row_at(dal_array *, int index);
LIB_API dal_data * dal_row_key(dal_array *, char * name);
LIB_API void dal_dump_result(dal_result *);

//internal
dal_key * dal_create_column(dal_conn * conn, int index, char * name);
dal_data * dal_create_data(dal_conn * conn, uint16 type, int length, char * data);
void dal_result_add_column(dal_result * result, dal_key * column);
void dal_result_remove_column(dal_result * result, dal_key * column);
dal_key * dal_result_find_column(dal_result * result, char * name);
dal_node * dal_node_add(dal_node * root, dal_node * node);
dal_node * dal_node_remove(dal_node * root, dal_node * node );
void dal_node_clear(dal_node * root);
dal_node * dal_record_add(dal_node * root, dal_node * node);
dal_node * dal_record_remove(dal_node * root, dal_node * node );
void dal_record_clear(dal_node * root);
dal_result * dal_conn_create_result (dal_conn * , void * result);
void dal_conn_add_result(dal_conn * conn, dal_result * result);
dal_result * dal_conn_remove_result(dal_conn * conn, dal_result * result) ;
void dal_release_result(dal_result * result) ;
void dal_conn_clear_results(dal_conn * conn);

dal_prepared_statement * dal_conn_create_statement(dal_conn * conn, char * name, char * query);
dal_prepared_statement * dal_conn_find_statement(dal_conn * conn, char * name);
void dal_conn_add_statement(dal_conn * conn, dal_prepared_statement * stmt);
static void dal_release_statement_safe(dal_prepared_statement * stmt);
void dal_release_statement(dal_prepared_statement * stmt);
dal_prepared_statement * dal_conn_remove_statement(dal_conn * conn, dal_prepared_statement * stmt);
void dal_conn_clear_statements(dal_conn * conn);

#ifdef __cplusplus
}
#endif
#define DAL__H
#endif
