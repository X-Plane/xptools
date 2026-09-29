# Knowledge Base Log

> Append-only chronological record of all compilation, update, and lint actions.
> Format: `date | action | subsystem: details`
> Quick lookup: `grep "2026-09-25" docs/knowledge/log.md`

2026-09-25 | setup | Knowledge base created (schema adapted from ~/code/design/docs/knowledge). WED_Architecture.md removed — it mixed verified facts with unverified claims (SQLite archive, WED_PROPERTY_* macros, factory table in WED_Entity.cpp); nothing was carried over without re-checking source.
2026-09-25 | ingest | DSF: src/DSF/README.txt moved to raw/dsf-library-readme.txt
2026-09-25 | compile | WEDCore (archive, undo, XML) + WEDEntities bases + Interfaces IResolver/IOperation → wed-object-model.md
2026-09-25 | compile | WEDCore services (app lifecycle, packages, library, resources, textures, prefs, enums) → wed-core-services.md
2026-09-25 | compile | WEDCore validation → wed-validation.md
2026-09-25 | compile | WEDEntities + Interfaces (IGIS, IPropertyObject, …) + property helpers → wed-entities.md
2026-09-25 | compile | WEDMap + WEDTCE → wed-map-and-tce.md
2026-09-25 | compile | WEDWindows + WEDProperties + WEDLibrary → wed-ui-panes.md
2026-09-25 | compile | WEDImportExport + XESCore AptIO → wed-import-export.md
2026-09-25 | compile | WEDFileCache + WEDNetwork + Network → wed-network-and-filecache.md
2026-09-25 | compile | GUI + UI + OGLE → gui-framework.md
2026-09-25 | compile | DSF + DSFTools (raw: raw/dsf-library-readme.txt, src/DSFTools/README.dsf2text) → dsf-library-and-dsftool.md
2026-09-25 | compile | Utils bitmap/texture + XPTools/DDSTool (raw: src/XPTools/README.DDSTool) → bitmap-texture-and-ddstool.md
2026-09-25 | compile | Utils geometry + libtess2 → utils-geometry.md
2026-09-25 | compile | Utils platform/files + Obj → utils-platform-and-files.md
2026-09-25 | lint | Cross-article reconciliation: bidirectional links, overlapping-topic contradictions
2026-09-25 | triage | Ben interview: Save/.bak logic confirmed real bug (punch list, high); Gateway-upload stale-DSF deletion → needs runtime verification (punch list)
2026-09-25 | triage | Ben interview round 2 (answered 'verify'; verified from source): Revert bare throw; and metadata-CSV null deref → punch list high; ImportSpecificVersion NULL g and pack-export dangling problem_children → punch list low
2026-09-25 | triage | Ben interview round 3: CIFP-skip on download failure = accepted policy; post-validation Gateway heuristics = by design; DSF export precedence bug + iso3166 table errors → punch list low
2026-09-25 | triage | Ben interview round 4: metadata AddMetaDataKey/EditMetaDataKey missing StateChanged → punch list high
2026-09-25 | triage | Ben: xml_compatibility policy = bump on every format change (code-review item); audit since 2.5 → punch list high
2026-09-25 | triage | Ben round 5: unknown-enum save crash → high; block XP-folder change with docs open, DSF polygon-pool best-fit, DSF import zero-length sides → punch list (low)
2026-09-25 | triage | Ben: keep WEDNetwork; cmake omission may be an oversight — check with author (punch list, needs verification)
2026-09-25 | triage | Ben: XP12 treats all DDS as sRGB, ignores DDSCAPS_COMPLEX gamma hint — not a bug
2026-09-25 | triage | Ben: no stale cache fallback (policy); upload stall timeout → low; README.dsf2text fixes → high; CA certs unknown → verify
2026-09-25 | triage | Ben: validation dup checks, lost 'Cancel Export' label, non-gating results dialog, has_dsf() omissions → punch list (low)
2026-09-25 | triage | Ben: jetway style codes 0 light_solid,1 light_glass,2 dark_solid,3 dark_glass; ImportJetway fallback table wrong → punch list high
2026-09-25 | triage | Ben: string-spacing import decode + ortho stale regen → punch list high; legacy row 15 for misc/all ramps is deliberate
2026-09-25 | triage | Ben: ResourceMgr purge = bug; EXPORT break = bug; Rescan self-compare, GetDem/miss caching, TexMgr flags key, GetFac realloc → punch list low
2026-09-25 | triage | Ben: splitter creep → high; Mac close box, clock(), mTrap, Mac timers, Windows DPI → low; Linux GL context reuse safe (start window permanent)
2026-09-25 | triage | Ben bulk round: main-thread waits, 4 DSFLib/DSFTool bugs, 7 image/DDS bugs → high; truncated-cache, slippy stall, cooling hang, Kpix → low
2026-09-25 | triage | Ben bulk round 2: 10 high (Polygon2cleaner, is_near, 7z non-ASCII, Linux mmap, GISEdge OOB, handle-tool open op, SlippyMap leak, prefs atoi, runway-marking break, import-prompt loop), 6 low
2026-09-25 | triage | Ben: Revert undoable = intended; Upgrade Art/EdgePavement = planned; AbortCommand redraw, preview commander → punch list low. Checked XP source: ATC flows first-match in file order; control textures bottom-row OK, index formula wrong → punch list
2026-09-25 | triage | Ben: move all earth math to WGS84 (punch list); antimeridian unsupported; TIFF premultiplied = bug; remove --quilt
2026-09-25 | triage | Ben: NPOT preview padding intended; unpaved_runways group, library filter mismatch → punch list; TryClose AsyncDestroy leftover → cleanup
2026-09-25 | triage | Ben: modal-in-command mechanism confirmed; DSF fixtures → automate; delete CarbonMemMap.h, MemIStreamBuf.h, WED_GroupCommands.cpp.better, WED_TerraserverLayer; keep ObjUtils.cpp in WED
2026-09-25 | triage | Ben: wire up WEDTCE dead handlers → punch list low
2026-09-25 | ingest | raw/ben-design-principles-2026-09-25.md (Ben's answers on 10 design questions) → wed-design-principles.md; back-links added; principle violations (export-target leak, GUI→WED includes, WED_GIS* casts) → punch list
