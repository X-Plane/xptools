# Ben's answers on WED design principles (2026-09-25, verbatim)

Answers given during knowledge-base triage, in response to ten questions about ambiguous
design rules. Compiled into ../wed-design-principles.md.

1. Who opens the undo command: the caller or the helper?

Generally the outer user-facing command code (e.g. DoUpgradeEntitiesToModern) opens the cmomand, creating an outer grouping for all subsequent code. Utilities need to specifically assume a command context for composability.

2. Object IDs vs raw pointers.

Pointers are okay for:
 - Caches that have correct invalidation semantics (for perf)
 - References to systems that are not WED_Things (art, etc.)
 - Local references where undo cannot happen (e.g. code inside a helper that _must_ be run inside a command start/end because it mutates state).

3. Concrete classes or GIS interfaces?

Exact class match + static cast is the right high-perf solution when a final class is necessary to do the validation (e.g. "We need a runway"). Use GIS Interfaces when the operation applies to a broad geometric category. Generally if a piece of code has to list a large collection of exact classes, that's a code smell.

I would expect structure drawing to be abstract and preview drawing to be concrete.

There should be no casts to concrete intermediate implementations of GIS interfaces, e.g. WED_GISXXXX.

4. Export by mutating the document, then rolling back.

Mutate and roll-back is fine; the roll-back needs to be the last step since it will invalidate pointers. The roll-back mechanism is expected to be completely robust, because undo bugs are data-loss bugs.

5. Error-handling contract.

Assert and DebugAssert are only for invariant violations. Basically, they should only fire if a programmer should (1) see the condition and (2) change WED as a result. This includes precondition violations, post condition failures, and broken invariants.

APIs should prefer returning error values to throw - return channel errors are type-safe, throw is not.

6. Is blocking the main thread allowed?

Blocking is allowed but not great. (It is not a "NEVER DO THIS" like in X-Plane itself). Prefer asynchony for long-running IO bound tasks like network downloads, with cancelability.

7. Lifetime of resource-manager pointers.

The contract should be "handles are good until a library changed message goes out", since invalidation is the rare case.

8. Layering.

WED should be strictly layered on top of GUI - GUI code that does WED-specific things is always a layering violation. There may be a need for "WED-specific widgets" that are built on top of GUI and know WED things, and this boundary might be messy right now.

A dev submitting a patch by hacking up GUI to use WED internals is an architecture violation.

9. Export target: global or per-document?

Per-document. The global is just an attempt to make new docs be like the last thing we saw - this is true of baiscally all per-doc prefs.

10. Which platform is the reference?

It varies, but the answer is never Linux. If Mac and Windows agree, that's authoritative. If they clash, a human needs to referee.
