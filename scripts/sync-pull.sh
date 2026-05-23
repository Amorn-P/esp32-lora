#!/bin/bash
# ============================================
# SYNC-PULL: Run at START of work session
# Pulls latest code + OpenClaw memory from git
# ============================================
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== SYNC PULL: Getting latest from other computer ===${NC}"

# --- 1. Pull PlatformIO project ---
echo -e "\n${YELLOW}[1/2] Pulling PlatformIO project...${NC}"
cd "/mnt/c/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation"
git pull origin main 2>/dev/null || echo "  (no remote yet - skipping)"
echo "  Done."

# --- 2. Pull OpenClaw workspace (memory/skills) ---
echo -e "\n${YELLOW}[2/2] Pulling OpenClaw workspace...${NC}"
cd "/mnt/c/Users/SKY/.openclaw/workspace"
git pull origin main 2>/dev/null || echo "  (no remote yet - skipping)"
echo "  Done."

echo -e "\n${GREEN}=== PULL COMPLETE ===${NC}"
echo "Project: C:/Users/SKY/Documents/PlatformIO/Projects/Project_Preperation"
echo "Memory:  C:/Users/SKY/.openclaw/workspace"
