# openssl-custom-engine
Custom MD5 engine for OpenSSL 1.1.1 example.

Use `make` to generate engine file `md5-engine.so` and `md5test` executable.

# Installing The Engine

## Check OpenSSL Engine Directory
```sh
$ openssl version -a
OpenSSL 1.1.1f  31 Mar 2020
built on: Mon Apr 20 11:53:50 2020 UTC
platform: debian-amd64
options:  bn(64,64) rc4(16x,int) des(int) blowfish(ptr) 
compiler: gcc -fPIC -pthread -m64 -Wa,--noexecstack -Wall -Wa,--noexecstack -g -O2 -fdebug-prefix-map=/build/openssl-P_ODHM/openssl-1.1.1f=. -fstack-protector-strong -Wformat -Werror=format-security -DOPENSSL_TLS_SECURITY_LEVEL=2 -DOPENSSL_USE_NODELETE -DL_ENDIAN -DOPENSSL_PIC -DOPENSSL_CPUID_OBJ -DOPENSSL_IA32_SSE2 -DOPENSSL_BN_ASM_MONT -DOPENSSL_BN_ASM_MONT5 -DOPENSSL_BN_ASM_GF2m -DSHA1_ASM -DSHA256_ASM -DSHA512_ASM -DKECCAK1600_ASM -DRC4_ASM -DMD5_ASM -DAESNI_ASM -DVPAES_ASM -DGHASH_ASM -DECP_NISTZ256_ASM -DX25519_ASM -DPOLY1305_ASM -DNDEBUG -Wdate-time -D_FORTIFY_SOURCE=2
OPENSSLDIR: "/usr/lib/ssl"
ENGINESDIR: "/usr/lib/x86_64-linux-gnu/engines-1.1"
Seeding source: os-specific
```
If your `ENGINESDIR` is different, change it in the `Makefile`.

## Install
```sh
$ sudo make install
```
This will install the engine into `/usr/lib/x86_64-linux-gnu/engines-1.1/`, make sure this is your OpenSSL engine directory.

# How it Works
- Take the md5 algo from rfc1321 (https://www.ietf.org/rfc/rfc1321.txt), extract three files and put in `rfc1321/` folder.
  - global.h -- global header file
  - md5.h -- header file for MD5
  - md5c.c -- source code for MD5
- Write engine implementation in `md5-engine.c`
  - Insight tutorial: https://www.openssl.org/blog/blog/2015/11/23/engine-building-lesson-2-an-example-md5-engine/
  - Updated version by hawell: https://gist.github.com/hawell/038fefff5a2614683824b7737419192e
  - We modified the `md5_update()` function and decrement `count` by `1` to make a difference between the custom and original one.
- Test our custom engine in `md5test.c`.
  - First we need to load our engine, 
  ```c
  const char *engine_id = "md5-engine";
  ENGINE_load_builtin_engines();
  e = ENGINE_by_id(engine_id);
  if (!e)
  { /* the engine isn't available */
    printf("Engine not available!\n");
    return 0;
  }
  if (!ENGINE_init(e))
  { /* the engine couldn't initialise, release 'e' */
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
  ```
  More details, see: https://www.openssl.org/docs/man1.1.1/man3/ENGINE_add.html)
  - Then we use high-level envelope API `md = EVP_get_digestbyname("MD5");` in order to get the digest name.
  - Create a new message digest context, digest the message, then print the output string.
  ```c
  mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, argv[1], strlen(argv[1]));
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  EVP_MD_CTX_free(mdctx);

  printf("Digest is: ");
  for (i = 0; i < md_len; i++)
    printf("%02x", md_value[i]);
  printf("\n");
  ```
  -Don't forget to release the engine reference
  ```c
  /* Release the functional reference from ENGINE_init() */
  ENGINE_finish(e);
  /* Release the structural reference from ENGINE_by_id() */
  ENGINE_free(e);
  ```
# Usage Example
Run `./md5test whatever`, this will output the digest of `whateve` instead of `whatever` because we modified the md5 engine to omit the last char.

# License 
GNU General Public License v3.0
