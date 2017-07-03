#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define min(a, b)    ((a < b) ? (a) : (b))
#define max(a, b)    ((a > b) ? (a) : (b))

#if 0 //DEBUG
#define dprintf    printf
#else
#define dprintf    no_printf
#endif

void usage(int argc, char *argv[])
{
	fprintf(stderr, "usage: \n"
		"%s pid address size\n"
		"\n"
		"    The virt2phys_usr resolve and converts to \n"
		"    physical address from virtual address of \n"
		"    process user space.\n"
		"\n"
		"    pid    : PID of process that has specified address.\n"
		"    address: Virtual address to be resolved.\n"
		"    size   : Size to be resolved.\n"
		"\n",
		argv[0]);
}

int no_printf(const char *format, ...)
{
	//do nothing
	return 0;
}

ssize_t readn(int fd, void *buf, size_t count)
{
	char *cbuf = buf;
	ssize_t nr, n = 0;

	while (n < count) {
		nr = read(fd, &cbuf[n], count - n);
		if (nr == 0) {
			//EOF
			break;
		} else if (nr == -1) {
			if (errno == -EINTR) {
				//retry
				continue;
			} else {
				//error
				return -1;
			}
		}
		n += nr;
	}

	return n;
}

/**
 * @return -1: error, -2: not present, other: physical address
 */
uint64_t virt_to_phys(int fd, int pid, uint64_t virtaddr)
{
	int pagesize;
	uint64_t tbloff, tblen, pageaddr, physaddr;
	off_t offset;
	ssize_t nr;

	uint64_t tbl_present;
	uint64_t tbl_swapped;
	uint64_t tbl_shared;
	uint64_t tbl_pte_dirty;
	uint64_t tbl_swap_offset;
	uint64_t tbl_swap_type;

	//1PAGE = typically 4KB, 1entry = 8bytes
	pagesize = (int)sysconf(_SC_PAGESIZE);
	//see: linux/Documentation/vm/pagemap.txt
	tbloff = virtaddr / pagesize * sizeof(uint64_t);
	dprintf("pagesize:%d, virt:0x%08llx, tblent:0x%08llx\n",
		pagesize, (long long)virtaddr, (long long)tbloff);

	offset = lseek(fd, tbloff, SEEK_SET);
	if (offset == (off_t)-1) {
		perror("lseek");
		return -1;
	}
	if (offset != tbloff) {
		fprintf(stderr, "Cannot found virt:0x%08llx, "
			"tblent:0x%08llx, returned offset:0x%08llx.\n",
			(long long)virtaddr, (long long)tbloff,
			(long long)offset);
		return -1;
	}

	nr = readn(fd, &tblen, sizeof(uint64_t));
	if (nr == -1 || nr < sizeof(uint64_t)) {
		fprintf(stderr, "Cannot found virt:0x%08llx, "
			"tblent:0x%08llx, returned offset:0x%08llx, "
			"returned size:0x%08x.\n",
			(long long)virtaddr, (long long)tbloff,
			(long long)offset, (int)nr);
		return -1;
	}

	tbl_present   = (tblen >> 63) & 0x1;
	tbl_swapped   = (tblen >> 62) & 0x1;
	tbl_shared    = (tblen >> 61) & 0x1;
	tbl_pte_dirty = (tblen >> 55) & 0x1;
	if (!tbl_swapped) {
		tbl_swap_offset = (tblen >> 0) & 0x7fffffffffffffULL;
	} else {
		tbl_swap_offset = (tblen >> 5) & 0x3ffffffffffffULL;
		tbl_swap_type = (tblen >> 0) & 0x1f;
	}
	dprintf("tblen: \n"
			"  [63   ] present    :%d\n"
			"  [62   ] swapped    :%d\n"
			"  [61   ] shared     :%d\n"
			"  [55   ] pte dirty  :%d\n",
			(int)tbl_present,
			(int)tbl_swapped,
			(int)tbl_shared,
			(int)tbl_pte_dirty);
	if (!tbl_swapped) {
		dprintf("  [ 0-54] PFN        :0x%08llx\n",
			(long long)tbl_swap_offset);
	} else {
		dprintf("  [ 5-54] swap offset:0x%08llx\n"
			"  [ 0- 4] swap type  :%d\n",
			(long long)tbl_swap_offset,
			(int)tbl_swap_type);
	}
	pageaddr = tbl_swap_offset * pagesize;
	physaddr = (uint64_t)pageaddr | (virtaddr & (pagesize - 1));
	dprintf("page:0x%08llx, phys:0x%08llx\n",
		(long long)pageaddr, (long long)physaddr);
	dprintf("\n");

	if (tbl_present) {
		dprintf("pid:%6d, virt:0x%08llx, phys:0x%08llx\n",
			pid, (long long)virtaddr, (long long)physaddr);
		return physaddr;
	} else {
		dprintf("pid:%6d, virt:0x%08llx, phys:%s\n",
			pid, (long long)virtaddr, "(not present)");
		return -2;
	}
}

int main(int argc, char *argv[])
{
	const char *arg_pid, *arg_addr, *arg_area;
	char procname[1024] = "";
	int pid, fd = -1, pagesize;
	uint64_t virtaddr, areasize, physaddr, v;
	int result = -1;

	if (argc < 4) {
		usage(argc, argv);
		return -1;
	}
	arg_pid = argv[1];
	arg_addr = argv[2];
	arg_area = argv[3];

	pid = (int)strtoll(arg_pid, NULL, 0);
	virtaddr = strtoll(arg_addr, NULL, 0);
	areasize = strtoll(arg_area, NULL, 0);
	dprintf("pid:%d, virt:0x%08llx\n", pid, (long long)virtaddr);

	memset(procname, 0, sizeof(procname));
	snprintf(procname, sizeof(procname) - 1,
		"/proc/%d/pagemap", pid);
	fd = open(procname, O_RDONLY);
	if (fd == -1) {
		perror("open");
		goto err_out;
	}

	pagesize = (int)sysconf(_SC_PAGESIZE);
	virtaddr &= ~(pagesize - 1);
	printf("pid:%6d:\n", pid);
	for (v = virtaddr; v < virtaddr + areasize; v += pagesize) {
		physaddr = virt_to_phys(fd, pid, v);
		//show result
		if (physaddr == -1) {
			printf(" virt:0x%08llx, (%s)\n",
				(long long)v, "not valid virtual address");
			break;
		} else if (physaddr == -2) {
			printf(" virt:0x%08llx, phys:(%s)\n",
				(long long)v, "not present");
		} else {
			printf(" virt:0x%08llx, phys:0x%08llx\n",
				(long long)v, (long long)physaddr);
		}
	}

	result = 0;

err_out:
	if (fd != -1) {
		close(fd);
		fd = -1;
	}

	return result;
}

