// there are slight chances that this code will cause a SIGSEV itself.
// this happens when the stack is smashed and the last instuction pointer is invalid.
// any ideas how to fix this?

// taken from the libcairo package, which originated
// basically from addr2line (binutils),
// sligthly modified to support detached debuginfo (not very good code tbh)

extern const char* program_name;

#define __USE_GNU
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <bfd.h>
#include <libiberty.h>
#include <dlfcn.h>
#include <link.h>

#define fatal(a, b) exit(1)
#define bfd_fatal(a) exit(1)
#define bfd_nonfatal(a) exit(1)
#define list_matching_formats(a) exit(1)

/* 2 characters for each byte, plus 1 each for 0, x, and NULL */
#define PTRSTR_LEN (sizeof(void *) * 2 + 3)
#define true 1
#define false 0

static asymbol **syms;		/* Symbol table.  */

/* 150 isn't special; it's just an arbitrary non-ASCII char value.  */
#define OPTION_DEMANGLER	(150)

static void slurp_symtab(bfd * abfd);
static void find_address_in_section(bfd *abfd, asection *section, void *data);

/* Read in the symbol table.  */

static void slurp_symtab(bfd * abfd)
{
	long symcount;
	unsigned int size;

	if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
		return;

	symcount = bfd_read_minisymbols(abfd, false, (PTR) & syms, &size);
	if (symcount == 0)
		symcount = bfd_read_minisymbols(abfd, true /* dynamic */ ,
						(PTR) & syms, &size);

	if (symcount < 0)
		bfd_fatal(bfd_get_filename(abfd));
}

/* These global variables are used to pass information between
   translate_addresses and find_address_in_section.  */

static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static int found;

/* Look for an address in a section.  This is called via
   bfd_map_over_sections.  */

static void find_address_in_section(bfd *abfd, asection *section, void *data __attribute__ ((__unused__)) )
{
	bfd_vma vma;
	bfd_size_type size;

	if (found)
		return;

	if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0)
		return;

	vma = bfd_get_section_vma(abfd, section);
	if (pc < vma)
		return;

	size = bfd_section_size(abfd, section);
	if (pc >= vma + size)
		return;

	found = bfd_find_nearest_line(abfd, section, syms, pc - vma,
				      &filename, &functionname, &line);
}

static char** translate_addresses_buf(bfd * abfd, bfd_vma *addr, int naddr)
{
	int naddr_orig = naddr;
	char b;
	int total  = 0;
	enum { Count, Print } state;
	char *buf = &b;
	int len = 0;
	char **ret_buf = NULL;
	char* alloc = 0;
	char* inter = 0;
	/* iterate over the formating twice.
	 * the first time we count how much space we need
	 * the second time we do the actual printing */
	for (state=Count; state<=Print; state++) {
	if (state == Print) {
		ret_buf = malloc(total + sizeof(char*)*naddr);
		buf = (char*)(ret_buf + naddr);
		len = total;
	}
	while (naddr) {
		if (state == Print)
			ret_buf[naddr-1] = buf;
		pc = addr[naddr-1];

		found = false;
		bfd_map_over_sections(abfd, find_address_in_section,
		(PTR) NULL);

		if (!found) {
			total += snprintf(buf, len, "[0x%llx] \?\?() \?\?:0",(long long unsigned int) addr[naddr-1]) + 1;
		} else {
			const char *name;

			name = functionname;
			if (name == NULL || *name == '\0')
				name = "??";
			else
			{
				alloc = bfd_demangle (abfd, name, (1 << 1) | (1 << 0));
				if (alloc != NULL)
					name = alloc;
				else
				{
					inter = malloc(strlen(name)+3);
					if (inter)
					{
						memset(inter, 0, strlen(name)+3);
						strcpy(inter, name);
						strcat(inter, "()");
					}
					else inter = name;
				}

			}
			if (filename != NULL) {
				char *h;

				h = strrchr(filename, '/');
				if (h != NULL)
					filename = h + 1;
			}
			total += snprintf(buf, len, "[%s(%s:%u)]: %s", abfd->filename, filename?filename:"??", line, alloc?alloc:inter) + 1;
			if (inter) free(inter);

		}
		if (state == Print) {
			/* set buf just past the end of string */
			buf = buf + total + 1;
		}
		naddr--;
	}
	naddr = naddr_orig;
	}
	return ret_buf;
}

