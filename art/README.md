# Artwork

| Folder | What | From | Licence |
| --- | --- | --- | --- |
| `valley/` | Rogan knobs (Med, MedSmall, Small; background, knob and highlight layers) for the PLATEAU, CV IN and SEQ pages | ValleyRackFree `res/v2/` (commit `86f02e4`), unchanged | Distributed in ValleyRackFree; the Rogan design is VCV's Component Library (below) |
| `vcv/` | Rogan 3PS, 2PS, 1PS white knobs for the TIDAL and RANDOM pages | VCV Rack `res/ComponentLibrary/` (commit `061ccf6`), unchanged | CC BY-NC 4.0, (c) VCV |
| `bogaudio/` | Knob68, Knob26, Knob16 for the LFO page | BogaudioModules `res/` (commit `656eaae`), unchanged | CC BY-SA 4.0, (c) Matt Demanett |
| `fonts/` | Titillium Web SemiBold | Google Fonts | SIL OFL 1.1 (`fonts/OFL.txt`) |

`tools/knob_art.py` renders the knobs into the skin's filmstrips, and `tools/post_skin.py` draws the pages. The
filmstrips made from Bogaudio's knobs are CC BY-SA 4.0 like their source; those made from VCV's Rogan knobs are
CC BY-NC 4.0, which is why this plugin is distributed free and never sold.
