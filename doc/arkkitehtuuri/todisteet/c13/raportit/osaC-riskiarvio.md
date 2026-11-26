# C-13 part C: uniform risk and priority assessment (draft)

Scope: one uniform priority scale for all 177 H rows (176 open, H-099 refuted). Inputs: `rekisteri/havainnot.md` (all H rows read in full), `osaA0/havainnot.csv`, the per-batch priority rationales (`muistiturvallisuus.md` 15, `seuranta.md` "Priorisoitu korjausjono", `saikeet-ja-lukot.md` 13.9, `gui-liiketoimintalogiikka.md` 11, `globaali-tila.md` 11-12, `virheenkasittely.md` 13.3, `riippuvuudet.md` 19.3, `ominaisuusvalitsimet.md` 13.2-13.3, `kaantajaerot.md` 13.6-13.7) and the part B relations (`osaB/suhteet.csv`). Output: `osaC/riskiarvio_luonnos.csv` (one row per H). This is not a new audit; no build or run. Macro values recorded by the C-10/C-12 scripts were not used.

Source code was opened only for borderline reachability, each noted in the row `huomio`: `Hk/HkCom32.cpp:600-660` and `Hk/HkGlobals.cpp:461` (H-008, H-060: EXTRA d1=6 reaches `tulkKurvinen` without a gate, `sak_laji` defaults to 2); `Hk/HkSak.cpp:55-175` (H-008/H-060 branch split); `Juk/VTulostus.cpp:2215-2230` and callers (H-001 guard `paikat[osuus]`); `Hk/HkEmit.cpp:3375-3410` (H-012 branch); `Hk/HkIx.cpp:1828-1840` (H-013 loop); `ViestiWin/UnitAjanotto.cpp:990-1106` (H-069 `srj`); `Juk/VIv.cpp:2680-2702` (H-054, inconclusive). Labels: [todennettu] read in code or register; [paatelty] inferred; [tuntematon] not established.

## 1. How P1-P5 has been used

| Batch | Definition used | Notes |
|---|---|---|
| C-4 (`muistiturvallisuus.md` 15) | severity x reachability: P1 network with default settings; P2 network or device with a setting; P3 shipped build, ordinary data; P4 local file or operator; P5 low (wrong value or display) | Applied to K1 and C-4 rows. Medium x network default gave P2 (H-060), but H-061 (medium) was put in P1 with "verify first (Q43)". A separate "urgent" flag (H-071, H-072, H-055, H-070) cut across the levels. |
| C-5 (`seuranta.md`) | C-4 scale, "provisional" | All local file or operator: P4/P5. |
| C-6 (13.9) | P2 = freeze or crash during a competition with an ordinary user action or setting | First use of P2 without network or device input (H-087, H-090, H-091, H-093, H-107). |
| C-7 (11) | C-4 scale "alongside C-6": P2 = data corruption or stopped workers in ordinary use | H-109, H-110 operator actions in P2; P3 for silent wrong results from forms (H-111..H-117). |
| C-8 (11-12) | only H-129 placed, P2 "provisional" (no attacker) | 15 other C-8 rows left without priority. |
| C-9 (13.3) | C-4 levels; H-129 cited as precedent; P2 = memory error recurring in the common configuration (H-149) or certain resource exhaustion within a day with a setting (H-150 P3 -> P2) | Environment-fault triggers (disk full) kept at P3 (H-145). |
| C-10 (19.3) | restated: P1 network default; P2 can stop timing or corrupt data during a competition in ordinary use or with a common setting; P3 medium or rarer precondition; P4 low; P5 roll-ups and cosmetic | H-160 P3 -> P2 although SQL is optional. |
| C-12 (13.2-13.3) | P2 = recurring overwrite in normal use (H-171), not P1 because not network-triggerable | H-134 P3 although fixed with H-129 (P2). |

K1, C-1, C-2, C-3 and most of C-8 never received priorities (61 open rows, 12 of them high).

## 2. Uniform definition

Priority is a fix-order judgement; technical severity (`tekninen_vakavuus`) is the impact if triggered. A row gets the highest level whose condition it meets:

