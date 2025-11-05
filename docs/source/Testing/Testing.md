# Automatic testing

X-HEEP includes a script to perform automatic tests on your modifications. In addition, it also has a CI setup that checks the code by simulating all the existing applications and handles publishing new X-HEEP releases.

## Simulation script

The testing script (`test/test_apps/test_apps.py`) can be used to perform local tests. For quick
debugging, you can check the global variables in the script such as the `BLACKLIST` and `WHITELIST`.

You can run it with the following command:

```bash
make test
```

This will output the results in the terminal and in the `test/test_apps/test_apps.log` file.

Additionally, you can check only the compilation of the applications with the following command:

```bash
make test TEST_FLAGS=--compile-only
```

This script is also integrated in the CI workflow described in the following section.

## Github CIs

The project's Continuous Integration (CI) is managed through GitHub Actions. The workflows are defined in the `.github/workflows` directory. The main CI workflow is `ci.yml`, which is triggered on every push and pull request to the `main` branch.

### CI Workflow (`ci.yml`)

This workflow ensures the stability and integrity of the codebase by running a series of checks, compilations, and simulations.

**Triggers:**

*   Push to any branch (`push: branches: [ "**" ]`).
*   Pull request to the `main` branch (`pull_request: branches: [ "main" ]`).

**Jobs:**

1.  **`determine-image-tag`**:
    *   **Purpose**: Determines the Docker image tag to be used by subsequent jobs.
    *   **Details**: It checks the Git history for the most recent tag. If no tag is found in the current branch, it looks for one in the `main` branch. If no tags are found at all, it defaults to `latest`. This ensures that the CI always uses a relevant toolchain version.

2.  **`compile-apps`**:
    *   **Purpose**: Compiles all software applications with both GCC and Clang to ensure they build correctly.
    *   **Dependencies**: Depends on `determine-image-tag` to select the correct Docker image.
    *   **Environment**: Runs inside the `ghcr.io/esl-epfl/x-heep/x-heep-toolchain` Docker container.
    *   **Steps**:
        *   Generates the MCU configuration using `make mcu-gen X_HEEP_CFG=configs/ci.hjson`.
        *   Executes `test/test_apps/test_apps.py` with the `--compile-only` flag to build all applications, without simulating them. This is done to offer a quick feedback about the apps' integrity, before their runtime behaviour is checked in RTL simulation.

3.  **`simulate-apps`**:
    *   **Purpose**: Runs Verilator RTL simulations for all applications (except the blacklisted ones) to verify their runtime behavior.
    *   **Condition**: This job only runs on pull requests to `main`.
    *   **Dependencies**: Depends on `determine-image-tag`.
    *   **Environment**: Runs inside the `x-heep-toolchain` Docker container.
    *   **Steps**:
        *   Generates the MCU configuration using `make mcu-gen X_HEEP_CFG=configs/ci.hjson`.
        *   Executes `test/test_apps/test_apps.py` to compile and simulate all applications.

4.  **`lint`**:
    *   **Purpose**: Checks that all auto-generated hardware files are up-to-date and have been formatted .
    *   **Dependencies**: Depends on `determine-image-tag`.
    *   **Environment**: Runs inside the `x-heep-toolchain` Docker container.
    *   **Steps**:
        *   Runs `make mcu-gen` to regenerate all hardware files.
        *   Uses `util/git-diff.py` to check for any differences between the working directory and the git HEAD. The job fails if any differences are found.

5.  **`gen-peripherals`**:
    *   **Purpose**: Tests the Python-based peripheral generation scripts and templates.
    *   **Dependencies**: Depends on `determine-image-tag`.
    *   **Environment**: Runs inside the `x-heep-toolchain` Docker container.
    *   **Steps**:
        *   Runs `make clean-all` to ensure a clean state.
        *   Executes `test/test_x_heep_gen/test_peripherals.py`.

6.  **`check-vendor`**:
    *   **Purpose**: Verifies that all third-party vendored dependencies are up-to-date.
    *   **Environment**: Runs inside a `ubuntu-latest` VM.
    *   **Steps**:
        *   Installs Python dependencies.
        *   Runs the `util/vendor.py` script for all `.vendor.hjson` files to re-vendor all dependencies.
        *   Uses `util/git-diff.py` to check for any differences, ensuring that any changes to vendored repositories are properly committed.

7.  **`black-formatter`**:
    *   **Purpose**: Checks that all Python code adheres to the `black` formatting standard.
    *   **Environment**: Runs inside a `ubuntu-latest` VM.
    *   **Steps**:
        *   Uses the `psf/black` GitHub Action to check the formatting of all relevant Python files.

### Release Workflows

The CI is also responsible for handling releases. There are two workflows for this purpose: `create-release.yml` and `publish-release.yml`. This automated process ensures that every release is consistently built and published with its corresponding toolchain and Docker image.

#### Create X-HEEP Release Workflow (`create-release.yml`)

This workflow is responsible for preparing a new release. It is triggered manually and requires a release tag (e.g., `v1.0.0`) as input parameter.

**Trigger:**

*   Manual dispatch (`workflow_dispatch`) with a `release_tag` input. This can be achieved by running the _Create X-HEEP Release_ workflow from the X-HEEP repository Actions tab on GitHub.

```{note}
This workflow is meant to be executed only when major changes are merged to `main`. An example is new tools versions, breaking changes, etc. Bug fixes that don't require rebuilding the toolchain or the Docker container, or do not represent a major update, don't necessarily require a new release to be created.
```

**Jobs:**

1.  **`prepare-release-branch`**:
    *   Creates a new branch named `release/vX.Y.Z` from the `main` branch.
    *   Updates the version number in `core-v-mini-mcu.core` and `util/docker/dockerfile` to match the release tag.
    *   Commits and pushes the version update to the new release branch.
    *   Opens a pull request from the release branch to `main`.

2.  **`build-toolchain`**:
    *   Runs in parallel with `prepare-release-branch`.
    *   Builds the RISC-V GCC and Clang/LLVM toolchains.
    *   Archives the compressed toolchain binaries as a workflow artifact named `x-heep-toolchain`.

3.  **`build-and-push-docker`**:
    *   Depends on the completion of `build-toolchain`.
    *   Builds the `x-heep-toolchain` Docker image, which includes the newly built toolchains and all other necessary dependencies (Verilator, Verible, etc.).
    *   Pushes the Docker image to the GitHub Container Registry (`ghcr.io/esl-epfl/x-heep/x-heep-toolchain`) with a tag corresponding to the release version.

4.  **`create-draft-release`**:
    *   Depends on the completion of `build-and-push-docker`.
    *   Creates a draft release on GitHub associated with the specified tag.
    *   The release notes are automatically generated based on the commit history since the last release.
    *   Downloads the `x-heep-toolchain` artifact and attaches it as a `.tar.gz` file to the draft release.

#### Publish Release Workflow (`publish-release.yml`)

This workflow finalizes the release process after the release pull request has been merged and the draft release has been manually published.

**Trigger:**

*   On a release being `published`, that is a merged PR from a `release/*` branch. Such PR should be automatically created by the _Create X-HEEP Release_ workflow detailed above.

**Jobs:**

1.  **`publish-docker-image`**:
    *   Pulls the Docker image for the release version from GHCR.
    *   Re-tags the image with the `latest` tag.
    *   Pushes the `latest` tag to GHCR. This ensures that the main CI workflow will use the most up-to-date toolchain for its runs.
