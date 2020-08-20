#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/engine.h>

int main(int argc, char *argv[])
{
       EVP_MD_CTX *mdctx;
       const EVP_MD *md;
       //char mess1[] = "whatever\n";
       //char mess2[] = "Hello World\n";
       unsigned char md_value[EVP_MAX_MD_SIZE];
       unsigned int md_len, i;

       ENGINE *e;
       const char *engine_id = "md5-engine";
       ENGINE_load_builtin_engines();
       e = ENGINE_by_id(engine_id);
       if (!e)
       { /* the engine isn't available */
              printf("Engine not available!\n");
              return 0;
       }
       if (!ENGINE_init(e))
       {
              /* the engine couldn't initialise, release 'e' */
              printf("Engine cannot be initialized!\n");
              ENGINE_free(e);
              return 0;
       }
       printf("Engine FOUND!\n");
       
       if (!ENGINE_set_default_digests(e))
       { /* This should only happen when 'e' can't initialise, but the previous
      * statement suggests it did. */
              abort();
       }
       

       if (argv[1] == NULL)
       {
              printf("Usage: mdtest <input_str>\n");
              exit(1);
       }

       md = EVP_get_digestbyname("MD5");

       mdctx = EVP_MD_CTX_new();
       EVP_DigestInit_ex(mdctx, md, NULL);
       EVP_DigestUpdate(mdctx, argv[1], strlen(argv[1]));
       //EVP_DigestUpdate(mdctx, mess2, strlen(mess2));
       EVP_DigestFinal_ex(mdctx, md_value, &md_len);
       EVP_MD_CTX_free(mdctx);

       printf("Digest is: ");
       for (i = 0; i < md_len; i++)
              printf("%02x", md_value[i]);
       printf("\n");



       /* Release the functional reference from ENGINE_init() */
       ENGINE_finish(e);
       /* Release the structural reference from ENGINE_by_id() */
       ENGINE_free(e);
       
       exit(0);
}