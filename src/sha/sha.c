#include "sha.h"
#include <std/std-dbg.h>
#include <std/std-str.h>

int
sha_hash (sha_ctx_t *sha_ctx, const unsigned char *input, size_t input_len,
          char *hash,
          int hash_len) // sizeof(hash) needs to be +2 > hash_len
{
  unsigned char hash_int[SHA256_DIGEST_LENGTH] = { 0 };
  unsigned int hash_int_len = 0;

  if (init_md_ctx (sha_ctx))
    return 1;

  if (digest_message (sha_ctx->mdctx, input, input_len,
                      (unsigned char *)hash_int, &hash_int_len))
    return 1;

  deinit_md_ctx (sha_ctx);
  sha_snprintf (hash_int, hash_int_len, hash, hash_len);
  return 0;
}

void
sha_snprintf (unsigned char *hash_int, size_t hash_int_len, char *hash,
              size_t hash_len) // sizeof(hash) needs to be +2 > hash_len
{
  for (size_t i = 0; i < 1 + hash_len / 2 && i < hash_int_len; i++)
    {
      // writes 3 bytes
      snprintf (hash + i * 2, 3, "%02x", hash_int[i]);
    }
  hash[hash_len] = 0;
}

int
init_md_ctx (sha_ctx_t *sha_ctx)
{

  if ((sha_ctx->mdctx = EVP_MD_CTX_new ()) == NULL)
    {
      DEBUG_WARN ("Could not create EVP_MD_CTX\n");
      return 1;
    }

  if (1 != EVP_DigestInit_ex (sha_ctx->mdctx, EVP_sha3_256 (), NULL))
    {
      DEBUG_WARN ("Could not init digest\n");
      return 1;
    }
  return 0;
}

void
deinit_md_ctx (sha_ctx_t *sha_ctx)
{
  EVP_MD_CTX_free (sha_ctx->mdctx);
}

int
digest_message (EVP_MD_CTX *mdctx, const unsigned char *message,
                size_t message_len, unsigned char *digest,
                unsigned int *digest_len)
{
  if (1 != EVP_DigestUpdate (mdctx, message, message_len))
    {
      DEBUG_WARN ("Could not update digest\n");
      return 1;
    }

  if (1 != EVP_DigestFinal_ex (mdctx, digest, digest_len))
    {
      DEBUG_WARN ("Could not digest\n");
      return 1;
    }

  return 0;
}

void
digest_display (unsigned char *digest, unsigned int digest_len)
{
  char buff[2 * digest_len + 1];

  for (size_t i = 0; i < digest_len; i++)
    {
      snprintf (&buff[i * 2], 3, "%02x", digest[i]);
    }

  printf ("%s\n", buff);
}
