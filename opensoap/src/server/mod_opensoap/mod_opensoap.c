/* 
**  mod_opensoap.c -- Apache sample opensoap module
**  [Autogenerated via ``apxs -n opensoap -g'']
**
**  To play with this sample module first compile it into a
**  DSO file and install it into Apache's modules directory 
**  by running:
**
**    $ apxs -c -i mod_opensoap.c
**
**  Then activate it in Apache's httpd.conf file for instance
**  for the URL /opensoap in as follows:
**
**    #   httpd.conf
**    LoadModule opensoap_module modules/mod_opensoap.so
**    <Location /opensoap>
**    SetHandler opensoap
**    </Location>
**
**  Then after restarting Apache via
**
**    $ apachectl restart
**
**  you immediately can request the URL /opensoap and watch for the
**  output of this module. This can be achieved for instance via:
**
**    $ lynx -mime_header http://localhost/opensoap 
**
**  The output should be similar to the following one:
**
**    HTTP/1.1 200 OK
**    Date: Tue, 31 Mar 1998 14:42:22 GMT
**    Server: Apache/1.3.4 (Unix)
**    Connection: close
**    Content-Type: text/html
**  
**    The sample page from mod_opensoap.c
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "apr_file_info.h"
#include "apr_strings.h"

#include "http_log.h"

#include "soapInterface4ap_dso.h"

//#define DEBUG

static int write_request(request_rec* r, char* req_id)
{
	char buff[HUGE_STRING_LEN];
	long length; /* length of request body */
	int len_read; /* size of reading data */
	int rsize; /* actual size which need to copy */
	int rpos = 0; /* current position */
	apr_file_t* fd;
	apr_status_t rv = 0;
	char req_fn[256];
	int rc = 0;

	/***** For Http Header ******/
	rc = GetNewId(req_id);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : GetNewId(req_id)");
		return EXIT_FAILURE;
	}
	
	WriteLog(8,req_id);
	
#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL,
		"*** mod *** : GetNewId req_id = %s", req_id);
#endif

	rc = AddHttpHeader(req_id, "Content-type", 
			apr_table_get(r->headers_in, "Content-type"));
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : AddHttpHeader");
		return EXIT_FAILURE;
	}		
		
#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL,
		"*** mod *** : AddHttpHeader = %s", apr_table_get(r->headers_in, "Content-type"));
#endif

	AddHttpHeader(req_id, "SOAPAction", apr_table_get(r->headers_in, "SOAPAction"));
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : AddHttpHeader");
		return EXIT_FAILURE;
	}		

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL,
		"*** mod *** : AddHttpHeader = %s", apr_table_get(r->headers_in, "SOAPAction"));
#endif
	
	/***** For Http Body ******/	
	/* check the receive data format, and initialize */
	if(ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : ap_setup_client_block");
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	/* check the existance of data to receive */
	if(!ap_should_client_block(r)) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : ap_should_client_block");
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	
	length = r->remaining;

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : content-len = %ld", length);
#endif
	
	rc = CheckLimitSizeMessage(length, req_id);
	if (rc == EXIT_FAILURE) { // is error, soap fault was created using req_id
		while((len_read = ap_get_client_block(r, buff, sizeof(buff))) > 0) ;
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : limit over message");
		return DSO_TOO_BIG;
	}
	
	rc = GetFileName(req_id, req_fn);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : GetFileName");
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : GetFilenName req_fn = %s", req_fn);
#endif

	rv = apr_file_open(&fd, req_fn, 
		APR_WRITE | APR_CREATE | APR_TRUNCATE | APR_BINARY, APR_OS_DEFAULT, r->pool);
	
	if (rv != APR_SUCCESS) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : apr_file_open");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	/* copy request body to buffer, and return the size */
	while((len_read = ap_get_client_block(r, buff, sizeof(buff))) > 0) {
		if((rpos + len_read) > length)
			rsize = length - rpos;
		else
			rsize = len_read;
		
		if(apr_file_write(fd, buff, &rsize) != APR_SUCCESS) {
			ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
				"*** mod_err *** : apr_file_write");
			return HTTP_INTERNAL_SERVER_ERROR;
		}
		rpos += rsize;
	}
	rv = apr_file_close(fd);

	if (rv != APR_SUCCESS) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : apr_file_close");
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	
	return EXIT_SUCCESS;
}

