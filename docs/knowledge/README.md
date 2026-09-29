# XPTools Knowledge Base

## What This Is

An LLM-compiled knowledge base for the xptools codebase (WED, DSFTool, DDSTool, and the
shared libraries under `src/`). The LLM writes and maintains everything here. Humans steer
and review.

**The knowledge base is FOR the LLM agent, not for human developers.** Its purpose is to make
Claude (or any future LLM) more effective at working on xptools code without breaking things.
It is not the user manual (`src/WEDDocs/`) and not build docs for humans (`Building.md`) — it is
the correction signal an agent needs for the things it would otherwise get wrong.

## Scope

Covered: WED (`src/WED*`), the libraries WED and the command-line tools share (`GUI/`,
`Interfaces/`, `Utils/`, `DSF/`, `Obj/`, `UI/`, `OGLE/`, `Network/`), DSFTool (DSF2Text,
`DSFTools/`), and DDSTool (`XPTools/DDSTool.cpp`).

Out of scope: RenderFarm, MeshTool and the `XESCore/` GIS engine, `OneOffs/`, and vendored
code (`lzma19/`, `SDK/`, `glew`). The index lists these so agents know they were skipped on
purpose, not forgotten.

## The Core Design Constraint

LLM-generated context that merely restates what the code already says can *reduce* task
success. Only genuinely non-inferable content helps — things the LLM would get wrong without
being told.

- Don't document what the code already says. If you can figure it out by reading the file, it doesn't belong here.
- DO document what you'd get wrong without being told — the traps, the coupling, the "why."
- The value test for every entry: **"Would knowing this have prevented a real bug or saved meaningful investigation time?"** If not, cut it.

## The Quality Bar

**Too obvious (worthless):** "`WED_Runway` represents a runway." Class-by-class tables that
repeat the headers. Describing control flow visible from reading the code. Explaining what an
observer pattern is.

**Too obscure (noise):** Internal details of a single function nobody will change. Historical
trivia with no bearing on current code.

**The sweet spot (valuable):** Knowledge that would make an experienced developer say "huh, I
didn't know that" or "that would have saved me a day of debugging."

**Concrete heuristic:** If the information is contained in a single file and discoverable from
that file's comments and signatures, it's too obvious. If it takes reading 2+ files or
understanding cross-file interactions to piece together, it likely belongs here.

Target content:
- **Cross-file coupling** invisible from reading one file in isolation
- **Invariants the compiler doesn't enforce** — undo/command wrapping, ordering, ownership and lifetime, platform assumptions
- **Why non-obvious choices were made** — not what, but why that way instead of the obvious way
- **What has been tried and failed**
- **Fragile areas** — places where small changes historically cause cascading failures (the git log is a good source)

### Accuracy over coverage

Every claim must be verified against the source before it is written. The predecessor to this
knowledge base (`WED_Architecture.md`, removed) mixed real facts with plausible-sounding guesses;
an agent can't tell the two apart, so one wrong entry costs more than a missing one. If you
can't confirm something from code, mark it `[Needs Human]` or `[Needs Runtime]` — or leave it out.

## How to Use Articles When Working on Code

1. Find the subsystem in `index.md` (Source Map, or Concept Index for cross-cutting topics)
2. Read **"Things That Will Bite You"** — highest bug-prevention-per-token section
3. Skim **"Connections to Other Systems"** to understand blast radius
4. If you discover something the article got wrong or missed, fix it before finishing your task

## Directory Structure

```
docs/knowledge/
  index.md     — Master index: Source Map + Concept Index + Raw Sources. Read first.
  log.md       — Append-only chronological record of compile/update/lint actions.
  README.md    — This file (how to operate the knowledge base — the "schema")
  raw/         — Unprocessed human-written source documents
  *.md         — Compiled articles (one per subsystem or cross-cutting concern)
```

All subsystem paths in `index.md` are relative to `src/`.

## How to Compile a New Subsystem Article

1. **Find the entry points.** Start with the headers in the directory, then grep for those
   headers being included from OTHER directories — that is the real API surface and coupling.
