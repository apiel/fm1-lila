<p align="center"><img src="docs/img/splash.png" width="240" alt="LILA"></p>

# LILA

A polyphonic toy synth for a very young musician, as custom firmware for the [M-VAVE FM-1](https://www.m-vave.com).
Built for Lila, on the hardware layer of [fm1-tribe](../fm1-tribe) (X0X, Felucca).

It is a normal synth underneath: ten voices, two oscillators and a sub, an FM partner, a filter, an envelope, an LFO,
echo, reverb, chorus and a robot crusher. On top, it is a toy: **25 worlds**, each its own instrument *and* its own animated picture.

![The worlds](docs/img/worlds.png)

- **Twelve animals and places**: a bird, a cow, a wolf, a frog, a spaceship, meteors, a block world, dinosaurs, a robot, the ocean, a fairy
  castle and the jungle. Each has **its own knobs** (the wolf howls or sounds a police siren, the frog croaks, the arcade has a chip
  arpeggio, the whale sings and pings), **its own effect** on the FLUFFY knob (chorus, doubler, flanger, phaser, auto-pan, ring modulator,
  fuzz) and **its own kind of filter** (soft, talky, thin, hollow, honk, vocal).
- **Eight instruments** where the keys light up on the screen: a piano, an electric piano and an organ (the keys become a keyboard, every key
  sends a coloured note rising), a xylophone, a harp, a ukulele (every key strums a chord), a flute and a kalimba.
- **Five stories, in French**, told by the synth: three fables of La Fontaine (*Le Corbeau et le Renard* in his own words, *La Grenouille qui
  voulait se faire aussi grosse que le Bœuf*, *Le Lion et le Rat*), *Boucle d'or et les trois ours* and *Le Petit Chaperon rouge*. Each key tells
  a part (the white keys only, when the story is short), PLAY tells it all (and it stops at the end), the words are written on the screen, and the voice goes through the synth, so the
  filter, the effects and the LFO still work on it. The speech is coded at 100 frames a second with a smoothed glottal pulse and a little breath.

**Whatever knob is turned, the picture changes with it**: more echo leaves ghosts, more filter makes the world brighter, the robot knob turns
everything into big square pixels, the size knob makes the animal grow.

For a child who is too young for a screen: **SAVE + GLO together** change the screen: all of it, then only the name of a world, a beat or a
song for one second, then none at all (the backlight is off).

Every button has one page and the four knobs always do something. The keys just play. See the **[manual](docs/MANUAL.md)**.

| | |
|---|---|
| Keys, OCT- / OCT+ | play, change octave |
| PRESETS | pick the world |
| ALGORITHM | the mood: sunny, night, magic, crazy |
| SELECT | pick the song, while one plays |
| MASTER | volume |
| FX SEL ENV LFO EDIT | the sound: effects, filter, envelope, wobble, shape |
| HOME | the magic knobs: bend, stars, size, wiggle |
| GLO | the party: tempo, key, easy notes, friends (chords) |
| ARP | arpeggiator |
| SEQ | a drum beat to play along to |
| PLAY / STOP | nineteen songs: listen, or let the keys light up and follow them; in a story, tells it all |
| REC | record a tune and loop it, even slowed down or sped up |
| SAVE | with GLO: the screen (all, a name, none) |

## Build and try

```
tests/run_tests.sh      # the audio on every world, then the whole app scripted in a simulator (screenshots in build/scen/)
desktop/build.sh        # a window with sound (needs SFML 2.x): build/desktop/lila
python3 tools/build.py  # the firmware package build/lila.fwsc (needs the JieLi toolchain and SDK: BUILDING.md)
tools/flash.sh --build  # check that an FM-1 answers; add --yes to install it
```

See [BUILDING.md](BUILDING.md). Installing firmware is at your own risk; LILA has not been run on a real FM-1 yet.

GPL-3.0-only. See [LICENSING.md](LICENSING.md) for where the parts come from. M-VAVE and FM-1 are trademarks of their owners.
