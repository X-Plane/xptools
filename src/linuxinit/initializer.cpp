#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libgen.h>
// we need <libgen.h> for dirname() but want to use GNU's basename()
#ifdef basename
#undef basename
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <execinfo.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include "initializer.h"

char* _strdup(const char* o)
{
	return strdup((char*)o);
}

char* _basename(const char* o)
{
	return basename((char*)o);
}

#define strdup _strdup
#define basename _basename

/* static variables */

volatile bool Initializer::m_init = false;

volatile sig_atomic_t Initializer::m_inhandler = 0;
volatile char* Initializer::m_homedir = 0;
volatile char* Initializer::m_programname = 0;
volatile char* Initializer::m_dirname = 0;
volatile char* Initializer::m_abspath = 0;

#if 0
volatile size_t Initializer::m_nsymbols = 0;
volatile void* Initializer::m_symbol_addresses[tracedepth] = {};
volatile size_t Initializer::m_iterator = 0;
volatile char** Initializer::m_symbol_names = 0;
volatile asymbol** Initializer::m_syms = 0;
volatile bfd_vma Initializer::m_pc = 0;
volatile const char* Initializer::m_filename = 0;
volatile const char* Initializer::m_functionname = 0;
volatile unsigned int Initializer::m_line = 0;
volatile int Initializer::m_found = 0;
#endif


static int
_find_stale_files(const struct dirent* e)
{
	if (strlen(e->d_name) < 3)
		return 0;
	return (!memcmp(e->d_name, ".wed_", 5) && (e->d_type == DT_REG));
}

static void
_unlink_stale_files(void)
{
	struct dirent** e;
	int32_t n;

	n = scandir("/tmp", &e, _find_stale_files, alphasort);
	if (n < 0)
		return;
	if (chdir("/tmp")) {
		::fprintf(::stderr, "can't chdir to '/tmp'\n");
		abort();
	}
	for (int32_t i = 0; i < n; ++i) {
		::fprintf(::stderr, "removing stale file: '%s'\n", e[i]->d_name);
		if (unlink(e[i]->d_name)) {
			::fprintf(::stderr, "unable to unlink '%s': %s\n", e[i]->d_name,
				strerror(errno));
			abort();
		}
	}
}

Initializer::Initializer(int* argc, char** argv[], bool loadgtk)
{
	char* temp, *temp1;

	atexit(_unlink_stale_files);

	if (!argc || !argv)
	{
		::fprintf(::stderr, "invalid startup parameters supplied\n");
		::exit(1);
	}
	if (m_init)
	{
		::fprintf(::stderr, "only one instance of class Initializer allowed\n");
		::exit(1);
	}
	m_init = true;
	// Try to resolve /proc/self/exe first
	if (!(m_abspath = canonicalize_file_name("/proc/self/exe")))
		// Try argv[0]
		if (!(m_abspath = canonicalize_file_name(*argv[0])))
			goto critical;

	temp = strdup(const_cast<const char* const>(m_abspath));
	if (!temp)
		goto critical;
	temp1 = dirname(temp);
	if (!temp1)
		goto critical;
	m_dirname = strdup(const_cast<const char* const>(temp1));
	if (!m_dirname)
		goto critical;
	free(temp);

	temp = strdup(const_cast<const char* const>(m_abspath));
	if (!temp)
		goto critical;
	temp1 = basename(const_cast<const char* const>(temp));
	if (!temp1)
		goto critical;
	m_programname = strdup(const_cast<const char* const>(temp1));
	if (!m_programname)
		goto critical;
	free(temp);

	setup_signalhandlers();
	return;
critical:
	::fprintf(::stderr, "couldn't determine location of executable\n");
	::exit(1);
}

Initializer::~Initializer()
{
// we don't free this to be able to catch segfaults which happen after
// main() returned
#if 0
	free((void*)m_programname);
	free((void*)m_dirname);
	free((void*)m_abspath);
#endif
}

const char* const Initializer::program_file()
{
	return const_cast<const char* const>(m_programname);
}

const char* const Initializer::program_abspath()
{
	return const_cast<const char* const>(m_abspath);
}

const char* const Initializer::program_dir()
{
	return const_cast<const char* const>(m_dirname);
}

const char* const Initializer::home_dir()
{
	return const_cast<const char* const>(m_homedir);
}

