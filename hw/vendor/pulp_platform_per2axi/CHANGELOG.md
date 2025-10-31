# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## 1.0.4 - 2020-04-23
### Fixed
- Fix handshake on `per_slave` port that could cause transactions to be lost if the downstream AW
  channel was ready but the W channel was not.

## 1.0.2 - 2018-09-12
### Changed
- Clean-up Bender dependencies

## 1.0.1 - 2018-03-16
### Changed
- Open source release.

### Added
- Initial commit.