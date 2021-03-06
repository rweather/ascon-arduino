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

#ifndef ASCON_XOF_H
#define ASCON_XOF_H

/**
 * \file ascon-xof.h
 * \brief ASCON-XOF and ASCON-XOFA extensible output functions (XOF's).
 *
 * References: https://ascon.iaik.tugraz.at/
 */

#include "ascon-permutation.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Size of the hash output for ASCON-HASH and the default hash
 * output size for ASCON-XOF.
 */
#define ASCON_HASH_SIZE 32

/**
 * \brief Size of the hash output for ASCON-HASHA and the default hash
 * output size for ASCON-XOFA.
 */
#define ASCON_HASHA_SIZE ASCON_HASH_SIZE

/**
 * \brief Rate of absorbing and squeezing data for ASCON-XOF,
 * ASCON-XOFA, ASCON-HASH, and ASCON-HASHA.
 */
#define ASCON_XOF_RATE 8

/**
 * \brief State information for ASCON-XOF incremental mode.
 */
typedef struct
{
    ascon_state_t state;    /**< Current hash state */
    unsigned char count;    /**< Number of bytes in the current block */
    unsigned char mode;     /**< Hash mode: 0 for absorb, 1 for squeeze */

} ascon_xof_state_t;

/**
 * \brief State information for ASCON-XOFA incremental mode.
 */
typedef struct
{
    ascon_state_t state;    /**< Current hash state */
    unsigned char count;    /**< Number of bytes in the current block */
    unsigned char mode;     /**< Hash mode: 0 for absorb, 1 for squeeze */

} ascon_xofa_state_t;

/**
 * \brief Hashes a block of input data with ASCON-XOF and generates a
 * fixed-length 32 byte output.
 *
 * \param out Buffer to receive the hash output which must be at least
 * 32 bytes in length.
 * \param in Points to the input data to be hashed.
 * \param inlen Length of the input data in bytes.
 *
 * Use ascon_xof_squeeze() instead if you need variable-length XOF ouutput.
 *
 * \sa ascon_xof_init(), ascon_xof_absorb(), ascon_xof_squeeze()
 */
void ascon_xof(unsigned char *out, const unsigned char *in, size_t inlen);

/**
 * \brief Initializes the state for an ASCON-XOF hashing operation.
 *
 * \param state XOF state to be initialized.
 *
 * \sa ascon_xof_absorb(), ascon_xof_squeeze(), ascon_xof()
 */
void ascon_xof_init(ascon_xof_state_t *state);

/**
 * \brief Initializes the state for an incremental ASCON-XOF operation,
 * with a fixed output length.
 *
 * \param state XOF state to be initialized.
 * \param outlen The desired output length in bytes, or 0 for arbitrary-length.
 *
 * In the ASCON standard, the output length is encoded as a bit counter
 * in a 32-bit word.  If \a outlen is greater than 536870911, it will be
 * replaced with zero to indicate arbitary-length output instead.
 *
 * \sa ascon_xof_init()
 */
void ascon_xof_init_fixed(ascon_xof_state_t *state, size_t outlen);

/**
 * \brief Re-initializes the state for an ASCON-XOF hashing operation.
 *
 * \param state XOF state to be re-initialized.
 *
 * This function is equivalent to calling ascon_xof_free() and then
 * ascon_xof_init() to restart the hashing process.
 *
 * \sa ascon_xof_init()
 */
void ascon_xof_reinit(ascon_xof_state_t *state);

/**
 * \brief Re-initializes the state for an incremental ASCON-XOF operation,
 * with a fixed output length.
 *
 * \param state XOF state to be re-initialized.
 * \param outlen The desired output length in bytes, or 0 for arbitrary-length.
 *
 * This function is equivalent to calling ascon_xof_free() and then
 * ascon_xof_init_fixed() to restart the hashing process.
 *
 * \sa ascon_xof_init_fixed()
 */
void ascon_xof_reinit_fixed(ascon_xof_state_t *state, size_t outlen);

/**
 * \brief Frees the ASCON-XOF state and destroys any sensitive material.
 *
 * \param state XOF state to be freed.
 */
void ascon_xof_free(ascon_xof_state_t *state);

/**
 * \brief Aborbs more input data into an ASCON-XOF state.
 *
 * \param state XOF state to be updated.
 * \param in Points to the input data to be absorbed into the state.
 * \param inlen Length of the input data to be absorbed into the state.
 *
 * \sa ascon_xof_init(), ascon_xof_squeeze()
 */
void ascon_xof_absorb
    (ascon_xof_state_t *state, const unsigned char *in, size_t inlen);

/**
 * \brief Squeezes output data from an ASCON-XOF state.
 *
 * \param state XOF state to squeeze the output data from.
 * \param out Points to the output buffer to receive the squeezed data.
 * \param outlen Number of bytes of data to squeeze out of the state.
 *
 * \sa ascon_xof_init(), ascon_xof_update()
 */
void ascon_xof_squeeze
    (ascon_xof_state_t *state, unsigned char *out, size_t outlen);

/**
 * \brief Absorbs enough zeroes into an ASCON-XOF state to pad the
 * input to the next multiple of the block rate.
 *
 * \param state XOF state to pad.  Does nothing if the \a state is
 * already aligned on a multiple of the block rate.
 *
 * This function can avoid unnecessary XOR-with-zero operations
 * to save some time when padding is required.
 */
void ascon_xof_pad(ascon_xof_state_t *state);

/**
 * \brief Clears the rate portion of an ASCON-XOF state to all-zeroes and
 * runs the permutation.
 *
 * \param state XOF state to clear the rate on.
 *
 * This operation is used in the SpongePRNG construction for pseudorandom
 * number generators.  The rate is zeroed and then the permutation is run.
 * This makes it difficult to run the PRNG backwards if the state is
 * captured sometime in the future.
 *
 * This function calls ascon_xof_pad() before clearing the rate to
 * ensure that the XOF is in absorb mode and aligned on a block boundary.
 */
