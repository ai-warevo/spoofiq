# Spoofiq MVP (v0.1.0)

**Proof-of-Concept**: Single image processing with hardcoded iPhone 16 Pro Max metadata.

## Features

- Load any image format (PNG, JPG, BMP, TIF, etc.) via stb_image
- Convert to JPEG with quality 95
- Inject EXIF metadata mimicking iPhone 16 Pro Max:
  - Make: Apple
  - Model: iPhone 16 Pro Max  
  - Software: iOS 18.1
  - Orientation: 1 (normal)
  - ExifOffset and GPSOffset pointers (minimal IFD structure)
- Output as `IMG_0001.jpg` in specified directory

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

```bash
./spoofiq-mvp <input_image> [output_dir]

# Example:
./spoofiq-mvp photo.png ./output
# → Creates output/IMG_0001.jpg with iPhone metadata
```

## Verification

```bash
file output/IMG_0001.jpg
# Should show: Exif standard: [TIFF image data ... manufacturer=Apple, model=iPhone 16 Pro Max ...]

exiftool output/IMG_0001.jpg  # if available
```

## Limitations (MVP)

- Single file only (no batch processing)
- Hardcoded metadata (no customization)
- Sequential processing (no concurrency)
- Simplified EXIF structure (IFD0 only, no full Exif/GPS sub-IFDs)
- No video support
- No UI (command-line only)

## Next: Alpha 1 (v0.2.0)

Will integrate full dependency stack via CMake FetchContent:
- Exiv2 for complete EXIF/IPTC/XMP support
- TagLib for video metadata
- nlohmann/json for configuration
- spdlog for logging
- nativefiledialog-extended for file browser

## Technical Details

- **Language**: C++23 (std::expected, std::format)
- **Image I/O**: stb_image + stb_image_write (vendored)
- **EXIF**: Manual TIFF/EXIF structure building
- **Build**: CMake 3.24+
- **Output Naming**: IMG_XXXX.jpg format

---

Developed as part of the incremental Spoofiq implementation roadmap.
