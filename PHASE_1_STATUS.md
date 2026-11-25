# Phase 1: Stability & Polish (Week 1-2)

## Compiler Warnings & Cross-Platform Builds

### Status: ✅ WARNINGS ELIMINATED

#### ✅ Completed
- MSVC C4267 (size_t → int conversion) fixed in batch_orchestrator.cpp
- MSVC C4100 (unused parameter total_files) removed from signature
- MSVC C4244 (unsigned char → char) fixed in media_file.cpp
- GCC -Wsign-compare (size_t vs int) fixed in image_processor.cpp:28
- Global `-DSPOOFIQ_WARNINGS_AS_ERRORS=ON` enabled in CMakeLists.txt
- Catch2 v3.4.0 integrated with manual test registration (no `catch_discover_tests`)
- debug-msvc preset now builds Catch2 tests locally

#### ✅ Platform Verification Complete (Linux)
- **Linux GCC (Debug)**: ✅ Clean build, 1/1 test passed, **0 warnings**
- **Linux Clang (Debug)**: ✅ Clean build, 1/1 test passed, **0 warnings**
- **Linux GCC (Release)**: ✅ Clean build (tests disabled), **0 warnings**

#### ✅ Smoke Test Complete
- Batch processed 5 test JPEGs: **5/5 successful** ✅
- Output files named IMG_0001.jpg–IMG_0005.jpg with correct EXIF injection
- File format verified: contains iPhone 16 Pro Max EXIF data + GPS tags
- No crashes or errors in CLI mode

#### 📋 Next Steps
1. **Windows MSVC verification** (need Windows CI or manual Windows test)
   ```bash
   # On Windows: requires Visual Studio 18 2026
   cmake --preset debug-msvc && cmake --build build/Debug-msvc
   cmake --preset release-msvc && cmake --build build/Release-msvc
   ```

2. **Clang18 + libc++ verification** (optional; requires C compiler support)
   - Skipping for now due to `cc` not supporting `-stdlib=libc++` flag
   - Standard Clang preset (`debug-clang`) works ✅

3. **Smoke test** (batch process 5 images)
   - Verify IMG_0001.jpg has correct iPhone EXIF
   - Check console logs for errors
   - Verify config.json created

4. **Documentation updates**
   - [ ] README: Mark features ✅/❌
   - [ ] Add Windows-specific build notes
   - [ ] Add troubleshooting section

---

## Task Checklist

- [x] Fix all MSVC warnings
- [x] Fix GCC -Wsign-compare warnings
- [x] Enable warnings-as-errors globally
- [x] Add Catch2 build to debug-msvc
- [x] Test Linux GCC ✅
- [x] Test Linux Clang ✅
- [x] Manual smoke test on Linux ✅
- [ ] Test Windows MSVC (blocked: no Windows machine; CI-only)
- [ ] Update CHANGELOG for v1.0
- [ ] Update README with current state
- [ ] Add troubleshooting guide

---

## Known Issues

| Issue | Workaround | Status |
|-------|-----------|--------|
| spdlog fmt dependency on Clang | Use stable v1.14.1 | Investigating |
| GL headers missing on fresh Linux | `apt install libgl1-mesa-dev` | Documented |
| Windows path spaces in batch input | Quote paths | v1.1 improvement |

---

## Remaining Work Estimate

- **Compiler & build**: 1-2 hours (platform testing)
- **Documentation**: 2-3 hours (README, troubleshooting)
- **Testing**: 3-4 hours (manual smoke tests)
- **Total Phase 1**: ~6-9 hours

**Target completion**: This week
