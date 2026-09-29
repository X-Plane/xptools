# WED Networking: HTTP, File Cache, and the (Dormant) Live Link

> Source: `src/Network/`, `src/WEDFileCache/`, `src/WEDNetwork/` · Consumers: `src/WEDImportExport/WED_Gateway*.cpp`, `WED_MetadataUpdate.cpp`, `src/WEDMap/WED_SlippyMap.cpp`, `src/WEDCore/WED_Validate.cpp`

Three directories, one real stack:

```
WED_GatewayImport / WED_GatewayExport / WED_MetadataUpdate / WED_SlippyMap / ReadCIFP (validation)
        │  poll from a GUI_Timer (or a blocking loop) on the main thread
        ▼
WED_FileCache (gFileCache)  ──owns──►  CACHE_CacheObject  ──owns──►  RAII_CurlHandle  ──►  curl_http_get_file
   src/WEDFileCache                    (one per URL)                  (src/WEDNetwork!)      (src/Network, one thread per request)
```

The gateway **upload** skips the cache and uses `curl_http_get_file` directly (PUT). `src/WEDNetwork` is
mostly a disabled TCP link to an X-Plane plugin; only `RAII_Classes.*` in that directory is built.

## Things That Will Bite You

### Nothing in `src/WEDNetwork` except `RAII_Classes` is compiled

`WED_Server`, `WED_Connection`, `WED_NWLinkAdapter`, `WED_NWInfoLayer` and `WED_NWDefs.h` are wrapped in
`#if WITHNWLINK`, and `Obj/XDefs.h` sets `WITHNWLINK 0` with the comment "really early stage of dev, do not
change." On top of that, `cmake/WED.cmake` lists only `src/WEDNetwork/RAII_Classes.*`, so setting the flag to 1
would fail at link time until the other `.cpp` files are added. The hooks in `WED_Document` (create
`WED_Server` for the first document only), `WED_Archive` (`mNWAdapter->ObjectChanged/Created/Destroyed`),
`WED_MapPane` (`WED_NWInfoLayer`), `WED_DocumentWindow` and `WED_Menus` ("Toggle LiveMode", with a
menu-index shift in the 3D Preview submenu) are all compiled out. **Don't count this code when you judge what
a change affects, and don't "fix" it as part of unrelated work.** The last real development was in 2012.
**Keep it (Ben, 2026-09-25).** Its absence from `cmake/WED.cmake` may be an oversight from the CMake
migration rather than a decision; confirm with the original author (the code was added in `9ec2a3a96`, 2012)
before deleting or re-enabling anything. Tracked on [the punch list](../bug-punch-list.md).

What it is (so nobody has to reverse-engineer it again): a **single-client** line-oriented TCP server
(default port pref `network/port` = 10300, `PCSBSocket`) polled from a 1 s `GUI_Timer`. It accepts one
client that must log in as `"WEDXPLUGIN"` with rev >= 100 within about 20 ticks. It pushes add/chg/del of
`WED_ObjPlacement`, `WED_FacadePlacement`, `WED_FacadeRing` and `WED_FacadeNode` (lat/lon/heading/resource),
and syncs a camera marker that you can drag on the map ("live mode"). It is a preview link to a running X-Plane.
It is **not** a gateway client and **not** multi-user collaboration. The X-Plane-side plugin is not in this repo.

`src/Network/PCSBSocket*` is compiled on Mac (the `.lin.cpp` variant) and Windows, but not on Linux. Its only
callers are `WED_Server` (disabled) and `WEDMap/WED_TerraserverLayer.cpp`, which is not in `WED.cmake`. It is
dead weight in the shipping binary.

### `RAII_CurlHandle` lives in `WEDNetwork`, not `Network` or `WEDFileCache`

The class the whole cache is built on is in `src/WEDNetwork/RAII_Classes.h`. That is why the directory can't
simply be dropped. Its member order matters: `m_dest_buffer` is declared **before** `m_curl_handle`, so the
buffer exists when the constructor starts the worker thread, and it is destroyed only after
`~curl_http_get_file` has joined that thread. If you reorder those members, the worker writes into
destroyed memory.

`RAII_FileHandle::path()` always returns `""` because neither constructor stores `mPath`. See the disk-write
bug below.

