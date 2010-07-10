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

/** Dict convinience functions **/
struct be_node *be_find(struct be_node *n,
		const char *key, const struct be_node *val);

/* NOTE: str is non-const due to data structure limitaions */
struct be_node *be_lookup(const struct be_node *n, char *str);

/** Direct interface **/
struct be_node *be_dict_find(struct be_dict *dict, 
		const struct be_str *key, const struct be_node *val);
struct be_node *be_dict_lookup(const struct be_dict *dict, 
		const struct be_str *key);
struct be_node *bdecode(const char *estr, size_t len, const char **ep);
void be_print(struct be_node *be, FILE *out);

#endif /* BEN_H_ */
