# Phase 3: Final QA (Post-Release Engineering)

**Goal**: Manual validation on real hardware and edge case testing  
**Timeline**: Week 3 (Dec 8–15, 2026)  
**Deliverable**: QA sign-off, known issues documented, v1.0.0 production ready

---

## Windows Validation (10+/11)

### Setup
- [ ] Fresh Windows VM (10 or 11)
- [ ] Download spoofiq_app.exe from GitHub Release
- [ ] Extract to `C:\Program Files\spoofiq\` or user folder

### Functional Testing
- [ ] Launch app (GUI mode): ImGui window appears
- [ ] Check **Presets & Devices** tab: "iPhone 16 Pro Max" preset exists
- [ ] Go to **Batch Spoofer** tab:
  - [ ] Enter folder path with test images
  - [ ] Select preset from dropdown
  - [ ] Click **Process Batch**
  - [ ] Progress bar appears and completes
  - [ ] Check output folder: IMG_0001.jpg, IMG_0002.jpg created
- [ ] Check **Console** tab: No error messages (only info/debug logs)
- [ ] Press ESC to quit cleanly

### EXIF Verification
- [ ] Install exiftool or use Windows properties
- [ ] Right-click IMG_0001.jpg → Properties → Details
- [ ] Verify: Camera Make = "Apple", Model = "iPhone 16 Pro Max"
- [ ] Verify: Software = "iOS 18.1"
- [ ] Verify: GPS coordinates present (±0.0 lat/long for default preset)

### CLI Testing
```cmd
C:\Program Files\spoofiq\spoofiq_app --cli ^
  --path-to-folder C:\test_images ^
  --out-dir C:\test_output ^
  --preset=iphone_16_pro_max
```
- [ ] Exit code = 0 (success)
- [ ] All images processed
- [ ] Output files created with correct naming

### Error Handling
- [ ] Invalid folder path → Error dialog displayed
- [ ] Missing preset → Error message in console
- [ ] Corrupt image file → Skipped with error log
- [ ] Batch cancel (mid-process) → Cleanly stops, partial output valid

### Configuration
- [ ] First run: `%APPDATA%\spoofiq\config.json` created
- [ ] Edit preset in UI → Changes saved to config.json
- [ ] Restart app → Settings persist

### Edge Cases
- [ ] Paths with spaces: `C:\My Documents\spoofiq_app.exe` works
- [ ] Unicode characters in folder names: `C:\日本語\images\` works
- [ ] Very long file paths (>260 chars) → Handled gracefully or documented

---

## Linux Validation (Ubuntu 22.04 LTS)

### Setup
- [ ] Fresh Ubuntu 22.04 VM
- [ ] Download spoofiq_app from GitHub Release
- [ ] `chmod +x spoofiq_app && ./spoofiq_app`

### Functional Testing (same as Windows)
- [ ] GUI launches
- [ ] Batch process 5+ images
- [ ] Output IMG_XXXX.jpg created
- [ ] EXIF metadata correct
- [ ] CLI mode works with same commands

### Configuration (Linux-specific)
- [ ] First run: `~/.config/spoofiq/config.json` created
- [ ] Permissions: User has read/write access
- [ ] XDG_CONFIG_HOME override respected (if set)

### Dependencies Check
- [ ] OpenGL library linked: `ldd ./spoofiq_app | grep GL`
- [ ] GLFW symbols resolved
- [ ] No "not found" errors

---

## Stress Testing

### High-Volume Batch
- [ ] Process 100 images at once
- [ ] Monitor memory usage (should be stable, <500MB)
- [ ] Monitor CPU (should max out during processing, then idle)
- [ ] All 100 images output correctly named IMG_0001–IMG_0100

### Large File Sizes
- [ ] 10MB JPEG → Processes without crash
- [ ] 50MB TIFF → Converts to RGB, outputs as JPEG

### Rapid Operations
- [ ] Start batch → Cancel after 10% → Start new batch → Completes
- [ ] No leftover threads or file handles

---

## Edge Cases & Error Conditions

| Scenario | Expected Behavior | Verified |
|----------|-------------------|----------|
| Path with 300+ characters | Error message or handled gracefully | [ ] |
| Filename with non-ASCII (中文, العربية, etc.) | Processed or skipped with log | [ ] |
| Very small image (1×1 px) | Processed successfully | [ ] |
| Image with no extension | Skipped with error log | [ ] |
| Duplicate filenames in input | IMG_XXXX increments correctly | [ ] |
| Read-only output directory | Error message; batch skipped | [ ] |
| Disk full during output | Graceful error; partial output valid | [ ] |
| Corrupted JPEG (truncated header) | Skipped with error log | [ ] |

---

## Documentation Verification

- [ ] README.md matches current feature set
- [ ] Build instructions are accurate and tested
- [ ] Troubleshooting section covers common issues
- [ ] CLI examples in README produce correct output
- [ ] Configuration file format documented

---

## Known Issues & Limitations

(To be populated from testing)

| Issue | Severity | Workaround | Fixed Version |
|-------|----------|-----------|---|
| (example) | — | — | — |

---

## Sign-Off Checklist

- [ ] Windows QA tester: Verified on Windows 10/11
- [ ] Linux QA tester: Verified on Ubuntu 22.04
- [ ] All 46 Catch2 tests passing
- [ ] 0 compiler warnings across all platforms
- [ ] Release notes published on GitHub
- [ ] Binaries downloadable and checksums verified
- [ ] No known blockers for production use

---

## Success Criteria

✅ **v1.0.0 is production-ready when:**
1. All functional tests passed on Windows + Linux
2. Edge cases documented (not all fixed, but known)
3. No critical bugs found
4. Performance acceptable (100-file batch < 2 min)
5. Configuration persists across restarts
6. EXIF metadata correctly injected on all test images
7. CLI mode works reliably for batch automation

---

## Blockers & Escalation

| Blocker | Impact | Escalation |
|---------|--------|-----------|
| Binary doesn't launch on fresh Windows | Critical | Repackage with deps; check VC Runtime |
| EXIF not embedded on Windows | Critical | Debug ImageProcessor on MSVC |
| CLI exit codes incorrect | Medium | Fix cli_runner.cpp error handling |
| Memory leak on 100-file batch | Medium | Profile with Valgrind/Dr. Memory |
| Config.json not created | Medium | Check permissions and directory creation |

---

**Last Updated**: 2026-11-25  
**Target Completion**: 2026-12-15  
**Next Step**: Begin Phase 2 CI/CD validation
