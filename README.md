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
Find `/usr/lib/x86_64-linux-gnu/engines-1.1/md5-engine.so` and change it into `<your-engines-directory>/md5-engine.so`

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

## Compare Our Engine with Default
Here, we try to compare our md5 engine with original openssl md5 digest.
[![asciicast](https://asciinema.org/a/354475.svg)](https://asciinema.org/a/354475)

## Set Our Engine as Default
This one is when we set our engine as default md5 digest for OpenSSL.
[![asciicast](https://asciinema.org/a/354487.svg)](https://asciinema.org/a/354487)

# How to Set Custom Engine as Default for OpenSSL
To use the engine by default, configure the engine in `openssl.cnf`.
See [openssl-config(5)](https://www.openssl.org/docs/man1.1.1/man5/config.html)
for details.

Include the following definition in the default section--the first
section before any other bracketed section header:

    openssl_conf=openssl_conf

This section, which contains the global openssl defaults, should include an
engines section for engine configuration:

    [openssl_conf]
    engines=engines

The engines section will have a list of engines to enable, pointing to that
engine's configuration section:

    [engines]
    md5-engine=md5-engine

Now, in the `md5-engine` section, we can configure the engine itself.  The
`default_algorithms` option is only used to enable the engine.  The selection
of ciphers and digests to enable is different:

    [md5-engine]
    default_algorithms=ALL

To test the configuration, run the following command:

    openssl engine -t -c -v

It should display the engine as available, along with the list of algorithms
enabled and the configuration commands accepted by the engine:
```
(rdrand) Intel RDRAND engine
 [RAND]
     [ available ]
(dynamic) Dynamic engine loading support
     [ unavailable ]
     SO_PATH, NO_VCHECK, ID, LIST_ADD, DIR_LOAD, DIR_ADD, LOAD
(MD5) A simple md5 engine for demonstration purposes
 [MD5]
     [ available ]
```
Now the OpenSSL will use our MD5 engine as default.

# License 
GNU General Public License v3.0