### Every `curl_http_get_file` is its own OS thread, and destroying it blocks

Each constructor calls `pthread_create` or `CreateThread`, with no pool and no concurrency limit. The
destructor sets `m_halt` and **joins**. The progress callback notices `m_halt`, but curl can't interrupt a
synchronous DNS lookup or the connect phase, so `delete` on an in-flight handle can block the **main thread**
for up to `CURLOPT_CONNECTTIMEOUT` (60 s). This happens through `CACHE_CacheObject::close_RAII_curl_hndl`,
`remove_cache_object`, `~WED_FileCache`, or the export dialog's `delete mCurl`. The header warns about this
("some paranoia is called for!").

`gFileCache` is a static global, so its destructor (which joins any live downloads) runs **after** `main()`
has already done `fclose(gLogFile)` in `WED_AppMain.cpp`. The worker thread writes to `gLogFile`, through
`LOG_MSG` and `CURLOPT_STDERR` + `CURLOPT_VERBOSE`. A download still in flight at quit could therefore write to
a closed `FILE*`. [Needs Runtime]: we haven't seen this crash reported.

### Cross-thread handoff is `volatile int`, not atomics

`curl_http.cpp` defines its own `atomic_load` and `atomic_store` as plain volatile reads and writes. The
worker fills `m_dl_buffer`, `swap`s it into the caller's destination vector, sets `m_errcode`, and **then**
stores `m_status`. The main thread's `is_done()` is the only synchronization. That works on x86 in practice,
but on ARM (Apple Silicon) nothing formally orders those writes (the comment explaining the ordering is cut
off mid-sentence). Rules that follow from this:
- **Don't read the destination buffer, `get_error()`, or `get_error_data()` until `is_done()` returns true.**
  The getters `DebugAssert` on status. `get_error_data()` *swaps* out the buffer, so it works only once.
- `get_progress()` can be read at any time and races by design. That's why the `DebugAssert` comparing
  progress snapshots in `WED_FileCache::request_file` is commented out (see "Tyler says").
- Don't touch any WED object, `gFileCache`, or GUI from curl callbacks. The worker only touches its own
  `curl_http_get_file`, the destination vector, and `gLogFile`.
- If you modernize this, switch to `std::atomic<int>` with release/acquire. Don't add locks around the buffer.

There are **no callbacks to the main thread**. Every consumer polls.

### The cache advances only when someone polls it

`WED_FileCache::request_file` is a state machine that does all its work *inside the call*: it checks whether
the handle is done, writes the file to disk, writes the `.cache_object_info` sidecar, and triggers the
cool-down. A download whose client stops polling (for example a closed import dialog) finishes into memory
and sits there. It isn't written to disk until someone requests that URL again. Two consequences:
- `file_in_cache()` isn't a passive query. It calls `request_file` and **starts a download** on a miss.
- All `gFileCache` calls must happen on the main thread. `CACHE_file_cache` is an unlocked vector.

### Only HTTP 200 counts as success

`thread_proc` treats any response code other than 200 as `done_error` and puts the HTTP code in
`m_errcode`. That includes 201, 204 and the 3xx codes left after `FOLLOWLOCATION` runs out. The gateway PUT
works only because the server answers 200. HTTP codes and `CURLcode`s share `m_errcode`, and consumers
separate them with `err <= CURL_LAST`. In curl 8.15 `CURL_LAST` is 102, so codes 100–102 would be
misclassified, which is harmless today.

### Timeouts: 60 s to connect, 30 s of stall, and no total limit

- `CURLOPT_CONNECTTIMEOUT` is 60 s. `CURLOPT_TIMEOUT` is commented out, so there is no overall limit.
- `progress_cb` aborts if **download** byte count hasn't grown for `TIMEOUT_SEC` (30 s). An abort by
  callback (stall *or* user halt) is rewritten to `CURLE_OPERATION_TIMEDOUT`, which `UTL_http_is_error_bad_net`
  classifies as a client-side network failure ("check your internet connectivity").
