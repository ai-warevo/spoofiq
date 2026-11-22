### UI/UX Design Specification for `spoofiq` (Dear ImGui)

Incorporate the following UI architecture and layout design into Step 2 and Milestone 5 of the `spoofiq` implementation plan.

Instead of a single cluttered window, design a clean, modern layout organized into **three top navigation tabs** using `ImGui::BeginTabBar`:

#### Tab 1: `Batch Spoofer` (Main Workspace)
1. **Source & Export Section:**
   - Input folder path (`ImGui::InputText`) + **"Browse..."** button (using `nativefiledialog-extended`) + full support for **Drag-and-Drop** folders directly onto the window.
   - Checkbox: **"Export to subfolder"** with an editable subfolder name input (default: `spoofiq_out`) to protect original directories from clutter/overwrites.
   - Checkbox: **"Skip existing `IMG_XXXX` files"**.
   - Starting index control: dropdown/toggle between **Auto-detect next free index** and **Manual start index** (`ImGui::InputInt`).
2. **Quick Spoofing Settings:**
   - **Device Preset Selector (`ImGui::Combo`):** quick selection from profiles saved in `config.json` (e.g., *iPhone 16 Pro Max*, *iPhone 15 Pro*).
   - **Timestamp Mode:** Radio buttons for **Current System Time** vs. **Custom Date/Time**, plus a checkbox for **Incremental/Random Time Shift** (`+N` seconds per file so batch assets don't share identical timestamps).
   - **Geolocation (GPS):** Toggle checkbox **"Embed GPS Coordinates"** + **"Randomize within radius (meters)"** (`ImGui::InputFloat`).
3. **Execution & Progress:**
   - Primary action button: **"Start Batch Processing (X files found)"** — dynamically switches to a red **"Stop / Cancel"** button while running (triggering `tpl`'s `CancellationToken`).
   - `ImGui::ProgressBar` showing percentage, processed count (`21 / 42`), and current filename being processed.
   - Summary status line showing processed images, processed videos, and error count.

#### Tab 2: `Presets & Devices` (JSON Profile Editor)
- **Two-column Master-Detail layout:**
  - **Left Pane:** Scrollable list of saved device presets from `config.json` with **"+ New"**, **"Duplicate"**, and **"Delete"** buttons.
  - **Right Pane:** Form fields for editing every parameter of the selected preset:
    - Preset Name (`Name`)
    - Camera `Make` & `Model` (e.g., `Apple`, `iPhone 16 Pro Max`)
    - OS `Software` version (e.g., `iOS 18.1`)
    - `LensModel` string
    - `FocalLength` (mm), `FocalLengthIn35mm`, `FNumber` (aperture), and `ISO`
    - Default `Latitude` & `Longitude`
  - **Bottom Action Bar:** **"Save to config.json"** button (serializes via `nlohmann/json`) and **"Reset Defaults"** button.

#### Tab 3: `Console` (`spdlog` Integration)
- **Live Badge:** Display unread warning/error count directly in the tab label (e.g., `Console (2)`), resetting when the user opens the tab.
- **Toolbar:**
  - Level filter checkboxes (`Info`, `Warn`, `Error`).
  - Live search bar (`ImGuiTextFilter`).
  - **"Clear"** button and **"Copy to Clipboard"** button.
- **Log Viewport (`ImGui::BeginChild`):**
  - Monospace-styled scrollable log feed receiving entries from `ImGuiConsoleSink` (`spdlog`).
  - Color-coded log levels (white/green for `info`, yellow for `warn`, red for `error`) with timestamps.
  - **"Auto-scroll"** checkbox pinned at the bottom.

Please update the architectural plan and Milestone 5 tasks to reflect this three-tab UI design and its data bindings to `ConfigManager`, `tobus`, and `spdlog`.


----

+-----------------------------------------------------------------------+

|  spoofiq — iPhone Metadata Forge                                _ [] X|
+-----------------------------------------------------------------------+

| [ Batch Spoofer ]  [ Presets & Devices ]  [ Console (2) ]             |
+=======================================================================+

|  TAB 1: Batch Spoofer (Main Workspace)                                |
|                                                                       |
|  1. Source & Export:                                                  |
|  Folder: [ C:\Users\User\Desktop\Media\Raw         ] [ Browse... ]    |
|  [x] Export to subfolder: [ spoofiq_out            ]                  |
|  [x] Skip existing (IMG_XXXX)           Start Index: [ Auto      v ]  |
|                                                                       |
|  2. Quick Spoofing Settings:                                          |
|  Device Preset: [ iPhone 16 Pro Max (iOS 18.1)              v ]       |
|  Timestamp:     (o) Current Time   ( ) Custom Date/Time               |
|                 [x] Shift timestamp per file: [ +3 ] sec              |
|  Geolocation:   [x] Embed GPS      [x] Randomize radius: [ 50 ] m     |
|                                                                       |
|  3. Execution:                                                        |
|  [              START BATCH PROCESSING (42 files)             ]       |
|  [=====================>------------------------]  50% (21/42)        |
|  Status: Processing clip_final.mov -> IMG_0021.mp4 | Errors: 0        |
+-----------------------------------------------------------------------+

+=======================================================================+

|  TAB 2: Presets & Devices (JSON Profile Editor)                       |
|                                                                       |
|  [ Saved Presets ]    |  [ Selected Preset Parameters ]               |
|  +-----------------+  |  Preset Name: [ iPhone 16 Pro Max          ]  |
|  | > iPhone 16 Pro |  |  Make:        [ Apple                      ]  |
|  |   iPhone 15 Pro |  |  Model:       [ iPhone 16 Pro Max          ]  |
|  |   iPhone 14     |  |  Software:    [ iOS 18.1                   ]  |
|  |   Custom Cam    |  |  Lens Model:  [ iPhone 16 Pro Max back...  ]  |
|  +-----------------+  |  Focal Len:   [ 6.77 ] mm  (35mm: [ 24 ])     |
|  [ + New ] [ Dup ] [-]|  Aperture:    f/[ 1.78 ]   ISO:   [ 50 ]      |
|                       |  Base GPS:    Lat [ 37.3349 ] Lon [-122.0090] |
|                       |                                               |
|                       |  [ Save to config.json ]   [ Reset Defaults ] |
+-----------------------------------------------------------------------+

+=======================================================================+

|  TAB 3: Console (spdlog Integration)                                  |
|                                                                       |
|  Filter: [x] Info  [x] Warn  [x] Error   Search: [          ] [Clear] |
|  +-----------------------------------------------------------------+  |
|  | 02:55:10 [info]  Loaded config.json: 3 presets available        |  |
|  | 02:55:14 [info]  Batch started (42 files) | Preset: iPhone 16   |  |
|  | 02:55:14 [info]  render_01.png -> IMG_0001.jpg (OK)             |  |
|  | 02:55:15 [warn]  sample.heic skipped (unsupported format)       |  |
|  | 02:55:16 [error] broken.mp4: failed reading moov atoms          |  |
|  +-----------------------------------------------------------------+  |
|  [x] Auto-scroll                                 [ Copy to Clipboard ]|
+-----------------------------------------------------------------------+
