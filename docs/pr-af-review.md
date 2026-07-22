---
class: gated
---

# PR-AF Review — Opt-in Deep Architectural Audit

Status: trial (2026-07-09). Adds a label-gated GitHub Actions workflow that
runs [Agent-Field's PR-AF](https://github.com/Agent-Field/pr-af) as a second
opinion beyond our NFR review and `/code-review ultra` stack.

## What it is

PR-AF spawns dimension-specific reviewer agents (correctness, design, security,
etc.), grounds findings in extracted source evidence, and posts inline PR
comments. It topped the Martian Code-Review-Bench benchmark but is explicitly
built as a **CI gate**, not fast inner-loop feedback — expect **35–50 minutes
per run**.

We wired it as **opt-in only**: it does nothing unless a maintainer adds the
`pr-af` label to a PR. It is not on any required-checks list, and it should
not be added to one.

## Budget & cost — read this first

- **Runtime**: 35–50 min typical. Workflow timeout is 75 min.
- **Cost per run**: OpenRouter tokens, default ceiling **$2.00 USD** per PR
  (from PR-AF's own `PR_AF_MAX_COST_USD` default). Higher if the reviewer
  is configured to use a frontier model tier.
- **Concurrency**: each labeled PR spins up its own AgentField Docker stack.
  Multiple labels applied close together will run in parallel and stack the
  cost.
- **Do not** add `pr-af-review` to branch protection required checks. It is
  slow and paid; a required check would block every merge for 45 min.

Rule of thumb: use the label when the PR touches architecture, engine
internals, RT-safety, or crosses subsystem boundaries. Skip it for CSS-only,
docs-only, or single-line fixes.

## How to trigger a review

1. Open (or already have open) a PR.
2. In the PR's right sidebar, click **Labels**.
3. Check the `pr-af` label. (If it isn't in the list, see "One-time setup"
   below.)
4. That's it — GitHub fires the `pull_request` event with `types: [labeled]`,
   the workflow's `if` gate matches on `label.name == 'pr-af'`, and the run
   starts within a minute or two.
5. To watch progress: **Actions** tab → **PR-AF Review (opt-in)** → the most
   recent run.
6. Findings land as inline PR comments authored by the workflow's
   `GITHUB_TOKEN` bot identity when the run finishes.

Removing the label after the run has started does **not** cancel it. Cancel
manually from the Actions tab if you need to stop it.

## One-time setup — foolproof steps

You must do both of these once before the first labeled PR will work.

### 1. Add the `OPENROUTER_API_KEY` repo secret

The reviewer agent calls OpenRouter for LLM inference. Without this secret
the workflow starts, spins up Docker, and then fails when the agent tries to
authenticate.

**Get the key:**

1. Go to https://openrouter.ai/ and sign in (or sign up).
2. Click your avatar (top right) → **Keys**.
3. Click **Create Key**. Give it a name like `poly-pr-af-ci`. Copy the
   generated key (starts with `sk-or-…`). You cannot view it again after
   closing the dialog — copy it now.
4. Fund the account with credits from **Settings → Credits** if you haven't
   already. $10 gets you a comfortable margin over the $2/run default cap.

**Add it to the repo:**

1. Go to https://github.com/JimAKennedy/poly (the repo, not your profile).
2. Click **Settings** (top nav, requires admin access on the repo).
3. In the left sidebar: **Secrets and variables** → **Actions**.
4. You are on the **Secrets** tab (not **Variables**). Confirm the tab header
   says "Actions secrets and variables".
5. Click the green **New repository secret** button (top right).
6. **Name**: `OPENROUTER_API_KEY` — exactly that, all caps, underscores. Case
   matters. Do not add quotes.
7. **Secret**: paste the `sk-or-…` key. Do not add quotes or trailing
   whitespace.
8. Click **Add secret**.
9. Verify it appears in the list as `OPENROUTER_API_KEY` with an "Updated
   just now" timestamp. GitHub will never show you the value again — that is
   expected.

**Do not** put the key in a `.env` file, commit it, or paste it into an
issue/PR/Slack. If you leak it, rotate it immediately from the OpenRouter
Keys page.

### 2. Create the `pr-af` label

The workflow only fires when a label named exactly `pr-af` is added. If the
label doesn't exist in the repo, you can't add it, and the workflow can't
fire.

1. Go to https://github.com/JimAKennedy/poly/labels.
2. Click **New label** (top right).
3. **Label name**: `pr-af` — lowercase, hyphen, no spaces.
4. **Description**: `Triggers PR-AF deep architectural review workflow (~45
   min, ~$2)`.
5. **Color**: any — the workflow doesn't care. Suggest a distinctive one so
   it's easy to spot in the PR list. `#8B5CF6` (purple) is unused elsewhere
   in this repo.
6. Click **Create label**.

## Verifying the setup works

After both steps above, open any small existing PR (a docs typo fix is
ideal — cheap and safe), add the `pr-af` label, then watch the Actions tab.
Within ~2 min you should see:

- A run named "PR-AF Review (opt-in)" appear.
- Step "Start AgentField & PR-AF" completes without an authentication error.
- Step "Execute deep architectural audit" begins.

If "Start AgentField & PR-AF" or "Execute deep architectural audit" fails
with an OpenRouter auth error, the secret is missing or misnamed — go back to
step 1.

If the workflow doesn't fire at all after labeling, either the label name
isn't exactly `pr-af`, or the workflow file hasn't landed on the default
branch yet (it lives at `.github/workflows/pr-af-review.yml` and must be on
`main` to be considered "installed").

## What we're evaluating during the trial

- **C++/VST3 signal quality** — PR-AF's benchmark is Python/JS-heavy. Does
  it catch RT-safety violations, allocation-in-`process()`, missing state
  version stamps? (Our existing `nfr-review.yaml` and
  `scripts/check-realtime-safety.sh` already catch these; PR-AF adds value
  only if it catches things they miss.)
- **False positive rate** — high enough that maintainers stop reading
  comments = kills adoption.
- **Cost vs. `/code-review ultra`** on the same PR — head-to-head is the
  fairest comparison, since both are "deep review, paid".

Log findings on the PR trial thread. Decide adopt / adopt-as-optional / drop
after 3 labeled PRs of varying shape (pure CSS, JS/WASM, C++ engine).

## Related

- Workflow file: `.github/workflows/pr-af-review.yml`
- Upstream: https://github.com/Agent-Field/pr-af
- Our existing review stack: `nfr-review.yaml`,
  `.github/workflows/nfr-review.yml`, `/code-review ultra` (via Claude Code)