static int read_response(request_rec* r, char* res_id)
{
	apr_file_t* fd;
	apr_status_t rv = 0;
	apr_size_t bytes_sent;
	apr_finfo_t fi;
	char res_fn[256];
	const char *res_content;
	const char *res_status;
	int rc = 0;
	
	/***** For Http Header ******/
	rc = GetHttpHeader(res_id, "Content-type", &res_content);
	if(rc == DSO_NO_MATCH) {// in case without maching key
		//true = success, false = failure
		res_content = "text/xml";
	}
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : GetHttpHeader(res_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	r->content_type = (char *) apr_pstrdup(r->pool, res_content);

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : GetHttpHeader header = %s", res_content);
#endif

	rc = GetHttpHeader(res_id, "status", &res_status);
	if(rc == DSO_NO_MATCH) {// in the case without maching key
		res_status = "200";
	}
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : GetHttpHeader(req_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	}		
		
	r->status = atoi(res_status);

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : GetHttpHeader header = %s", res_status);
#endif

	/***** For Http Body ******/
	rc = GetFileName(res_id, res_fn);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : GetFileName(res_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	}		

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL,
		"*** mod *** : GetFileName res_fn = %s", res_fn);
#endif

	rv = apr_file_open(&fd, res_fn, APR_READ, -1, r->pool);
	if (rv != APR_SUCCESS) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : apr_file_open");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	/****** Get file size of response message ******/
	rv = apr_stat(&fi, res_fn, APR_FINFO_SIZE, r->pool);
	if (rv != APR_SUCCESS) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : apr_stat");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : apr_stat res_size = %ld", (apr_off_t)fi.size);
#endif

	WriteLog(9,"send to client start");
	/* send the file with size if known */
	if (r->proto_num <1001) ap_set_content_length(r,fi.size);
	rv = ap_send_fd(fd, r, 0, ((fi.size > 0) ? fi.size : -1), &bytes_sent);
	if (rv != APR_SUCCESS) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : ap_send_fd");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	/* Must not file close
	rv = apr_file_close(fd);
	*/
	
	return EXIT_SUCCESS;
}

static int delete_files(char* num_id)
{
	apr_status_t rv = 0;
	int rc = 0;	

	/****** Delete Files ******/
	rc = DeleteFiles(num_id);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : DeleteFiles(req_id)");
	}		

	return EXIT_SUCCESS;
}

static int opensoap_handler(request_rec *r)
{	
	apr_status_t rv = 0;
	char req_id[128];
	char res_id[128];
	int rc = 0;	

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : start");
#endif
	
	if (strcmp(r->handler, "opensoap")) {
		return DECLINED;
	}
	
	if (r->header_only)
		return HTTP_BAD_REQUEST;
	
	if(r->method_number != M_POST)
		return HTTP_BAD_REQUEST;

	SetProcessInfo();
	WriteLog(8,"mod_opensoap start");
	
	/***** Read Request and Write to File ******/
	rc = write_request(r, req_id);
 	if ((rc == EXIT_FAILURE) || (rc == HTTP_INTERNAL_SERVER_ERROR)) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : write_request(r, req_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	} else if (rc == DSO_TOO_BIG) {
		read_response(r, req_id);
		rc = delete_files(req_id);
	 	if (rc == EXIT_FAILURE) {
			ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
				"*** mod_err *** : delete_files(res_id)");
		}		
		return OK;
	}

	/****** Invoke Req_Id and Get Res_ID *******/
	WriteLog(9,"invoke start");

	rc = InvokeOpenSOAPServer(req_id, res_id);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : InvokeOpenSOAPServer(req_id, res_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	}		

	WriteLog(9,res_id);

#ifdef DEBUG
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : InvokeOpenSOAPServer req_id = %s", req_id);
	ap_log_error(APLOG_MARK, APLOG_NOTICE, rv, NULL, 
		"*** mod *** : InvokeOpenSOAPServer res_id = %s", res_id);
#endif

	/******** Load Response form File **********/
	rc = read_response(r, res_id);
 	if ((rc == EXIT_FAILURE) || (rc == HTTP_INTERNAL_SERVER_ERROR)) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : read_response(r, res_id)");
		return HTTP_INTERNAL_SERVER_ERROR;
	}		

	/****** Delete Files ******/
	rc = delete_files(req_id);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : delete_files(req_id)");
	}		

	rc = delete_files(res_id);
 	if (rc == EXIT_FAILURE) {
		ap_log_error(APLOG_MARK, APLOG_EMERG, rv, NULL, 
			"*** mod_err *** : delete_files(res_id)");
	}		
	
	WriteLog(8,"mod_opensoap end");

	return OK;
}

static void opensoap_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(opensoap_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA opensoap_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    opensoap_register_hooks  /* register hooks                      */
};
