# Spoofiq Release Plan: Beta 2 → v1.0

**Goal**: Deliver production-ready v1.0 by **end of Q1 2025**  
**Current state**: v0.5.0-Beta2 (image batch processor, stable)  
**Strategy**: Minimal viable 1.0 with deferred video/advanced features to 1.1+

---

## v1.0 Release Criteria

### Must-Have (Blocking)
- ✅ Image batch processing (working)
- ✅ Device preset management (working)
- ✅ Event-driven logging (working)
- ✅ Cross-platform (Linux/Windows, macOS pending GL test)
- ✅ All MSVC warnings fixed (strict compiler flags enabled)
- ✅ 50+ test assertions pass (Catch2 v3)
- 📋 CI/CD pipeline produces signed/notarized binaries
- 📋 Final test on Windows MSVC + Linux Clang18

### Nice-to-Have (Won't Block)
- File browser (nfd requires GTK3, can defer)
- Video processing (requires TagLib, v1.1+)
- GPS randomization (v1.1+)
- Timestamp randomization (v1.1+)

---

## Phases

### Phase 1: Stability & Polish (Week 1-2)
**Goal**: Fix all warnings, verify cross-platform builds

#### Tasks
1. **Compiler warnings** ✅ DONE
   - MSVC C4267, C4100, C4244 fixed
   - Enable `-DSPOOFIQ_WARNINGS_AS_ERRORS=ON` globally
   - Verify clang18, gcc15, MSVC all build warning-free

2. **Platform testing**
   - [ ] Test on Windows MSVC (debug-msvc + release-msvc)
   - [ ] Test on Linux Clang18 (clang18-libcxx)
   - [ ] Test on Linux GCC (debug-gcc, release-gcc)
   - [ ] Manual smoke test: batch process 10 images, verify EXIF

3. **Documentation**
   - [ ] Update CHANGELOG: Beta 2 → Release notes
   - [ ] Verify README matches current feature set
   - [ ] Add troubleshooting section (Windows path issues, missing GL libs)

**Deliverable**: All platforms green, warnings = 0

---

### Phase 2: Release Engineering (Week 2-3)
**Goal**: Build system, CI/CD, versioning

#### Tasks
1. **Version & Release checklist**
   - [ ] Bump version: 0.5.0 → 1.0.0
   - [ ] Update CMakeLists.txt: `project(spoofiq VERSION 1.0.0)`
   - [ ] Update VERSION file: `1.0.0`
   - [ ] Update CHANGELOG: `## [1.0.0] - 2025-Q1 Release`

2. **CI/CD improvements**
   - [ ] Verify .github/workflows/release.yml:
     - Windows MSVC binary (debug-msvc, release-msvc)
     - Linux Clang18 binary
     - Linux GCC binary
   - [ ] Add signature/checksum generation (SHA256)
   - [ ] Test artifact download from draft release

3. **Binary packaging**
   - [ ] Windows: spoofiq_app.exe standalone
   - [ ] Linux: spoofiq_app ELF binary
   - [ ] Add install instructions to README (extract & run)

**Deliverable**: CI produces ready-to-download binaries

---

### Phase 3: Final QA (Week 3)
**Goal**: Manual validation on real hardware

#### Tasks
1. **Windows validation**
   - [ ] Fresh Windows 10/11 VM
   - [ ] Download release binary
   - [ ] Run app, process 5+ images
   - [ ] Verify output IMG_XXXX.jpg has correct EXIF (use exiftool)
   - [ ] Check error handling (invalid preset, missing folder)

2. **Linux validation**
   - [ ] Ubuntu 22.04 LTS
   - [ ] Download release binary
   - [ ] Run app, same test as Windows
   - [ ] Verify ~/.config/spoofiq/config.json is created

3. **Stress testing**
   - [ ] 100-file batch (memory/performance)
   - [ ] Cancel mid-batch
   - [ ] Invalid folder paths
   - [ ] Corrupt image files (graceful error)

4. **Edge cases**
   - [ ] Whitespace in presets
   - [ ] Non-ASCII characters in device names
   - [ ] Very long file paths

**Deliverable**: QA sign-off, known issues documented

---

## Blockers & Risks

| Risk | Impact | Mitigation |
|------|--------|-----------|
| Windows MSVC build fails | High | Test weekly on Windows VM |
| GL context initialization differs on Windows | High | Verify GLFW + OpenGL on Windows early |
| File paths with spaces break batch input | Medium | Add quoting support, test |
| spdlog/fmt dependency issue (clang build) | Medium | Use stable spdlog v1.14.1, verify FetchContent pin |
| TagLib integration incomplete | Low | Defer video to v1.1, don't ship stubs |

---

## Success Criteria

✅ **v1.0 is ready when:**
1. All compiler warnings eliminated across MSVC/Clang/GCC
2. CI produces signed binaries for Windows + Linux
3. Manual QA passes on Windows 10+ and Ubuntu 22.04+
4. 50+ test assertions passing
5. CHANGELOG updated with final v1.0 notes
6. GitHub Release published with artifacts and checksums

---

## Post-v1.0 Roadmap (v1.1+)

### v1.1 (Q2 2025)
- GPS coordinate randomization (manual seed, random offset)
- Timestamp jitter (±days/hours within preset range)
- Progress event publishing (real-time UI updates)

### v1.2 (Q3 2025)
- File browser (nativefiledialog-extended or OS-native)
- Drag-and-drop file input

### v1.3 (Q4 2025)
- Video processing (MP4/MOV QuickTime atoms, requires TagLib)
- Batch preset application templates

### v1.4+ (2026)
- Bulk preset import/export
- Multi-threaded batch processing
- WebRTC overlay mode (network streaming)

---

## Sign-Off

| Role | Name | Date |
|------|------|------|
| Product Owner | — | — |
| Tech Lead | — | — |
| QA Lead | — | — |

---

**Last Updated**: 2026-11-24  
**Next Review**: Weekly during Phase 1 & 2
