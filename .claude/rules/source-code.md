---
globs:
  - "src/**"
---
A compiled knowledge base exists at `docs/knowledge/`. Before modifying, analyzing, or reviewing code in a subsystem, read the matching article — it contains cross-file coupling, undo/persistence invariants, and gotchas you will not find by reading individual source files.

Find the right article: check `docs/knowledge/index.md` (already in your context via CLAUDE.md import) — the Source Map links each `src/` directory to its article. For cross-cutting topics, use the Concept Index.

After modifying code, check whether the article is still accurate. If you found something wrong or learned something new, update the article and append to `docs/knowledge/log.md`.
