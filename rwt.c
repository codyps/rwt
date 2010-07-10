#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ben.h"

#define NAME "rwt"
static const char usage_str[] =
"usage: %s <torrent> -d <dict_key> -l <list_index>\n";


#define FN_USE(...) do {                  \
	fprintf(stderr, "%s : ", __func__); \
	fprintf(stderr, __VA_ARGS__);       \
} while(0)

static void usage(char *name)
{
	fprintf(stderr, usage_str, name);
}

int rm_fn(struct be_node *t, int argc, char **argv)
{
	return -1;
}

int add1_fn(struct be_node *t, int argc, char **argv)
{
	if (argc < 1) {
		FN_USE("need 1 tracker to add");
		return -1;
	}

	char *tracker = argv[1];

	fprintf(stderr, "ADD: '%s'\n", tracker);

	struct be_str *tr = malloc(sizeof(*tr));
	tr->len = strlen(tracker);
	tr->data = tracker;

	struct be_node *n_tr = malloc(sizeof(*n_tr));
	n_tr->type = BE_STR;
	n_tr->u.s = tr;

	struct be_node *a = be_find(t, "announce", n_tr);
	if (!a) {
		fprintf(stderr, "ADD: no mem\n");
		return -1;
	} else if (a == n_tr) {
		fprintf(stderr, "ADD: '%s' => 'announce'.\n", tracker);
		return 2;
	} else {
		fprintf(stderr, "ADD: 'announce' already used.\n");
		/* TODO: check if identical to the one we are adding? */

	}

	struct be_node *al = be_lookup(t, "announce-list");
	if (!al) {
		fprintf(stderr, "ADD: 'announce-list' not found.\n");
		return -1;
	}

	return 2;
}

static int t_show(struct be_node *tf, int argc, char **argv)
{
	if (argc > 1) {
		if (*argv[1] == '*') {
			be_print(tf, stdout);
			return 2;
		}
		if (tf->type != BE_DICT) {
			fprintf(stderr,"not dict.\n");
			return -1;
		}
		struct be_dict *d = tf->u.d;

		struct be_str s = { strlen(argv[1]), argv[1] };

		struct be_node *val = be_dict_lookup(d, &s);
		if (val) {
			be_print(val, stdout);
			return 2;
		} else {
			fprintf(stderr, "Not found: %s\n", argv[1]);
			return 2;
		}
	} else {
		be_print(tf, stdout);
		return 1;
	}
}

static int t_proc(struct be_node *t, int argc, char **argv)
{
	if (argc == 0) {
		// show current trackers.
		return 0;
	}

	for( ;argc > 0;) {
		char *cur_opt = *argv;
		if ( *cur_opt != '-' || *cur_opt == '\0') {
			fprintf(stderr, "Unexpected argument '%s'\n.",
					cur_opt);
			return 1;
		}

		int ret;
		switch(cur_opt[1]) {
		case 'a':
			ret = add1_fn(t, argc, argv);
			break;
		case 'r':
			ret = rm_fn(t, argc, argv);
			break;
		case 'p':
			ret = t_show(t, argc, argv);
		}

		if (ret > 0) {
			argc -= ret;
			argv += ret;
		} else if (!ret) {
			argc -= 1;
			argv += 1;
		} else { /* ret < 0 */
			fprintf(stderr, "Bad option '%c'.\n", *cur_opt);
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		if (argc < 1)
			usage(NAME);
		else
			usage(argv[0]);
		return 1;
	}

	char *torrent = argv[1];
	FILE *tf = fopen(torrent, "r+b");
	if (!tf) {
		fprintf(stderr, "torrent \"%s\": fopen: %s\n", torrent, 
			strerror(errno));
		return 2;
	}

	/* FIXME: this is generally a Bad Thing.
	 * Should process the FILE * directly. 
	 */
	fseek(tf, 0, SEEK_END);
	long tf_sz = ftell(tf);
	char *tf_t = malloc(tf_sz);
	fseek(tf, 0, SEEK_SET);
	size_t read = fread(tf_t, tf_sz, 1, tf);
	if (read != 1) {
		fprintf(stderr, "Failed to read entire file, %zi.\n", read);
		return 1;
	}

	const char *ep;
	struct be_node *tf_be = bdecode(tf_t, tf_sz, &ep);

	//char *spec = argv[2];
	return t_proc(tf_be, argc-2, argv+2);
}
