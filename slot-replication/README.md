# Slot Replication Code and Examples

This module implements a mechanism to replicate slots across ciphertexts.
It roughly implements the recursive method from https://ia.cr/2014/106 section 4.2, but using depth-first traversal of the recursion tree so it never needs to maintain all the leaves at memory at the same time.
The implementation is optimized to use the "hoisting" method from https://ia.cr/2018/244, section 5, in nodes of the tree with degree more than two (as mentioned in https://ia.cr/2020/563, method 3 on page 26).

This implementation is geared towards sequential use of the output ciphertexts, the main interfaces are `init(ct)` and `next_replica()` methods that returns the first/next output ciphertext.

The basic use-case is taking a packed ciphertext as input, outputting a vector of ciphertexts with all the slots of the i'th output equal to the i'th slot of the input.
More generally, the input ciphertext may already be partially replicated, with the same length-x pattern repeated enough times to fill all the slots.
In that case the output will be a vector of x ciphertexts, with all the slots of the i'th output equal to the i'th slot in the input.

## Content

The main implementation is found in the file `slot-replication.cpp` under `src` directory (and the corresponding `slot-repliction.h` under the `include` directory).
Some helper code to set the openfhe parameters and generate keys is found in the `utils.[cpp,h]` files.
An example usage is included in `examples/replication.cpp`, and unit tests can be found in `tests/test-slot-replication.cpp`.

The example code runs by default with parameters that enables bootstrapping, with ring dimension 2^16.
Setting the macro SMALLRING in `include/utils.h` will run the code with ring dimension 2^10, this is useful for testing purposes.
(If/when openfhe exposes standardized parameter-sets, it is recommended to replace the code in `utils.[h,cpp]` with calls to those parameter-sets.)

## Installation and usage

This is a CMake project, which depends on openfhe.
It was tested with openfhe v1.2.3 (on a Linux system), but should also work on other Unix systems.
If openfhe is installed in the default directories, then you can run the example program (after cloning this code into the directory `slot-replication`) via:
```
   cd slot-replication; mkdir build; cd build
   cmake ..
   make
   ./replication  # This is a long example, expect it to run for 15-20 minutes
```

See the code in the file `examples/replication.cpp` for usage examples, the basic flow is this:
```
    std::vector<int> tree_shape = {8,2,2,2,2}; // degree-8 root, degree-2 below

    // ... some code to create the crypto-context cc and generate a keyPair ...

    // Ensure that we have all the key-switching gadgets needed for rotations
    std::vector<int> rotation_amts = DFSSlotReplicator::get_rotation_amounts(tree_shape);
    cc->EvalAtIndexKeyGen(keyPair.secretKey, rotation_amts);

    // A replicator object for this tree, with all the required masks
    DFSSlotReplicator replicator(tree_shape);

    // ... encrypt a packed ciphertext ct ...

    // The replication itself: the i'th ct_i is a packed Ciphertext<DCRTPoly>
    // with all the slots equal to the i'th slot of ct
    for (auto ct_i = replicator.init(ct); ct_i != nullptr;
                                        ct_i = replicator.next_replica())
    {
        // do something with the replicated ciphertext ct_i
    }
```