void Initializer::setup_signalhandlers()
{
	struct sigaction action;
	sigset_t to_block;

	// block signals during stack trace to prevent
	// further memory corruption
	::sigemptyset(&to_block);
	::sigaddset(&to_block, SIGINT);
	::sigaddset(&to_block, SIGQUIT);
	::sigaddset(&to_block, SIGTERM);
	::sigaddset(&to_block, SIGABRT);

	::memset(&action, 0, sizeof(action));
	::sigemptyset(&action.sa_mask);
	action.sa_sigaction = _handle_signal;
	action.sa_mask = to_block;
	action.sa_flags = SA_SIGINFO;
	::sigaction(SIGSEGV, &action, 0);
	::sigaction(SIGTERM, &action, 0);
	::sigaction(SIGINT, &action, 0);
	::sigaction(SIGQUIT, &action, 0);
	::sigaction(SIGABRT, &action, 0);
}

void Initializer::_handle_signal(int signal, siginfo_t* info, void* context)
{
	if (m_inhandler) return;
	m_inhandler = 1;
	::exit(0);
}

# if 0
void Initializer::stack_trace(void)
{
	string logfile = ::getenv("HOME");
	logfile += "/.xpt_trace";
	FILE* logf = fopen(logfile.c_str(), "ab");
	::fprintf(::stderr, "--stack trace start--\n\n");
	if (logf) fprintf(logf, "--stack trace start--\n\n");
	m_nsymbols = ::backtrace((void**)m_symbol_addresses, tracedepth);

	// GPL variant (using libbfd), TODO: write malloc-free variant, looking
	// up bfd symbols via weak symbol mechanism and fall back to libc's
	// backtrace if libbfd isn't available
	m_symbol_names = (volatile char**)backtrace_symbols_bfd((void* const*)m_symbol_addresses, m_nsymbols);
	for(m_iterator = 0; m_iterator < m_nsymbols; ++m_iterator)
	{
		::fprintf(::stderr, "%s\n", m_symbol_names[m_iterator]);
		if (logf) ::fprintf(logf, "%s\n", m_symbol_names[m_iterator]);
	}

	// LGPL variant (using libc)
	//::backtrace_symbols_fd((void* const*)m_symbol_addresses, m_nsymbols, fileno(::stderr));

	::fprintf(::stderr, "\n--stack trace fini--\n");
	if (logf) ::fprintf(logf, "\n--stack trace fini--\n\n");
	if (logf) ::fflush(logf);
	if (logf) ::fsync(fileno(logf));
	if (logf) ::fclose(logf);
	::fflush(::stderr);
	return;
}

char** Initializer::backtrace_symbols_bfd(void* const* buffer, int size)
{
	int stack_depth = size - 1;
	int x,y;
	int total = 0;
	char*** locations;
	char** final;
	char* f_strings;

	locations = (char***)malloc(sizeof(char**) * (stack_depth+1));
	bfd_init();
	for(x=size-1, y=0; x>=0; x--, y++){
		file_match match;
		match.address = buffer[x];
		char** ret_buf;
		bfd_vma addr;
		dl_iterate_phdr((int(*)(dl_phdr_info*, size_t, void*))find_matching_file, &match);
		addr = (bfd_vma)((intptr_t)buffer[x] - (intptr_t)match.base);
		if (match.file && strlen(match.file))
			ret_buf = process_file(match.file, &addr, 1);
		else {
			ret_buf = process_file((const char*)m_abspath, &addr, 1);
#if 0
			if (strstr(ret_buf[0], "?\?() ??:0"))
				ret_buf = process_file(g_debugfile, &addr, 1);
#endif
		}
		locations[x] = ret_buf;
		total += strlen(ret_buf[0]) + 1;
	}
	final = (char**)malloc(total + (stack_depth + 1) * sizeof(char*));
	f_strings = (char*)(final + stack_depth + 1);
	for(x=stack_depth; x>=0; x--)
	{
		strcpy(f_strings, locations[x][0]);
		free(locations[x]);
		final[x] = f_strings;
		f_strings += strlen(f_strings) + 1;
	}
	free(locations);
	return final;
}