- **The upload trap:** the stall check looks only at `NowDownloaded`. During a gateway PUT nothing is
  downloaded until the server replies, so an upload plus server processing that takes longer than about
  30 s is aborted as a "timeout" even though bytes are flowing up. The dialog tells the user "This could take
  up to one minute". The POST/PUT constructor also leaves `m_last_dl_amount` and `m_errcode` uninitialized,
  unlike the other two constructors. Not seen in the wild (Ben, 2026-09-25); on [the punch list](../bug-punch-list.md), low priority.

### `CACHE_domain` values are persisted as integers, so append only

`.cache_object_info` stores `"domain": <int>` alongside `"last_time_modified"`. At startup `init()` looks up
`GetDomainPolicy(domain)` by that integer to decide whether to keep the file. If you insert or reorder entries
in `enum CACHE_domain`, users' existing cache files get the wrong max-age: they are deleted early or kept too
long. Add new domains before `cache_domain_end`. **Don't put them after the `#if DEV cache_domain_debug`
entry**, because that entry exists only in DEV builds, so DEV and release builds would disagree about the
new value. Keep `k_domain_policies[]` in the same order as the enum; nothing checks this at compile time.

### Policy fields that look meaningful but aren't

- `cache_domain_pol_min_server_cool_down_snds` is never read. Every error type uses the *client* cool-down.
- A max-KB-on-disk field is commented out. There is no size limit and no bandwidth rule.
- `cache_error_type_disk_write` has **no** cool-down (0 s).
- All current domains use a 60 s cool-down. Max ages: metadata CSV 1 day, airports JSON 1 hour, airport
  versions JSON 10 min, scenery pack and OSM tile 4 weeks, `none` 10 min, and debug 0.
- `WED_Validate.cpp`'s `ReadCIFP` borrows `cache_domain_metadata_csv` for `runway_coordinates.txt` to get the
  1-day expiry.

### No stale fallback when you're offline

If a cached file is older than its domain's max age, `request_file` deletes the `CACHE_CacheObject` and
starts a new download. The old file stays on disk and is overwritten on success. If that download fails, the
client gets `cache_status_error` followed by `cache_status_cooling`, **not** the stale file. An offline user
loses the metadata CSV and CIFP data a day after the last successful fetch. **This is the intended
policy (Ben, 2026-09-25):** stale Gateway data is worse than none — don't add a stale fallback.

### Cache identity is `folder_prefix` + URL basename

`url_to_cache_path` = `CACHE_folder / in_folder_prefix / FILE_get_file_name(in_url)`. The lookup matches an
existing object by that computed path **or** by `m_last_url`. Objects loaded from disk at startup have an
empty `m_last_url`, so after a restart only the path matches. Rules for callers:
- The prefix must make the basename unique. The gateway code puts per-ICAO files under
  `scenery_packs/GatewayImport/<ICAO>/` because `.../airport/KSEA` → `KSEA` and `.../scenery/12345` →
  `12345`. Commit f11243070 fixed a tile-mixing bug that came from not comparing the prefix.
- Build the prefix with `DIR_STR` and no leading or trailing separator. `init()` rebuilds paths as
  `dir + DIR_STR + name`, and the comparison is a plain string compare, so any other spelling never matches
  after a restart.
- Query strings go into the filename (`FILE_get_file_name` splits on the last `/`). For a custom slippy-map
  URL with `?key=...` that makes a `?` in the filename, which is invalid on Windows, so the save becomes a
  disk-write error. [Needs Runtime]
- Cache root: Mac `~/Library/Caches/wed_file_cache` (with a doubled separator, which is harmless). Windows
  `%APPDATA%\wed_file_cache` (**roaming** AppData, `CSIDL_APPDATA`, not Local). Linux
  `$XDG_CACHE_HOME` or `~/.cache`, and `GetCacheFolder()` returns `""` if `$HOME` doesn't match the passwd
  entry (for example under `sudo`). If that happens, `init()` calls `AssertPrintf`, which runs **before**
  `WED_AssertInit()` in `WED_AppMain`, so the default handler throws `assert_fail_exception` with no catch.

### Disk-write failure reports "available" and doesn't clean up

