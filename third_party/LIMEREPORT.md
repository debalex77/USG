# LimeReport

USG uses LimeReport as a dynamically linked third-party dependency.
USG also embeds 17 byte-identical LimeReport interface images from
`demo_r2/images`; their inventory is recorded in `THIRD_PARTY_ICONS.md`.

## Pinned source

- Upstream project: <https://github.com/fralx/LimeReport>
- Upstream release tag: `1.7.23`
- Pinned Git commit: `e6210b58a46ab14c0ff8a409b5fa5d339937ea7c`
- Required Qt version for the USG build: `6.9.3`
- Build system: qmake

The release build must use the pinned commit, not the moving tip of a branch.
Compiled LimeReport libraries and copied headers are build products and are not
stored in the USG Git repository.

The automated reference build is defined in
`.github/workflows/build-linux.yml`. It downloads that exact commit, applies
the versioned patch, builds LimeReport and then builds USG. No database or
application configuration is used by this job.

## USG patch

Before building LimeReport, apply:

`patches/limereport/1.7.23/0001-make-singleton-destruction-idempotent.patch`

The patch makes the LimeReport singleton shutdown idempotent. It prevents the
same singleton from being deleted both from `QCoreApplication::aboutToQuit` and
again from the `atexit` handler, which previously caused heap corruption while
USG was closing.

Keep this patch separate from the upstream source. On every LimeReport upgrade:

1. pin the new upstream tag and commit;
2. check whether the patch is still necessary;
3. rebase or remove the patch;
4. build and test USG shutdown, PDF preview/export, Designer and e-mail export;
5. update this file and the release notes.

## License files

The upstream source contains its applicable license texts in `LICENSE` and
`COPYING`. The CI artifact preserves both files, the exact unmodified upstream
source archive and the USG patch next to the LimeReport libraries. A
distributed USG package must also preserve the applicable LimeReport license
text, notices and corresponding source or an otherwise compliant source offer.

This inventory documents how the dependency is obtained and built; it is not a
substitute for legal advice.
