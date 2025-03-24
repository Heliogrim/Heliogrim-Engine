<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Issues][issues-shield]][issues-url]
[![Private License][license-shield]][license-url]
[![Twitch][twitch-shield]][twitch-url]

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://git.neogc.net/heliogrim-workgroup/engine">
    <img src="https://puu.sh/JIMnc/6aa7352720.png" alt="Logo" width="355" height="200" style="margin-top: -32px; margin-bottom: -32px">
  </a>

<h3 align="center">Heliogrim Engine</h3>

  <p align="center">
    <!-- Project Description -->
    Pseudo Next-Gen Experimental Engine
    <br />
    <a href="#"><strong>Explore the docs »</strong></a>
    <br />
    <br />
<!--
    <a href="#">View Demo</a>
    ·
-->
    <a href="https://discord.gg/VzcbyGH8Hk">Chat</a>
    ·
    <a href="https://github.com/Heliogrim/Heliogrim-Engine/issues/new?template=1-bug_report.yml">Report Bug</a>
    ·
    <a href="https://github.com/Heliogrim/Heliogrim-Engine/issues/new?template=2-feature_request.yml">Request Feature</a>
  </p>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project

The [Heliogrim Engine](https://github.com/Heliogrim/Heliogrim-Engine) Project started as a private solution to learn and explore the complex
setup and sub-systems of a general purpose game engine.
While progressing, the whole project underwent multiple re-conceptions and a shift of priorities towards a new goal; Reaching a state of
usability to allow the creation of software
based on the presented solution.

Therefore, as stated later in the [contribution](https://github.com/Heliogrim/Heliogrim-Engine#contributing) section, we highly appreciate
participation and contribution, as the general scope is overwhelming in contrast to the lack of available working capacity.

<!--
Here's a blank template to get started: To avoid retyping too much info. Do a search and replace with your text editor for the following: `github_username`, `repo_name`, `twitter_handle`, `linkedin_username`, `email_client`, `email`, `project_title`, `project_description`
-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

* [![C++][C++]][c++-url]
* [![Vulkan][Vulkan]][vulkan-url]
* [![CMake][CMake]][cmake-url]
* [![CLion][CLion]][clion-url]
* [![Visual Studio][Visual Studio]][vs-url]
* [![Visual Studio Code][Visual Studio Code]][vs-code-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

Install, Build, Run... or the other way round 🤔

### Prerequisites

List of things you need to install to use the software and build the solution.
* Compiler for C++23 ( `gcc` | `clang` | `msvc` )
* CMake ( `cmake` )
* Git + Git LFS ( `git`, `git-lfs` )
* Graphics Driver (Dedicated GPU)

### Installation

1. Clone the repo

   ```sh
   # Via GitLab
   git clone git@git.neogc.net:heliogrim-workgroup/engine.git
   # Or GitHub
   git clone git@github.com:Heliogrim/Heliogrim-Engine.git
   ```

2. Hook/Update Git LFS

   ```sh
   git lfs install
   git lfs fetch
   ```

3. Configuration

    ```sh
    <open project root directory>

    # On Windows
    start .\Options.cmake
    # On Other
    vi | vim | nano Options.cmake
    ```

4. Setup

    ```sh
    cmake -S . -B build/Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug --fresh
    ```

    ```sh
    cmake --build build/Debug -t Heliogrim-Heliogrim --parallel $(nproc)
    cmake --build build/Debug -t Heliogrim-Editor --parallel $(nproc)

    # when editing the engine, we recommend:
    cmake --build build/Debug -t Heliogrim-AllTests --parallel $(nproc)
    ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- USAGE EXAMPLES -->
## Usage

_Sorry, but there is currently no usage description available._

<!-- _For more examples, please refer to the [Documentation](#)_ -->

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

* [GitHub Project](https://github.com/orgs/Heliogrim/projects/1/views/6)

See [open issues](https://track.neogc.net:8443/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please provide such information to us in person by visiting our socials or simply open
an issue with the tag "enhancement".  
Thanks for the support again!

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

As of now, this is only distributed under the private usage license, which will require a personal agreement between the main authors and
you as a consumer of this project.

_We will provide a more permissive and open license as soon as possible. When ready, see `LICENSE` for more information._

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

Julius Richter - [Twitch]([twitch-url]) | [LtJulius#1720](LtJulius#1720)

GitLab Group Link: [https://git.neogc.net/heliogrim-workgroup](https://git.neogc.net/heliogrim-workgroup)  
GitLab Project Link: [https://git.neogc.net/heliogrim-workgroup/engine](https://git.neogc.net/heliogrim-workgroup/engine)

GitHub Group Link: [https://github.com/Heliogrim](https://github.com/Heliogrim)  
GitHub Project Link: [https://github.com/Heliogrim/Heliogrim-Engine](https://github.com/Heliogrim/Heliogrim-Engine)

Bug Tracking : [GitHub Project Bugs](https://github.com/orgs/Heliogrim/projects/1/views/1)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* ...

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->

[contributors-shield]: https://img.shields.io/github/contributors/Heliogrim/Heliogrim-Engine?style=for-the-badge

[contributors-url]: https://github.com/Heliogrim/Heliogrim-Engine

[issues-shield]: https://img.shields.io/github/issues/Heliogrim/Heliogrim-Engine?style=for-the-badge

[issues-url]: https://github.com/Heliogrim/Heliogrim-Engine/issues

[license-shield]: https://img.shields.io/github/license/Heliogrim/Heliogrim-Engine?style=for-the-badge

[license-url]: https://github.com/Heliogrim/Heliogrim-Engine/blob/master/LICENSE
[twitch-shield]: https://img.shields.io/twitch/status/ltjulius?style=for-the-badge
[twitch-shield-flat]: https://img.shields.io/twitch/status/ltjulius
[twitch-url]: https://www.twitch.tv/ltjulius
[C++]: https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white
[c++-url]: https://isocpp.org
[CLion]: https://img.shields.io/badge/CLion-000000?style=for-the-badge&logo=clion&logoColor=white
[clion-url]: https://www.jetbrains.com/clion/
[Visual Studio]: https://img.shields.io/badge/Visual_Studio-5C2D91?style=for-the-badge&logo=visual%20studio&logoColor=white
[vs-url]: https://visualstudio.microsoft.com
[Visual Studio Code]: https://img.shields.io/badge/Visual_Studio_Code-0078D4?style=for-the-badge&logo=visual%20studio%20code&logoColor=white
[vs-code-url]: https://code.visualstudio.com
[CMake]: https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white
[cmake-url]: https://cmake.org
[Vulkan]: https://img.shields.io/badge/Vulkan-AC162C.svg?style=for-the-badge&logo=Vulkan&logoColor=white
[vulkan-url]: https://www.vulkan.org