static char **process_file(const char *file_name, bfd_vma *addr, int naddr)
{
	bfd *abfd;
	char **matching;
	char **ret_buf;
	unsigned long crc32;

	abfd = bfd_openr(file_name, "default");

	if (abfd == NULL)
		bfd_fatal(file_name);

	if (bfd_check_format(abfd, bfd_archive))
		fatal("%s: can not get addresses from archive", file_name);
	if (!bfd_check_format_matches(abfd, bfd_object, &matching)) {
		bfd_nonfatal(bfd_get_filename(abfd));
		if (bfd_get_error() ==
		    bfd_error_file_ambiguously_recognized) {
			list_matching_formats(matching);
			free(matching);
		}
		xexit(1);
	}

	slurp_symtab(abfd);

	ret_buf = translate_addresses_buf(abfd, addr, naddr);

	if (syms != NULL) {
		free(syms);
		syms = NULL;
	}

	bfd_close(abfd);
	return ret_buf;
}

#define MAX_DEPTH 16

struct file_match {
	const char *file;
	void *address;
	void *base;
	void *hdr;
};

static int find_matching_file(struct dl_phdr_info *info,
		size_t size, void *data)
{
	struct file_match *match = data;
	/* This code is modeled from Gfind_proc_info-lsb.c:callback() from libunwind */
	long n;
	const ElfW(Phdr) *phdr;
	ElfW(Addr) load_base = info->dlpi_addr;
	phdr = info->dlpi_phdr;
	for (n = info->dlpi_phnum; --n >= 0; phdr++) {
		if (phdr->p_type == PT_LOAD) {
			ElfW(Addr) vaddr = phdr->p_vaddr + load_base;
			if (match->address >= vaddr && match->address < vaddr + phdr->p_memsz) {
				/* we found a match */
				match->file = info->dlpi_name;
				match->base = info->dlpi_addr;
			}
		}
	}
	return 0;
}

char **backtrace_symbols(void *const *buffer, int size)
{
	int stack_depth = size - 1;
	int x,y;
	/* discard calling function */
	int total = 0;

	char ***locations;
	char **final;
	char *f_strings;

	locations = malloc(sizeof(char**) * (stack_depth+1));

	bfd_init();
	for(x=stack_depth, y=0; x>=0; x--, y++){
		struct file_match match = { .address = buffer[x] };
		char **ret_buf;
		bfd_vma addr;
		dl_iterate_phdr(find_matching_file, &match);
		addr = buffer[x] - match.base;
		if (match.file && strlen(match.file))
			ret_buf = process_file(match.file, &addr, 1);
		else
			ret_buf = process_file(program_name, &addr, 1);
		locations[x] = ret_buf;
		total += strlen(ret_buf[0]) + 1;
	}

	/* allocate the array of char* we are going to return and extra space for
	 * all of the strings */
	final = malloc(total + (stack_depth + 1) * sizeof(char*));
	/* get a pointer to the extra space */
	f_strings = (char*)(final + stack_depth + 1);

	/* fill in all of strings and pointers */
	for(x=stack_depth; x>=0; x--){
		strcpy(f_strings, locations[x][0]);
		free(locations[x]);
		final[x] = f_strings;
		f_strings += strlen(f_strings) + 1;
	}
	free(locations);
	return final;
}


enum _btconstants
{
	max_tracedepth = 300
};
static void* symbol_addr[max_tracedepth];
static char** symbol_str;
static size_t nsymbols;
static size_t it;

static void print_stack_trace(void)
{
	fprintf(stderr, "--stack trace start--\n");
	nsymbols = backtrace(symbol_addr, max_tracedepth);
	symbol_str = backtrace_symbols(symbol_addr, nsymbols);
	for(it = 0; it < nsymbols; ++it)
		fprintf(stderr, "%s\n", symbol_str[it]);
	fprintf(stderr, "--stack trace fini--\n");
	fflush(stderr);
	return;
}

static void signal_segv(int signum, siginfo_t* info, void* arg)
{
	fprintf(stderr, "\ncaught segmentation fault.\n");
	print_stack_trace();
	exit(1);
}

static void signal_int(int signum, siginfo_t* info, void*ptr)
{
	printf("\ncaught interrupt signal.\n");
	print_stack_trace();
	exit(1);
	return;
}

static void setup_sigsegv(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
	sigemptyset(&action.sa_mask);
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &action, NULL);
}

static void setup_sigint(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
	sigemptyset(&action.sa_mask);
    action.sa_sigaction = signal_int;
    action.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &action, NULL);
}

static void __attribute((constructor)) init(void)
{
    setup_sigsegv();
	setup_sigint();
}
