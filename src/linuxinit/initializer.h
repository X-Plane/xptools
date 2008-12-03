// Ben, if you want fancy stack traces on MacOS, let me know.
// with a few fixes i might be able to make this work on MacOS

/**
** intantiate as very first local variable in main() like:
** Initializer init(&argc, &argv);
**
** sets up signal handlers for SIGINT and SIGSEGV, which
** will print a detailed stacktrace. code using this class
** needs to be linked against libbfd (statically), libiberty
** and libz (needed by static libbfd).
**/

#if !LIN
	#error Use the Initializer class under Linux only.
#endif

#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <csignal>
#include <string>
#include <dlfcn.h>
#include <link.h>
#include <bfd.h>

enum InitializerConstants
{
	// this consumes 300k memory and should be ok
	tracedepth = 300,
	symbollength = 1024
};

typedef struct file_match
{
	const char* file;
	void* address;
	void* base;
	void* hdr;
} file_match;

class Initializer
{
/**
** not a singleton. we allow construction under main() only.
** thread safety is not an issue when instantiating the class
** as a local variable in main() before any thread creation code.
** the only possible way to produce more than one instance is a
** manual call or recursion of main(). we use a static bool
** variable and throw an exception to avoid that.
**/
public:
	// the only public member
	static const char* const programname();
private:
	// intentionally left undefined
	Initializer();
	~Initializer();
	friend int main(int argc, char* argv[]);
	Initializer(int* argc, char** argv[], bool loadgtk = true);
	Initializer(const Initializer&);
	Initializer& operator =(const Initializer& other);

	void setup_signalhandlers();
	/**
	** static members, when processing signals we have no
	** safe possiblity to access specific class instances.
	** we also need to lower the use of the stack in functions
	** (i.e. local variables) and malloc as we are in a
	** faulty environment anyway when this code is executed
	**/
	static void _handle_signal(int signal, siginfo_t* info, void* context);
	static void stack_trace(void);
	/**
	** being paranoid and using volatile here to avoid
	** memory optimizations by the compiler
	**/
	static volatile bool m_init;
	static volatile char* m_programname;
	static volatile sig_atomic_t m_inhandler;
	static volatile size_t m_nsymbols;
	static volatile void* m_symbol_addresses[tracedepth];
	// currently unused
	// static volatile char m_symbol_names[tracedepth][symbollength];
	static volatile asymbol** m_syms;
	static volatile char** m_symbol_names;
	static volatile size_t m_iterator;
	// used to pass information between translate_addresses
	// and find_address_in_section.
	static volatile bfd_vma m_pc;
	static volatile const char* m_filename;
	static volatile const char* m_functionname;
	static volatile unsigned int m_line;
	static volatile int m_found;
	/**
	** libbfd related functions (only use in GPL compatible code).
	** this code will only work when linked statically against
	** libbfd.
	** similar functionality can be achieved by backtracing with
	** libc's backtrace_symbols() and demangle the names with
	** abi::__cxa_demangle(), which would allow the use in
	** proprietary applications, but then we would have to write
	** own code for looking up debugging information (i.e.
	** filename and line of the current symbol)
	**/
	static char** backtrace_symbols_bfd(void* const* buffer, int size);
	static int find_matching_file(struct dl_phdr_info *info, size_t size, file_match* match);
	static char** process_file(const char* file_name, bfd_vma* addr, int naddr);
	static void slurp_symtab(bfd* abfd);
	static void find_address_in_section(bfd* abfd, asection* section, void* data __attribute__((__unused__)));
	static char** translate_addresses_buf(bfd* abfd, bfd_vma* addr, int naddr);
};

#endif /*  INITIALIZER_H */
