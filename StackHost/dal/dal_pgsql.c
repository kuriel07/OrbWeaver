#include "dal.h"
#include "StackVM/midgard.h"

#if defined(HAVE_LIBPQ)
uint8 pgsql_open(dal_conn * conn) ;
void pgsql_close(dal_conn * conn);
dal_result * pgsql_query(dal_conn * conn, char * statement);
dal_prepared_statement * pgsql_prepare(dal_conn * conn, char * name, char * statement);
dal_result * pgsql_execute(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]);
dal_array * pgsql_fetch_all(dal_result * result);
int pgsql_num_rows(dal_result * result );
dal_array * pgsql_fetch_first(dal_result * result);
dal_array * pgsql_fetch_next(dal_result * result);

dal_interface pg_sql_interface = {
	pgsql_open,
	pgsql_close,
	pgsql_query,
	pgsql_prepare,
	pgsql_execute,
	pgsql_fetch_all,
	pgsql_num_rows,
	pgsql_fetch_first,
	pgsql_fetch_next,
};

//open connection
uint8 pgsql_open(dal_conn * conn) {
	conn->conn = PQconnectdb(conn->connect_str);
	printf("connection string : %s\n", conn->connect_str);
	if(conn->conn != NULL) {
		if(PQstatus((PGconn *)conn->conn) == CONNECTION_OK) {
			return TRUE;
		}
	}
	return FALSE;
}
//close connection
void pgsql_close(dal_conn * conn) {
	PQfinish((PGconn *)conn->conn);
}
//query statement
dal_result * pgsql_query(dal_conn * conn, char * statement) {
	int i;
	dal_result * result = NULL;
	printf("query : %s\n", statement);
	PGresult * pgres = PQexec((PGconn *)conn->conn, statement);
	if(pgres != NULL) {
		result = dal_conn_create_result (conn, pgres);
		for(i=0;i<PQnfields(pgres);i++) {
			dal_result_add_column(result, dal_create_column(conn, i, PQfname(pgres, i)));
		}
	}
	return result;
}

//prepare statement
dal_prepared_statement * pgsql_prepare(dal_conn * conn, char * name, char * query) {
	dal_prepared_statement * pstatement = NULL;
	PGresult * pgres = PQprepare((PGconn *)conn->conn,
                    name,
                    query,
                    0,
                    NULL);
	if(PQresultStatus(pgres) == PGRES_COMMAND_OK) {
		pstatement = dal_conn_create_statement(conn, name, query);
	}
	return pstatement;
}

//execute statement
dal_result * pgsql_execute(dal_conn * conn, dal_prepared_statement * statement, int count, char * args[]) {
	char * str;
	int i;
	dal_result * result = NULL;
	PGresult * pgres;
	//int nParams = -1;
	//char * arguments[128];
	//va_list args;
    //va_start( args, statement );
	//do {
	//	nParams++;
	//	str = va_arg(args, char*);
	//	arguments[nParams] = str;
	//} while (str != NULL);
	//va_end(args);
	pgres = PQexecPrepared((PGconn *)conn->conn, statement->name, count, args, NULL, NULL, NULL);
	if(PQresultStatus(pgres) == PGRES_COMMAND_OK) {
		result = dal_conn_create_result(conn, pgres);
		for(i=0;i<PQnfields(pgres);i++) {
			dal_result_add_column(result, dal_create_column(conn, i, PQfname(pgres, i)));
		}
	}
	return result;
}

//fetch all result
dal_array * pgsql_fetch_all(dal_result * result) {
	int i=0, j=0;
	int length;
	dal_data * record, * first_record = NULL;
	dal_data * first_row = NULL;
	PGresult * pgres = (PGresult * )result->result;
	for(i=0;i<PQntuples(pgres);i++) {
		first_record = NULL;
		for(j=0;j<PQnfields(pgres);j++) {
			if(PQgetisnull(pgres, i, j)) {
				length = 0;
			} else {
				length = PQgetlength(pgres, i, j);
			}
			record = dal_create_data(result->conn, DAL_TYPE_STRING, length, PQgetvalue(pgres, i, j));
			if(first_record == NULL) {
				first_record = record;
			} else {
				dal_node_add((dal_node *)first_record, (dal_node *)record);
			}
		}
		if(first_row == NULL) {
			first_row = first_record;
		} else {
			dal_record_add((dal_node *)first_row, (dal_node *)first_record);
		}
	}
	result->enumerator = first_row;
	result->records = first_row;
	return (dal_array *)first_row;
}
//get result count
int pgsql_num_rows(dal_result * result ) {
	return PQntuples((PGresult *)result->result);
}
//get first result
dal_array * pgsql_fetch_first(dal_result * result) {
	dal_array * arr;
	arr = (dal_array *)result->records;
	if(arr == NULL) { 
		arr = pgsql_fetch_all(result);		//must fetch all
	}
	result->enumerator = arr;
	return arr;
}
//get next result
dal_array * pgsql_fetch_next(dal_result * result){
	dal_array * arr = (dal_array *)result->enumerator;
	if(arr != NULL) {
		result->enumerator = arr = (dal_array *)((dal_node *)arr)->next_node;
	}
	return arr;
}
#endif
