# Contributing to UME

First off, thank you for considering contributing to the Unified Memory Engine (UME)! It's people like you that make UME such a great tool for AI memory virtualization.

## Development Setup

To set up a local development environment:

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/ume-project/ume.git
    cd ume
    ```

2.  **Ensure you have the prerequisites:**
    *   CMake >= 3.25
    *   C++20 compatible compiler (Clang, GCC, MSVC)
    *   (Optional) GPU SDKs for specialized backends (CUDA, HIP, SYCL)

3.  **Build the project:**
    ```bash
    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -j $(nproc)
    ```

4.  **Run tests:**
    ```bash
    ctest --output-on-failure
    ```

## Branch Naming Convention

We use a structured branch naming convention to help track work. Please use one of the following prefixes:

*   `feature/` - For new features (e.g., `feature/cuda-backend`)
*   `fix/` - For bug fixes (e.g., `fix/memory-leak-event-store`)
*   `perf/` - For performance improvements
*   `docs/` - For documentation changes
*   `refactor/` - For code refactoring that doesn't change behavior

## Pull Request Process

1.  Ensure your branch is up-to-date with `main`.
2.  Run the formatting tool (`cmake --build . --target format`).
3.  Ensure all tests pass locally.
4.  Submit a Pull Request targeting the `main` branch.
5.  Fill out the PR template thoroughly.
6.  Wait for CI to pass and a maintainer to review your code.

## Commit Message Format

We follow the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) specification. This helps in generating automated changelogs.

Format:
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

Examples:
*   `feat(core): implement initial event store architecture`
*   `fix(cuda): resolve race condition in asynchronous allocation`
*   `docs(readme): add build instructions for Windows`

Common types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`.

## Code Review Expectations

All submissions, including submissions by project members, require review. We use GitHub pull requests for this purpose. 

Reviewers will look for:
*   Adherence to C++20 standards and project coding guidelines.
*   Correctness and robustness of the implementation.
*   No exceptions in core code (must use Status/Result types).
*   Adequate test coverage.
*   Clear Doxygen documentation for all new public APIs.

## Testing Requirements

Before submitting a PR, ensure:
1.  **Unit Tests:** You have added unit tests for new functionality.
2.  **Regression Tests:** Existing tests must pass.
3.  **No Warnings:** Code should compile without warnings on all major compilers.

## RFC Process

For major architectural changes, new core features, or significant modifications to the API, we require an RFC (Request for Comments). 

Please see the [RFC Template](../rfcs/0000-rfc-template.md) for how to write an RFC, and submit it to the `docs/rfcs/` directory via a PR before implementing the change.
