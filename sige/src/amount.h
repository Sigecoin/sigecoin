// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_amount_h__
#define __sig_amount_h__ 

#include "serialize.h"

#include <stdlib.h>
#include <string>

/** Amount in sigmas (Can be negative) */
typedef int64_t CAmount;

static const CAmount COIN = 100000000;
static const CAmount CENT = 1000000;

extern const std::string CURRENCY_UNIT;

/** No amount larger than this (in sige) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Sigecoin
 * currently happens to be less than 84,000,000 SGC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static const CAmount MAX_MONEY = 84000000 * COIN;
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/**
 * Fee rate in siges per kilobyte: CAmount / kB
 */
class CFeeRate
{
private:
    CAmount nSigesPerK; // unit is sige-per-1,000-bytes
public:
    /** Fee rate of 0 siges per kB */
    CFeeRate() : nSigesPerK(0) { }
    explicit CFeeRate(const CAmount& _nSigesPerK): nSigesPerK(_nSigesPerK) { }
    /** Constructor for a fee rate in siges per kB. The size in bytes must not exceed (2^63 - 1)*/
    CFeeRate(const CAmount& nFeePaid, size_t nBytes);
    CFeeRate(const CFeeRate& other) { nSigesPerK = other.nSigesPerK; }
    /**
     * Return the fee in siges for the given size in bytes.
     */
    CAmount GetFee(size_t nBytes) const;
    /**
     * Return the fee in siges for a size of 1000 bytes
     */
    CAmount GetFeePerK() const { return GetFee(1000); }
    friend bool operator<(const CFeeRate& a, const CFeeRate& b) { return a.nSigesPerK < b.nSigesPerK; }
    friend bool operator>(const CFeeRate& a, const CFeeRate& b) { return a.nSigesPerK > b.nSigesPerK; }
    friend bool operator==(const CFeeRate& a, const CFeeRate& b) { return a.nSigesPerK == b.nSigesPerK; }
    friend bool operator<=(const CFeeRate& a, const CFeeRate& b) { return a.nSigesPerK <= b.nSigesPerK; }
    friend bool operator>=(const CFeeRate& a, const CFeeRate& b) { return a.nSigesPerK >= b.nSigesPerK; }
    CFeeRate& operator+=(const CFeeRate& a) { nSigesPerK += a.nSigesPerK; return *this; }
    std::string ToString() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(nSigesPerK);
    }
};

#endif  /* __sig_amount_h__ */
