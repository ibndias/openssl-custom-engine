# openssl-custom-engine
Custom engine for openssl

Use `make` to generate engine (md5-engine.so) and executable `md5test`

# Install engine:
`sudo make install` will install the engine into `/usr/lib/x86_64-linux-gnu/engines-1.1/`, make sure this is your OpenSSL engine directory.

# Usage Example
Run `./md5test whatever`, this will output the digest of `whateve` instead of `whatever` because we modified the md5 engine to omit the last char.
