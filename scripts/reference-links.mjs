#!/usr/bin/env node
// reference-links.mjs — check every URL the bibliographies print, and report
// which are dead (verifiable-references M006/S01, VR16).
//
// Before this, nothing fetched anything: research-provenance checks that a
// citation resolves to an anchor and citation-tier checks the declared tier,
// so a Tier-A source could rot to a 404 with every gate green.
//
// Classes (M006-decisions.md):
//   ok       2xx after redirects
//   blocked  401, 403, 406, 429, or a 200 that is a bot challenge — reachable
//            by a person, refused to a script; never reported as dead
//   dead     404, 410, DNS failure, refused or reset connection, or no
//            response within the timeout (retried once first)
//   error    anything else, 5xx included: check by hand, usually transient
//
// Advisory, never a gate: the checker exits 0 whatever it finds, and 2 only
// when it cannot run. A link checker that fails builds gets disabled.

import { isChallengePage } from './fetch-references.mjs';

const USER_AGENT = 'poly-reference-links (+https://github.com/JimAKennedy/poly)';
const BODY_SNIFF_BYTES = 64 * 1024;
const BLOCKED = new Set([401, 403, 406, 429]);
const DEAD = new Set([404, 410]);
const DEAD_CODES = new Set(['ENOTFOUND', 'ECONNREFUSED', 'ECONNRESET', 'EAI_AGAIN']);

export function classifyResponse({ status, body }) {
  if (status >= 200 && status < 300) return isChallengePage(body ?? '') ? 'blocked' : 'ok';
  if (BLOCKED.has(status)) return 'blocked';
  if (DEAD.has(status)) return 'dead';
  return 'error';
}

function errorCode(err) {
  return err?.cause?.code ?? err?.code ?? null;
}

function isNoResponse(err) {
  return err?.name === 'TimeoutError' || err?.name === 'AbortError';
}

export function classifyError(err) {
  if (isNoResponse(err)) return 'dead';
  return DEAD_CODES.has(errorCode(err)) ? 'dead' : 'error';
}

async function sniff(res) {
  if (!res.body) return '';
  const reader = res.body.getReader();
  const chunks = [];
  let size = 0;
  try {
    while (size < BODY_SNIFF_BYTES) {
      const { done, value } = await reader.read();
      if (done) break;
      chunks.push(value);
      size += value.length;
    }
  } finally {
    reader.cancel().catch(() => {});
  }
  return Buffer.concat(chunks).toString('utf8');
}

async function attempt(url, timeoutMs, fetchImpl) {
  const res = await fetchImpl(url, {
    headers: { 'User-Agent': USER_AGENT, Accept: 'text/html,application/pdf;q=0.9,*/*;q=0.8' },
    redirect: 'follow',
    signal: AbortSignal.timeout(timeoutMs),
  });
  const body = res.status >= 200 && res.status < 300 ? await sniff(res) : (res.body?.cancel().catch(() => {}), '');
  return { status: res.status, finalUrl: res.url || url, body };
}

export async function checkUrl(url, { timeoutMs = 20_000, fetchImpl = fetch } = {}) {
  let lastErr;
  for (let attempts = 1; attempts <= 2; attempts += 1) {
    try {
      const { status, finalUrl, body } = await attempt(url, timeoutMs, fetchImpl);
      return { url, class: classifyResponse({ status, body }), status, finalUrl, attempts };
    } catch (err) {
      lastErr = err;
      // Only a no-response earns a retry; a DNS failure or a refusal is an
      // answer, and asking twice would not change it.
      if (!isNoResponse(err)) {
        return { url, class: classifyError(err), error: errorCode(err) ?? err.name, attempts };
      }
    }
  }
  return { url, class: classifyError(lastErr), error: 'no response', attempts: 2 };
}
