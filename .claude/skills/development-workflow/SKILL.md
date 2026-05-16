---
name: development-workflow
description: >-
  Standard end-to-end workflow for working on a task in this repository:
  branch naming, implement + tests + style, self-review, commit,
  `docs/ISSUES.md` cleanup, push the branch, and open a PR with `gh`.
  Use when starting work on an entry from `docs/ISSUES.md` (or, when one is filed, a GitHub issue).
---

# Development Workflow

When working on a task, follow these steps in order:

1. create and switch to a branch with the following pattern `feature/<task-name>` (or `feature/<taskid>-<task-name>` when a GitHub issue has been filed — see step 1 for details)
2. implement the features, corresponding tests, check build & style
3. take a step back to review carefully the changes: are they complete & accurate? Is this really the best solution possible?
4. make a commit with a short description
5. remove the task from the `docs/ISSUES.md`
6. push the branch to `origin` and open a pull request with `gh pr create`, then report the PR URL back in chat

**The agent owns the full local-to-PR loop.** Once the build is green, tests pass, the diff has been self-reviewed, and `docs/ISSUES.md` has been cleaned up, the agent is expected to `git push -u origin HEAD` and `gh pr create` itself. The only remote-write commands explicitly out of scope are destructive ones (`git push --force`, `gh pr merge`, anything that rewrites or merges shared history): those still require an explicit user request. Merging the PR remains the user's call.

## Scope rule (read first)

A "task" is one entry in `docs/ISSUES.md` (or, when one is filed, one GitHub issue), not one commit. **One task = one branch.**

Any new requirement that falls within the scope of the current task — supporting changes asked for mid-implementation, additional acceptance criteria, polish, follow-up cleanup, or related tooling — stays on the **same** task branch as additional commits. Do **not** create a new branch for it: skip step 1 and restart from step 2.

Only create a new branch (back to step 1) when the work is genuinely a new task: a different `docs/ISSUES.md` entry (or filed issue), or a separate concern that does not belong under the current task's scope.

When in doubt, ask the user before creating a new branch.

## Operational anchors

These anchors operationalize the steps above without changing them.

### Step 1 — branch

- The default branch pattern is `feature/<task-name>`, where `<task-name>` is a short kebab-case slug derived from the `docs/ISSUES.md` entry title (or the GitHub issue title, when one exists).
- If — and only if — a GitHub issue has been filed for the task, prefix the slug with its number: `feature/<taskid>-<task-name>` (e.g. `feature/12-extract-production-source-list`). Issues are no longer routinely filed on GitHub for this repo; tasks normally live only in `docs/ISSUES.md`, so most new branches will use the unprefixed form.
- Always branch a new task from an up-to-date `main`.
- Repo-maintenance fixes that do not correspond to any `docs/ISSUES.md` entry (a tweak to a skill, a stray typo, a small docs follow-up) do **not** warrant a dedicated branch. Land them on whichever of these two paths is most natural:
  - directly on `main` when no task is in flight, or
  - on the current task branch when the fix is related to the work in progress (or is small enough to ride along).
  Ask the user when it is genuinely unclear which of the two paths fits.

### Step 2 — implement, test, style

- Use `scripts/build.ps1` (Windows) or `scripts/build.sh` (Linux/macOS); see the `build` skill. A non-zero exit code means the build failed — surface it, don't retry blindly.
- Run the test suite with `scripts/test.ps1` (Windows) or `scripts/test.sh` (Linux/macOS). The raw `ctest --test-dir build --output-on-failure` invocation is also fine. Add or extend tests under `tests/` for any new logic.
- Verify configure-time toggles still work when relevant (e.g. `-DBUILD_TESTING=OFF`).
- Honour `.editorconfig` and `.clang-format` (when present) for style.

### Step 3 — review

Before committing, re-read the diff with these checks:

- All acceptance criteria of the task (so far) are met.
- The change is minimal and focused; no drive-by edits unrelated to the task.
- Edge cases and failure modes are covered by tests.
- Documentation (`README.md`, `docs/`, skill files) is updated when behaviour or workflow changes.
- A simpler or more idiomatic alternative was considered and consciously rejected.

### Step 4 — commit

- Stage only the files that belong to the task.
- Write a short, imperative commit description (one line is usually enough). Match the style of recent commits (`git log --oneline`).
- Multiple commits per task are fine — one task = one branch, not one commit. Each commit should be a coherent, reviewable slice of the task.
- Do not amend without explicit user request.
- A plain `git push` of the task branch (step 6) is part of the workflow. Force-pushes (`--force`, `--force-with-lease`) and any rewrite of already-pushed history still require an explicit user request — see step 4b.

**Do not sprinkle issue references in commit messages.** When a GitHub issue exists, the branch name already encodes the number (`feature/<taskid>-…`) and the PR description carries the formal `Closes #<taskid>`. When the task lives only in `docs/ISSUES.md`, there is no number to reference at all. Boilerplate like `Refs #11.` or `For #11:` at the start of every commit body is pure noise — it makes individual commits read as bureaucratic instead of natural, and it duplicates information that already lives one click away. Save explicit issue links for the PR description and, very occasionally, for a single commit whose subject genuinely needs the context to be understood standalone.

**Bodies are optional, not the default.** Most commits in this repo are subject-only (see `git log --oneline -20` on `main`). Add a body only when the change has a non-obvious *why* that the subject cannot carry: an architectural trade-off, a deliberate deviation from a spec, a subtle behaviour change, or a foot-gun a future reader could miss. If a body is added, keep it tight (a short paragraph is plenty); long expository bodies belong in the PR description.

