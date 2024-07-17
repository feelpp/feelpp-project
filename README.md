# Feel++ Template Project 
Christophe Prud’homme &lt;[@prudhomm](https://github.com/prudhomm)>

![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.12762669.svg)
![GitHub Release](https://img.shields.io/github/v/release/feelpp/feelpp-project)
![CI](https://github.com/feelpp/feelpp-project/workflows/CI/badge.svg)

This repository provides a basic starting point for a Feel++ application including:

* [x] Feel++ applications in C++ to use Feel++ and Feel++ toolboxes in `src`
* [x] documentation using asciidoc and antora
* [x] python Feel++ notebooks that can be downloaded from the documentation
* [x] continuous integration including tests for the C++ applications
* [x] docker image generation for the project

The documentation for feelpp-project is available at [here](https://feelpp.github.io/feelpp-project) and you can build on it for your project by enabling the [github pages](https://docs.github.com/en/pages) for your repository.

## Renaming the project

By default the project is named  `feelpp-project` if you cloned the repository `feelpp/feelpp-project`.
However if you used the previous repository as a template, then the project is renamed using the name of the repository using the script `rename.sh` at the initialization of the repository.
If the name does not suit you, you can change it again using the script `rename.sh` and providing the new name as argument.

**⚠️ WARNING**\
the script `rename.sh` will rename the project however some url might be set properly if you rename the project yourself. You need to check the following files: `docs/site.yml` and `docs/package.json` and fix the urls after the rename process is done.

## Updating the feelpp-project  version

The version of the project is defined in the files `CMakeLists.txt`, `docs/antora.yml` and `docs/package.json`. 
You need to update with the same version in all files.

## Release process

* [x] update the version in CMakeLists.txt
* [x] update the version in docs/antora.yml
* [x] commit the changes with the message "Release vx.y.z". At this point the CI will generate the docker image and push it to docker hub