| P | Condition (uniform) |
|---|---|
| **P1** | A third party on the timing network can trigger it with default settings (only the ordinary link enabled; one or a few forged packets), **and** the impact is high integrity loss: memory or file write, file overwrite, or silent result corruption. |
| **P2** | (a) Network-default reachable with medium or availability-only high impact; or (b) network with a setting, or a timing device, with at least medium impact; or (c) without any attacker, in ordinary operation (ordinary data, routine operator actions, or a setting used in real events) it stops, freezes or crashes the program or a required thread during a competition, or silently corrupts, loses or misattributes results, and it recurs or is likely in an ordinary event of that kind. |
| **P3** | Reachable in a shipped product with ordinary data, a setting or an operator action, but either the impact is narrower (one feature or output stops, visible error, restart recovers) or the P2 impact needs a rarer precondition (environmental I/O error, rare option combination, very large event, stage change in a running process, operator holding a dialog). Also matala-keskitaso rows with ordinary-use or device reach. |
| **P4** | Needs a malformed or hand-edited local file, or a specific operator mistake with limited effect; or low severity with a functional effect. Low severity caps at P4 even when network-reachable (H-033, H-101). |
| **P5** | Low: cosmetic, wrong value or display without effect on times or results, practically dead or not shipped (`ei_toimitetussa`), and roll-up rows (with items flagged separately, section 5). |

Rules used with the table:

1. **Severity and priority stay separate.** A korkea row can be P4 (H-002: legacy file only) and a keskitaso row can be P2 (H-060, H-150). Where I disagree with the register severity, `huomio` says so (H-107 keskitaso -> keskitaso-korkea, H-158 matala -> keskitaso).
2. **Verification strength does not lower a confirmed mechanism.** `todennus` is copied from the base (experiment if any, else review state). A reproduced mechanism keeps its rank even if the full consequence was not run (H-025, H-040, H-129). Where reachability or the harmful effect itself is [paatelty] or [tuntematon], `huomio` names the assumption and, where it matters, the level the row would move to (H-061 back to P1 if Q43 shows an overflow; H-030 to P2 if XML-TCP links are common; H-115 to P2 if the VCL exception is confirmed).
3. **Fix-together pairs.** The row priority is the standalone rank. A fix package is scheduled at the highest priority of its members, so lower members (H-134, H-102, H-161 ...) are fixed in that slot without changing their own value.
4. **Order inside a level** (proposal): C-4 urgent flags first (H-055, H-070 in P2; H-071, H-072 in P1), then deterministic or recurring before timing-dependent, run-confirmed before inferred, wider installations before narrow (C-10: H-159 before H-160).

### 2.1 Old -> new correspondence

| Old (batch usage) | New | Effect |
|---|---|---|
| P1 network default (C-4) | P1, now explicitly also high integrity impact | H-032, H-071, H-072 stay; H-061 (medium, effect unknown) -> P2 |
| P2 network or device with a setting (C-4) | P2 (b); network-default medium explicitly P2 (a) | no change for C-4 rows; adds H-008, H-024, H-028, H-029, H-026 (availability only) |
| P2 freeze, crash or corruption in ordinary use (C-6..C-12) | P2 (c) | codified; applied to old unprioritised rows (H-007, H-022, H-023, H-025, H-027, H-031, H-040, H-128) and to H-111, H-151, H-164 |
| P3 ordinary data (C-4) / medium or rarer precondition (C-10) | P3 (union) | C-9 environment-fault rule kept (H-145) |
| P4 local file or operator / low (C-4, C-10) | P4, low severity capped here | unchanged |
| P5 low; roll-ups (C-10) | P5 plus `ei_toimitetussa`; roll-up items flagged | unchanged values |

## 3. Distribution and changes

| | none | P1 | P2 | P3 | P4 | P5 | total |
|---|---|---|---|---|---|---|---|
| before (base) | 61 | 4 | 18 | 37 | 37 | 19 | 176 |
| after (this draft) | 0 | 3 | 35 | 62 | 49 | 27 | 176 |

H-099 (refuted) is excluded; the base count 62 without priority includes it.

Changed priorities (had a value):

