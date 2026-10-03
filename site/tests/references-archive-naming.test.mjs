// verifiable-references M003/S01: the archive naming convention, as code.
//
// VR09. Four sources were collected by hand under an `NN - Name.pdf` convention
// and nothing recorded it, so the next file saved took whatever name its host
// gave it (`cohnreich1992.pdf`). The convention is now a function, and every
// archived file the manifest names is held to it.
//
// Numbered entries are `NN - Name.pdf`: the number zero-padded to two digits,
// the Name the lead author's surname, or for an authorless entry its title.
// Further Reading entries are `FR - Surname Year.pdf`, derived from the anchor
// alone — anchors are never reassigned, so the name cannot drift from the
// entry it belongs to.

import { test } from 'node:test';
import assert from 'node:assert/strict';

import { archiveFileName } from '../src/data/references-archive.mjs';
import { parseLine, readBibliography } from '../src/data/references-bibliography.mjs';

test('a numbered entry is named by its lead author', () => {
  assert.equal(
    archiveFileName('ref-1', 'Toussaint, G. T. (2005). "The Euclidean Algorithm Generates Traditional Musical Rhythms."'),
    '01 - Toussaint.pdf',
  );
  assert.equal(
    archiveFileName('ref-46', 'Bjorklund, E. (2003). "The Theory of Rep-Rate Pattern Generation in the SNS Timing System."'),
    '46 - Bjorklund.pdf',
  );
});

test('an author given without initials ends at the first period', () => {
  assert.equal(
    archiveFileName('ref-20', 'Yudane. "Introduction to Balinese Gamelan." *Gamelan New Zealand*.'),
    '20 - Yudane.pdf',
  );
});

test('an authorless entry is named by its title', () => {
  assert.equal(
    archiveFileName('ref-4', '"Rhythm in Sub-Saharan Africa." *Wikipedia*.'),
    '04 - Rhythm in Sub-Saharan Africa.pdf',
  );
});

test('a Further Reading entry is named from its anchor', () => {
  assert.equal(archiveFileName('fr-anku-2000', 'Anku, W. (2000). "Circles and Time."'), 'FR - Anku 2000.pdf');
  assert.equal(
    archiveFileName('fr-polak-london-2014', 'Polak, R., & London, J. (2014).'),
    'FR - Polak London 2014.pdf',
  );
});

test('characters a filesystem refuses are removed and spaces collapsed', () => {
  assert.equal(
    archiveFileName('ref-7', '"Musical Traditions: West / Central Africa?" *Scalar*.'),
    '07 - Musical Traditions West Central Africa.pdf',
  );
});

test('a Name longer than 80 characters is cut at the last space before 80', () => {
  const title = 'Word '.repeat(30).trim();
  const name = archiveFileName('ref-9', `"${title}."`);
  const stem = name.slice('09 - '.length, -'.pdf'.length);
  assert.ok(stem.length <= 80, `stem is ${stem.length} characters`);
  assert.ok(!stem.endsWith(' '), 'stem must not end in a space');
  assert.ok(title.startsWith(stem), 'stem must be a prefix of the title, cut at a word');
});

test('an anchor outside both conventions throws rather than inventing a name', () => {
  assert.throws(() => archiveFileName('see-also-1', 'Anyone.'), /anchor/);
  assert.throws(() => archiveFileName('fr-undated', 'Anyone.'), /anchor/);
});

test('readBibliography spans both bibliographies', async () => {
  const bib = await readBibliography();
  assert.ok(bib.size > 100, `expected more than 100 anchors, got ${bib.size}`);
  assert.ok(bib.has('ref-20'), 'ref-20 lives only in the theory bundle and must be read');
  assert.match(bib.get('ref-20').url, /gamelan\.org\.nz/);
  assert.match(bib.get('ref-20').text, /^Yudane\./);
});

test('where both bibliographies carry an anchor, the shipping appendix wins', async () => {
  const bib = await readBibliography();
  // The appendix links the 2005 BRIDGES paper the entry names; the theory
  // bundle links a different 2007 arXiv paper (M002/S02's mismatch verdict).
  assert.match(bib.get('ref-1').url, /bridgesmathart\.org/);
});

test('an entry with no link has a null url, and its text keeps no markup', async () => {
  const bib = await readBibliography();
  const anku = bib.get('fr-anku-2000');
  assert.equal(anku.url, null);
  assert.match(anku.text, /^Anku, W\. \(2000\)/);
  assert.doesNotMatch(anku.text, /<\/?span|\*\*\[/);
});

// M004/S01: this case read ref-43, a Wikipedia link the citation policy then
// retired. The rule it locks is the parser's, so it now runs on a fixed line.
test('a link whose URL contains parentheses is read whole', () => {
  const [anchor, entry] = parseLine(
    '<span id="ref-99" data-tier="A">**[99]**</span> "X." [Link](https://en.example.org/wiki/A_(b))',
  );
  assert.equal(anchor, 'ref-99');
  assert.equal(entry.url, 'https://en.example.org/wiki/A_(b)');
});

test('an entry offering a PDF link and another link is routed to the PDF', async () => {
  const bib = await readBibliography();
  // ref-34 links JSTOR first (paywalled) and a free PDF second; the
  // open-access verdict is about the PDF.
  assert.match(bib.get('ref-34').url, /unicamp\.br.*\.pdf$/);
});
