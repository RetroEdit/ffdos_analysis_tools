# ffdos_analysis_tools

Currently, this is a command line program for analyzing Final Fantasy I & II: Dawn of Souls' fifteen puzzle ship minigame.

The fifteen puzzle analyzer (`analyze_puzzle`) currently operates using a set of 1-letter commands.

## The basics

The fifteen puzzle is a minigame in Final Fantasy I. It's accessed by pressing 'B' 23 times while holding down 'A' while in a (sea) ship. The fifteen puzzle is useful for speedrunning because it gives prize items for placing (achieving a time below the previous best time for a particular place). Specifically in the GBA version of Final Fantasy, the possible prizes are as follows: 

- 1st place: Megalixir
- 2nd place: Turbo Ether Elixir, Remedy, Hermes' Shoes, Emergency Exit
- 3rd place: Spider's Silk, White Fang, Red Fang, Blue Fang, Red Curtain, White Curtain, Blue Curtain, Vampire Fang, Cockatrice Claw
- Bonus prizes: Potion, Antidote, Gold Needle, Ether, Eye Drops, Echo Grass, Phoenix Down, 100 Gil

It also gives a bonus prize whether you place or not, though the bonus prize is less useful for speedrunning.

Currently, this program is designed for finding puzzle outcomes from scrambles after the game has been reset (either through `A+B+Start+Select` or through a power-cycle; the former being faster for speedrunning). This is for a few reasons. With the current code, it would be extremely inefficient to search through all possible scrambles the game can generate (this is not necessarily unsolvable; future optimizations like a puzzle database may speedup searching). Additionally, at the moment, there is a pratical requirement for speedrunners to reset the game to have any hopes of reliably reproducing RNG outcomes. With more sophisticated routing (e.g. routing every encounter/stat-gain), this could change, but for the moment this program works, and it could be improved later as speedrunning evolves.

## Scramble formatting

To enter a scramble: input a line containing the 16 tile values representing the scramble. Enter the tiles left-to-right, top-to-bottom. Use '0' for the blank space. A solved scramble looks like this:

```
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0
```

## Prize file formatting

One goal for this program is to be able to smartly plan prize collection and puzzle skipping. Currently, that functionality is limited, but the prize configuration can be found in `prizes.txt`. This contains "prize goal" lines formatted like this:

```
<prize name>,<minimum required>,<maximum required>
```

It also contains separate lines like `SkipWait: 2` and `SkipsAllowed: 1` that could configure other aspects of prize searching.

This may be reformulated later.

## Commands

### `a` - Analyze a seed

Intended to combine the functionality of the below `f` and `p` commands into one command. WIP.

### `f` - Find scramble seed

Gets the seed for a particular scramble (see above section [#Scramble formatting]). The seed can be entered into `p` to get a list of puzzles and prizes that will come after the current scramble.

### `p` - Predict puzzles/prizes

Predicts future puzzles and prizes. Currently, this assumes the player will place in every solve. Sometimes, not placing in certain solves may be preferred for setting up later puzzle prizes.

### `r` - Report on multiple scrambles

Takes a file name and basically runs `f` on each scramble in the file.

### `i` - Index an RNG value

Calculate the number of RNG advances before arriving at a specific goal. This was more useful before I understood that the 15-puzzle worked internally by adding `VCOUNT` to the RNG and deviating from standard advancement.

## How does it work?

Seed-finding currently functions by brute-force generation of RNG configurations until the scramble is found. 

More specifically, the game uses the seed as a starting point for generating the scramble; the same seed will always result in the same outcome. The game starts out with a solved puzzle configuration, and then applies exactly 4096 random valid one-tile moves to the puzzle.

Certain seeds converge on the same scramble outcome because while the game does exactly 4096 valid moves, a notable percentage (1/4 to 1/3) will be skipped and deemed invalid because they try to move pieces outside of the puzzle. If an invalid move occurs at the start of the scrambling process, the seed can be treated as identical to the  In rare cases, the `steps` value output from `f` could potentially be incorrect because of RNG convergence. This is not relevant for speedrunning, as the number of steps

An even more unlikely result is for the seed itself to be wrong. This has not been extensively investigated, but the probability of this is low, and likely completely irrelevant for speedrunning usage of this program because of the small number of seeds that actually need to be explored.
