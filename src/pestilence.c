#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>

#include "pestilence.h"
#include "utils.h"
#include "death.h"
#include "syscall.h"

/* open /proc/ directory and check if the process in char forbidden[] is running 
 * example /proc/1/comm
 */

void junk_pestilence(void) {
	char a;
	char b;
	char c;

	a = 1; 
	b = 2;
	c = 3;

	for (int i = 0; i < 1; i++) {
		a += b;
		b += c;
		c += a;
		if (a > b) {
			a = b;
		} else if (b > c) {
			b = c;
		} else if (c > a) {
			c = a;
		}
	}

}

/* removed git cause of ohmyzsh conflitcs (plugin) */

static int check_proc(const char *dir_path) {

	const char *forbidden[] = {
		STR("hexdump"),
		STR("test"),
		(void *)0
	}; JUNK;

	char file[PATH_MAX];

	int fd = open(dir_path, O_RDONLY);
	if (fd == -1) {
		return 1; JUNK;
	}

	ssize_t ret = read(fd, file, PATH_MAX);
	if (ret == -1) {
		close(fd);
		return 1;
	}

	file[ret] = '\0'; JUNK;

	for (size_t i = 0; forbidden[i]; ++i) {
		if (ft_memcmp(file, forbidden[i], ft_strlen(forbidden[i])) == 0) {
			close(fd);
			return 1;
		}
	}

	close(fd);

	return 0;
}

static int check_digit(const char *str) {
	for (size_t i = 0; str[i]; i++) {
		if (str[i] < '0' || str[i] > '9')
			return 1;
	}
	JUNK;
	return 0;
}

static int forbid_proc(void)
{
	const char proc[] = "/proc";

	int fd = open(proc, O_RDONLY); JUNK;

	if (fd == -1)
		return 1;


	char buf[PATH_MAX];
	struct dirent *dir;
	ssize_t ret; JUNK;

	for(;;)
	{
		ret = getdents64(fd, buf, PATH_MAX);
		if (ret <= 0)
			break;
		for (ssize_t i = 0; i < ret; i += dir->d_reclen)
		{
			dir = (struct dirent *)(buf + i); JUNK;

			if (dir->d_name[0] == '.'
				&& (dir->d_name[1] == '\0' || (dir->d_name[1] == '.' && dir->d_name[2] == '\0')))
				continue;

			if (dir->d_type == DT_DIR && check_digit(dir->d_name) == 0) {

				char new_path[NAME_MAX];

				char comm[] = "/comm";
				char slash[] = "/"; JUNK;

				char *ptr = new_path;
				ptr = ft_stpncpy(ptr, proc, NAME_MAX);
				ptr = ft_stpncpy(ptr, slash, NAME_MAX - (ptr - new_path));
				ptr = ft_stpncpy(ptr, dir->d_name, NAME_MAX - (ptr - new_path));
				ptr = ft_stpncpy(ptr, comm, NAME_MAX - (ptr - new_path));

				if (check_proc(new_path) != 0) {
					close(fd);
					return 1;
				}
			}
		}
	}

	close(fd);

	return 0;
}

static int is_debugged(void) {

	int res = 0;
	int fd = open(STR("/proc/self/status"), O_RDONLY); JUNK;

	if (fd == -1)
		return 1;

	char buf[4096];

	ssize_t ret = read(fd, buf, 4096);
	if (ret == -1) 
		return 1;
	buf[ret] = '\0'; JUNK;

	char *ptr = ft_memmem(buf, ret, STR("TracerPid:"), 10);
	if (ptr != 0) {
		ptr += 10; // skip "TracerPid:"
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr != '0') {
			res = 1;
		}
	} JUNK;

	close(fd);
	return res;
}

int pestilence(void)
{
	if (is_debugged() != 0 || forbid_proc() != 0)
		return 1;
	JUNK;
	return 0;
}

/* this function is not used anymore, it can cause bugs when using forks (tracing childs) */

/*
static int	is_debugged(void)
{
	if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1)
		return (1);
	return (0);
}
*/
