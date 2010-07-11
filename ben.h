#ifndef BEN_H_
#define BEN_H_
#include <stdlib.h> /* size_t */

enum be_type {
	BE_STR,
	BE_DICT,
	BE_INT,
	BE_LIST
};

struct be_str {
	size_t len;
	char *data;
};

struct be_node;
struct be_dict {
	size_t len;
	struct be_str **keys;
	struct be_node **vals;
};

struct be_list {
	size_t len;
	struct be_node **nodes;
};

struct be_node {
	enum be_type type;
	union {
		long long i;
		struct be_list *l;
		struct be_dict *d;
		struct be_str *s;
	} u;
};


/* be_dict_find wrapper.
 */
struct be_node *be_find(struct be_node *n,
		char *key, struct be_node *val);

/* be_dict_lookup wrapper.
 * NOTE: str is non-const due to data structure limitaions
 */ 
struct be_node *be_lookup(const struct be_node *n, char *str);

int be_str_cmp(const struct be_str *a1, const struct be_str *a2);

/** be_list_find_str
 * If str exsists in list:
 * 	return str that is in list.
 * Else, insert str into list:
 * 	return newly inserted str.
 * On error, return 0.
 */
struct be_node *be_list_find_str(struct be_list *list,
		struct be_str *str);

/**
 * Look up key in dict.
 * If found:
 * 	return a pointer to the val the already exsisting key
 * 	is paired with.
 * Else if not found:
 * 	If val is non-null, insert the key:val pair and return
 * 		a pointer to the inserted val
 * 	Otherwise return 0.
 *
 * On error: returns 0.
 */
struct be_node *be_dict_find(struct be_dict *dict, 
		struct be_str *key, struct be_node *val);
struct be_node *be_dict_lookup(const struct be_dict *dict, 
		const struct be_str *key);
struct be_node *bdecode(const char *estr, size_t len, const char **ep);

/* Writes pretty printed data to out */
void be_print(struct be_node *be, FILE *out);

/* Writes bencoded data to out */
void be_write(struct be_node *be, FILE *out);

#endif /* BEN_H_ */
