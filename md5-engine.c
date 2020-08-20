#include "openssl/engine.h"
#include "openssl/evp.h"
#include "rfc1321/global.h"
#include "rfc1321/md5.h"

#define MD5_DIGEST_LENGTH 16
#define MD5_CBLOCK 64

static const char *engine_id = "MD5";
static const char *engine_name = "A simple md5 engine for demonstration purposes";

static int supported_digests[] = {NID_md5, 0};

static EVP_MD *md5_digest = NULL;

static int md5_init(EVP_MD_CTX *ctx) {
	MD5Init((MD5_CTX*)EVP_MD_CTX_md_data(ctx));

	return 1;
}

static int md5_update(EVP_MD_CTX *ctx, const void *data, size_t count) {
	MD5Update((MD5_CTX*)EVP_MD_CTX_md_data(ctx), data, count-1);
	return 1;
}

static int md5_final(EVP_MD_CTX *ctx, unsigned char *md) {
	MD5Final(md, (MD5_CTX*)EVP_MD_CTX_md_data(ctx));
	return 1;
}

int get_digest_info(ENGINE *e, const EVP_MD **digest, const int **nids, int nid) {
	if (nids == NULL) {
		if (nid != NID_md5) {
			return -1;
		}
		if (md5_digest == NULL) {
			EVP_MD *md;
			md = EVP_MD_meth_new(NID_md5, NID_md5WithRSAEncryption);
			EVP_MD_meth_set_result_size(md, MD5_DIGEST_LENGTH);
			EVP_MD_meth_set_input_blocksize(md, MD5_CBLOCK);
			EVP_MD_meth_set_app_datasize(md, sizeof(EVP_MD*) + sizeof(MD5_CTX));
			EVP_MD_meth_set_flags(md, 0);
			EVP_MD_meth_set_init(md, md5_init);
			EVP_MD_meth_set_update(md, md5_update);
			EVP_MD_meth_set_final(md, md5_final);
			md5_digest = md;
		}
		*digest = md5_digest;
		return 1;
	} else {
		*nids = supported_digests;
		return 1;
	}
}

static int engine_destroy(ENGINE *e) {
    EVP_MD_meth_free(md5_digest);
    md5_digest = NULL;
    return 1;
}

static int engine_init(ENGINE *e) {
    return 1;
}


static int engine_finish(ENGINE *e) {
    return 1;
}

static int bind(ENGINE *e, const char *id) {
    int ret = 0;

    if (!ENGINE_set_id(e, engine_id)) {
        fprintf(stderr, "ENGINE_set_id failed\n");
        goto end;
    }
    if (!ENGINE_set_name(e, engine_name)) {
        fprintf(stderr, "ENGINE_set_name failed\n");
        goto end;
    }

    if (!ENGINE_register_digests(e)) {
    	fprintf(stderr, "ENGINE_register_digests failed\n");
    	goto end;
    }

    if (!ENGINE_set_digests(e, get_digest_info)) {
    	fprintf(stderr, "ENGINE_set_digests failed\n");
    	goto end;
    }

    if (!ENGINE_set_destroy_function(e, engine_destroy)) {
    	fprintf(stderr, "ENGINE_set_destroy_function failed\n");
    	goto end;
    }

    if (!ENGINE_set_init_function(e, engine_init)) {
    	fprintf(stderr, "ENGINE_set_init_function failed\n");
    	goto end;
    }

    if (!ENGINE_set_finish_function(e, engine_finish)) {
    	fprintf(stderr, "ENGINE_set_finish_function failed\n");
    	goto end;
    }

    ret = 1;
    end:
    return ret;
}

IMPLEMENT_DYNAMIC_BIND_FN(bind)
IMPLEMENT_DYNAMIC_CHECK_FN()
