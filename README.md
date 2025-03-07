## Table of Contents

- [Purpose and Disclaimer](#prupose-and-disclaimer)
- [Community Projects in Contrib Repository](#community-projects-in-Contrib-Repository)
- [User Projects in Other OpenFHE Repositories](#user-projects-in-other-openfhe-repositories)
- [Winning Solutions for FHERMA Challenges based on OpenFHE](winning-solutions-for-fherma-challenges-based-on-openfhe)
- [Guidelines for Submitting a New Project](#guidelines-for-submitting-a-new-project)

# Purpose and Disclaimer
The `contrib` repository is intended to host community-contributed projects that provide examples of using OpenFHE and/or extend OpenFHE.
These projects may be useful to OpenFHE users but are not considered part of the core OpenFHE project functionality. 
Contributions within this repository are not officially supported by the project maintainers and may not be actively maintained or rigorously tested. 
Users are responsible for evaluating the quality and compatibility of any code they choose to utilize from the contrib repository and should not expect bug fixes or updates unless explicitly stated by the contributor.

# Community Projects in Contrib Repository

All contributed projects are listed below. Please check out the project folder for more details.

| Project          | Description          | Scheme(s)| Language(s)| Author(s)          |
| :--------------- |:---------------------|:----------|:---------:|:-------------------|
| [Image classification with Resnet20](images-resnet20-low-mem) | Encrypted image classification using a pre-trained ResNet-20 CNN model based on the CIFAR-10 dataset. | CKKS      | C++       | Lorenzo Rovida (University of Milano-Bicocca)  |
| [Slot replication](slot-replication) | The basic use-case is taking a packed ciphertext as input, outputting a vector of ciphertexts with all the slots of the $i^{th}$ output equal to the $i^{th}$ slot of the input. More advanced use cases are also supported. | CKKS      | C++       | Shai Halevi (AWS)  |

# User Projects in Other OpenFHE Repositories

The user projects that exist in other OpenFHE repositories are listed below.

| Project          | Description          | Scheme(s)| Language(s)| Author(s)          |
| :--------------- |:---------------------|:---------:|:---------:|:-------------------|
| [Boolean circuit evaluator](https://github.com/openfheorg/openfhe-boolean-circuit-evaluator) | Various Boolean circuits in the Bristol format          | CGGI      | C++       | David Bruce Cousins (Duality)  |
| [Genomic analysis](https://github.com/openfheorg/openfhe-genomic-examples) | Logistic Regression Approximation and Chi-Square GWAS protocols          | CKKS      | C++       | Duality  |
| [Google Transpiler examples](https://github.com/openfheorg/openfhe-transpiler-examples)| Routing algorithms and AES evaluation| CGGI      | C++       | David Bruce Cousins (Duality) |
| [Logistic regression training](https://github.com/openfheorg/openfhe-logreg-training-examples) | Logistic Regression Trianing in C++ using Nesterov's Accelerated Gradient Descent method | CKKS      | C++       | Duality  |
| [Logistic regression training (Python)](https://github.com/openfheorg/python-log-reg-examples) | Logistic Regression Trianing in Python          | CKKS      | Python       | Ian Quah (University of Washington)  |
| [Network-oriented algorithms](https://github.com/openfheorg/openfhe-network-examples) | Encrypted network measurement/control, secure data distribution          | BGV, BFV, CKKS      | C++       | Duality  |
| [Serialization examples](https://github.com/openfheorg/python-svm-examples)| FHE, Proxy Re-Encryption, and Threshold FHE examples| BGV, CKKS     | C++       | David Bruce Cousins (Duality)  |
| [Substring search](https://github.com/openfheorg/openfhe-integer-examples)| Substring search using Rabin-Karp algorithm | BFV     | C++       | David Bruce Cousins (Duality)  |
| [Support Vector Machine inference](https://github.com/openfheorg/python-svm-examples)| Linear Support Vector Machine inference in Python| CKKS      | Python       | Rener Oliveira  |

# Winning Solutions for FHERMA Challenges based on OpenFHE

Winning solutions for [FHERMA](https://fherma.io/challenges) challenges are available as part of the [FHE Components Library](https://github.com/fairmath/polycircuit). The FHE components library already includes efficient implementation examples for evaluating image classication, matrix multiplication, logistic function, look-up tables, and maximum element, and for computing parity, ReLu and sign functions.

# Guidelines for Submitting a New Project
Please review existing contributions and use one of them as a template.
1. Create a user CMake project following the [Building User Applications](https://openfhe-development.readthedocs.io/en/latest/sphinx_rsts/intro/building_user_applications.html) article.
2. Follow the standard folder naming convention, e.g., `src`, `include`, `lib`, `examples`, `tests`.
3. Add a license (BSD-2 license is recommended).
4. Add a README.md file to the root of the project. Specify the versions of OpenFHE under which your project has been tested.
5. Email contact@openfhe.org to have your github account added to the contrib reposlitory.
6. Submit a PR with the contribution.
