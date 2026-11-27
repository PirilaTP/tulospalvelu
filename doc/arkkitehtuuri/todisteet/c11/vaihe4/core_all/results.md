# Core test results

| | |
|---|---|
| started | `2026-11-24T22:18:26` |
| git_head | `432ea3df6c8759d894a59e36c6f9a3ed3e57f85c` |
| git_branch | `testit/vikojen-todentaminen` |
| production_rev | `fc03b67cd19c130b1bd1474416c9d1e644c829e3 2026-11-07T08:55:28+02:00` |
| production_dirty | `False` |
| tests_rev | `432ea3df6c8759d894a59e36c6f9a3ed3e57f85c 2026-11-24T21:33:16+02:00` |
| tests_dirty | `True` |
| repo_dirty | `True` |
| host | `Windows-11-10.0.26100-SP0` |
| python | `3.13.5` |
| doctest_version | `2.4.11` |
| summary | `{'PASS': 23, 'XFAIL': 24}` |
| TpCoreJuk | `TPsource/V52/VS/Tests/Release/TpCoreJuk.exe` sha256 `29e467e77b0369c0`; program=TpCoreJuk, msc_full_ver=195237029, config=Release, asan=0, char_unsigned=1, MAXOSUUSLUKU=30, VAIKALUKU=12, MAXSARJALUKU=250, sizeof_void_ptr=4 |
| TpCoreHk | `TPsource/V52/VS/Tests/Release/TpCoreHk.exe` sha256 `3650831ff8526c0c`; program=TpCoreHk, msc_full_ver=195237029, config=Release, asan=0, char_unsigned=1, sizeof_void_ptr=4 |
| TpCoreHkAsan | `TPsource/V52/VS/Tests/Asan/TpCoreHk.exe` sha256 `729d0eee4e0a5ff4`; program=TpCoreHk, msc_full_ver=195237029, config=Release, asan=1, char_unsigned=1, sizeof_void_ptr=4 |
| TpCoreJukAsan | `TPsource/V52/VS/Tests/Asan/TpCoreJuk.exe` sha256 `314c146ca96e3d95`; program=TpCoreJuk, msc_full_ver=195237029, config=Release, asan=1, char_unsigned=1, MAXOSUUSLUKU=30, VAIKALUKU=12, MAXSARJALUKU=250, sizeof_void_ptr=4 |