| ID | old | new | reason |
|---|---|---|---|
| H-061 | P1 | P2 | Uninitialised pkglen on partial serial or binary-TCP packets plus an unchecked class index read: mechanism confirmed, but the run-time value and so any write are unknown (Q43), and medium x network default gives P2, not P1. |
| H-064 | P4 | P3 | Deterministic NULL dereference at card reading in console relay patrol events with AUTORAP and no list printer; a crash in the core timing path with a setting is P3 under the uniform rule. |
| H-069 | P5 | P4 | The never-set srj in the ViestiWin timing grid feeds tulkOsuuskoodi, so a typed leg code can resolve against a garbage series and a time can land on the wrong leg; raised from P5. |
| H-111 | P3 | P2 | Raised: saving a change on an A or L row in the HK timing panel turns it into a finish row and writes the start-clock time as the competitor finish, a silent wrong result in routine corrections in start-clock events. |
| H-151 | P3 | P2 | Raised: the HK counterpart of H-149; after any TCP send the next autofile cycle reads freed memory and frees the PRFILE again in H/M mode, recurring each cycle; reproduced with ASan. |
| H-152 | P4 | P3 | Raised: changing the result printer from the menu leaves the auto-print alias on a freed PRFILE used at every automatic print; a routine menu action, not reproduced. |
| H-158 | P5 | P3 | Raised: with XML= the relay automatic XML file always holds only the closing part, the same output failure as H-172 (P3). |
| H-164 | P3 | P2 | Raised: an unplugged or crashed USB-serial reader is never reopened and not reported, so the device feed stops silently until restart, the same impact as H-159 (P2) and more silent. |

Newly assigned (had none), by level:

- P1 (0): -
- P2 (13): H-007, H-008, H-022, H-023, H-024, H-025, H-026, H-027, H-028, H-029, H-031, H-040, H-128
- P3 (25): H-009, H-010, H-011, H-012, H-013, H-014, H-015, H-016, H-017, H-018, H-021, H-030, H-038, H-039, H-043, H-044, H-045, H-046, H-047, H-050, H-130, H-131, H-136, H-137, H-140
- P4 (13): H-033, H-034, H-036, H-048, H-051, H-053, H-054, H-132, H-133, H-135, H-138, H-139, H-141
- P5 (10): H-004, H-005, H-006, H-019, H-020, H-035, H-037, H-142, H-143, H-144

Severity x priority after: keskitaso/P2 8, keskitaso/P3 52, keskitaso/P4 12, keskitaso/P5 1, keskitaso-korkea/P2 2, korkea/P1 3, korkea/P2 25, korkea/P3 5, korkea/P4 1, matala/P4 21, matala/P5 26, matala-keskitaso/P3 5, matala-keskitaso/P4 15.

## 4. Borderline cases

