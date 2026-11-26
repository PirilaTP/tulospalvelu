# C-10 osa C / part C: HTTP (WinHTTP) and browser components

Agent `defect-concurrency-auditor` (Sonnet), 2026-11-26, about 122 k tokens, 29 tool calls.
Report saved by the main session from the agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Static reading and grep only. Paths relative to `TPsource/V52`.
Tags: [verified], [inferred], [unknown]. Candidate IDs T1-T13 and Q-C1..Q-C7 are the
agent's working labels; the register IDs are assigned in `riippuvuudet.md`.

## Conclusion

- **HTTP.** Exactly one HTTP client, `com/gethttp.cpp`: plain `GET` with WinHTTP, one purpose: polling emiTag/GPRS split-time messages (`ETGPRS` setting, `EMITHTTP` macro). Callers always pass `secure=0`: no TLS, no authentication, no certificate handling. No code path in the five products makes an https request.
- **Defaults.** Default server is the hard-coded IPv4 literal `195.159.103.189:1379`; path `/emitphp/get_stream.php` hard-coded.
- **No timeouts, no status check, no content type check.** Error pages from a proxy or server are fed to the line parser as data (1.5).
- **Failure reporting** via `writeerror_w`: GUI = modal "Virhe" dialog in the main thread; relay build shows only an error number.
- **Browser** is `TWebBrowser` (IE/MSHTML ActiveX, `cbTp/UnitBrowser.dfm:196`), only in HkKisaWin and ViestiWin.
- **No `TEdgeBrowser`, `WebView2` or `Edge`** in any `.cpp`, `.h`, `.dfm`, `.cbproj` [verified, grep]. `riippuvuudet.md:126` ("TWebBrowser/TEdgeBrowser") is **refuted**.
- **Browser emulation key.** `setBrowserEmulation()` (`cbTp/cbTpApu.cpp:579-636`) writes `HKCU\...\FEATURE_BROWSER_EMULATION\<exe>` = 1000 × IE major version (9000/10000/11000).

## 1. WinHTTP use

### 1.1 `com/gethttp.cpp` (165 lines, read in full)

| Function | Lines | Behaviour |
|---|---|---|
| `httphaku(host, port, page, secure, buf, buflen, *haettu)` | `:125-158` | `WinHttpOpen("WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, no proxy, no bypass, 0)` `:134-137`; `memset(buf)` `:141`; calls `getwebPage`; maps result; closes session |
| `getwebPage(...)` | `:27-123` | `WinHttpConnect` `:43`, `WinHttpOpenRequest(L"GET", page, ..., secure?WINHTTP_FLAG_SECURE:0)` `:47-50`, `WinHttpSendRequest` without headers/body `:54`, `WinHttpReceiveResponse` `:61`; loop `WinHttpQueryDataAvailable`/`malloc`/`WinHttpReadData`/`memcpy` into caller buffer `:66-112` |

Return codes (comment `:160-165`) [verified]: -6 session open failed, -5 connect handle missing, -4 request handle missing, -3 out of memory, -2/-1/1 Win32 error via `GetLastError()` (`:148-149`). The `outfile`/`fputs` branch `:103-106` is unreachable (`tmpfile = NULL` `:130`).

### 1.2 Callers

| Caller | `#if` | Products | Thread |
|---|---|---|---|
| `Hk/HkIV.cpp:3540` in `vahaku` (`:3500-3558`), started `:3582` from `etHaku` (`:3559-3609`), called by `tausta()` `:3699-3701` | `EMITHTTP` | HkMaali, HkKisaWin | `vahaku` own `_beginthread` thread (40960-byte stack); `etHaku` in `taustaThr` (`Hk/HkIV.cpp:3711`, started `Hk/HkInit.cpp:3708`) |
| `Juk/VIv.cpp:2397` in `vahaku` (`:2360-2403`), started `:2476` from `tausta` (`:2405`, `:2461-2500`) | `EMITHTTP` | ViestiMaali, ViestiWin | same; `taustaThr` `Juk/VIv.cpp:2530`, started `Juk/VInit.cpp:2958` |

