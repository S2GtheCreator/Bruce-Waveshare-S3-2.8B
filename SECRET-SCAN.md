# Publication secret scan

Date: 2026-10-23.

The original local history (7 commits, 1,582 unique file blobs), original source ZIP (1,569 entries), original source tree and top-level reports were scanned for common GitHub tokens, AWS access IDs, Google API keys, Slack tokens, private-key headers and credential-bearing HTTP URLs. No credential-signature finding was identified in that history/archive scan. The ZIP's tracked source matched the original snapshot; three local editor files were intentionally absent.

The sanitized public source set and the 4,633 files in the exact dependency-source archive were scanned again. One dependency candidate was manually inspected: LibSSH-ESP32/src/libssh/pki_priv.h defines literal RSA/EC/OpenSSH key-format delimiters for its parser, without a private key body. This is recorded as a false positive in RELEASE-VALIDATION.json. No actual private credential was identified by the checks.

Generic password/API-token configuration candidates were inspected: example placeholders/default configuration, not user credentials. Device flash backups, NVS/configuration files, Git histories of dependencies, original firmware, old source ZIP and development bundle are not published. Local home/project paths in the published build log were replaced with placeholders. Git commits use a GitHub noreply identity.

This is a pattern-based scan with manual inspection of candidates, not proof that every possible secret format is absent. Embedded protocol manufacturer data was treated separately from personal credentials and removed because its redistribution rights were not established. No excluded database/key material is published in the fresh history; binary absence checks are recorded in RELEASE-VALIDATION.json.
