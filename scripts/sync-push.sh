#!/bin/bash
# ============================================
# SYNC-PUSH: Run at END of work session
# Commits + pushes code + OpenClaw memory to git
# ============================================
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

MACHINE=$(hostname)
TIMESTAMP=$(date '+%Y-%m-%d %H:%M')

echo -e "${GREEN}=== SYNC PUSH: Sharing changes to other computer ===${NC}"

# --- 1. Push PlatformIO project ---
echo -e "\n${YELLOW}[1/2] Pushing PlatformIO project...${NC}"
cd "/mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation"

if [ -n "$(git status --porcelain)" ]; then
    git add -A
    git commit -m "[${MACHINE}] ${TIMESTAMP}" 2>/dev/null || true
    echo "  Committed changes."
else
    echo "  No changes to commit."
fi
git push origin main 2>/dev/null || echo -e "  ${RED}(no remote - skipping push)${NC}"

# --- 2. Push OpenClaw workspace (memory/skills) ---
echo -e "\n${YELLOW}[2/2] Pushing OpenClaw workspace...${NC}"
cd "/mnt/c/Users/SKY/.openclaw/workspace"

if [ -n "$(git status --porcelain)" ]; then
    git add -A
    git commit -m "[${MACHINE}] ${TIMESTAMP}" 2>/dev/null || true
    echo "  Committed changes."
else
    echo "  No changes to commit."
fi
git push origin main 2>/dev/null || echo -e "  ${RED}(no remote - skipping push)${NC}"

echo -e "\n${GREEN}=== PUSH COMPLETE ===${NC}"
