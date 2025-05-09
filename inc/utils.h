#ifndef UTILS_H
# define UTILS_H

# include <stdint.h>
# include <sys/types.h>

typedef struct data_s data_t;

#define DEFAULT_KEY /*0xdeadbeef*/ 0x9e3779b97f4a7c15
/* print */
void	putnbr(size_t n);
void	print_key(uint8_t *key, size_t size);

/* len */
int		ft_strlen(const char *s);
int		ft_strnlen(const char *s, size_t n);

/* mem functions */
void	ft_memmove(void *dst, const void *src, size_t size);
int		ft_memcmp(const void *s1, const void *s2, size_t size);
void	*ft_memset(void *b, int c, size_t len);
void	*ft_memcpy(void *dst, const void *src, size_t size);
void	*ft_memmem(const void *haystack, size_t haystack_len, const void *needle, size_t needle_len);
void	*ft_mempcpy(void *dst, const void *src, size_t size);

/* str functions */
char *ft_stpncpy(char *dst, const char *src, size_t sz);
char *ft_strncat(char *dst, const char *src, size_t sz);
char *ft_strncpy(char *dst, const char *src, size_t sz);
int	ft_strcmp(const char *s1, const char *s2);
int	ft_strncmp(const char *s1, const char *s2, size_t n);
char *ft_strchr(const char *s, int c);
char *ft_strrchr(const char *s, int c);

/* rest of the functions */
void	*search_signature(data_t *data, const char *key);

int	_printf(char *fmt, ...);

int64_t	gen_key_64(void);

void	encrypt(uint8_t *data, const size_t size, uint8_t *key);
void	decrypt(uint8_t *data, const size_t size, int64_t key);
void	encrypt_offset(uint8_t *data, const size_t size, uint8_t *key, size_t offset);

#endif
