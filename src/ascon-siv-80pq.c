/*
 * Copyright (C) 2022 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ascon-siv.h"
#include "utility/ascon-aead-common.h"
#include "utility/ascon-util-snp.h"
#include <string.h>

/**
 * \brief Initialization vector for ASCON-80pq-SIV, authentication phase.
 */
#define ASCON80PQ_IV1   0xa1400c06U

/**
 * \brief Initialization vector for ASCON-80pq-SIV, encryption phase.
 */
#define ASCON80PQ_IV2   0xa2400c06U

/**
 * \brief Initializes the ASCON state for ASCON-80pq-SIV.
 *
 * \param state ASCON state to be initialized.
 * \param npub Points to the nonce.
 * \param k Points to the key.
 * \param iv Initialization vector value for the ASCON state.
 */
static void ascon80pq_siv_init
    (ascon_state_t *state, const unsigned char *npub,
     const unsigned char *k, uint32_t iv)
{
    be_store_word32(state->B, iv);
    memcpy(state->B + 4, k, ASCON80PQ_KEY_SIZE);
    memcpy(state->B + 24, npub, ASCON80PQ_NONCE_SIZE);
    ascon_from_regular(state);
    ascon_permute(state, 0);
    ascon_absorb_partial(state, k, 20, ASCON80PQ_KEY_SIZE);
}

/**
 * \brief Encrypts a block of data with an ASCON state and an 8-byte rate.
 *
 * \param state The state to encrypt with.
 * \param dest Points to the destination buffer.
 * \param src Points to the source buffer.
 * \param len Length of the data to encrypt from \a src into \a dest.
 * \param first_round First round of the permutation to apply each block.
 *
 * This operates the ASCON permutation in OFB mode, which can be used to
 * perform both encryption and decryption.
 */
static void ascon_siv_encrypt_8_80pq
    (ascon_state_t *state, unsigned char *dest,
     const unsigned char *src, size_t len, uint8_t first_round)
{
    unsigned char block[8];
    while (len >= 8) {
        ascon_permute(state, first_round);
        ascon_squeeze_8(state, block, 0);
        lw_xor_block_2_src(dest, block, src, 8);
        dest += 8;
        src += 8;
        len -= 8;
    }
    if (len > 0) {
        ascon_permute(state, first_round);
        ascon_squeeze_8(state, block, 0);
        lw_xor_block_2_src(dest, block, src, len);
    }
}

int ascon80pq_siv_encrypt
    (unsigned char *c, size_t *clen,
     const unsigned char *m, size_t mlen,
     const unsigned char *ad, size_t adlen,
     const unsigned char *npub,
     const unsigned char *k)
{
    ascon_state_t state;

    /* Set the length of the returned ciphertext */
    *clen = mlen + ASCON80PQ_TAG_SIZE;

    /* Initialize the ASCON state for the authentication phase */
    ascon80pq_siv_init(&state, npub, k, ASCON80PQ_IV1);

    /* Absorb the associated data into the state */
    if (adlen > 0)
        ascon_aead_absorb_8(&state, ad, adlen, 6, 1);

    /* Separator between the associated data and the payload */
    ascon_separator(&state);

    /* Absorb the plaintext data into the state */
    ascon_aead_absorb_8(&state, m, mlen, 6, 0);

    /* Compute the authentication tag */
    ascon_absorb_partial(&state, k, 8, ASCON80PQ_KEY_SIZE);
    ascon_permute(&state, 0);
    ascon_absorb_16(&state, k + 4, 24);
    ascon_squeeze_16(&state, c + mlen, 24);

    /* Re-initalize the ASCON state for the encryption phase */
    ascon80pq_siv_init(&state, c + mlen, k, ASCON80PQ_IV2);

    /* Encrypt the plaintext to create the ciphertext */
    ascon_siv_encrypt_8_80pq(&state, c, m, mlen, 6);
    return 0;
}

int ascon80pq_siv_decrypt
    (unsigned char *m, size_t *mlen,
     const unsigned char *c, size_t clen,
     const unsigned char *ad, size_t adlen,
     const unsigned char *npub,
     const unsigned char *k)
{
    ascon_state_t state;

    /* Set the length of the returned plaintext */
    if (clen < ASCON80PQ_TAG_SIZE)
        return -1;
    clen -= ASCON80PQ_TAG_SIZE;
    *mlen = clen;

    /* Initalize the ASCON state for the encryption phase */
    ascon80pq_siv_init(&state, c + clen, k, ASCON80PQ_IV2);

    /* Decrypt the ciphertext to create the plaintext */
    ascon_siv_encrypt_8_80pq(&state, m, c, clen, 6);

    /* Re-initialize the ASCON state for the authentication phase */
    ascon80pq_siv_init(&state, npub, k, ASCON80PQ_IV1);

    /* Absorb the associated data into the state */
    if (adlen > 0)
        ascon_aead_absorb_8(&state, ad, adlen, 6, 1);

    /* Separator between the associated data and the payload */
    ascon_separator(&state);

    /* Absorb the plaintext data into the state */
    ascon_aead_absorb_8(&state, m, clen, 6, 0);

    /* Compute and check authentication tag */
    ascon_absorb_partial(&state, k, 8, ASCON80PQ_KEY_SIZE);
    ascon_permute(&state, 0);
    ascon_absorb_16(&state, k + 4, 24);
    ascon_to_regular(&state);
    return ascon_aead_check_tag
        (m, clen, state.B + 24, c + clen, ASCON80PQ_TAG_SIZE);
}