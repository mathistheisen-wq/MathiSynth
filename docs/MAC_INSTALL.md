# Installing MathiSynth on a Mac

This guide takes you from downloading MathiSynth to playing it in FL Studio, Ableton Live or Logic Pro.
It takes about 5 minutes. MathiSynth runs on Apple Silicon (M1–M4) and Intel Macs.

---

## Why macOS warns you

When you open the installer, macOS will probably say:

> *"Apple could not verify 'MathiSynth-macOS-Installer.pkg' is free of malware that may harm your Mac or compromise your privacy."*

**This is expected.** macOS shows this for every app or installer that isn't signed with a paid Apple Developer account ($99/year), and MathiSynth isn't signed yet. The file is built automatically by GitHub from the public source code in this repository, so you can check exactly what goes into it.

You can still install it. Pick **one** of the methods below.

---

## Step 1: Download

1. Go to the **[latest release](https://github.com/mathistheisen-wq/MathiSynth/releases/latest)**.
2. Under **Assets**, click **`MathiSynth-macOS-Installer.pkg`**. It goes to your **Downloads** folder.

---

## Step 2: Open the installer

### Method A: System Settings (no Terminal)

1. Double-click **`MathiSynth-macOS-Installer.pkg`** in Downloads.
2. When the warning appears, click **Done**. **Don't** click *Move to Trash*.
3. Open the ** Apple menu → System Settings → Privacy & Security**.
4. Scroll down to the **Security** section. You'll see:
   *"MathiSynth-macOS-Installer.pkg was blocked to protect your Mac."*
5. Click **Open Anyway**, then enter your Mac password (or use Touch ID).
6. The warning appears one more time. Click **Open Anyway** again.
7. The installer opens. Go to [Step 3](#step-3-run-the-installer).

> **On macOS 14 Sonoma or older**, there's a quicker route: **right-click** (or Control-click) the `.pkg` file, choose **Open**, then click **Open** in the dialog.
> macOS 15 Sequoia and newer removed this shortcut, so use the System Settings steps above.

### Method B: Terminal (one command)

1. Open **Terminal**. Press ⌘ + Space, type `Terminal` and press Return.
2. Paste this command and press Return. It removes the "downloaded from the internet" block from the installer file:

   ```bash
   xattr -d com.apple.quarantine ~/Downloads/MathiSynth-macOS-Installer.pkg
   ```

3. Double-click the `.pkg` in Downloads. It now opens without the warning.

If your browser saved the file somewhere other than Downloads, type `xattr -d com.apple.quarantine ` (with the space at the end), drag the file into the Terminal window, then press Return.

---

## Step 3: Run the installer

Click **Continue → Install**, enter your Mac password, then click **Close**.

The installer puts three things on your Mac:

| What | Where | Used by |
|---|---|---|
| `MathiSynth.vst3` | `/Library/Audio/Plug-Ins/VST3/` | FL Studio, Ableton Live, most other DAWs |
| `MathiSynth.component` (Audio Unit) | `/Library/Audio/Plug-Ins/Components/` | Logic Pro, GarageBand, Ableton Live |
| `MathiSynth.app` | `/Applications/` | Standalone: play it without a DAW |

You only need to deal with the macOS warning once per download. The installed plugins aren't blocked, so your DAW loads them normally.

---

## Step 4: Load it in your DAW

### FL Studio
1. Open **Options → Manage plugins**.
2. Click **Find installed plugins** (or **Rescan**).
3. Find **MathiSynth** in the list and click the **star** next to it. This adds it to your plugin picker.
4. In the Channel Rack, click **+** and choose **MathiSynth**.

### Ableton Live
1. Open **Settings → Plug-Ins**.
2. Turn on **Use VST3 Plug-In System Folders** (and/or **Use Audio Units**).
3. Click **Rescan**.
4. In the browser, go to **Plug-Ins → VST3 → Mathi Audio → MathiSynth** and drag it onto a MIDI track.

### Logic Pro / GarageBand
1. Create a **Software Instrument** track.
2. Click the instrument slot and choose **AU Instruments → Mathi Audio → MathiSynth**.

Then use the **`<` `>`** arrows at the top of MathiSynth to browse the presets.

---

## Troubleshooting

**MathiSynth doesn't show up in my DAW.**
- Check that the plugin is installed. In Finder, press ⌘ + Shift + G, paste `/Library/Audio/Plug-Ins/VST3/` and look for `MathiSynth.vst3`.
- Rescan plugins in your DAW. If that doesn't help, restart the DAW.
- **Logic only:** if the AU still doesn't appear, run this in Terminal, then reopen Logic:
  ```bash
  killall -9 AudioComponentRegistrar
  ```

**My DAW says the plugin is damaged or blocked.**
This can happen if you installed from the `.zip` instead of the installer. Remove the block from the plugin files:
```bash
sudo xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/MathiSynth.vst3 /Library/Audio/Plug-Ins/Components/MathiSynth.component
```
If you copied the plugins into your user folder instead, use these paths:
```bash
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/MathiSynth.vst3 ~/Library/Audio/Plug-Ins/Components/MathiSynth.component
```

**I'd rather install by hand (zip).**
1. Download `MathiSynth-macOS.zip` from the release and double-click it to unzip.
2. Copy `MathiSynth.vst3` to `~/Library/Audio/Plug-Ins/VST3/`. To get there in Finder, press ⌘ + Shift + G and paste the path.
3. Optionally, copy `MathiSynth.component` to `~/Library/Audio/Plug-Ins/Components/`.
4. Run the second `xattr` command above, then rescan in your DAW.

---

## Updating

Download the newest `.pkg` from the [latest release](https://github.com/mathistheisen-wq/MathiSynth/releases/latest) and install it the same way. It replaces the old version.
Your own saved presets live in `~/Documents/MathiSynth/Presets/` and are kept.

## Uninstalling

Run this in Terminal:

```bash
sudo rm -rf /Library/Audio/Plug-Ins/VST3/MathiSynth.vst3 /Library/Audio/Plug-Ins/Components/MathiSynth.component /Applications/MathiSynth.app
```

Delete `~/Documents/MathiSynth` too if you don't want to keep your saved presets.

---

## Getting rid of the warning for good

The warning goes away once the downloads are signed and checked by Apple ("notarized"). That needs an Apple Developer Program membership ($99/year). The automatic GitHub build can be extended to sign the Mac downloads with your certificate, which would be stored in the repository's GitHub secrets.
