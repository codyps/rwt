#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ben.h"

#define NAME "rwt"
static const char usage_str[] =
"usage: %s <torrent> [options]\n"
"options:\n"
"   -p dict_key     print torrent\n"
"   -a tracker      add tracker\n"
"   -d tracker      del tracker\n"
"   -w[mode]        write torrent to a file\n"
"       modes:\n"
"           f filename    to filename\n"
"           -             to stdout\n"
"              (blank)    to original torrent\n";


#define FN_USE(...) do {                  \
	fprintf(stderr, "%s : ", __func__); \
	fprintf(stderr, __VA_ARGS__);       \
} while(0)

static void usage(char *name)
{
	fprintf(stderr, usage_str, name);
}

int del_fn(struct be_node *t, int argc, char **argv)
{
	if (argc < 1) {
		FN_USE("need 1 tracker to delete");
		return -1;
	}

	char *tracker = argv[1];

	fprintf(stderr, "DEL: '%s'\n", tracker);

	struct be_str *tr = malloc(sizeof(*tr));
	tr->len = strlen(tracker);
	tr->data = tracker;

	struct be_node *n_tr = malloc(sizeof(*n_tr));
	n_tr->type = BE_STR;
	n_tr->u.s = tr;


	struct be_kv_pair *ap = be_lookup(t, "announce");
	if (!ap) {
		fprintf(stderr, "DEL: 'announce' non-exsistent, skipping.\n");
	} else {
		struct be_node *a = ap->val;
		fprintf(stderr, "DEL: 'announce' already used.\n");
		if (a->type == BE_STR) {
			if (!be_str_cmp(a->u.s, tr)) {
				/* TODO REMOVE IT */

			}
		} else {
			fprintf(stderr, "WARN: 'announce' is not a str\n");
		}
	}

	struct be_kv_pair *al_p = be_lookup(t, "announce-list");
	if (!al_p) {
		fprintf(stderr, "DEL: 'announce-list' non-exsistent.\n");
		fprintf(stderr, "DEL: nothing to remove.\n");
		goto done_succ;
	}
	struct be_node *al_n = al_p->val;
	fprintf(stderr, "DEL: 'announce-list' already exsists.\n");

	if (al_n->type != BE_LIST) {
		fprintf(stderr, "ERR: 'announce-list' not a list.\n");
		goto done_fail;
	}

	struct be_list *al = al_n->u.l;
	struct be_node *ag_n;
	for_each_list_node(al, ag_n) {
	
		if (ag_n->type != BE_LIST) {
			fprintf(stderr, 
				"WARN: announce group %zu is not a list.\n",
				list_node_index(al, ag_n));
			continue;
		}

		struct be_list *ag = ag_n->u.l;
		struct be_node *c_tr_n;
		for_each_list_node(ag, c_tr_n) {
			if (c_tr_n->type != BE_STR) {
				fprintf(stderr,
					"WARN: tracker %zu:%zu is not a"
					"str.\n",
					list_node_index(al, ag_n),
					list_node_index(ag, c_tr_n));
				continue;
			}

			struct be_str *c_tr = c_tr_n->u.s;


			/* TODO: operate on c_tr */
			if (!be_str_cmp(c_tr, tr)) {
				//
			}
		}
	}

done_succ:
	return 2;

done_fail:
	fprintf(stderr, "WARN: failed to find tracker '%s' for removal.\n",
			tracker);
	return 2;
}

#if 0
static int set_announce(struct be_node *t, char *tracker, size_t t_len)
{
	struct be_str strack = { t_len, tracker };
	struct be_node strackn = { BE_STR, strack };
	return 0
}
#endif

