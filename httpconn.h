#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_

enum http_version {
	HTTP_UNKNOWN,
	HTTP_10,
	HTTP_11
};

enum http_state {
	STATE_IDLE,
	STATE_READ_FIRSTLINE,
	STATE_READ_HEADERS,
	STATE_READ_BODY,
	STATE_MANGLED
};

enum http_type {
	HTTP_CLIENT,
	HTTP_SERVER
};

enum http_method {
	METH_GET,
	METH_HEAD,
	METH_POST,
	METH_PUT,
	METH_CONNECT
};

enum http_te {
	TE_IDENTITY,
	TE_CHUNKED
};

enum http_conn_error {
	ERROR_NONE,
	ERROR_IDLE_CONN_TIMEDOUT,
	ERROR_CLIENT_POST_WITHOUT_LENGTH,
	ERROR_INCOMPLETE_HEADERS,
	ERROR_INCOMPLETE_BODY,
	ERROR_HEADER_PARSE_FAILED,
	ERROR_CHUNK_PARSE_FAILED,
	ERROR_WRITE_FAILED
};

struct evbuffer;
struct event_base;
struct http_conn;
struct headers;

struct http_request {
	TAILQ_ENTRY(http_request) next;
	enum http_method meth;
	char *uri;
	enum http_version vers;
	struct header_list *headers;
};
TAILQ_HEAD(http_request_list, http_request);

struct http_response {
	enum http_version vers;
	int code;
	char *reason;
	struct header_list *headers;
};

struct http_cbs {
	void (*on_error)(struct http_conn *, enum http_conn_error, void *);
	void (*on_client_request)(struct http_conn *, struct http_request *, void *);
	void (*on_server_response)(struct http_conn *, struct http_response *, void *);
	void (*on_read_body)(struct http_conn *, struct evbuffer *, void *);
	void (*on_msg_complete)(struct http_conn *, void *);

	/* called when it is ok to write more data after choaking */
	void (*on_write_more)(struct http_conn *, void *);
};

struct http_conn *http_conn_new(struct event_base *base, evutil_socket_t sock,
				enum http_type type, struct http_cbs *cbs,
				void *cbarg);

void http_conn_free(struct http_conn *conn);

void http_conn_add_request(struct http_conn *conn, struct http_request *req);
void http_conn_del_request(struct http_conn *conn, struct http_request *req);

void http_conn_write_response(struct http_conn *conn, struct http_response *resp);

/* return: -1 on failure, 0 on choaked, 1 on queued. */
int http_conn_write_buf(struct http_conn *conn, struct evbuffer *buf);

int http_conn_has_body(struct http_conn *conn);

#endif
