#ifndef SLOT_REPLICATION_H_
#define SLOT_REPLICATION_H_
/// slot-replication.h - a mechanism to replicate slots across ciphertexts
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
/// This module implements a memeory-efficient mechanism to replicate slots
/// across ciphertexts, using the recursive method from https://ia.cr/2014/106
/// section 4.2, but traversing the recursion tree using depth-first search
/// so as to save on memory when the replicas are needed one at a time.
///
/// The simplest use-case is taking a packed ciphertext as input, outputting
/// a vector of ciphertexts with all the slots of the i'th output equal to
/// the i'th slot of the input. More generally, the input ciphertext may
/// already be partially replicated, with the same length-x pattern
/// repeated number of tie to fill all the slots. In that case the output
/// will be a vector of x ciphertexts, with all the slots of the i'th
/// output equal to the i'th slot in the input.
///
/// This implementation is geared towards sequential use of the output
/// ciphertexts, after construction it provides a next_replica() method that
/// returns the next output ciphertext. It is optimized to use the "hoisting"
/// method from https://ia.cr/2018/244 in nodes of the tree with degree>2.

#include <vector>
#include <memory>
#include <utility>
#include "openfhe.h"

// A convenience typedef for a ciphertext in openfhe, also to hint about
// this being a shared_ptr and not a struct that takes up a lot of space.
typedef lbcrypto::Ciphertext<lbcrypto::DCRTPoly> OpenFHE_CtxtSharedPtr;

class ReplicatorNode;  // forward decleration

/// @breif DFSSlotReplicator, the APIs to this implementation.

class DFSSlotReplicator {
private:
    std::shared_ptr<ReplicatorNode> handle;
public:
    /// @brief Builds a replication tree to replicate the slots of ciphertexts
    /// @param tree_degrees The degrees of nodes in the tree, one number per
    /// level, ordered from root to leaves. (This does not change the return
    /// value, but can affect performance.)
    /// The entries must all be larger than than one, and their product times
    /// the input_replication parameter must equal the number of slots.
    /// @param input_replication if >0, the number of times a repeated pattern
    /// appears in the input ciphertext. This must divide the number of slots,
    /// and the pattern length is num_slots/input_replication. Default is 1
    /// (no repeated pattern)
    explicit DFSSlotReplicator(lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& cc,
        std::vector<int> tree_degrees, int input_replication = 1);

    /// "Install" a ciphertext and return the 1st replicated ciphertext
    /// @param ct the ciphertext whose slots we want to replicate
    /// @return the first replicated ciphertext, with all the slots equal
    /// to the 1st slot of ct
    OpenFHE_CtxtSharedPtr init(OpenFHE_CtxtSharedPtr& ct);

    /// returns the next output ciphertext from the replication algorithm
    OpenFHE_CtxtSharedPtr next_replica();

    /// @brief Replicates each slot into a separate full ciphertext
    /// Parameters are the same as for the DFSSlotReplicator constructor.  
    /// @return a vector of ciphertext of size equal to the length of the
    /// pattern in the input. All the slots in the i'th output are eual to
    /// the i'th input slot.
    static std::vector<OpenFHE_CtxtSharedPtr> batch_replicate(
        OpenFHE_CtxtSharedPtr ct, std::vector<int> tree_degrees, int input_replication = 1);

    // Helper methods

    /// Get the tree degrees for an existing tree
    std::vector<int> get_degrees();

    /// @brief A helper function that returns the rotation amounts that will
    /// be used for a given tree-shape. This is meant for generating evaluation
    /// keys for these rotation amounts, before building any trees. The
    /// tree_degrees parameter is as for the DFSSlotReplicator constructor.
    /// @return a vector with all the rotation amounts, that can be passed
    /// as parameter to CryptoContext->EvalAtIndexKeyGen(...)
    static std::vector<int> get_rotation_amounts(
        std::vector<int> tree_degrees);

    /// @brief A placeholder for a tool to help determine the best tree shape.
    ///   Currently this is a simplistic program that returns a root of degree
    ///   8 or 16 and the rest of the tree with degree 2. The "best" shape is
    ///   expected to be different between different environments, depending
    ///   on hardware/software configuration.
    /// @param num_outputs how many output ciphertexts should be produced
    /// @return a vector of degrees that can be fed to the constructor
    static std::vector<int> suggest_degrees(int num_outputs);
};
#endif  // SLOT_REPLICATION_H_