When the save fails, `request_file` fills in `out_error_human` and sets `last_error_type = disk_write`, but
the response still has **`out_status = cache_status_available`** and `out_path` set. The cleanup then calls
`FILE_delete_file(f.path())`, which is `""` because of the `RAII_FileHandle` bug above, and it runs while the
file is still open anyway. So a partial file can stay on disk and be served as valid until it expires.
`ferror` is also checked before `fclose`, so a flush failure at close time (disk full) goes unnoticed.
Callers that check only `out_status` will try to parse the partial file.

### What consumers do with each status

Only `available`, `downloading` and `error` are handled consistently. **`cache_status_cooling` is handled
differently by each caller**, so check it when you touch any of these:
- `WED_GatewayImportDialog::TimerFired` polls the *single* `mCacheRequest` on every tick at 0.1 s.
  Cooling ≠ downloading, so it `Stop()`s and does `mPhase++` without showing the cooling message.
- `WED_GatewayExportDialog` and `WED_UpdateMetadataDialog` stop the timer on cooling and hang in the
  download phase. On **error** they call `InterpretNetworkError(&mAirportMetadataCURLHandle->get_curl_handle())`,
  but `mAirportMetadataCURLHandle` is only ever set to `NULL`. **That's a null dereference whenever the metadata
  CSV download fails.** Use `res.out_error_human` instead. (Confirmed 2026-09-25; on [the punch list](../bug-punch-list.md).)
- `WED_SlippyMap` keeps one request in flight at a time (`m_cache_request`). On error it records
  `m_cache[res.out_path] = 0`, but `out_path` is `""` on error, so the failed tile isn't marked and gets
  requested again next frame. That request comes back as cooling, which the slippy map ignores, so it polls
  the same tile for about 60 s and **all other tiles stall** behind it. [Needs Runtime] to confirm what users
  see.
- `ReadCIFP` (validation) waits on the main thread with `sleep_for(1s)`, five times. If the download takes
  longer, it alerts and **skips CIFP validation**, and the ToDo there says gateway submission should really
  wait.
- `WED_DoInvisibleUpdateMetadata`, called from `WED_SceneryImport` under `GATEWAY_IMPORT_FEATURES`,
  **busy-spins** on `request_file` with no sleep until the status leaves downloading. It freezes the UI for
  the whole download.
- The moderator-mode version prefetch inside `FillICAOFromJSON` sleeps 100 ms up to 30 times per airport
  on the main thread.

### Gateway upload has no teardown

`WED_GatewayExportDialog` owns a raw `curl_http_get_file* mCurl` that writes into the member `mResponse`, and
it has **no destructor**. That's safe today only because the upload phase calls `Reset("", "", "", false)`,
which removes every button, and the window is modal with no close box. If you add a Cancel button during
upload, you also have to delete `mCurl` (which blocks, see above) before the dialog is destroyed. Otherwise
the worker writes into freed memory.

## TLS / Certificates

WED sets **no** TLS options: no `CAINFO`, no `SSL_VERIFY*`, and no `curl_global_init` (curl initializes
itself lazily on the first worker thread; libcurl 8.x does this thread-safely). Certificate handling comes
entirely from how Conan builds libcurl (`conanfile.py`, `libcurl/8.15.0`):
- **Windows:** `with_ssl = "schannel"`, which uses the Windows certificate store. Because Conan's curl
  targets don't link schannel's dependencies, `WED.cmake` links `secur32 crypt32 wldap32` by hand (commit
  86d7ae779). OpenSSL on Windows was tried the day before and then dropped.
- **Mac and Linux:** the Conan recipe defaults. On this dev machine the macOS package is built with
  `with_ssl=openssl`, `with_ca_bundle=auto`, `with_ca_path=auto` (curl 8.15 removed Secure Transport). With
  `auto`, the CA bundle path is found **on the build machine** and compiled into the binary. On Linux that
  path may not exist on a user's distro. [Needs Runtime] Nobody knows (asked Ben, 2026-09-25) where release
  Linux and Mac builds find CA certificates on users' machines — verification is on [the punch list](../bug-punch-list.md).
- Self-signed or custom-CA support (`m_cert` → `CURLOPT_CAINFO`) was removed in 614fa888f (2020) when the
  build moved to system curl.