void ascon_xof_clear_rate(ascon_xof_state_t *state);

/**
 * \brief Clones a copy of an ASCON-XOF state.
 *
 * \param dest Destination XOF state to copy into.
 * \param src Source XOF state to copy from.
 *
 * The destination will be initialized by this operation, so it must
 * not previously have been initialized or it has already been freed.
 * The source must be already initialized.
 */
void ascon_xof_copy(ascon_xof_state_t *dest, const ascon_xof_state_t *src);

/**
 * \brief Hashes a block of input data with ASCON-XOFA and generates a
 * fixed-length 32 byte output.
 *
 * \param out Buffer to receive the hash output which must be at least
 * 32 bytes in length.
 * \param in Points to the input data to be hashed.
 * \param inlen Length of the input data in bytes.
 *
 * Use ascon_xofa_squeeze() instead if you need variable-length XOF ouutput.
 *
 * \sa ascon_xofa_init(), ascon_xofa_absorb(), ascon_xofa_squeeze()
 */
void ascon_xofa(unsigned char *out, const unsigned char *in, size_t inlen);

/**
 * \brief Initializes the state for an ASCON-XOFA hashing operation.
 *
 * \param state XOF state to be initialized.
 *
 * \sa ascon_xofa_absorb(), ascon_xofa_squeeze(), ascon_xofa()
 */
void ascon_xofa_init(ascon_xofa_state_t *state);

/**
 * \brief Initializes the state for an incremental ASCON-XOFA operation,
 * with a fixed output length.
 *
 * \param state XOF state to be initialized.
 * \param outlen The desired output length in bytes, or 0 for arbitrary-length.
 *
 * In the ASCON standard, the output length is encoded as a bit counter
 * in a 32-bit word.  If \a outlen is greater than 536870911, it will be
 * replaced with zero to indicate arbitary-length output instead.
 *
 * \sa ascon_xofa_init()
 */
void ascon_xofa_init_fixed(ascon_xofa_state_t *state, size_t outlen);

/**
 * \brief Re-initializes the state for an ASCON-XOFA hashing operation.
 *
 * \param state XOF state to be re-initialized.
 *
 * This function is equivalent to calling ascon_xofa_free() and then
 * ascon_xofa_init() to restart the hashing process.
 *
 * \sa ascon_xof_init()
 */
void ascon_xofa_reinit(ascon_xofa_state_t *state);

/**
 * \brief Re-initializes the state for an incremental ASCON-XOFA operation,
 * with a fixed output length.
 *
 * \param state XOF state to be re-initialized.
 * \param outlen The desired output length in bytes, or 0 for arbitrary-length.
 *
 * This function is equivalent to calling ascon_xofa_free() and then
 * ascon_xofa_init_fixed() to restart the hashing process.
 *
 * \sa ascon_xof_init_fixed()
 */
void ascon_xofa_reinit_fixed(ascon_xofa_state_t *state, size_t outlen);

/**
 * \brief Frees the ASCON-XOFA state and destroys any sensitive material.
 *
 * \param state XOF state to be freed.
 */
void ascon_xofa_free(ascon_xofa_state_t *state);

/**
 * \brief Aborbs more input data into an ASCON-XOFA state.
 *
 * \param state XOF state to be updated.
 * \param in Points to the input data to be absorbed into the state.
 * \param inlen Length of the input data to be absorbed into the state.
 *
 * \sa ascon_xofa_init(), ascon_xofa_squeeze()
 */
void ascon_xofa_absorb
    (ascon_xofa_state_t *state, const unsigned char *in, size_t inlen);

/**
 * \brief Squeezes output data from an ASCON-XOFA state.
 *
 * \param state XOF state to squeeze the output data from.
 * \param out Points to the output buffer to receive the squeezed data.
 * \param outlen Number of bytes of data to squeeze out of the state.
 *
 * \sa ascon_xofa_init(), ascon_xofa_update()
 */
void ascon_xofa_squeeze
    (ascon_xofa_state_t *state, unsigned char *out, size_t outlen);

/**
 * \brief Absorbs enough zeroes into an ASCON-XOFA state to pad the
 * input to the next multiple of the block rate.
 *
 * \param state XOF state to pad.  Does nothing if the \a state is
 * already aligned on a multiple of the block rate.
 *
 * This function can avoid unnecessary XOR-with-zero operations
 * to save some time when padding is required.
 */
void ascon_xofa_pad(ascon_xofa_state_t *state);

/**
 * \brief Clears the rate portion of an ASCON-XOFA state to all-zeroes and
 * runs the permutation.
 *
 * \param state XOF state to clear the rate on.
 *
 * This operation is used in the SpongePRNG construction for pseudorandom
 * number generators.  The rate is zeroed and then the permutation is run.
 * This makes it difficult to run the PRNG backwards if the state is
 * captured sometime in the future.
 *
 * This function calls ascon_xofa_pad() before clearing the rate to
 * ensure that the XOF is in absorb mode and aligned on a block boundary.
 */
void ascon_xofa_clear_rate(ascon_xofa_state_t *state);

/**
 * \brief Clones a copy of an ASCON-XOFA state.
 *
 * \param dest Destination XOF state to copy into.
 * \param src Source XOF state to copy from.
 *
 * The destination will be initialized by this operation, so it must
 * not previously have been initialized or it has already been freed.
 * The source must be already initialized.
 */
void ascon_xofa_copy(ascon_xofa_state_t *dest, const ascon_xofa_state_t *src);

#ifdef __cplusplus
}
#endif

#endif