| ID | Kind | Finding | Status | Verification | Actual |
|---|---|---|---|---|---|
| juk-build-info | info |  | **PASS** | - (kontrolli / control) | all checks passed |
| hk-build-info | info |  | **PASS** | - (kontrolli / control) | all checks passed |
| hk-smoke | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| hk-asan-build-info | info |  | **PASS** | - (kontrolli / control) | all checks passed |
| juk-asan-build-info | info |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-025a | defect | H-025 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | n_os=4: combufsize=204 largest class=13 len=204 header+len=218; class 13 does not fit; n_os=30: combufsize=244 largest class=9 len=244 header+len=258; class 9 does not fit;  |
| H-025b | defect | H-025 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | n_os=4 class=13 len=204 header=14 packet=218 stride(combufsize)=204 bytes_not_stored=14 file_size=612 (=3 strides) slot1_whole=0 slot1_tail_position_holds_slot2_header=1; n_os=30 class=9 len=244 header=14 packet=258 stride(combufsize)=244 bytes_not_stored=14 file_size=732 (=3 strides) slot1_whole=0 slot1_tail_position_holds_slot2_header=1;  |
| H-040-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-040 | defect | H-040 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | d=1 numrec=2 other_thread_got_lock=0 owner_is_caller=1 RecursionCount=2 (internal field; 2 = two Enter, no Leave) |
| H-042-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-042 | defect | H-042 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | luesarjat=0 n_os=31 (max 30) n_os_akt=31 valuku=13 (max 12) ui_messages=0; derived kilprecsize=7330 vs KILPRECSIZE=6918 (overflow itself not run) |
| H-041a | defect | H-041 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | n_pv=5 valuku=250 kilprecsize0=360 kilppvtpsize0=152 vatpsize=8 kilppvtpsize=2168 kilprecsize=11200 KILPBUFSIZE=10000 |
| H-041b3 | defect | H-041 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | n_pv=5 valuku=250 kilprecsize0=360 kilppvtpsize0=152 vatpsize=8 kilppvtpsize=2168 kilprecsize=11200 KILPBUFSIZE=10000; pack wrote bytes [0,11200), 1200 bytes at or after offset 10000 |
| H-041b2 | defect | H-041 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | ASan report matched all expected strings; WRITE of size 8 at 0x02cff738 thread T0 |
| H-041b4-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-041b4 | defect | H-041 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | ASan report matched all expected strings; WRITE of size 8 at 0x003befb8 thread T0 |
| H-031-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-031 | defect | H-031 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | refused: r1=1 flag_after=1; later allowed call: r2=0 k_pv=0 arv_pv=0 vaiheenvaihto=1 lah_alku0=0 new_messages=0 (control: k_pv becomes 1) |
| H-008-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-008-control-valid | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-008 | defect | H-008 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | worker returned in 15 ms; other thread got lock=0; owner=109180 worker=109180 (worker has ended: lock orphaned); message: Viallinen sanoma ampumapaikalta, virhe -3 (ampumakerta) |
| H-024-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-024 | defect | H-024 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | packet consumed; worker ended in 31 ms; other thread got lock=0; owner=67208 worker=67208 (worker ended: lock orphaned); leg 1 status still K |
| H-027-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-027a | defect | H-027 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | (i) SYNs seen while rejecting=5, comopen after failure=0, hComm0=1, TCP_on0=0x0100007F; (ii) client thread ended=1 after 2000 ms; attempts in the following 15 s while accepting=0; comopen at end=0 |
| H-027b | defect | H-027 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | read access to 0x0000000C on thread tcp-client in reconnectTCP |
| H-032-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-032a | defect | H-032 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | class 7 (class_len 0) len 1000 into slot 49 of 51 x 258 bytes: 498 bytes past the block overwritten (expected 498), all packet data=1; the UDP length check itself (:1001) is shown by class_len 0 only |
| H-032b | defect | H-032 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | ASan report matched all expected strings; WRITE of size 1014 at 0x05a03466 thread T0 |
| H-026-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-026 | defect | H-026 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | worker still running after comfl=0; inbens=0 (valid packet at slot 1 processed=0); loop alive=1, error calls in 300 ms=6, total=85 |
| H-022-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-022 | defect | H-022 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | packet bib 13 for record 1 (bib 12): tasoitus=600, messages=0; the packet's bib field is never read; two-station same-record-number allocation (Hk/HkDat.cpp:626-636) not run |
| H-023a | defect | H-023 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | packet for stage 2 (d.v.k_pv=1), receiver k_pv=0: stage 1 tasoitus=600, stage 2 tasoitus=0, messages=0 |
| H-023b-control-refused | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-023b-control-allowed | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-023b | defect | H-023 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | stage change to 2 refused (k_pv stays 0, message: Avoimen yhteyden l?hett?m?tt? olevat sanomat est?v?t vaihdon); tark_alku returned false (link accepted) |
| H-007-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-007 | defect | H-007 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | local club change to UUSI made while the receive thread waited at tall_CS; after the packet: club=VANHA, tasoitus=600 |
| H-001a | defect | H-001 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | ASan report matched all expected strings; WRITE of size 160 at 0x0142d450 thread T0 |
| H-001b | defect | H-001 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | ASan report matched all expected strings; WRITE of size 160 at 0x00ded6f8 thread T0 |
| H-002-control | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| H-002a | defect | H-002 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | parameter line '1 1 1 0 0 4242': luesarjat=0; bytes changed in the reserved page=5; at 0x30303030: "4242" (expected "4242"); the address is the int value 0x30303030 of paikat[0] after memset '0' |
| H-002b | defect | H-002 | **XFAIL** | toistettu testikehyksessa / reproduced in test harness | parameter line '1 1 1 0 0 0 0 0 TPH002': luesarjat=0; bytes changed in the reserved page=7; at 0x30303030: "TPH002" (expected "TPH002"); the address is the int value 0x30303030 of paikat[0] after memset '0' |
| hk-data-run1-kilpsrj | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| hk-data-h041-v220 | control |  | **PASS** | - (kontrolli / control) | all checks passed |
| hk-data-h041-v221 | control |  | **PASS** | - (kontrolli / control) | all checks passed |

Details, commands, expected results and data: `results.json` and `<id>.log`.
