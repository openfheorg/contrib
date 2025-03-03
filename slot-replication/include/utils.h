#ifndef SLOT_ROTATION_UTILS_H_
#define SLOT_ROTATION_UTILS_H_
// init-crypto.h - initialize crypto parameters and key
//==================================================================================
// BSD 2-Clause License
//
// Copyright (c) 2025, Amazon Web Services
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//==================================================================================
#include <sys/resource.h>
#include "openfhe.h"

// swap these two lines below to toggle real/demo. In SMALLRING mode it
// runs with ring dimension 2^10, otherwise the ring dimension is 2^16.
// #define SMALLRING
#undef SMALLRING

// Generate a "standard" parameter set
std::pair<lbcrypto::CCParams<lbcrypto::CryptoContextCKKSRNS>, std::vector<uint32_t>> set_params();

// Generate keys that include all the rotations needed for replication
// as well as for bootstrapping
lbcrypto::KeyPair<lbcrypto::DCRTPoly> key_gen(
    lbcrypto::CCParams<lbcrypto::CryptoContextCKKSRNS>& prms,
    std::vector<uint32_t> levelBudget, std::vector<int>& rotations_needed);

// Print the amount of RAM used so far
void printMemoryUsage(const std::string& stage);

#endif // ifdef SLOT_ROTATION_UTILS_H_
