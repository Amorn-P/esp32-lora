# Two-Computer Sync Setup

## Architecture

```
[Computer A] ──git push──> [GitHub/GitLab] <──git pull── [Computer B]
     │                                                          │
     ├── USB ESP32 (upload + serial monitor)                    ├── USB ESP32
     ├── C:\...\Project_Preperation\  (code repo)               ├── C:\...\Project_Preperation\
     └── C:\...\.openclaw\workspace\  (memory + skills)         └── C:\...\.openclaw\workspace\
```

Two repos:
1. **Code repo** - `Project_Preperation/` (PlatformIO ESP32 firmware)
2. **Brain repo** - `.openclaw/workspace/` (memory, skills, agent identity)

## Machine-Specific Setup

### One-time: GitHub Remote

On BOTH computers, after cloning or setting up:

```bash
# Code repo
cd C:\Users\SKY\Documents\PlatformIO\Projects\Project_Preperation
git remote add origin git@github.com:YOUR_USER/esp32-code.git

# Brain repo
cd C:\Users\SKY\.openclaw\workspace
git remote add origin git@github.com:YOUR_USER/openclaw-brain.git
```

### One-time: platformio.ini Path

`ESP32_Lora/platformio.ini` line 8 contains:
```
lib_extra_dirs = C:\Users\SKY\Documents\PlatformIO\Projects\libraries
```

If Computer B has different username or path, update this line. Otherwise it works as-is.

## Daily Workflow

### START of session (on either computer):
```
WSL:  bash scripts/sync-pull.sh
CMD:  wsl bash scripts/sync-pull.sh
```
This pulls latest code + memory from the other computer.

### END of session:
```
WSL:  bash scripts/sync-pull.sh   # pull any updates first
      ... work ...
      bash scripts/sync-push.sh   # share your changes
```

### Quick commands:
```bash
# Pull everything
cd /mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation && git pull origin main
cd /mnt/c/Users/SKY/.openclaw/workspace && git pull origin main

# Push everything
cd /mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation && git add -A && git commit -m "update" && git push origin main
cd /mnt/c/Users/SKY/.openclaw/workspace && git add -A && git commit -m "update" && git push origin main
```

## Serial Monitor / Upload

Each computer needs its own USB cable to the ESP32. Not shareable remotely.

```bash
# Upload firmware
cd /mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation/ESP32_Lora
/mnt/c/Users/SKY/.platformio/penv/Scripts/pio.exe run --target upload -e slave

# Serial monitor
/mnt/c/Users/SKY/.platformio/penv/Scripts/pio.exe device monitor
```

## Project Structure

```
Project_Preperation/
├── .git/
├── .gitignore
├── scripts/
│   ├── sync-pull.sh       ← Run at session start
│   └── sync-push.sh       ← Run at session end
├── SETUP.md               ← This file
└── ESP32_Lora/
    ├── platformio.ini
    ├── src/
    └── lib/

.openclaw/workspace/
├── .git/
├── .gitignore             ← Excludes .openclaw/ runtime state
├── AGENTS.md              ← Agent behavior
├── SOUL.md                ← Agent personality
├── IDENTITY.md            ← Agent identity
├── USER.md                ← User profile
├── TOOLS.md               ← Tool config
├── memory/                ← Project memory/context
│   ├── ESP32_Lora/
│   ├── ESP32_AutoPump/
│   └── ...
└── skills/                ← Reusable workflows
    ├── esp32-platformio-testing/
    └── esp32-software-engineer/
```
# Machine B was here: Sun May 24 00:15:28 +07 2026
