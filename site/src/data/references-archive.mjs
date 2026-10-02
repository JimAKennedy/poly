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

// verifiable-references M003/S01: what retrieval has done for each record.
// `archived` holds exactly when the record names an archiveFile; the other
// states say why it does not — refused by a host's bot policy and queued for a
// person, free only as an unsearchable scan, free only through an institution,
// nothing to archive because the source is a video, judged unsuitable by the
// owner and left for M004 to replace (M003/S02), or not yet tried.
export const RETRIEVAL_STATUS = [
  'archived',
  'script-refused',
  'scan-only',
  'institution-only',
  'no-text',
  'to-replace',
  'not-attempted',
];

const NAME_MAX = 80;

function cleanName(raw) {
  let name = raw.replace(/[\\/:*?"<>|]/g, '').replace(/\s+/g, ' ').trim();
  if (name.length > NAME_MAX) {
    const cut = name.lastIndexOf(' ', NAME_MAX);
    name = name.slice(0, cut > 0 ? cut : NAME_MAX).trim();
  }
  return name;
}

// The archive naming convention (VR09). `text` is the entry as
// readBibliography() returns it; Further Reading names come from the anchor
// alone, so `text` is only read for numbered entries.
export function archiveFileName(anchor, text) {
  const numbered = /^ref-(\d+)$/.exec(anchor);
  if (numbered) {
    const nn = numbered[1].padStart(2, '0');
    const t = String(text ?? '').trim();
    let lead;
    if (t.startsWith('"')) {
      const close = t.indexOf('"', 1);
      lead = (close > 0 ? t.slice(1, close) : t.slice(1)).replace(/\.$/, '');
    } else {
      const end = t.search(/,|\.|\s\(/);
      lead = end > 0 ? t.slice(0, end) : t;
    }
    const name = cleanName(lead);
    if (name === '') throw new Error(`cannot derive a name for anchor ${anchor} from ${JSON.stringify(text)}`);
    return `${nn} - ${name}.pdf`;
  }
  const further = /^fr-([a-z]+(?:-[a-z]+)*)-(\d{4})$/.exec(anchor);
  if (further) {
    const parts = further[1].split('-').map((p) => p[0].toUpperCase() + p.slice(1));
    return `FR - ${parts.join(' ')} ${further[2]}.pdf`;
  }
  throw new Error(`anchor ${JSON.stringify(anchor)} is neither ref-N nor fr-<surname>-<year>`);
}
