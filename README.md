## Table of Contents

- [Purpose and Disclaimer](#prupose-and-disclaimer)
- [Community Projects in Contrib Repository](#community-projects-in-Contrib-Repository)
- [User Projects in Other OpenFHE Repositories](#user-projects-in-other-openfhe-repositories)
- [Guidelines for Submitting a New Project](#guidelines-for-submitting-a-new-project)

# Purpose and Disclaimer
The `contrib` repository is intended to host community-contributed projects that either provide examples of using OpenFHE or extend OpenFHE.
These projects may be useful to OpenFHE users but are not considered part of the core OpenFHE project functionality. 
Contributions within this repository are not officially supported by the project maintainers and may not be actively maintained or rigorously tested. 
Users are responsible for evaluating the quality and compatibility of any code they choose to utilize from the contrib repository and should not expect bug fixes or updates unless explicitly stated by the contributor.

# Community Projects in Contrib Repository

All contributed projects are listed below. Please check out the project folder for more details.

| Project          | Description          | Scheme(s)| Language(s)| Author(s)          |
| ---------------- |:--------------------:|:---------:|:---------:|:------------------:|
| Slot replication | To be added          | CKKS      | C++       | Shai Halevi (AWS)  |


# User Projects in Other OpenFHE Repositories


# Guidelines for Submitting a New Project
Please review existing contributions and use one of them as a template.
1. Create a user CMake project following the [Building User Applications](https://openfhe-development.readthedocs.io/en/latest/sphinx_rsts/intro/building_user_applications.html) article.
2. Follow the standard folder naming convention, e.g., `src`, `include`, `lib`, `examples`, `tests`.
3. Add a license (BSD-2 license is recommended).
4. Add a README.md file to the root of the project. Specify the versions of OpenFHE under which your project has been tested.
5. Email contact@openfhe.org to have your github account added to the contrib reposlitory.
6. Submit a PR with the contribution.