ViestiLuenta defines no `EMITHTTP` and does not compile `gethttp.cpp` [verified]. `gethttp.cpp` is also in `VS/Tests/TpCoreHk.vcxproj:119`, `TpCoreJuk.vcxproj:121`. Dead UI: `cbHk/WinHk.cpp:1167-1171` `TGetHttpForm` only under `EI_OLE` (defined nowhere); `HkConsole/GetHttp.h` listed in `HkMaali.vcxproj:213`, no `GetHttp.cpp` [verified].

### 1.3 Purpose

Polls the emiTag server for split-time messages from up to 4 ETS1 devices (`eTParam.eTserial[4]`, `Hk/HkDef.h:1333-1345`). Manual: `docs-site/docs/hkkisawin/liite_9._emitag-laitteiden_kaytto.md:116` (http fetch of splits from Emit's server), default host `:193-194`, re-request of the 10 latest messages after a transmission fault `:218-220` = code `msgno = atoi(...) - 10` (`Hk/HkIV.cpp:3596`). Console Alt-W "J)atka, K)eskeytä, hae A)lusta" (`HkConsole/HkCons2.cpp:1855-1860`). No result upload or other HTTP use [verified].

### 1.4 Endpoints

| Item | Source |
|---|---|
| host | `eTParam.eThost`, default `L"195.159.103.189"` (`Hk/HkGlobals.cpp:624`, `Juk/VGlobals.cpp:506`); key `ETHOST=host:port` (`Hk/HkInit.cpp:2446-2452`, `Juk/VInit.cpp:1926-1932`) |
| port | `eTport` default 1379 (`Hk/HkGlobals.cpp:625`); `_wtoi` without range check (`Hk/HkInit.cpp:2450`); `int` into WORD parameter → >65535 truncated [inferred] |
| path/query | hard-coded `/emitphp/get_stream.php?Y=<serial>&time=<date>%20<time>` + `&Mmin=<n>` when `msgno > 1` (`Hk/HkIV.cpp:3530-3533`, `Juk/VIv.cpp:2392-2395`) |
| date | `ETDATE=` or today (`HkIV.cpp:3515-3528`) |
| start time | `ETTIME=`, default `05:00:00` (`Hk/HkGlobals.cpp:632`) |
| serials | `ETGPRSy=a,b,c,d` or `ETSQL...` (`HkInit.cpp:2389-2407`) |
| poll interval | default `eThakuvali` 54 (`HkGlobals.cpp:627`); `ETHAKUVÄLI=n` → 2n (`HkInit.cpp:2411`); GUI × 50 ms (`cbTp/UnitEmitnouto.cpp:44`, `:77`) → about 2.7 s [inferred] |
| method | `GET` only (`gethttp.cpp:47`) |
| response | text lines → `tulk_emiTag` (`HkIV.cpp:3591`); message number from trailing `M<number>` (`:3594-3597`); server format [unknown] |

`cbTp/UnitEmitnouto.cpp:46-47` only displays host/port; change only through the config file [verified].

### 1.5 Parsing (availability only; memory safety in C-4)

- A line ≤ 30 characters, or a last line without terminator, hits `else *eTp = 0` (`HkIV.cpp:3601-3602`): rest of the batch dropped, `haettu = 0`, `msgno` not advanced → same data re-requested; a consistently short first line would stall polling permanently; real server behaviour [unknown].
- No status check: a 404/500/407 HTML body is parsed as data; `M` in HTML → `atoi` 0 → `msgno` 1 → next request without `Mmin` (everything from the start); effect depends on `tulk_emiTag` duplicate/garbage handling [unknown].

## 2. Transport security and settings

| Question | Finding |
|---|---|
| Proxy | `WINHTTP_ACCESS_TYPE_DEFAULT_PROXY` (`:134-137`) [verified]: machine-wide WinHTTP proxy, not per-user IE settings, no auto-discovery [inferred] |
| User agent | `"WinHTTP Example/1.0"` (`:134`) [verified] |
| HTTPS | supported by flag (`:50`) but both callers pass `0` (`HkIV.cpp:3540`, `VIv.cpp:2397`); no config key → plain HTTP on port 1379 [verified] |
| Certificates | no `WinHttpSetOption`, no `SECURITY_FLAG_IGNORE_*` [verified]; no TLS, so n/a |
| Authentication | none (`:54-56`) [verified] |
| Timeouts | none set [verified]; WinHTTP defaults resolve infinite, connect 60 s, send 30 s, receive 30 s [inferred, Microsoft docs]; read loop has no overall limit |
| Redirects/keep-alive | defaults; new session per poll [inferred] |
| Response size | `static char httpbuf[100000]`; per-chunk check `*dwTot + dwSize < buflen` (`:97`); a chunk that does not fit is silently skipped, later smaller ones appended; no error flag [verified] |
| Short reads | `memcpy` uses `dwSize` not `dwDownloaded` (`:98`) [inferred unlikely in sync mode] |
| Encoding | raw bytes as `char*` [verified] |

## 3. Error handling and loss of service

- Each poll runs in a worker; a slow server does not block `tausta` or the main thread [verified]. `inhttpHaku` stays 1 during the request (`HkIV.cpp:3507`, `:3556`), blocking new polls/parsing (`:3566`) for 30-60 s or more [inferred].
- `*dwTot` is updated per chunk, so on failure (`:70-71`, `:90-95`) the caller parses the partial buffer and shows the error (`:3540-3550`).
- GUI: `writeerror_w` (`cbTp/cbTpApu.cpp:467-478`) from the worker `SendMessage`s to the main window; `errorMsgHandler` (`:445-463`) shows modal `TFormSelect3` ("OK"/"Poistu ohjelmasta"), auto-close 2 s (`cbTp/UnitSelect3.cpp:63-124`); worker waits with `Sleep(200)` (`:475-477`). A permanently failing server → a modal dialog every poll cycle.
- Console: `writeerror_w` in `HkConsole/HkCons0.cpp:102`, `JukConsole/laskent0.cpp:59`; behaviour of `tputilv2/Writeerr.cpp:66` from a worker thread not traced [unknown].
- Messages: HK maps codes ≤ -3 to texts (`HkIV.cpp:3510-3514`, `:3541-3542`), positive via `FormatMessage` (H-157); relay only `"Virhe %d väliaikahaussa"` (`VIv.cpp:2399`) [verified].
- `GetLastError()` read after two `WinHttpCloseHandle` calls (`:148-149`, `:115`, `:119`) → failure might be reported as success [inferred, low].
- No backoff; retried at fixed interval; stopped only by `eThaku == 0` [verified]. Recovery of losses depends on the 10-message overlap.

## 4. Browser

| Item | Evidence |
|---|---|
| `TWebBrowser` in `FormBrowser` | `cbTp/UnitBrowser.dfm:196`; `SHDocVw.hpp` (`cbTp/UnitBrowser.h:27`) |
| `TWebBrowser` in `FormHtmlEdit` | `cbTp/UnitHtmlEdit.dfm:50` |
| Products | HkKisaWin, ViestiWin only |
| Engine | IE/MSHTML ActiveX [inferred] |

| Navigate call | Content |
|---|---|
| `cbHk/TulostusUnit2.cpp:766`, `ViestiWin/TulostusUnit2.cpp:477` | local `file:///<CurrentDir>\<kohdenimi>`, just generated HTML (`:758-766`) |
| `cbTp/UnitHtmlEdit.cpp:51` | local temp `___tmp_html_1.html` (`:36`, `:48-51`) |
| `cbTp/UnitBrowser.cpp:51` | any URL typed in `EditURL` [verified] |
| `cbHk/UnitKilpailijaOnline.cpp:1443` | link from user list `Tietolista.txt` (`:1338-1413`) |

No hard-coded remote URL [verified]. `TIEDOSTO` base accepted as remote only if it starts with `HTTP:` (`UnitKilpailijaOnline.cpp:1385`); `https://` becomes `file:///<CurrentDir>/https://...` (`:1385-1393`). No restriction of scripts, zones or navigation [verified]; consequence [unknown].

`setBrowserEmulation()` (`cbTp/cbTpApu.cpp:579-636`): reads `HKLM\SOFTWARE\Microsoft\Internet Explorer\Version` (`KEY_WOW64_32KEY`, `:591-592`), `svcVersion` for 9.1x (`:597-601`); IE ≥ 9 → writes REG_DWORD `1000*IEVersio` under HKCU (`:611-628`) named by exe (`cbHk/WinHk.cpp:1695-1703`, `ViestiWin/UnitMain.cpp:1523`). Calls `cbHk/WinHk.cpp:381`, `ViestiWin/UnitMain.cpp:290`. `EnvData.IEEmul < 9000` turns off "CBNaytaheti" (`cbHk/TulostusUnit2.cpp:239`) and shows a warning (`:1171`). `len` reused between the two `RegQueryValueEx` calls (`:592`, `:599`) and version parsed from the first digit [inferred, low].

No fallback if the control cannot be created; `Application->CreateForm(TFormBrowser)` at start-up (`RADStudio10/HkKisaWin.cpp:140`, `ViestiWin.cpp:121`) is a start-up risk; behaviour without MSHTML [unknown]. No TEdgeBrowser/WebView2 [verified].

## 5. Other internet-facing code

| Item | Location | Purpose |
|---|---|---|
| `195.159.103.189` | `Hk/HkGlobals.cpp:624`, `Juk/VGlobals.cpp:506` | default emit server |
| `255.255.255.255` | `Tp/TpComY.cpp:426` | UDP broadcast for a monitor connection (local) |
| `000.000.000.000` | `cbHk/UnitCfg.cpp:1714`, `:1720`, `:1843`, `ViestiWin/UnitCfg.cpp:1502` | UI placeholder |
| XML namespaces | `Hk/HkXml_IOF30.cpp:964-965`, `Hk/HkXmlTulokset.cpp:1682`, `Juk/VXml_IOF30.cpp:950-951` | namespace strings, no network |
| `system(cst)` / `_wsystem(cst)` | `Hk/HkTulostus.cpp:7345`, `Juk/VTulostus.cpp:7329` (`AUTOFILE`) | user-configured "autokomento" after automatic output |
| `system(st)` | `tputilv2/lue_v_pv.cpp:48` | `DATE d.m.y` sets the system date |
| `SHBrowseForFolder` | `cbTp/cbTpApu.cpp:351` | folder picker |
| `OleVariant` | `cbTp/UnitBrowser.cpp:57-65` | `TWebBrowser` event signatures |

No `ShellExecute`, `URLDownloadToFile`, WinINet, CryptoAPI/CNG, `CoInitialize`/`OleInitialize` [verified].

## 6. Versions and modernisation

- MSVC links `winhttp.lib` (`VS/Hk/HkMaali/HkMaali.vcxproj:126`, `:143`, `:164`, `:185`, `:206`; `VS/V/JukMaali520.vcxproj:105`, `:123`, `:145`, `:167`). GUI: `RADStudio10/HkKisaWin.cbproj:755` `..\cbHk\winhttp.lib`, `ViestiWin.cbproj:634` `winhttp.lib`; neither file exists in the tree; how the link resolves it [unknown].
- Binds to OS `winhttp.dll`; TLS would follow the OS [inferred]. API set is XP-era; no Windows 8.1+ features [inferred].
- `TWebBrowser` binds to the system MSHTML control and the emulation key; code handles IE 9-11 only (`cbTpApu.cpp:593-601`, `:625`). IE11 desktop app is retired; MSHTML remains a legacy component [inferred]. Replacement `TEdgeBrowser` needs the WebView2 runtime on each machine, matching C++Builder support and changed forms/event signatures (`UnitBrowser.cpp:57-69`) [inferred].
- Modernising HTTP needs `WinHttpSetTimeouts`, status check (`WinHttpQueryHeaders`), HTTPS option + config switch, server-side change; server response format must be known first.

## 7. Candidates (agent labels)

| ID | Sev. | Mechanism | Evidence | Products | Consequence | Confidence |
|---|---|---|---|---|---|---|
| T1 | medium | `lopetus1()` (main thread) waits `while (inhttpHaku) Sleep(200)` without message pump; if the worker is in `writeerror_w` (`SendMessage` to main window) neither proceeds; else exit delayed by WinHTTP timeout | `Hk/HkInit.cpp:3802-3804`, `Juk/vApu.cpp:1008-1010`, `cbHk/WinHk.cpp:802`, `ViestiWin/UnitMain.cpp:986`, `cbTp/cbTpApu.cpp:474-477` | HkKisaWin, ViestiWin | program may not close when the server fails at exit | [inferred] |
| T2 | medium | no timeouts; worker held tens of seconds; no poll/parse meanwhile | `com/gethttp.cpp:134-157` | 4 | splits late or in bursts | [inferred] |
| T3 | medium | modal error dialog on every failed poll (2 s auto-close) | `cbTp/cbTpApu.cpp:445-478`, `Hk/HkIV.cpp:3550` | GUI | operator interrupted repeatedly | [verified] mechanism |
| T4 | medium | no HTTP status check; error pages parsed; short head line stalls; stray `M` resets `msgno` | `Hk/HkIV.cpp:3586-3604`, `Juk/VIv.cpp:2480-2498` | 4 | stalled polling or full re-download | [inferred] |
| T5 | medium | chunks beyond 100000 bytes dropped silently | `gethttp.cpp:97-101` | 4 | messages lost without notice beyond the 10-message overlap | [verified] mechanism |
| T6 | medium | plain HTTP, hard-coded public IP, non-standard port, sample UA, no auth | `Hk/HkGlobals.cpp:624-625`, `HkIV.cpp:3540`, `gethttp.cpp:134` | 4 | integrity/confidentiality; fixed server | [verified] |
| T7 | low | relay shows only the error number | `Juk/VIv.cpp:2397-2400` | ViestiMaali, ViestiWin | cause unknown to operator | [verified] |
| T8 | low | `haettu`, `inhttpHaku`, `buf` shared without lock/volatile | `Hk/HkIV.cpp:3507`, `:3566-3604` | 4 | C-6 area, likely benign | [inferred] |
| T9 | low | `https://` base link rewritten as local path | `cbHk/UnitKilpailijaOnline.cpp:1385-1393` | HkKisaWin | remote https link fails | [verified] |
| T10 | low | `setBrowserEmulation` `len` reuse, first-digit parse | `cbTp/cbTpApu.cpp:592-601` | GUI | IE10/11 may fall back to 9000 | [inferred] |
| T11 | low | doc error TEdgeBrowser | `riippuvuudet.md:126` | n/a | misleading | [verified] |
| T12 | low | `dwDownloaded` ignored; `GetLastError()` after close | `gethttp.cpp:98`, `:148-149` | 4 | rare | [inferred] |
| T13 | low | dead `GetHttpForm`; `winhttp.lib` paths absent | `cbHk/WinHk.cpp:1167-1171`, `RADStudio10/HkKisaWin.cbproj:755` | HkKisaWin | maintenance | [verified] |

Open questions: Q-C1 response format of `get_stream.php`; Q-C2 does `tulk_emiTag` tolerate duplicates/garbage; Q-C3 how the GUI link finds `winhttp.lib`; Q-C4 console `writeerror_w` from a worker thread; Q-C5 behaviour when MSHTML is unavailable; Q-C6 is `195.159.103.189:1379` still operated and by whom; Q-C7 is the `eThakuvali` unit 50 ms.
