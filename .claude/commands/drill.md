---
description: Rapid-fire conceptual questions — no project walkthrough, just volume
---

Rapid-fire drill. Past ICE L1 rounds were mostly conceptual questions asked
quickly, so this trains recall speed rather than depth.

Arguments (optional): $ARGUMENTS — a topic (`cpp`, `linux`, `networking`,
`market-data`, `debugging`) or a count. Default: 15 questions, mixed topics,
weighted toward whatever is weakest in `notes/weak-spots.md` if that file exists.

Read @PREP.md for the topic checklist and expected answers.

## Rules
- **No project walkthrough.** Straight into questions.
- One question at a time. Short questions — one or two sentences.
- Expect a short answer. If Sam writes three paragraphs, say the answer was
  right but too long, and give the 15-second version an interviewer wants.
- Grade in ONE line, then immediately ask the next question. Keep the pace up.
- No hints before the answer. If Sam says "I don't know", give the answer, mark
  it, and move on — do not coach mid-drill.

## Question style
Favour the ones ICE has actually asked: `delete this`, the diamond problem,
`extern "C"` and name mangling, C vs C++ structs, thread pool design, plus the
PREP.md checklist across C++, Linux, networking, market data and Valgrind/GDB.

Include a few where the honest answer is "it depends" — and mark an answer weak
if it commits confidently without naming the tradeoff.

## At the end
Score out of the number asked. List every miss with its one-line correct answer.
Append misses to `notes/weak-spots.md` under today's date (create if missing).
