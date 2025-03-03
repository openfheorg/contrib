/// test-slot-replication.cpp - tesing the replication mechanisms
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
#include <cstdio>
#include <cassert>
#include "../include/slot-replication.h"
using namespace lbcrypto;

constexpr size_t RING_DIM = (1 << 6);   // 64
constexpr size_t N_SLOTS = RING_DIM/2;  // 32

// Are these two numbers close to each other?
inline bool close(double x, double y) {return std::abs(x-y)<1e-5;}

// Generate a "standard" parameter set
CCParams<CryptoContextCKKSRNS> set_crypto_params();

// Generate keys that include all the rotations needed for replication
KeyPair<DCRTPoly> key_gen(CCParams<CryptoContextCKKSRNS>& prms, std::vector<int>& rotations_needed);

static OpenFHE_CtxtSharedPtr generate_ciphertext(KeyPair<DCRTPoly>& keys);

void test_suggest_degree();
void test_get_degrees();
void test_replication();
void test_batch_replication();

int main() {
    test_suggest_degree();
    std::cout << "test_suggest_degree...    PASSED\n";
    test_get_degrees();
    std::cout << "test_get_degrees...       PASSED\n";
    test_replication();
    std::cout << "test_replication...       PASSED\n";
    test_batch_replication();
    std::cout << "test_batch_replication... PASSED\n";
    return 0;
}

// This would need to change if we change the suggestions
void test_suggest_degree()
{
    auto degs = DFSSlotReplicator::suggest_degrees(8);
    assert(degs.size()==1 && degs[0]==8);

    degs = DFSSlotReplicator::suggest_degrees(16);
    assert(degs.size()==2 && degs[0]==8 && degs[1]==2);

    degs = DFSSlotReplicator::suggest_degrees(128);
    assert(degs.size()==4 && degs[0]==8 && degs[1]==4 && degs[2]==2 && degs[3]==2);
}

// Build a tree, and check that get_degrees() returns the original vector
void test_get_degrees()
{
    std::vector<int> degrees = DFSSlotReplicator::suggest_degrees(N_SLOTS);
    assert(std::accumulate(degrees.begin(), degrees.end(), 1, std::multiplies<int>())==N_SLOTS);

    auto prms = set_crypto_params();
    auto rotations = DFSSlotReplicator::get_rotation_amounts(degrees);
    auto keys = key_gen(prms, rotations);
    auto cc = keys.publicKey->GetCryptoContext(); // crypto context
    auto ct = generate_ciphertext(keys);
    DFSSlotReplicator replicator(cc, degrees);
    assert(replicator.get_degrees() == degrees);
}

void test_replication()
{
    std::vector<int> degrees = DFSSlotReplicator::suggest_degrees(N_SLOTS);
    assert(std::accumulate(degrees.begin(), degrees.end(), 1, std::multiplies<int>())==N_SLOTS);
    auto rotations = DFSSlotReplicator::get_rotation_amounts(degrees);

    auto prms = set_crypto_params();
    auto keys = key_gen(prms, rotations);
    auto cc = keys.publicKey->GetCryptoContext(); // crypto context
    auto ct = generate_ciphertext(keys);

    // Decrypt for checking later
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    auto v = pt->GetRealPackedValue();

    // Test #1: full replication
    {DFSSlotReplicator replicator(cc, degrees);

    // Repeat twice to test that the same replicator object
    // can be reused acorss ciphertexts
    for (auto k = 0; k < 2; k++) {
        std::vector<OpenFHE_CtxtSharedPtr> replicas;
        for (auto ct_i = replicator.init(ct);
                    ct_i != nullptr; ct_i = replicator.next_replica()) {
            replicas.push_back(ct_i);
        }
        // Decrypt and verify. The expected result has N_SLOTS ciphertexts:
        // [1,1,...,1], [2,2,..,2], ...
        assert(replicas.size()==N_SLOTS);
        for (size_t i = 0; i < N_SLOTS; i++) {
            cc->Decrypt(keys.secretKey, replicas[i], &pt);
            auto vv = pt->GetRealPackedValue();
            for (size_t j = 0; j < N_SLOTS; j++) {
                assert(close(vv[j], v[i]));
            }
        }
    }}

    // Test #2: Partial replications: pretend that the input ciphertext
    // has a pattern that repreat twice, and replicate to get N_SLOTS/2 
    // ciphertext, each with two values {i, i+N+SLOTS/2}.
    degrees = { 2, 2, 2, 2 };
    DFSSlotReplicator replicator(cc, degrees, 2);
    std::vector<OpenFHE_CtxtSharedPtr> replicas;
    for (auto ct_i = replicator.init(ct);
                    ct_i != nullptr; ct_i = replicator.next_replica()) {
        replicas.push_back(ct_i);
    }
    // Decrypt and verify. The expected result has N_SLOTS/2 ciphertexts:
    // [1,1,...,1] (mod N_SLOTS/2), [2,2,...,2] (mod N_SLOTS/2), ...
    // In more detail, the ciphertexts contain:
    //    [1 1...1 17 17...17]
    //    [18 2...2 2 18...18]
    //    [19 19 3...3 19...19]
    //    ...
    //    [31...31 15...15 31 31]
    //    [31...32 16...16 32]
    assert(replicas.size()==N_SLOTS/2);
    for (size_t i = 0; i < N_SLOTS/2; i++) {
        auto val1 = int(round(v[i])) % (N_SLOTS/2);
        cc->Decrypt(keys.secretKey, replicas[i], &pt);
        auto vv = pt->GetRealPackedValue();
        for (size_t j = 0; j < N_SLOTS; j++) {
            auto val2 = int(round(vv[j])) % (N_SLOTS/2);
            assert(val1==val2);
        }
        // std::cout << vv << std::endl;
    }
}