- **12 unprioritised high rows (C-1..C-3).** P2: H-007 (silent lost update, propagated), H-022 (results to wrong competitor), H-023 (wrong stage, multi-stage), H-025 (corrupted packet tail with valid checksum), H-026, H-027 (TCP link never retries, TCPXML crash), H-031 (HkMaali background processing stops after one refused stage change), H-040 (relay addfrec lock leak) and H-008, H-024 (one shooting or TARKT message orphans `tall_CS`; consistent with H-110). All ten P2 rows are reproduced at mechanism level in the C-11 harness; full consequences (freeze, propagation, frequency) were not run, which `huomio` states. P3: H-009 (deadlock candidate, cycle not reproduced, ranked with H-092) and H-014 (SQL mirror only, master intact; the terminating form is H-160 P2).
- **H-061** (medium, P1 in C-4). Mechanism [todennettu], run-time value [tuntematon] (Q43), class index read only. Uniform rule gives P2 (medium x network default). Decision for the main session: keep P2, or keep P1 on the argument that it is fixed in the same receive-validation change as H-032; either way it is fixed with H-032.
- **H-041** (only shipped-program confirmation). Stays P3: needs n_pv 5 and valuku >= 221, a configuration fixed before the event and visible at the first addition. Confirmation does not raise the rank; reachability, not proof strength, decides.
- **H-026**. Network default, one packet (or a peer running another program version) stops all receiving permanently. P2, not P1, because no memory or file is written. It is also a compatibility hazard: mixing versions with a new packet class silently splits a relay network. Main session may choose P1 if availability from the network is to count as P1.
- **H-029, H-016, H-030, H-039 (protocol design).** H-029 is network default and the spoofing root of H-071/H-072: P2 (a). H-016, H-030, H-039 need concurrent edits during an outage, a resend tool or XML-TCP, or a meshed topology: P3. Their fix is a protocol change (version or origin, authentication), so the row priority is less useful than a design decision; recommend one owner decision covering T-022 and these four.
- **H-128 vs H-129.** H-129 confirmed P2 (heap overflow at an HK stage change from about 1,500 competitors; mechanism reproduced, overflow arithmetic). H-128 raised from none to P2: routine deletes and bib changes silently drop competitors from bib-order walks (export, draw, penalty and points lists, course recognition), reproduced; heals on restart. They share `setpos`/`idindex` and are fixed together; H-128 also lowers the H-129 threshold.
- **H-134** stays P3 (missing or unparsable stage course XML disables emit checking with a message; parse branch touches freed memory). Fixed in the stage-change package with H-129, scheduled at P2 (rule 3).
- **H-014 vs H-160.** SQL is optional, but when enabled H-160 terminates the whole program (P2, C-10). H-014 silently stops only the mirror writer (P3). Same package KK-C10-1, scheduled at P2.
- **H-107** stays P2: with RASTIVATULOSTE or ToimintaTila 1 HkKisaWin dereferences a NULL form from a worker thread at the first card read, and again after each restart until the form is opened. How often the option is used is [paatelty]; severity read as keskitaso-korkea.
- **H-111** raised P3 -> P2: in HK start-clock events (ski starts, LAJAT, waves) saving a change on an A or L row writes the start-clock time as the finish, a silent wrong result in a routine correction. If the owner considers L-row editing rare, P3 is defensible.
- **H-145** stays P3 (first in P3): the card is ACKed (Emit clock) but missing; needs an environmental write failure and the operator sees a modal message; recovery by hand or restart and re-read.
- **Other changes.** H-151 P3 -> P2 (HK counterpart of H-149: use-after-free and double free each autofile cycle after a TCP send, reproduced with ASan; Release crash not measured). H-164 P3 -> P2 (vanished USB-serial reader never reopened and not reported; same impact as H-159 P2 and more silent; fix pair). H-064 P4 -> P3 (deterministic crash at card reading in a rare console relay configuration). H-152 P4 -> P3 (routine printer change leaves a freed alias used by auto printing). H-158 P5 -> P3 (relay automatic XML always near-empty with `XML=`; same failure as H-172 P3). H-069 P5 -> P4 (never-set `srj` in the ViestiWin timing grid can put a time on the wrong leg; effect not traced).
- **Further candidates not changed:** H-001 (recurring stack zeroing on biathlon-relay text output, Release effect not run; P3, would be P2 under the H-171 argument if biathlon relays are common), H-115 (P2 if the VCL exception is confirmed), H-003 (two crash sites untraced), H-054 (rewrite trigger untraced), H-028 and H-029 (medium network rows now P2; main session may prefer P3 for H-028 since a TCP server port is a setting and the lock-hold effect is inferred).

## 5. Roll-up rows

All roll-ups stay P5 as rows (C-10 rule), but some items deserve a higher level and should get their own ids before the fix phase. [paatelty] unless stated; most roll-ups are not independently reviewed.

| Row | Items above P5 | Suggested |
|---|---|---|
| H-126 (ViestiWin minor) | none above P4 (N-11 negative leg index, N-24 memo number consumed) | P4 items |
| H-127 (HK minor) | N-22 `tall_tulos(k_pv, 0)` arguments swapped could store a result on the wrong point or stage | own row, P3 if confirmed |
| H-142 (integrity) | N-C14 series deletion can set series -1 (cf. H-114); N-C11 leg-count change keeps split links | P4 |
| H-143 (lifecycle) | N-B6 `maxvaluku`/`lsakmax` not updated on the XML path (size limits, cf. H-041/H-042); N-B8 `kaksibadge` 2 -> 1 keeps `emitjarr[1]` (cf. H-140 P3) | own rows, P3 |
| H-144 (threads) | none (N-D6 ~ H-019) | - |
| H-156 (persistent data) | N-B7 = H-071 (already P1 by own row); N-B9 `vaihevalinta.cfg` emptied before an unchecked write -> wrong stage after restart (cf. H-023) | N-B9 own row, P3 |
| H-157 (error paths) | `samatleimat` = H-168 (P4 own row); others P4 at most | - |
| H-170 (dependencies) | N-11 `comajanotto` spins when the port vanishes (same fix as H-164, P2); N-13 `sallisummavirhe` skips the checksum for all readers (P3) | N-11 into KK-C10-2 at P2; N-13 own row P3 |
| H-176 (settings) | ViestiLuenta SIRIT/ZEBRA/FEIG/IMPINJ key selects ALGE timing (T-052): the reader does not work | own row P3 (visible at setup) |

