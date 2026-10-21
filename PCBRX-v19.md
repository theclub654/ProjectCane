# PCBRX v19 unified level container

PCBRX v19 is independent of the retail disc region. A level may contain any
non-empty subset of ProjectCane's language IDs:

| ID | Language |
|---:|---|
| 0 | English |
| 1 | French |
| 2 | German |
| 3 | Spanish |
| 4 | Italian |

Each language entry reconstructs a complete PCBRX v17 payload. This deliberately
keeps the stable NTSC-style world/object parser as the canonical payload format.
Language selection changes only which payload and localized media directories are
used; it does not depend on whether the source package came from an NTSC or PAL ISO.

All integers are little-endian.

```text
u32 "PCBR"
u32 19
u32 "PUNI"
u32 schema (1)
u32 languageCount (1..5)
u32 defaultLanguage
u32 chunkCount

repeat languageCount:
    u32 languageId
    u64 reconstructedPayloadSize
    u32 referenceCount
    u32 chunkIds[referenceCount]

repeat chunkCount:
    u32 chunkSize
    u32 crc32
    u8  data[chunkSize]
```

Language IDs must be unique, the default language must be present, every chunk
reference must be valid, and reconstructed sizes and CRC-32 values must match.
ProjectCane continues to read legacy PCBRX v17 and PAL PCBRX v18 files.

Localized level media uses this layout beside the combined BRX:

```text
Level Name/
    Level Name.brx
    English/Dialogs/
    French/Dialogs/
    German/Dialogs/
    Spanish/Dialogs/
    Italian/Dialogs/
```

Only directories for languages declared by the container are required. Dialogue
WAV names include both descriptor identity and source sector. The runtime never
falls through into another language or level when resolving localized dialogue.
