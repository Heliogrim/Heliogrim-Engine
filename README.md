<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->

<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
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
    Next-Gen experimental Engine
    <br />
    <a href="https://git.neogc.net/heliogrim-workgroup/engine/-/wikis/home#content-table"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://git.neogc.net/heliogrim-workgroup/engine">View Demo</a>
    ·
    <a href="https://track.neogc.net:8443/newIssue">Report Bug</a>
    ·
    <a href="https://track.neogc.net:8443/newIssue">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

[![Heliogrim Engine][product-screenshot]](https://git.neogc.net/heliogrim-workgroup/engine)

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

Sorry, but there is currently no usage description available.

_For more examples, please refer to the [Documentation](git.neogc.net/heliogrim-workgroup/engine/-/wikis/examples/)_

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ROADMAP -->
## Roadmap

* [YouTrack Desk](https://track.neogc.net:8443/issues)

See the milestones or [open issues](https://track.neogc.net:8443/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please provide such information to us in person by visiting our socials. You can also simply open an issue with the tag "enhancement".  
Thanks for the support again!

<!--
1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request
-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the Private License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTACT -->
## Contact

Julius Richter - [Twitch]([twitch-url]) | [LtJulius#1720](LtJulius#1720)

GitLab Group Link: [https://git.neogc.net/heliogrim-workgroup](https://git.neogc.net/heliogrim-workgroup)  
GitLab Project Link: [https://git.neogc.net/heliogrim-workgroup/engine](https://git.neogc.net/heliogrim-workgroup/engine)

GitHub Group Link: [https://github.com/Heliogrim](https://github.com/Heliogrim)  
GitHub Project Link: [https://github.com/Heliogrim/Heliogrim-Engine](https://github.com/Heliogrim/Heliogrim-Engine)

Bug Tracking : [https://track.neogc.net:8443/issues](https://track.neogc.net:8443/issues)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* ...

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/gitlab/contributors/heliogrim-workgroup/engine?gitlab_url=https%3A%2F%2Fgit.neogc.net&style=for-the-badge
[contributors-url]: https://git.neogc.net/heliogrim-workgroup/engine/-/graphs
[issues-shield]: https://img.shields.io/gitlab/issues/all/heliogrim-workgroup/engine?gitlab_url=https%3A%2F%2Fgit.neogc.net&style=for-the-badge
[issues-url]: https://git.neogc.net/heliogrim-workgroup/engine/-/issues
[license-shield]: https://img.shields.io/gitlab/license/heliogrim-workgroup/engine?gitlab_url=https%3A%2F%2Fgit.neogc.net&style=for-the-badge
[license-url]: https://git.neogc.net/heliogrim-workgroup/engine/-/blob/master/LICENSE
[twitch-shield]: https://img.shields.io/twitch/status/ltjulius?style=for-the-badge
[twitch-shield-flat]: https://img.shields.io/twitch/status/ltjulius
[twitch-url]: https://www.twitch.tv/ltjulius
[product-screenshot]: images/screenshot.png
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