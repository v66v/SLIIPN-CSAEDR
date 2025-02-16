#include "sha/sha.h"

int
main ()
{
  unsigned char input[] = "002302E";
  unsigned int digest_len;
  unsigned char output[SHA256_DIGEST_LENGTH];
  sha_ctx_t sha_ctx;

  init_md_ctx (&sha_ctx);
  digest_message (sha_ctx.mdctx, input, sizeof (input) - 1, output,
                  &digest_len);

  printf ("SHA3-256: ");
  digest_display (output, digest_len);

  deinit_md_ctx (&sha_ctx);
}
