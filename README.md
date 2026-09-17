# Glyph

A compiler for a small imperative programming language whose token vocabulary is composed entirely of Unicode emoji, rather than ASCII keywords.

Glyph is built as a compiler-theory exploration, not a practical end-user tool — in the tradition of esoteric languages like Brainfuck and APL. Its core technical focus: most compilers (and tools like Lex/Flex) assume single-byte ASCII input. Glyph's lexer has to correctly decode multi-byte UTF-8 sequences and handle Unicode grapheme clusters, which is a genuinely different systems problem than parsing plain text.

> Course project — BCSE307P Compiler Design Laboratory, Phase 1.

## Why emoji?

Not a gimmick, its a constraint. Building a lexer that's correct under variable-width, multi-byte Unicode input (rather than assumed single-byte ASCII) is the actual technical challenge this project explores. 

## Pipeline

Emoji Source (UTF-8)
→ UTF-8 Decoder / Grapheme Grouping
→ Lexer (Token Stream)
→ Parser (AST)
→ Semantic Analyzer (Annotated AST + Symbol Table)
→ IR Generator (Three-Address Code)
→ Optimizer (Optimized IR)
→ Code Generator
→ Runnable Python/C Output


## Language Specification

| Emoji | Codepoint | Token | Meaning |
|---|---|---|---|
| 🖨 | U+1F5A8 | PRINT | Output statement |
| 🔢 | U+1F522 | NUM_PREFIX | Marks following digits as a number |
| ➕ | U+2795 | PLUS | Addition |
| ➖ | U+2796 | MINUS | Subtraction |
| ✖ | U+2716 | MUL | Multiplication |
| ➗ | U+2797 | DIV | Division |
| 🔁 | U+1F501 | LOOP | Loop construct |
| ❓ | U+2753 | IF | Conditional construct |



## Example

Input ('input.txt'):
🖨 5 ➕ 3 ✖ 2
❓ 5 🖨 10
🔁 3 🖨 1

Output:
-- Statement 1: AST ---
PRINT
BINOP(+)
NUM(5)
BINOP(*)
NUM(3)
NUM(2)

--- Statement 2: AST ---
IF
COND:
NUM(5)
BODY:
PRINT
NUM(10)

--- Statement 3: AST ---
LOOP
COUNT:
NUM(3)
BODY:
PRINT
NUM(1)



## Build and run

```bash
gcc glyph.c -o glyph
./glyph
```

The program reads from 'input.txt' in the same directory — create that file (UTF-8 encoded) with your Glyph program, one statement per line, ending with a blank line.

## Known Technical Challenges

- **UTF-8 / ZWJ lexing** — some emoji are multiple Unicode codepoints joined by zero-width joiners into one visual character. The current token set avoids these cases; full grapheme-cluster handling is a stretch goal.
- **Column tracking** — a 4-byte emoji is one visible character, not four. Error reporting needs to count decoded characters, not raw bytes.
- **Nested-loop backpatching** — generating correct jump targets for nested `🔁`/`❓` in three-address code is flagged as the highest-risk part of Phase 2.
- **Scope trade-off** — no functions/recursion (and therefore no activation records), by deliberate choice, to prioritize depth on IR generation and optimization instead of partial coverage everywhere.


## Tech Stack

- **Language:** C
- **Lexer/Parser:** hand-written (Flex/Bison evaluated only as a stretch goal, given Flex's byte-oriented limitations)
- **Target output:** Python or C source

## Background

- Lex/Flex are byte-oriented by design and don't natively guarantee correct multi-byte/ZWJ handling — this motivates Glyph's custom lexer.
- Esoteric languages (Brainfuck, Whitespace) prove non-conventional token sets can form complete languages, but typically over single-byte ASCII.
- [Unicode Standard Annex #29](https://unicode.org/reports/tr29/) defines grapheme cluster boundaries — the formal basis this project applies.
