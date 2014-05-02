/*
 *  curl_http.h
 *
 */

#ifndef curl_http_H
#define curl_http_H

#if !IBM
#include <pthread.h>
#endif

/*
 * curl_http_get_file
 *
 * curl_http_get_file runs a single asynchronous HTTP request for one file.  It provides delivery
 * in memory or on disk, and can unzip a delivery to disk.  
 *
 * Operation is truly async - a worker thread is spawned, and the worker thread blocks on network
 * I/O; the worker is destroyed once the connection is completed.
 *
 * Asynchronous progress can be queried from any thread via get_status and get_progress; a request to
 * abort is made by deleting the file.  Note that deleting the file is a -blocking- operation until
 * the request is fully canceled, so halting may take a little time.  In particular, CURL can't short-
 * circuit synchronous host DNS lookup, so when blocked on a true URL, some paranoia is called for!
 *
 * To determine: asynchronuos service is provided by CURL callbacks - mailing lists imply it's about
 * 1 per second.
 *
 */

class	curl_http_get_file {
public:

	enum {
		in_progress		= 0,		// In progress - use get_progress to see percent-DL
		done_OK			= 1,		// Sucessful download - buffer or dest file is now complete.
		done_error		= 2			// Failure of some kind - err code available...
	};

				curl_http_get_file(
							const string&			inURL,
							const string&			outDestFile);

				curl_http_get_file(
							const string&			inURL,
							const string *			inPostInfo,
							const string *			inPutInfo,
							vector<char>*			outBuffer);
				
				~curl_http_get_file();
	
	bool		is_done(void);
	bool		is_ok(void);			// IF done, did the op finish without error?
	bool		is_net_fail(void);		// IF we had an error, is it one that MIGHT be local (e.g. bad net connection)?

	float		get_progress(void);		// if nt done, progress, percent, e.g 53.4f;
	int			get_error(void);		// If done and not ok, what is error
	void		get_error_data(vector<char>& out_data);	// If an error, any stuff the server sent -- might be text, HTML, who knows!
	
private:

		volatile	int			m_progress;		// Out of 100
		volatile	int			m_status;
		volatile	int			m_halt;
		volatile	int			m_errcode;
		
		#if IBM
		HANDLE					m_thread;
		#else
		pthread_t				m_thread;
		#endif
		
		static	size_t		write_cb(void *contents, size_t size, size_t nmemb, void *userp);
		static	size_t		read_cb(void *contents, size_t size, size_t nmemb, void *userp);
		static	int			progress_cb(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);

		#if IBM
		static	DWORD WINAPI	thread_proc(void * param);
		#else
		static	void *		thread_proc(void * param);
		#endif
	
		vector<char>			m_dl_buffer;
		vector<char>*			m_dest_buffer;
		
		string					m_dest_path;
		string					m_url;
		string					m_post;
		string					m_put;
		
		double					m_last_dl_amount;
		time_t					m_last_data_time;
};

bool	UTL_http_is_error_bad_net(int err);


#endif /* UTL_http_H */