Other fixed request settings: `FOLLOWLOCATION` (the source comment says it's there because URLs are served through redirects, so
server moves don't break old builds),
`ACCEPT_ENCODING ""` (gzip, so `get_progress()` returns **negative kB** when the total size is unknown; the
import UI prints that as "kB received"), a `Referer` of the WED tools page, a `User-Agent` of
`WorldEditor/<ver>` (the OSM tile server requires both), and `VERBOSE` always on (`#if 1 // DEV`), so every
request's headers and TLS handshake go into `WED_Log.txt`. The gateway password travels in the PUT's JSON
body, which verbose mode doesn't log. `UTL_http_encode_url` encodes only spaces.

## Architecture Notes

- **Startup:** `gFileCache.init()` runs from `WED_AppMain` during the splash screen. It walks the cache
  folder, pairs `file` with `file.cache_object_info` after sorting, deletes orphans and expired or unparsable
  pairs, and deletes empty directories (WED-1149). DEV-only `KEEP_EXPIRED_CACHE_FILES` turns the deletions
  into a dry run.
- **Per-URL state** lives in `CACHE_CacheObject`: the cool-down timestamp, the last error type, the disk
  location, and at most one live `RAII_CurlHandle`. The handle is closed as soon as the result has been
  processed. A cool-down is cleared only by destroying the object (`trigger_cool_down` `DebugAssert`s that
  its timestamp is 0).
- **Gateway URLs:** `WED_get_GW_api_url()` (in `WED_GatewayExport.cpp`) defaults to
  `https://gateway.x-plane.com/apiv1/`. The `--gateway_api_url` command-line argument overrides it for
  testing against a staging server. The CSV and CIFP URLs are fixed macros in `WEDCore/WED_Url.h`.
- **Gateway import payload:** the scenery JSON has a base64 `masterZipBlob`, decoded with `decode()` from
  `src/Network/b64.c`. That file is C, called through `extern "C"` declared inline in `WED_GatewayImport.cpp`.
  The whole downloaded scenery file is read into a string on the main thread.
- `HAS_GATEWAY` (`Obj/XDefs.h`, set to 1) wraps `curl_http.*` and the gateway import, export and metadata
  code. It's there for building without curl or SSL. It isn't a runtime preference.

## Connections to Other Systems

- **Design rules** ([wed-design-principles.md](wed-design-principles.md)): the stated rules for new code and reviews that this subsystem's conventions should follow — command ownership, pointer vs ID, class vs interface casts, error handling, layering, per-doc prefs, platform reference.
- [wed-import-export.md](wed-import-export.md): gateway import (wizard phases, `ImportSpecificVersion`),
  export or upload, and metadata update. These are the main clients of this stack.
- [wed-map-and-tce.md](wed-map-and-tce.md): `WED_SlippyMap` (OSM and ESRI tiles through the cache, domain
  `osm_tile`), plus the compiled-out `WED_NWInfoLayer` hook in `WED_MapPane`.
- [wed-validation.md](wed-validation.md): `ReadCIFP` fetches runway coordinates through the cache during
  gateway validation, and silently degrades on failure.
- [wed-core-services.md](wed-core-services.md): the `WED_AppMain` startup order (`gFileCache.init()` runs
  before `WED_AssertInit()`), `gLogFile` lifetime, and the `WED_Document` hook that starts `WED_Server`
  for the first document.
- [wed-object-model.md](wed-object-model.md): the compiled-out `WED_Archive` hook that would forward every
  create, change and destroy to `WED_NWLinkAdapter`. Turning `WITHNWLINK` on puts network code on the
  archive's mutation path.
- [bitmap-texture-and-ddstool.md](bitmap-texture-and-ddstool.md): what happens to a cached tile after it is
  on disk. `WED_SlippyMap` decodes it as PNG with gamma 0 (raw bytes), falls back to JPEG, and tints the
  pixels in place assuming `pad == 0` and bottom-up BGR rows.
- [gui-framework.md](gui-framework.md): `GUI_Timer` drives all polling, and `GUI_FormWindow::Reset`
  controls which dialog buttons exist during uploads.
- [utils-platform-and-files.md](utils-platform-and-files.md): `GetCacheFolder()` per platform, and
  `FILE_get_directory_recursive` path building, which the cache's path matching depends on.
