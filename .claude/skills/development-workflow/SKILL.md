---
name: development-workflow
description: >-
  Standard end-to-end workflow for working on a task in this repository:
  branch naming, implement + tests + style, self-review, commit, and
  `docs/ISSUES.md` cleanup.
  Use when starting work on a numbered task from a GitHub issue.
---

# Development Workflow

When working on a task, follow these steps in order:

1. create and switch to a branch with the following pattern `feature/<taskid>-<task-name>`
2. implement the features, corresponding tests, check build & style
3. take a step back to review carefully the changes: are they complete & accurate? Is this really the best solution possible?
4. make a commit with a short description
5. remove the task from the `docs/ISSUES.md`

## Scope rule (read first)

A "task" is one filed GitHub issue (or one entry in `docs/ISSUES.md`), not one commit. **One task = one branch.**

Any new requirement that falls within the scope of the current task — supporting changes asked for mid-implementation, additional acceptance criteria, polish, follow-up cleanup, or related tooling — stays on the **same** task branch as additional commits. Do **not** create a new branch for it: skip step 1 and restart from step 2.

Only create a new branch (back to step 1) when the work is genuinely a new task: a different filed issue, or a separate concern that does not belong under the current task's scope.

When in doubt, ask the user before creating a new branch.

## Operational anchors

These anchors operationalize the steps above without changing them.

### Step 1 — branch

- `<taskid>` is the GitHub issue number (e.g. `1`, `12`); `<task-name>` is a short kebab-case slug derived from the issue title.
- Always branch from an up-to-date `main` and never commit directly to `main`.
- For tasks without a filed issue (rare; e.g. authoring a new agent skill from scratch), prefer filing the issue first. If that is not appropriate, fall back to a `chore/<task-name>` branch.

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
- Do not amend or force-push without explicit user request.
- Do not push to remote unless the user asks for it.

### Step 5 — remove from `docs/ISSUES.md`

Do this only once the task as a whole is complete (all required scope has landed on the branch), not after every intermediate commit.

- Delete the entire `### <task title>` block for the completed task, including its `#### Description`, `#### Tasks`, `#### Acceptance criteria`, and any `#### Notes` sub-sections.
- Remove a now-orphaned `---` separator if one is left dangling between sections.
- If the milestone heading still has remaining entries, leave it; if it is now empty, leave the heading and update the milestone preamble note (the `> The following deliverables…` line) to reflect what is now done.
- The cleanup may go in the final implementation commit or in a small dedicated follow-up commit on the same branch.

## When this skill applies

Apply this workflow for every numbered task in `docs/ISSUES.md` and every filed GitHub issue. For trivial out-of-band fixes (typos, comment-only edits) the branch + commit + ISSUES cleanup steps still apply, but steps 2 and 3 collapse to a single read-through.