int Initializer::find_matching_file(struct dl_phdr_info *info, size_t size, file_match* match)
{
	long n;
	const ElfW(Phdr) *phdr;
	ElfW(Addr) load_base = info->dlpi_addr;
	phdr = info->dlpi_phdr;
	for (n = info->dlpi_phnum; --n >= 0; phdr++)
	{
		if (phdr->p_type == PT_LOAD)
		{
			ElfW(Addr) vaddr = phdr->p_vaddr + load_base;
			if (match->address >= (void*)vaddr && match->address < (void*)(vaddr + phdr->p_memsz))
			{
				match->file = info->dlpi_name;
				match->base = (void*)info->dlpi_addr;
			}
		}
	}
	return 0;
}

char** Initializer::process_file(const char* file_name, bfd_vma* addr, int naddr)
{
	bfd* abfd;
	char** matching;
	char** ret_buf;

	abfd = bfd_openr(file_name, "default");
	if (abfd == NULL)
		::exit(1);
	if (bfd_check_format(abfd, bfd_archive))
		::exit(1);
	if (!bfd_check_format_matches(abfd, bfd_object, &matching))
		::exit(1);
	slurp_symtab(abfd);
	ret_buf = translate_addresses_buf(abfd, addr, naddr);
	if (m_syms != 0)
	{
		::free(m_syms);
		m_syms = 0;
	}
	bfd_close(abfd);
	return ret_buf;
}

void Initializer::slurp_symtab(bfd* abfd)
{
	long symcount;
	unsigned int size;

	if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
		return;

	symcount = bfd_read_minisymbols(abfd, false, (void**)(PTR)&m_syms, &size);
	if (symcount == 0)
		symcount = bfd_read_minisymbols(abfd, 1, (void**)(PTR)&m_syms, &size);

	if (symcount < 0)
		::exit(1);
}

void Initializer::find_address_in_section(bfd* abfd, asection* section, void* data __attribute__((__unused__)))
{
	bfd_vma vma;
	bfd_size_type size;

	if (m_found)
		return;
	if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
		return;
	vma = bfd_get_section_vma(abfd, section);
	if (m_pc < vma)
		return;
	size = bfd_section_size(abfd, section);
	if (m_pc >= vma + size)
		return;
	if (m_syms)
		m_found = bfd_find_nearest_line(abfd, section, (bfd_symbol**)m_syms, m_pc - vma, (const char**)&m_filename, (const char**)&m_functionname, (unsigned int*)&m_line);
}

char** Initializer::translate_addresses_buf(bfd* abfd, bfd_vma* addr, int naddr)
{
	int naddr_orig = naddr;
	char b;
	int total  = 0;
	enum state
	{ Count, Print };
	char *buf = &b;
	int len = 0;
	char **ret_buf = NULL;
	char* alloc = 0;
	char* inter = 0;
	/* iterate over the formating twice.
	 * the first time we count how much space we need
	 * the second time we do the actual printing */
	for (int st=Count; st<=Print; st++)
	{
		if (st == Print)
		{
			ret_buf = (char**)malloc(total + sizeof(char*)*naddr);
			buf = (char*)(ret_buf + naddr);
			len = total;
		}
	while (naddr) {
		if (st == Print)
			ret_buf[naddr-1] = buf;
		m_pc = addr[naddr-1];

		m_found = 0;

		bfd_map_over_sections(abfd, find_address_in_section, (PTR) NULL);
		if (!m_found) {
			total += snprintf(buf, len, "[0x%llx] \?\?() \?\?:0",(long long unsigned int) addr[naddr-1]) + 1;
		} else {
			const char *name;

			name = (const char*)m_functionname;
			if (name == NULL || *name == '\0')
				name = "??";
			else
			{
				alloc = bfd_demangle (abfd, name, (1 << 1) | (1 << 0));
				if (alloc != NULL)
					name = alloc;
				else
				{
					inter = (char*)malloc(strlen(name)+3);
					if (inter)
					{
						memset(inter, 0, strlen(name)+3);
						strcpy(inter, name);
						strcat(inter, "()");
					}
					else inter = (char*)name;
				}
			}
			if (m_filename != NULL) {
				char *h;

				h = strrchr((char*)m_filename, '/');
				if (h != NULL)
					m_filename = h + 1;
			}
			total += snprintf(buf, len, "[%s(%s:%u)]: %s", abfd->filename, m_filename?m_filename:"??", m_line, alloc?alloc:inter) + 1;
			if (inter) free(inter);

		}
		if (st == Print) {
			/* set buf just past the end of string */
			buf = buf + total + 1;
		}
		naddr--;
	}
	naddr = naddr_orig;
	}
	return ret_buf;
}
#endif
