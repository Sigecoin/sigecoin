// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config/sigeconfig.h"
#include "byteswap.h"
#include <cstring>

#if 0
size_t strnlen( const char *start, size_t max_len)
{
    const char *end = (const char *)memchr(start, '\0', max_len);
    return end ? (size_t)(end - start) : max_len;
}
#endif // HAVE_DECL_STRNLEN