void test_batch_replication()
{
    std::vector<int> degrees = { 8, 2, 2 }; // need to multiply to N_SLOTS
    assert(std::accumulate(degrees.begin(), degrees.end(), 1, std::multiplies<int>())==N_SLOTS);
    auto rotations = DFSSlotReplicator::get_rotation_amounts(degrees);

    auto prms = set_crypto_params();
    auto keys = key_gen(prms, rotations);
    auto cc = keys.publicKey->GetCryptoContext(); // crypto context
    auto ct = generate_ciphertext(keys);

    // Decrypt for checking later
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    auto v = pt->GetRealPackedValue();

    // Test full replication
    auto reps = DFSSlotReplicator::batch_replicate(ct, degrees);
    // Decrypt and verify. The expected result has N_SLOTS ciphertexts:
    // [1,1,...,1], [2,2,..,2], ...
    assert(reps.size()==N_SLOTS);
    for (size_t i = 0; i < N_SLOTS; i++) {
        cc->Decrypt(keys.secretKey, reps[i], &pt);
        auto vv = pt->GetRealPackedValue();
        for (size_t j = 0; j < N_SLOTS; j++) {
            assert(close(vv[j], v[i]));
        }
    }
}

// Generate a "standard" parameter set
CCParams<CryptoContextCKKSRNS> set_crypto_params()
{
    CCParams<CryptoContextCKKSRNS> parameters;

    parameters.SetSecretKeyDist(UNIFORM_TERNARY);
    parameters.SetSecurityLevel(HEStd_NotSet);
    parameters.SetRingDim(RING_DIM);
    parameters.SetScalingTechnique(FLEXIBLEAUTO);
    parameters.SetScalingModSize(50);
    parameters.SetFirstModSize(57);
    parameters.SetNumLargeDigits(4);
    parameters.SetKeySwitchTechnique(HYBRID);
    parameters.SetMultiplicativeDepth(10);
    return parameters;
}

// Generate keys that include all the rotations needed for replication
KeyPair<DCRTPoly> key_gen(
    CCParams<CryptoContextCKKSRNS>& prms, std::vector<int>& rotations_needed)
{
    CryptoContext<DCRTPoly> cc = GenCryptoContext(prms);

    // Enable features that you wish to use
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);  // re-linearization key
    cc->EvalAtIndexKeyGen(keyPair.secretKey, rotations_needed);  // replication rotation keys
    return keyPair;
}


// Encrypt a test ciphertext, containing 1,2,...,n
static OpenFHE_CtxtSharedPtr generate_ciphertext(KeyPair<DCRTPoly>& keys)
{
    auto cc = keys.publicKey->GetCryptoContext();
    size_t num_slots = cc->GetRingDimension() /2;

    // generate a test ciphertext with i in the i'th slot
    std::vector<std::complex<double>> slots;
    for (size_t i = 0; i < num_slots; i++) {
        slots.push_back(std::complex<double>(i+1, 0));
    }
    Plaintext pt = cc->MakeCKKSPackedPlaintext(slots);
    return cc->Encrypt(keys.publicKey, pt);
}