## 6. Consistency checks

Checks run by script on `osaC/riskiarvio_luonnos.csv`: 177 rows, ids H-001..H-177 unique and in order, all cells within the fixed vocabularies, `peruste` present for every changed or new row.

(c) Rows with `ajanoton_jatkuvuus = pysayttaa` and reach verkko_oletus, verkko_asetus, laite or normaalikaytto below P3: none. Stopping rows at P4 all need a local file, an operator action or a legacy setting: H-002 (paikallinen_tiedosto), H-049 (paikallinen_tiedosto), H-057 (paikallinen_tiedosto), H-058 (paikallinen_tiedosto), H-063 (operaattori), H-074 (paikallinen_tiedosto), H-075 (asetus), H-078 (paikallinen_tiedosto), H-079 (paikallinen_tiedosto), H-085 (operaattori), H-125 (operaattori), H-153 (operaattori).

(a), (b) Pairs from `osaB/suhteet.csv` (korjattava_yhdessa, hk_viesti_vastine, ristiriita; H rows only):

| relation | A | P | B | P | |
|---|---|---|---|---|---|
| hk_viesti_vastine | H-149 | P2 | H-151 | P2 | same |
| hk_viesti_vastine | H-117 | P3 | H-121 | P4 | differs |
| hk_viesti_vastine | H-126 | P5 | H-127 | P5 | same |
| hk_viesti_vastine | H-123 | P4 | H-124 | P4 | same |
| hk_viesti_vastine | H-130 | P3 | H-139 | P4 | differs |
| hk_viesti_vastine | H-041 | P3 | H-042 | P3 | same |
| hk_viesti_vastine | H-106 | P3 | H-018 | P3 | same |
| korjattava_yhdessa | H-129 | P2 | H-134 | P3 | differs |
| korjattava_yhdessa | H-129 | P2 | H-102 | P4 | differs |
| korjattava_yhdessa | H-129 | P2 | H-128 | P2 | same |
| korjattava_yhdessa | H-129 | P2 | H-133 | P4 | differs |
| korjattava_yhdessa | H-134 | P3 | H-131 | P3 | same |
| korjattava_yhdessa | H-134 | P3 | H-132 | P4 | differs |
| korjattava_yhdessa | H-134 | P3 | H-133 | P4 | differs |
| korjattava_yhdessa | H-091 | P2 | H-092 | P3 | differs |
| korjattava_yhdessa | H-092 | P3 | H-103 | P3 | same |
| korjattava_yhdessa | H-093 | P2 | H-091 | P2 | same |
| korjattava_yhdessa | H-160 | P2 | H-161 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-162 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-163 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-169 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-014 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-043 | P3 | differs |
| korjattava_yhdessa | H-160 | P2 | H-100 | P4 | differs |
| korjattava_yhdessa | H-159 | P2 | H-164 | P2 | same |
| korjattava_yhdessa | H-138 | P4 | H-108 | P4 | same |
| korjattava_yhdessa | H-159 | P2 | H-138 | P4 | differs |
| korjattava_yhdessa | H-149 | P2 | H-150 | P2 | same |
| korjattava_yhdessa | H-149 | P2 | H-172 | P3 | differs |
| korjattava_yhdessa | H-034 | P4 | H-168 | P4 | same |
| korjattava_yhdessa | H-141 | P4 | H-168 | P4 | same |
| korjattava_yhdessa | H-025 | P2 | H-050 | P3 | differs |
| korjattava_yhdessa | H-011 | P3 | H-137 | P3 | same |
| korjattava_yhdessa | H-078 | P4 | H-079 | P4 | same |
| korjattava_yhdessa | H-036 | P4 | H-171 | P2 | differs |
| korjattava_yhdessa | H-171 | P2 | H-173 | P3 | differs |
| korjattava_yhdessa | H-065 | P4 | H-002 | P4 | same |
| ristiriita | H-008 | P2 | H-110 | P2 | same |
| ristiriita | H-024 | P2 | H-110 | P2 | same |
| ristiriita | H-040 | P2 | H-087 | P2 | same |
| ristiriita | H-014 | P3 | H-160 | P2 | differs |
| ristiriita | H-015 | P3 | H-145 | P3 | same |
| ristiriita | H-044 | P3 | H-145 | P3 | same |
| ristiriita | H-061 | P2 | H-032 | P1 | differs |
| ristiriita | H-029 | P2 | H-071 | P1 | differs |
| ristiriita | H-055 | P2 | H-032 | P1 | differs |
| ristiriita | H-134 | P3 | H-129 | P2 | differs |
| ristiriita | H-170 | P5 | H-164 | P2 | differs |
| ristiriita | H-156 | P5 | H-071 | P1 | differs |