static int add1_fn(struct be_node *t, int argc, char **argv)
{
	if (argc < 1) {
		FN_USE("need 1 tracker to add");
		return -1;
	}

	char *tracker = argv[1];

	fprintf(stderr, "ADD: '%s'\n", tracker);

	struct be_str *tr_raw = malloc(sizeof(*tr_raw));
	tr_raw->len = strlen(tracker);
	tr_raw->data = malloc(tr_raw->len);
	memcpy(tr_raw->data, tracker, tr_raw->len);

	struct be_node *tr_node = malloc(sizeof(*tr_node));
	tr_node->type = BE_STR;
	tr_node->u.s = tr_raw;

	struct be_node *a = be_find_insert(t, "announce", tr_node);
	if (!a) {
		be_free(tr_node);
		fprintf(stderr, "ERR: %s\n", strerror(errno));
		return -1;
	} else if (a == tr_node) {
		fprintf(stderr, "ADD: '%s' => 'announce'.\n", tracker);
		be_free(tr_node);
		return 2;
	} else {
		fprintf(stderr, "ADD: 'announce' already used.\n");
	}

	struct be_list *new_al = malloc(sizeof(*new_al));
	new_al->len = 0;
	new_al->nodes = 0;

	struct be_node *new_al_n = malloc(sizeof(*new_al_n));
	new_al_n->type = BE_LIST;
	new_al_n->u.l = new_al;

	struct be_node *al_n = be_find_insert(t, "announce-list", new_al_n);
	if (!al_n) {
		fprintf(stderr, "ERR: %s\n", strerror(errno));
		be_free(tr_node);
		free(new_al);
		free(new_al_n);
		return -1;
	} else if (al_n == new_al_n) {
		fprintf(stderr, "ADD: created 'announce-list'.\n");
		/* FIXME: this is not currently handled properly. */
	} else {
		fprintf(stderr, "ADD: 'announce-list' already exsists.\n");
		free(new_al_n);
		/* new_al is reused below */
	}

	if (al_n->type != BE_LIST) {
		fprintf(stderr, "ERR: 'announce-list' not a list.\n");
		be_free(tr_node);
		free(new_al_n);
		free(new_al);
		/* TODO: remove non list and add list on force tag? */
		return -1;
	}

	struct be_list *al = al_n->u.l;

	al->len ++;
	al->nodes = realloc(al->nodes, al->len * sizeof(*al->nodes));

	new_al->len = 1;
	new_al->nodes = malloc(sizeof(*new_al->nodes));
	new_al->nodes[0] = *tr_node;
	free(tr_node);

	al->nodes[al->len - 1].type = BE_LIST;
	al->nodes[al->len - 1].u.l  = new_al;

	return 2;
}

static int write_be_file(struct be_node *n, char *fn)
{
	FILE *fp = fopen(fn, "w");
	if (!fp) {
		perror(0);
		return -1;
	}

	be_write(n, fp);

	fclose(fp);

	return 2;
}

static int write_be(struct be_node *tf, char *tn, int argc, char **argv)
{
	char opt = argv[0][2];
	switch(opt) {
	case '-':
		be_write(tf, stdout);
		break;
	case 'f':
		if (argc < 2) {
			FN_USE("option -wf requires filename arg\n");
			return -1;
		}
		if (write_be_file(tf, argv[1]) >= 0)
			return 2;
		else
			return -1;
	case '\0':
		if (write_be_file(tf, tn) >= 0)
			return 2;
		else
			return -1;
	default:
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

		struct be_kv_pair *pair = be_dict_lookup(d, &s);
		struct be_node *val = pair->val;
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

static int t_proc(struct be_node *t, char *tn, int argc, char **argv)
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
			ret = del_fn(t, argc, argv);
			break;
		case 'p':
			ret = t_show(t, argc, argv);
			break;
		case 'w':
			ret = write_be(t, tn, argc, argv);
			break;
		default:
			fprintf(stderr, "Bad option '%c'.\n", *cur_opt);
			break;
		}

		if (ret > 0) {
			argc -= ret;
			argv += ret;
		} else if (!ret) {
			argc -= 1;
			argv += 1;
		} else { /* ret < 0 */
			fprintf(stderr, "'%c' killed us.\n", *cur_opt);
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
	fclose(tf);
	const char *ep;
	struct be_node *tf_be = bdecode(tf_t, tf_sz, &ep);

	//char *spec = argv[2];
	int ret = t_proc(tf_be, torrent, argc-2, argv+2);
	free(tf_t);
	be_free(tf_be);
	return ret;
}
