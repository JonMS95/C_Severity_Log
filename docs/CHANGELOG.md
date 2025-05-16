# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1] - 16-05-2025
### Added
* Logging thread's TID can now be displayed.
* Added SeverityLogInitWithMask function, which makes severity log initialization function call shorter (less input parameters).

### Changed
* Syslog/Journal information contains more details (such as severity level as text and executable file's name) now.

## [2.0] - 11-05-2025
### Added
* Settings can now be modified. Use functions such as SeverityLogInit (read API).
* Logging to syslog (journal in modern systems) is allowed from now on.
* Logs composed of multiple lines are allowed now.
* Directory names have been changed to follow the new directory/file naming convention.

## [1.3] - 31-07-2024
### Added
* Documentation (license, contribution rules, readme and changelog)

### Changed
* Using latest version of C_Common_shell_files (v1_2)


## [1.2] - 24-11-2023
### Added
* Added visibility modifiers in order to avoid potential naming conflicts in the future


## [1.1] - 02-11-2023
### Added
* **_SetSeverityLogPrintTimeStatus_** API function: sets time and date option


## [1.0] - 15-10-2023
### Added
* First version
