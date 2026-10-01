# Contributing to USG

Thank you for your interest in contributing to USG.

USG is a Qt/C++ application intended for managing ultrasound examination workflows, reports, patient records, printing, and related clinical data.

## Reporting Bugs

Before opening a new issue, please check whether the problem has already been reported.

When reporting a bug, please include:

- USG version;
- operating system and version;
- Qt version, when relevant;
- database backend used (SQLite or MariaDB), when relevant;
- steps required to reproduce the problem;
- expected behavior;
- actual behavior;
- relevant logs or screenshots.

Do not include real patient data, credentials, database contents, private keys, access tokens, or other sensitive information.

For security vulnerabilities, please follow the instructions in [SECURITY.md](SECURITY.md) instead of opening a public issue.

## Development

USG is primarily developed using:

- C++ and Qt 6;
- qmake;
- SQLite and MariaDB;
- LimeReport for report generation and printing.

Development is primarily performed and tested on Linux, with release builds also provided for Windows.

## Building

Clone the repository:

    git clone https://github.com/debalex77/USG.git
    cd USG

Make sure the required Qt development environment and project dependencies are installed before building.

Build instructions and required dependencies may change between releases. Please consult the project documentation and build configuration files for the current requirements.

## Making Changes

When contributing code:

1. Create a separate branch for your change.
2. Keep changes focused on a single issue or feature.
3. Follow the existing coding style and project structure.
4. Avoid unrelated formatting or refactoring.
5. Make sure the project builds successfully before submitting your changes.
6. Test the affected functionality where possible.

## Commit Messages

Use short and descriptive commit messages.

Examples:

    fix: prevent order journal crash on empty result
    feat: add new report printing option
    docs: update installation instructions
    refactor: simplify database initialization

## Pull Requests

Before submitting a pull request:

- make sure your branch is up to date;
- verify that the project builds successfully;
- describe what was changed and why;
- mention any relevant issue;
- describe how the change was tested;
- do not include generated files or unrelated changes unless they are required.

Pull requests may be reviewed and changes may be requested before merging.

## Translations

Changes and improvements to translations are welcome.

When modifying translations, keep the source strings and translation files synchronized and verify the affected user interface where possible.

## Medical and Patient Data

Never use real patient information in:

- issues;
- pull requests;
- screenshots;
- test databases;
- logs;
- examples;
- commits.

Use anonymized or synthetic data for development, testing, and bug reports.

## License

By contributing to USG, you agree that your contributions will be distributed under the license used by this repository.