2. **Read headers first**, then implementations for non-obvious behavior: error handling,
   ownership, initialization/shutdown ordering, platform branches.
3. **For large directories**, follow the spine: inheritance (`grep "class.*:.*public"`), the
   largest `.cpp`, and the call sites from other subsystems.
4. **Hunt specifically for:**
   - Undo/command discipline: `StartCommand` / `CommitCommand` / `AbortCommand`, `StartOperation` (`Interfaces/IOperation.h`)
   - Persistence hooks: `ReadFrom` / `WriteTo` / `ToXML` / `FromXML`, `DECLARE_PERSISTENT` / `IMPLEMENTATION` macros
   - Change notification and message IDs (`GUI_Broadcaster`, `GUI_Listener`, `msg_*`)
   - Platform branches (`#if APL` / `IBM` / `LIN`) and the forced-include `Obj/XDefs.h`
   - Comments that explain "why" (`HACK`, `TODO`, `WTF`, `NOTE`, `Ben says`, …)
   - `git log -- <dir>` for bugs that recurred
5. **Write the article** in the format below.
6. **Resolve what you can.** Mark the rest `[Needs Human]` / `[Needs Runtime]`.
7. **Cross-reference** other articles. Links must be **bidirectional**.
8. **Check for contradictions** with existing articles; fix both sides immediately.
9. **Update `index.md`** and append to `log.md`.

### Article Format

```markdown
# <Subsystem Name>

> Source: `src/<dir>/` · Raw source: [optional link into raw/]

## Things That Will Bite You

[Non-obvious invariants, traps, gotchas. Each entry names the file/function/class and the
exact constraint. Names double as staleness anchors — if a named symbol disappears, the entry
needs review.]

## Architecture

[How the pieces connect across files. Include hierarchy only where it helps.]

## Connections to Other Systems

[What depends on / interacts with this. Link other articles, bidirectionally.]
```

Add, drop, or rename sections as the subsystem needs ("Recipes", "Persistence", "Threading",
"Format Quirks" …). There is no "API Reference" section — the code is the API reference.

**Length:** target 150–400 lines. Over 500 means too broad (split) or too obvious (prune).

### Rules for Referencing Code

- **Describe patterns, don't quote code** unless the exact syntax IS the point; when quoting, name the file.
- **No volatile counts** ("there are 212 validation checks") — they rot immediately.
- **Reference by function/class name, not line number.**

## How to Update an Existing Article

The most valuable updates come from real work: you touch a subsystem, find the article wrong
or incomplete, and fix it — in the same commit as the code change when the change alters an
invariant. Also update when new raw sources arrive, when you spent real time figuring
something out, or when you find a contradiction. Append to `log.md` every time.

## How to Ingest Raw Sources

1. Copy the document to `raw/` with a descriptive name (unless it must stay in place, e.g. a README that `scripts/bundle.sh` ships — then reference it in place)
2. Add it to the Raw Sources table in `index.md`
3. Fold its insights into the relevant article(s); add a `Raw source:` link at the top
4. Log it in `log.md`

## Linting and Health Checks

Periodically (or when asked):
- **Stale anchors** — grep for symbols named in "Things That Will Bite You"; missing names need review.
- **Index/disk consistency** — every article is in `index.md`, every index link exists.
- **Missing cross-references** — interacting subsystems link each other both ways.
- **Contradictions** — fix both articles.
- **Stale unknowns** — re-investigate `[Needs Human]` / `[Needs Runtime]` items older than 3 months.

## Branch-Specific Knowledge

The knowledge base travels with branches. When a `wed_NNN_release` branch changes an invariant,
update the article there; the normal release→master merge carries it forward.

## Design Decisions

- **"Things That Will Bite You" comes first** — if context runs out mid-article, the most critical content was already read.
- **No API reference.** The code is the API reference.
- **Articles follow source directories**, with a Concept Index for cross-cutting topics.
- **LLM-maintained.** Humans provide raw sources and review; the LLM compiles, updates, and lints.