**Format bodies like prose, not like a wall.** Wrap lines at roughly 72 columns and split distinct ideas into blank-line-separated paragraphs. Avoid one ~300-character line that crams three sentences together — it is hard to read in `git log`, in PR review UIs, and in narrow terminals.

**Multi-line commit messages and shell portability.** Single-line subjects work the same everywhere: `git commit -m "Subject."`. For a subject + body, do not rely on bash heredocs (`git commit -m "$(cat <<'EOF' ... EOF)"`): they are a parse error on Windows PowerShell, which is the default shell for this repo's contributors. Use one of these instead:

- Cross-shell, recommended: write the message to a file and pass it with `-F`. The git directory is a convenient scratch location that never gets committed:

    ```
    # write COMMIT_MSG.txt (subject line, blank line, body)
    git commit -F .git/COMMIT_MSG.txt
    # then delete the temp file
    ```

- Two `-m` flags also works on both shells and produces the same "subject + body" layout: `git commit -m "Subject." -m "Body paragraph."`.

This note exists because the agent harness instructions assume a POSIX shell and would otherwise default to a heredoc that silently fails on Windows.

### Step 4b — updating earlier commits (amend / rebase)

A mid-task refactor or revert from the user is a signal to rewrite the relevant earlier commit in place, not to pile a "fix the previous commit" patch on top.

- **Trigger**: a refactor, revert, or rename that makes an earlier commit's diff or message inaccurate.
- **Update both**: the diff *and* the commit message — a refactor often makes an old subject or body stale (wrong helper count, removed function name, deleted test case). Reread every touched message.
- **Why it's safe here**: no further work has landed on top, so conflict risk is minimal.
- **Pushed branches**: rewriting a not-yet-merged task branch is fine, but ask the user before force-pushing — they may have linked the old SHA somewhere.
- **Never**: rewrite `main` or any branch already merged into `main`.

The default rule from step 4 ("do not amend without explicit user request") still holds; a user-driven refactor *is* the explicit request.

The agent cannot run `git rebase -i` interactively in its harness. Use a sequence of `git cherry-pick` + `git commit --amend -m … -m …` calls instead, with a `backup-pre-rewrite` branch as safety net until the user confirms the result.

### Step 5 — remove from `docs/ISSUES.md`

Do this only once the task as a whole is complete (all required scope has landed on the branch), not after every intermediate commit.

- Delete the entire `### <task title>` block for the completed task, including its `#### Description`, `#### Tasks`, `#### Acceptance criteria`, and any `#### Notes` sub-sections.
- Remove a now-orphaned `---` separator if one is left dangling between sections.
- If the milestone heading still has remaining entries, leave it; if it is now empty, leave the heading and update the milestone preamble note (the `> The following deliverables…` line) to reflect what is now done.
- The cleanup may go in the final implementation commit or in a small dedicated follow-up commit on the same branch.

### Step 6 — push the branch and open the PR

Once the task is supposedly complete (all scope landed, build green, tests passing, `docs/ISSUES.md` cleaned up), the agent pushes the branch and opens the PR itself. If the build, tests, or self-review revealed unfinished work, go back to step 2 instead.

Concrete steps:

1. `git push -u origin HEAD` to publish the branch and set the upstream. If the push is rejected (typically because `main` advanced underneath the branch), stop and report — do not force-push without an explicit user request.
2. Open the PR with `gh pr create`, passing the title with `--title` and the description with `--body-file` (write the body to a temp file such as `.git/PR_BODY.md` and delete it afterwards). `--body "$(cat <<EOF…EOF)"` heredocs are not portable to Windows PowerShell — see the multi-line message note in step 4 for the same reason.
3. Capture the PR URL `gh` prints and post it in chat alongside the rest of the hand-off (see below).

PR content rules:

- **Title**: short and imperative, matching the style of recent commits / PRs. Prefix with the issue number only when a GitHub issue has been filed (e.g. `#12 Add paddle input handling`); when the task lives only in `docs/ISSUES.md`, omit the prefix entirely (e.g. `Inject a Clock abstraction into Application`).
- **Description**:
  - A one- or two-sentence summary of what the change does and why.
  - A `Closes #<taskid>` (or `Refs #<taskid>`) line **only when a GitHub issue exists**. For docs-only tasks, drop this line and instead reference the `docs/ISSUES.md` entry by its heading title in the summary.
  - A short bullet list of the main changes (features, tests, docs).
  - Any noteworthy trade-offs, follow-ups, or things explicitly out of scope.
  - Test evidence: which build/test commands were run and their outcome.

Hand-off contents (post in chat after the PR is open):

- **Branch summary**: the branch name and `git log --oneline main..HEAD` so the user can see what is queued for review.
- **Verification done**: which build / test / format commands ran and what they returned (one line each is plenty).
- **PR URL**: the link printed by `gh pr create`, so the user can jump straight to review and merge.

Do not run `gh pr merge`, `git push --force`, `git push --force-with-lease`, `gh pr edit` against an already-merged PR, or any other rewrite of shared history without an explicit user request. Merging the PR is always the user's call.

## When this skill applies

Apply this workflow for every entry in `docs/ISSUES.md` and every filed GitHub issue. Trivial out-of-band fixes (typos, comment-only edits, small skill tweaks) skip the branching and PR steps entirely — see step 1 for where they land — and steps 2 and 3 collapse to a single read-through.
