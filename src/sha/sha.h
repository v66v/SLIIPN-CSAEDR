#pragma once
#include <openssl/evp.h>
#include <openssl/sha.h>

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    EVP_MD_CTX *mdctx;
    unsigned char hash_int[SHA256_DIGEST_LENGTH];
    unsigned int hash_int_len;
  } sha_ctx_t;

  int sha_hash (sha_ctx_t *sha_ctx, const unsigned char *input,
                size_t input_len, char *hash,
                int hash_len); // sizeof(hash) needs to be +2 > hash_len

  void
  sha_snprintf (unsigned char *hash_int, size_t hash_int_len, char *hash,
                size_t hash_len); // sizeof(hash) needs to be +2 > hash_len
  int init_md_ctx (sha_ctx_t *sha_ctx);
  void deinit_md_ctx (sha_ctx_t *sha_ctx);

  int digest_message (EVP_MD_CTX *mdctx, const unsigned char *message,
                      size_t message_len, unsigned char *digest,
                      unsigned int *digest_len);
  void digest_display (unsigned char *digest, unsigned int digest_len);

#if defined(__cplusplus)
}
#endif
