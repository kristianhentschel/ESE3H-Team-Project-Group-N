typedef struct request * request
typedef struct response * response


void worker(int fd);

int parse_request();
int parse_header(const char *line);
int parse_method(const char *line);
int handle_GET(const request);


int response_add_header(response, const header);
int response_set_file(response, const char* filename);
int response_write(const response, const fd);

