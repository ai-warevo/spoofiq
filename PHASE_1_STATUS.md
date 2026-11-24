# Phase 1: Stability & Polish (Week 1-2)

## Compiler Warnings & Cross-Platform Builds

### Status: IN PROGRESS

#### ✅ Completed
- MSVC C4267 (size_t → int conversion) fixed in batch_orchestrator.cpp
- MSVC C4100 (unused parameter total_files) fixed
- MSVC C4244 (unsigned char → char) fixed in media_file.cpp
- Global `-DSPOOFIQ_WARNINGS_AS_ERRORS=ON` enabled in CMakeLists.txt
- debug-msvc preset now builds Catch2 tests locally

#### 📋 Next Steps
1. **Platform verification** (all warnings = 0)
   ```bash
   # Windows (MSVC)
   cmake --preset debug-msvc && cmake --build build/Debug-msvc
   
   # Linux Clang18
   cmake --preset clang18-libcxx && cmake --build build/clang18-libcxx
   
   # Linux GCC
   cmake --preset debug-gcc && cmake --build build/debug-gcc
   ```

2. **Smoke test** (batch process 5 images)
   - Verify IMG_0001.jpg has correct iPhone EXIF
   - Check console logs for errors
   - Verify config.json created

3. **Documentation updates**
   - [ ] README: Mark features ✅/❌
   - [ ] Add Windows-specific build notes
   - [ ] Add troubleshooting section

---

## Task Checklist

- [x] Fix all MSVC warnings
- [x] Enable warnings-as-errors globally
- [x] Add Catch2 build to debug-msvc
- [ ] Test Windows MSVC (need Windows machine or CI)
- [ ] Test Linux Clang18 ← **CURRENT**
- [ ] Test Linux GCC
- [ ] Manual smoke test on each platform
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
