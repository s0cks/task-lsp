# Changelog

## [1.6.0](https://github.com/s0cks/task-lsp/compare/taskfile-lsp-v1.5.1...taskfile-lsp-v1.6.0) (2026-09-11)


### Features

* ✨ cleanup versioning and build process around it ([dbf0bdb](https://github.com/s0cks/task-lsp/commit/dbf0bdbd57747b3e6d818de2e81d7ca9bb23b629))
* ✨ first commit ([a837188](https://github.com/s0cks/task-lsp/commit/a83718880bdc15f23af89b6516f5b51949e18645))
* **cli:** ✨ cleanup version command and add info command ([11d569d](https://github.com/s0cks/task-lsp/commit/11d569dcd7025d9d6b2c7494457b658f48d21d5a))
* **cli:** ✨ get version from the build ([7e171c3](https://github.com/s0cks/task-lsp/commit/7e171c3ac5086b6af181d9396ecc5325e5ed4c20))
* **lexer:** ✨ add desc support with quoted and unquoted strings ([493010e](https://github.com/s0cks/task-lsp/commit/493010e9fa1a0eb01d9f9512fe58cd65b7c5f15e))
* **lexer:** ✨ lex tasks: and vars: ([1c2558c](https://github.com/s0cks/task-lsp/commit/1c2558c27b12966e4fef5458ef2f11097d115bd5))
* **lsp:** ✨ add basic variable support ([80e0326](https://github.com/s0cks/task-lsp/commit/80e0326ceba700180353554badfe4bb6d956b38a))
* **lsp:** ✨ add some cleanup and some task refactor code actions for adding fields ([d0c7f04](https://github.com/s0cks/task-lsp/commit/d0c7f043f85ace8f09a9aa21a318f193138908e3))
* **parser:** ✨ add seq types ([3389e21](https://github.com/s0cks/task-lsp/commit/3389e211c1e9176833db6472f64dbf00ed0a0dc7))
* **parser:** ✨ cleanup some parse logic and implement some symbols in the lexer ([94f9284](https://github.com/s0cks/task-lsp/commit/94f9284f358465bed0abf712a523b4ff12d07439))
* **parser:** ✨ initial commit for C based parser ([a448257](https://github.com/s0cks/task-lsp/commit/a4482578715d82c5693fa727743dc865c4f0fef4))
* **vscode:** ✨ add vscode extension ([3b355c6](https://github.com/s0cks/task-lsp/commit/3b355c65c7a7963dac2a693b0dbf310a7789b694))


### Bug Fixes

* 🐛 hopefully fix a bunch of stuff with the release workflow ([493f5bc](https://github.com/s0cks/task-lsp/commit/493f5bcb0f32e9bbe88750b2e82c9d5495d59a2c))
* **build-env:** 🐛 *test* release workflow again ([827a5a9](https://github.com/s0cks/task-lsp/commit/827a5a945484142906a309d230064c7811edeaa5))
* **build-env:** 🐛 *test* the release workflow ([94948f8](https://github.com/s0cks/task-lsp/commit/94948f81bb1a4ae96b69e090d65649b5b0d67133))
* **build-env:** 🐛 fix release tag format ([7e001e0](https://github.com/s0cks/task-lsp/commit/7e001e032c801ee44d9f98fa3adc6e989f4f389f))
* **build-env:** 🐛 hopefully fix release-please process with build-env ([8dd5f30](https://github.com/s0cks/task-lsp/commit/8dd5f30c894125523440dab0ae53e370078986ac))
* **lsp:** 🐛 fix references working ([870a029](https://github.com/s0cks/task-lsp/commit/870a02911c52ac8dc542a421b933bc2c8a79c743))
* **meson:** 🐛 fix meson min dep ([8872935](https://github.com/s0cks/task-lsp/commit/88729355e3982d5c732ca72d0587985c21a8a43f))
* **meson:** 🐛 hopefully fix meson C versioning for CI ([46176d7](https://github.com/s0cks/task-lsp/commit/46176d718e647334422276be6ee8a943fe6058fb))
* **release-please:** 🐛 hopefully fix release-please workflow ([64e3d11](https://github.com/s0cks/task-lsp/commit/64e3d11f10c5d78f6d8ef56ee5292a40ca81c1dd))
* **vscode:** 🐛 attempt to fix pnpm versioning again ([793f262](https://github.com/s0cks/task-lsp/commit/793f262ca7fa5a4e48ae867047f7253c6b78ea74))
* **vscode:** 🐛 fix pnpm locking? ([f11de05](https://github.com/s0cks/task-lsp/commit/f11de05e5b001bff2d77dc4a3694fecddb9b85cc))
* **vscode:** 🐛 fix vscode plugin pnpm packageManager issue ([8f1283c](https://github.com/s0cks/task-lsp/commit/8f1283c39585c00cdda909837383379fcc7e23dd))
* **vscode:** 🐛 try to fix pnpm versioning again ([b3b3c7b](https://github.com/s0cks/task-lsp/commit/b3b3c7b2f48f50697c7ff9096c6a48f72e169ecf))


### Documentation

* 📚 bump docs version to force a release ([fea4f90](https://github.com/s0cks/task-lsp/commit/fea4f90b8c9de8670d869a1e0e0d589319c77b41))
* 📚 bump version in docs package ([32e4959](https://github.com/s0cks/task-lsp/commit/32e4959ba3f8469d02f215fadeba7dc53ce07c32))
* 📚 hopefully this fixes the docs versioning ([fe4c0b7](https://github.com/s0cks/task-lsp/commit/fe4c0b7c5ad7c5de77b3c0af1359bf21e4d2a51c))
* **README:** 📚 a little bit more README cleanup ([89fb633](https://github.com/s0cks/task-lsp/commit/89fb63380b8adf74a8d53ff8b505dd2b42601c58))
* **README:** 📚 add another credit to the README ([8070885](https://github.com/s0cks/task-lsp/commit/80708858ab89de2cd16ea22a29dcfde1a729a1f7))
* **README:** 📚 add demo gif ([155819c](https://github.com/s0cks/task-lsp/commit/155819cd9c5bf8aab759074f011526acf7917496))
* **README:** 📚 cleanup README a bit ([0a1961f](https://github.com/s0cks/task-lsp/commit/0a1961f2de8cb9c62e133fe64724e41d020b327e))
* **README:** 📚 cleanup README a bunch ([0f10e06](https://github.com/s0cks/task-lsp/commit/0f10e064ee5a81a6a807c3e7c4a9a7d472e25c90))
* **README:** 📚 cleanup README more ([de236f6](https://github.com/s0cks/task-lsp/commit/de236f6bce634606b3c80c6505727fa6ca7cb822))
* **README:** 📚 update README ([2df9ec6](https://github.com/s0cks/task-lsp/commit/2df9ec64f338a303a594c828feb0a6878d689bd2))
* **vale:** 📚 cleanup vale vocabs ([da3e964](https://github.com/s0cks/task-lsp/commit/da3e964912399fe335976adadf759b2939dc9de1))
* **vale:** 📚 update vale.ini to use Google + write-good styles ([de2f72c](https://github.com/s0cks/task-lsp/commit/de2f72cf15efdc5adfbd18716b4900c1328d1143))

## [1.5.1](https://github.com/s0cks/task-lsp/compare/v1.5.0...v1.5.1) (2026-09-11)


### Bug Fixes

* **release-please:** 🐛 hopefully fix release-please workflow ([64e3d11](https://github.com/s0cks/task-lsp/commit/64e3d11f10c5d78f6d8ef56ee5292a40ca81c1dd))

## [1.5.0](https://github.com/s0cks/task-lsp/compare/v1.4.1...v1.5.0) (2026-09-11)


### Features

* **lexer:** ✨ add desc support with quoted and unquoted strings ([493010e](https://github.com/s0cks/task-lsp/commit/493010e9fa1a0eb01d9f9512fe58cd65b7c5f15e))
* **lexer:** ✨ lex tasks: and vars: ([1c2558c](https://github.com/s0cks/task-lsp/commit/1c2558c27b12966e4fef5458ef2f11097d115bd5))
* **lsp:** ✨ add some cleanup and some task refactor code actions for adding fields ([d0c7f04](https://github.com/s0cks/task-lsp/commit/d0c7f043f85ace8f09a9aa21a318f193138908e3))
* **parser:** ✨ add seq types ([3389e21](https://github.com/s0cks/task-lsp/commit/3389e211c1e9176833db6472f64dbf00ed0a0dc7))
* **parser:** ✨ cleanup some parse logic and implement some symbols in the lexer ([94f9284](https://github.com/s0cks/task-lsp/commit/94f9284f358465bed0abf712a523b4ff12d07439))


### Bug Fixes

* **lsp:** 🐛 fix references working ([870a029](https://github.com/s0cks/task-lsp/commit/870a02911c52ac8dc542a421b933bc2c8a79c743))
* **meson:** 🐛 fix meson min dep ([8872935](https://github.com/s0cks/task-lsp/commit/88729355e3982d5c732ca72d0587985c21a8a43f))

## [1.4.1](https://github.com/s0cks/task-lsp/compare/v1.4.0...v1.4.1) (2026-09-10)


### Bug Fixes

* **vscode:** 🐛 fix pnpm locking? ([f11de05](https://github.com/s0cks/task-lsp/commit/f11de05e5b001bff2d77dc4a3694fecddb9b85cc))

## [1.4.0](https://github.com/s0cks/task-lsp/compare/v1.3.0...v1.4.0) (2026-09-10)


### Features

* **cli:** ✨ cleanup version command and add info command ([11d569d](https://github.com/s0cks/task-lsp/commit/11d569dcd7025d9d6b2c7494457b658f48d21d5a))

## [1.3.0](https://github.com/s0cks/task-lsp/compare/v1.2.2...v1.3.0) (2026-09-10)


### Features

* **cli:** ✨ get version from the build ([7e171c3](https://github.com/s0cks/task-lsp/commit/7e171c3ac5086b6af181d9396ecc5325e5ed4c20))
* **parser:** ✨ initial commit for C based parser ([a448257](https://github.com/s0cks/task-lsp/commit/a4482578715d82c5693fa727743dc865c4f0fef4))


### Bug Fixes

* **vscode:** 🐛 attempt to fix pnpm versioning again ([793f262](https://github.com/s0cks/task-lsp/commit/793f262ca7fa5a4e48ae867047f7253c6b78ea74))

## [1.2.2](https://github.com/s0cks/task-lsp/compare/v1.2.1...v1.2.2) (2026-09-09)


### Bug Fixes

* **vscode:** 🐛 try to fix pnpm versioning again ([b3b3c7b](https://github.com/s0cks/task-lsp/commit/b3b3c7b2f48f50697c7ff9096c6a48f72e169ecf))

## [1.2.1](https://github.com/s0cks/task-lsp/compare/v1.2.0...v1.2.1) (2026-09-09)


### Bug Fixes

* **vscode:** 🐛 fix vscode plugin pnpm packageManager issue ([8f1283c](https://github.com/s0cks/task-lsp/commit/8f1283c39585c00cdda909837383379fcc7e23dd))

## [1.2.0](https://github.com/s0cks/task-lsp/compare/v1.1.0...v1.2.0) (2026-09-09)


### Features

* **vscode:** ✨ add vscode extension ([3b355c6](https://github.com/s0cks/task-lsp/commit/3b355c65c7a7963dac2a693b0dbf310a7789b694))

## [1.1.0](https://github.com/s0cks/task-lsp/compare/v1.0.0...v1.1.0) (2026-09-09)


### Features

* **lsp:** ✨ add basic variable support ([80e0326](https://github.com/s0cks/task-lsp/commit/80e0326ceba700180353554badfe4bb6d956b38a))

## 1.0.0 (2026-09-09)


### Features

* ✨ first commit ([a837188](https://github.com/s0cks/task-lsp/commit/a83718880bdc15f23af89b6516f5b51949e18645))
