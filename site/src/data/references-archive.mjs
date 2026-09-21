// Where archived reference PDFs live, resolved at read time.
//
// VR05. The archive root differs per machine and is a personal path:
// check-personal-paths rejected the absolute form in the verifiable-references
// vision's own first draft. So the manifest stores a bare filename and nothing
// else, and the root arrives from the environment here. There is no form of the
// tracked data that could carry someone's home directory into git.
//
// Set POLY_REFERENCES_ARCHIVE to point at wherever the PDFs actually are — a
// Dropbox folder, an external drive. Unset, it falls back to a gitignored
// .references/ in the project root, so the repo works out of the box without
// anyone having to configure anything to run the tests.

import { fileURLToPath } from 'node:url';
import { dirname, join, isAbsolute } from 'node:path';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = join(HERE, '..', '..', '..');

export function archiveRoot() {
  const fromEnv = process.env.POLY_REFERENCES_ARCHIVE;
  // A whitespace-only value falls back rather than resolving to an empty root:
  // an exported-but-empty variable is a configuration mistake, not a request to
  // treat the filesystem root as the archive.
  if (typeof fromEnv === 'string' && fromEnv.trim() !== '') return fromEnv;
  return join(REPO_ROOT, '.references');
}

// A manifest entry names a file in the archive, never a path to one. Anything
// with a separator or a parent traversal is rejected rather than normalised:
// silently accepting `../../etc/passwd` and then cleaning it up would make the
// manifest's contract "a path we will try to make safe" instead of "a name".
export function resolveArchiveFile(name) {
  if (typeof name !== 'string' || name.trim() === '') {
    throw new Error('archive file must be a bare filename, got an empty value');
  }
  if (/[\\/]/.test(name) || name.split(/[\\/]/).includes('..') || isAbsolute(name)) {
    throw new Error(
      `archive file must be a bare filename with no path separators: ${JSON.stringify(name)}`,
    );
  }
  return join(archiveRoot(), name);
}