Other HK/relay counterparts named in rows: H-149/H-151 both P2; H-041/H-042 both P3; H-087 (relay) vs H-091 (HK) both P2; H-093 HK only (relay refuted); H-107 HK only (relay refuted).

Explanations for the differing pairs (rule 3 unless stated):

- Stage-change package (H-129 P2 with H-134 P3, H-102 P4, H-133 P4; H-134 with H-132 P4, H-133 P4): one change in `vaihdavaihe`/`teejarj`/`initindex`; scheduled at P2; members keep their own impact rank.
- Lock hierarchy (H-091 P2 with H-092 P3): H-091 edges reproduced and tied to a GUI option; H-092 window is one report. One lock-order change fixes both at P2.
- KK-C10-1 (H-160 P2 with H-014, H-043, H-161, H-162, H-163, H-169 P3 and H-100 P4): only H-160 terminates the program; the package is scheduled at P2 as C-10 proposed.
- KK-C10-2 (H-159 P2 with H-138 P4): back-off delay only; scheduled at P2.
- KK-C12-1 (H-149 P2 with H-172 P3) and KK-C12-2 (H-171 P2 with H-036 P4, H-173 P3): settings and publication packages scheduled at P2.
- H-025 P2 with H-050 P3: the viewer is a diagnostic tool; the queue stride fix covers both.
- Counterparts H-117 P3 / H-121 P4: opposite one-sided fixes in the same timing form; H-117 silently clears a result, H-121 only ignores a forcing key and skips one link. H-130 P3 / H-139 P4: relay counts feed a buffer size (overflow possible), HK counters only affect counts.
- Part B conflicts now consistent: H-008, H-024, H-040 equal H-110/H-087 (P2); H-015, H-044 equal H-145 (P3). Remaining differences are intended: H-014 vs H-160 (above), H-061 vs H-032 (section 4), H-029 vs H-071 (root cause vs payload defect; fixed separately), H-055 vs H-032 (urgent flag kept as ordering inside P2, rule 4), H-134 vs H-129 (rule 3), H-170 and H-156 roll-up items (section 5).

## 7. Limitations

- Draft for the main session; no priority in the register was changed by this file.
- Reachability classes are taken from the register and batch reports; only the cases listed at the top were re-read in source. Network reachability beyond the analysed packet fields stays [tuntematon] (`muistiturvallisuus.md` 15).
- Frequency of settings (XML-TCP links, biathlon relays, TUNNJONO, RASTIVATULOSTE, AUTOFILE.LST with XML, multi-reader setups) is [tuntematon]; several P2/P3 boundaries depend on it and are named in `huomio`.
- Roll-up rows are rated as rows; their items were not re-verified.
- HK and relay, console (MSVC) and GUI (bcc32) differences are recorded in `huomio` but a row carries one priority for its most exposed product.
- The P2 level grew from 18 to 35; most additions are old high rows that never had a priority. If the main session wants a smaller P2, the documented order inside the level (rule 4) is the intended lever rather than changing the definition.
