# Phase 2: Release Engineering (In Progress)

**Goal**: Prepare CI/CD pipeline and release binaries for v1.0.0  
**Timeline**: Week 2–3 (Nov 25–Dec 10, 2026)  
**Deliverable**: GitHub Release with signed/verified binaries for Windows + Linux

---

## Tasks

### 1. CI/CD Pipeline Validation

#### Windows MSVC Binary
- [ ] Trigger CI workflow with debug-msvc + release-msvc presets
- [ ] Verify artifact builds without warnings
- [ ] Download and test on Windows 10/11 VM
- [ ] Verify config directory creation (%APPDATA%\spoofiq)
- [ ] Smoke test: batch process 5 images

#### Linux Clang18 Binary
- [ ] Trigger CI with clang18-libcxx preset OR debug-clang (fallback)
- [ ] Verify artifact builds without warnings
- [ ] Test on Ubuntu 22.04 LTS
- [ ] Verify config directory (~/.config/spoofiq)
- [ ] Smoke test: batch process 5 images

#### Linux GCC Binary
- [ ] Trigger CI with release-gcc preset
- [ ] Verify artifact builds without warnings
- [ ] Test on Ubuntu 22.04 LTS

### 2. Artifact Generation & Signing

- [ ] Generate SHA256 checksums for all binaries
- [ ] Create checksums.txt manifest
- [ ] (Optional) GPG sign binaries if infrastructure exists
- [ ] Upload to draft GitHub Release

### 3. GitHub Release Publishing

- [ ] Create release notes from CHANGELOG
- [ ] Attach binaries + checksums
- [ ] Verify download URLs work
- [ ] Publish as final release (not draft)

---

## CI Workflow Files

**Expected locations:**
- `.github/workflows/release.yml` — Main release pipeline
- `.github/workflows/ci.yml` (optional) — Lint/test on push

**Needed checks:**
```yaml
- name: Configure (MSVC)
  run: cmake --preset debug-msvc
  
- name: Build (MSVC)
  run: cmake --build build/Debug-msvc --config Debug -j4

- name: Configure (Clang)
  run: cmake --preset debug-clang -DGLFW_BUILD_WAYLAND=OFF
  
- name: Build (Clang)
  run: cmake --build build/Debug-clang -j4

- name: Configure (GCC)
  run: cmake --preset release-gcc -DGLFW_BUILD_WAYLAND=OFF
  
- name: Build (GCC)
  run: cmake --build build/Release-gcc -j4
```

---

## Success Criteria

✅ Phase 2 complete when:
1. All three platform binaries build on CI without warnings
2. Artifacts downloadable from GitHub Release draft
3. Manual smoke test passed on Windows + Linux
4. Checksums verified
5. Release published as v1.0.0

---

## Blockers

| Blocker | Status | Mitigation |
|---------|--------|-----------|
| CI workflow missing or broken | TBD | Check .github/workflows/ structure |
| Windows build fails on CI | TBD | Debug MSVC preset locally first |
| GL context issue on headless CI | TBD | May need X11 or Mesa for Linux CI |
| Artifact upload path not configured | TBD | Verify release.yml artifact handling |

---

## Next Steps (After Phase 2)

**Phase 3: Final QA (Week 3)**
- Manual validation on real hardware (Windows VM, Linux desktop)
- Edge case testing (whitespace in paths, long filenames, etc.)
- Known issues documentation
- v1.0.0 sign-off

**Phase 3+: Post-Release**
- Monitor issue reports
- Plan v1.1 features (GPS randomization, timestamp jitter)
- Collect user feedback

---

**Last Updated**: 2026-11-25  
**Status**: Ready for Phase 2 (awaiting CI pipeline trigger)
