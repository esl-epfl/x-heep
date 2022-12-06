# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## 0.1.2 - 2022-12-04
### Changelog
- Added make dependencies to auto-setup python env for reconfiguration

### Fixed
- Fix some small issues reported by linter

## 0.1.1 - 2022-10-07
### Changed
- Bumped AXI version to v0.35.3
- Added NumRepetitions to tb_gpio to choose test duration
- Refactored TB

### Fixed
- Fix tx_en inversion bug for open-drain mode 1
- Fix bug in TB that caused open-drain misbehavior not to be catched


## 0.1.0 - 2022-04-14
Initial release
