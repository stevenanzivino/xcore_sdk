# Continuous Integration Tools

## Building Docker container

To build the docker container locally, run the following command:

    $ docker build -t ghcr.io/xmos/xcore_sdk_builder:develop .

To run the container:

    $ docker run -it ghcr.io/xmos/xcore_sdk_builder:develop

## Running Source Checks

Source and license checks must currently be performed manually.  The `infr` tools must be installed first.

To install te `infr` tools, run:

    $ pip install --src infr -r inf_requirements.txt

### Source Check

First copy the ignore list to the root of the repository

    $ cp tools/ci/.xmos_ignore_source_check {path to root of repository}

To run the check

    $ xmos_source_check check {path to root of repository} xmos_public_v1

To run the update 

    $ xmos_source_check update {path to root of repository} xmos_public_v1

### License Check

To run the check

    $ xmos_license_check check {path to root of repository} xmos_public_v1
