// replication.cpp - an example of the replication mechanisms
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
#include <chrono>
#include <algorithm>  // For std::max

#include "slot-replication.h"
#include "utils.h"

using namespace lbcrypto;

#ifdef SMALLRING
constexpr size_t RING_DIM = (1 << 10);
#else
constexpr size_t RING_DIM = (1 << 16); 
#endif
constexpr size_t N_SLOTS = RING_DIM/2; 

inline bool close(double x, double y) {return std::abs(x-y)<1e-5;}

// Generate a "standard" parameter set
std::pair<CCParams<CryptoContextCKKSRNS>, std::vector<uint32_t>> set_params();

// Generate keys that include all the rotations needed for replication
// as well as for bootstrapping
KeyPair<DCRTPoly> key_gen(CCParams<CryptoContextCKKSRNS>& prms,
    std::vector<uint32_t> levelBudget, std::vector<int>& rotations_needed);

static OpenFHE_CtxtSharedPtr generate_ciphertext(KeyPair<DCRTPoly>& keys, int n_reps);
void printMemoryUsage(const std::string& stage);

static std::vector< std::vector<int> > tree_shapes = {
    {2,2,2,2,2,2,2,2,2},
    {4,2,2,2,2,2,2,2},
    {4,4,2,2,2,2,2},
    {8,2,2,2,2,2,2},
    {8,4,2,2,2,2}
};

void try_tree(std::vector<int>& degrees) {
#ifdef SMALLRING
    std::cout << "Replication example, this can take 1-2 minutes to run\n";
#else
    std::cout << "Replication example, this can take 15-20 minutes to run\n";
#endif
    std::cout << "degrees: "<< degrees <<std::endl;
    int n_outputs = std::accumulate(degrees.begin(), degrees.end(), 1, std::multiplies<int>());

    // The input ciphertext is assumed to include a pattern of length n_outputs,
    // repeated N_SLOTS/n_outputs many times to fill up all the slots.
    int n_reps = N_SLOTS/n_outputs;

    // The rotation keys that we need
    auto rotations = DFSSlotReplicator::get_rotation_amounts(degrees);
    std::cout << "rotation amounts: "<<rotations<<std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    auto [prms, lvl_budget] = set_params();            // openFHE parameters
    auto keys = key_gen(prms, lvl_budget, rotations);  // generate keys, including rotation keys
    auto cc = keys.publicKey->GetCryptoContext();
    auto end = std::chrono::high_resolution_clock::now(); 
    std::chrono::duration<double> calculationTime = end - start;
    std::cout << "Setup + keygen in " << calculationTime.count() << " seconds."  << std::endl;

    // Generate a replication tree
    start = std::chrono::high_resolution_clock::now();
    DFSSlotReplicator replicator(cc, degrees, n_reps);
    end = std::chrono::high_resolution_clock::now(); 
    calculationTime = end - start;
    printMemoryUsage("Build Replication Tree");
    std::cout << "Replicator object setup in " << calculationTime.count() << " seconds."  << std::endl;

    auto ct = generate_ciphertext(keys, n_reps); // The ciphertext to replicate
    
    // Decrypt for checking later
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    auto v = pt->GetRealPackedValue();

    // Repeat replication twice, demonstrating that you can use
    // the same replication object for more than one ciphertext.

    calculationTime = std::chrono::seconds::zero();
    for (int k = 0; k < 2; k++) {
        // Replicate then decrypt and check the result
        std::vector<double> vv;
        for (int i = 0; i < n_outputs; i++) {
            start = std::chrono::high_resolution_clock::now();
            auto ct_i = (i==0)? replicator.init(ct) : replicator.next_replica();
            end = std::chrono::high_resolution_clock::now();
            if (i == n_outputs - 1) {printMemoryUsage("Last Replica");}
            calculationTime += end - start;

            // Decrypt and check
            assert(ct_i != nullptr);
            cc->Decrypt(keys.secretKey, ct_i, &pt);
            vv = pt->GetRealPackedValue();
            for (size_t j = 0; j < N_SLOTS; j++) {
                assert(close(vv[j], v[i]));
            }
        }
        // A better way of using the replicator is as follows:
        //   for (auto ct_i = replicator.init(ct); ct_i != nullptr;
        //                          ct_i = replicator.next_replica()) { ... }
        // The reason we didn't use it above is to enable timing.
    }

    // An alternative way of replicating:
    start = std::chrono::high_resolution_clock::now();

    auto outputs = DFSSlotReplicator::batch_replicate(ct, degrees, n_reps);
    // This sets up a replicator and makes all the calls to init(ct) and
    // next_replica(), returning a vector with the same n_outputs replicas.

    end = std::chrono::high_resolution_clock::now();
    calculationTime += end - start;

    assert(int(outputs.size()) == n_outputs);
    for (int i = 0; i < n_outputs; i++) {
        // Decrypt and check
        cc->Decrypt(keys.secretKey, outputs[i], &pt);
        auto vv = pt->GetRealPackedValue();
        for (size_t j = 0; j < N_SLOTS; j++) {
            assert(close(vv[j], v[i]));
        }
    }
    std::cout << "Three replications in "<< calculationTime.count() << " seconds."  << std::endl;
    std::cout << "------------------------------------" << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
    int idx = 0;
    if (argc > 1) {
        idx = std::stoi(argv[1]);
    }
    if (idx < 0 || idx >= int(tree_shapes.size())) {
        idx = 0;
    }
    try_tree(tree_shapes[idx]);
    return 0;
}

static OpenFHE_CtxtSharedPtr generate_ciphertext(KeyPair<DCRTPoly>& keys, int n_reps)
{
    auto cc = keys.publicKey->GetCryptoContext();
    auto pattern_length = N_SLOTS / n_reps;

    // generate a test ciphertext with i in the i'th slot
    std::vector<std::complex<double>> slots;
    for (auto j = 0; j<n_reps; j++)
        for (size_t i = 0; i < pattern_length; i++) {
            slots.push_back(std::complex<double>(i+1, 0));
    }
    Plaintext pt = cc->MakeCKKSPackedPlaintext(slots);
    return cc->Encrypt(keys.publicKey, pt);
}
