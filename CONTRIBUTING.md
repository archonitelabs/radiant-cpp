# How to contribute to Radiant

Please read the [Code of Conduct](CODE_OF_CONDUCT.md).

## Reporting Bugs

* Do not open a GitHub issue if the bug is a security vulnerability in Radiant,
  and instead refer to our [security policy](SECURITY.md).
* Ensure the bug was not already reported by searching
  [issues][github.issues.bug].
* If you're unable to find an open issue addressing the problem,
  [open one][github.issues.bug.new].

## Requesting Features

* Feature requests are welcome. But take a moment to find out whether your idea
  fits with the scope of the project. Features always deserve discussions before
  implementation. We strongly encourage opening or joining discussions on
  existing requests.
* Ensure the feature request ("enhancement") was not already requested by
  searching [issues][github.issues.enhancement] and does not
  have an active [discussion][github.discussions].
* If you're unable to find an open issue or discussion:
    * Consider, [starting a discussion][github.discussions] (optional).
    * Otherwise, [open a feature request][github.issues.enhancement.new].

## Development Setup

* Fork the repository on GitHub and clone it.
* Radiant uses Bazel as the primary build system. Install Bazel
  ([official instructions][bazel.install]). [Bazelisk][bazel.bazelisk] is
  recommended.
* Python is required for tooling. Install Python
  ([official instructions][python.install]).
* Radiant uses python package to aid in the remaining setup and subsequent
  workflows. Install it into your python environment by running:
  `python -m pip install -e ./tools/rad`.
* Initialize the development environment: `rad init`.
* Build the project: `rad build`.
* Run the tests: `rad test`.
* Check out `rad --help` for more information. 😎 🎉

### Windows

If you're working on ARM64 Windows, you should use the Bazel binary directly.
There is a [bug in bazilisk](https://github.com/bazelbuild/bazelisk/issues/572)
that ends up using the x64 version on Bazel on ARM64 systems. To work around
this it is recommended to use the Bazel binary directly.

Building with a normal Visual Studio install is possible, but
[Bazel has bugs](https://github.com/bazelbuild/bazel/issues/22164) in their
default toolchain resolution that may cause issues when cross compiling
architectures. To work around this we recommend using the
[EWDK toolchain](https://github.com/0xf005ba11/bazel_ewdk_cc/) instead. It is
already configured as a bzlmod developer dependency for the project.

1. Download and [Windows EWDK][microsoft.ewdk] and mount the ISO.
2. Set `EWDKDIR` environment variable to the EWDK directory.
3. You're all set! You might have to `rad clean --expunge`.

## Pull Requests

* After opening a pull request you will be required to sign the
  [Contributor License Agreement](CLA.md).
* The changes must be tested and covered by unit tests, as appropriate.
* The pull request must pass the required checks.
* Open a new pull request with the patch.
    * Ensure the pull request clearly describes the subject/problem and solution.
    * Include the relevant issue number if applicable.

[//]: # (Hyperlink IDs)
[github.issues.bug]: https://github.com/archonitelabs/radiant-cpp/issues?q=is%3Aissue+label%3Abug+
[github.issues.bug.new]: https://github.com/archonitelabs/radiant-cpp/issues/new?assignees=&labels=bug&projects=&template=01-bug-report.yml
[github.issues.enhancement]: https://github.com/archonitelabs/radiant-cpp/issues?q=is%3Aissue+label%3Aenhancement
[github.issues.enhancement.new]: https://github.com/archonitelabs/radiant-cpp/issues/new?assignees=&labels=enhancement&projects=&template=02-feature-request.yml
[github.discussions]: https://github.com/archonitelabs/radiant-cpp/discussions
[bazel.install]: https://docs.bazel.build/versions/main/install.html
[bazel.bazelisk]: https://bazel.build/install/bazelisk
[python.install]: https://www.python.org/downloads/
[microsoft.ewdk]: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk#download-icon-for-ewdk-enterprise-wdk-ewdk
