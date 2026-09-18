---
description: 2–3 cross-questions on code just written — short, no full mock
---

Quiz Sam on the code most recently written in this repo. This is the
"after each milestone" check from CLAUDE.md, not a full interview.

Arguments (optional): $ARGUMENTS — a file or component to focus on
(e.g. `spsc_queue.hpp`, `gap detection`). Default: whatever changed last.

## How to run it
1. If no argument, find the most recent work: `git log --oneline -3` and
   `git diff HEAD~1 --stat`. Read the actual files involved.
2. Ask **2–3 questions**, one at a time, about that specific code. Quote real
   lines (`messages.hpp:38`).
3. Aim at the design decisions an interviewer would poke:
   - Why this type / layout / memory order, and what breaks with the obvious
     alternative?
   - What happens at the boundary — empty, full, wrapped, duplicate, reordered?
   - What does this cost, and how would you know without guessing?
4. Grade each answer in a line or two. If wrong, give the tight correct answer.
5. Keep it short. This is a checkpoint between coding sessions.

Answers and background are in @PREP.md if Sam gets stuck — offer them only
after an attempt, never before.
