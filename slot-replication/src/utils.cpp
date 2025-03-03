// init-crypto.cpp - initialize crypto parameters and key
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
#include "openfhe.h"
#include "utils.h"

using namespace lbcrypto;

// Generate a "standard" parameter set
std::pair<CCParams<CryptoContextCKKSRNS>, std::vector<uint32_t>> set_params()
{
    CCParams<CryptoContextCKKSRNS> parameters;

    parameters.SetSecretKeyDist(UNIFORM_TERNARY);

#ifdef SMALLRING
    parameters.SetSecurityLevel(HEStd_NotSet);
    parameters.SetRingDim(1 << 10);
#else
    parameters.SetSecurityLevel(HEStd_128_classic);
    // parameters.SetRingDim(1 << 16);
#endif

    parameters.SetScalingTechnique(FLEXIBLEAUTO);
    parameters.SetScalingModSize(50);
    parameters.SetFirstModSize(57);

    parameters.SetNumLargeDigits(11);
    parameters.SetKeySwitchTechnique(HYBRID);

    // Specify the number of iterations to run bootstrapping. Note that
    // openfhe currently only support 1s or 2 iterations. Two iterations
    // yields approximately double the precision of one iteration.
    uint32_t numIterations = 2;

    std::vector<uint32_t> levelBudget      = {3, 3};
    uint32_t levelsAvailableAfterBootstrap = 9;
    usint depth = levelsAvailableAfterBootstrap + (numIterations - 1)
        + FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
    parameters.SetMultiplicativeDepth(depth);

    std::cout << "Level consumption for SlotsToCoeffs: " << levelBudget[0] << std::endl;
    std::cout << "Level consumption for EvalMod: "
              << depth - levelsAvailableAfterBootstrap - levelBudget[0] - levelBudget[1] -1
              << std::endl;
    std::cout << "Level consumption for CoeffsToSlots: " << levelBudget[1] << std::endl;
    std::cout << "depth = " << depth << std::endl;
    return std::make_pair(parameters, levelBudget);
}

// Generate keys that include all the rotations needed for replication
// as well as for bootstrapping
KeyPair<DCRTPoly> key_gen(
    CCParams<CryptoContextCKKSRNS>& prms, std::vector<uint32_t> levelBudget, std::vector<int>& rotations_needed)
{
    CryptoContext<DCRTPoly> cc = GenCryptoContext(prms);

    // Enable features that you wish to use
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);

    usint numSlots = cc->GetRingDimension() / 2;
    
    cc->EvalBootstrapSetup(levelBudget);
    printMemoryUsage(" - after setup, before keygen");
    auto keyPair = cc->KeyGen();
    printMemoryUsage(" - after keygen");
    cc->EvalMultKeyGen(keyPair.secretKey);  // re-linearization key
    printMemoryUsage(" - after re-linearization key");
    cc->EvalBootstrapKeyGen(keyPair.secretKey, numSlots);  // bootstrapping keys
    printMemoryUsage(" - after bootstrapping keys");
    cc->EvalAtIndexKeyGen(keyPair.secretKey, rotations_needed);  // replication rotation keys
    printMemoryUsage(" - after replication rotation keys");
    return keyPair;
}


void printMemoryUsage(const std::string& stage) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    double memoryInGB;

    #if defined(__APPLE__)
        // On macOS, ru_maxrss is in bytes.
        memoryInGB = usage.ru_maxrss / (1024.0 * 1024.0 * 1024.0);  // bytes to GB
    #elif defined(__linux__)
        // On Linux, ru_maxrss is in kilobytes.
        memoryInGB = usage.ru_maxrss / (1024.0 * 1024.0);  // KB to GB
    #else
        // Default case if the platform is neither macOS nor Linux
        memoryInGB = usage.ru_maxrss / (1024.0 * 1024.0);  // Assume KB to GB
    #endif

    std::cout << "Memory usage at " << stage << ": "
              << memoryInGB << " gigabytes" << std::endl;
}
