# Enemy combat integration

Scope: no VFX; connect the 25 imported StrikeJaeger clips to the existing C++ enemy and arena blueprint. No claim of shot-for-shot equivalence with the Bilibili reference has been validated.

## Behavior

- Chase at 300 cm/s; slow near the player. During attack cooldown, retreat when too close and alternate clockwise/counterclockwise circling.
- Use facing-relative forward/back/left/right walk clips; use run start/loop/end for faster forward travel. Start and stop transitions are interruptible by combat.
- Cycle Attack01–06. Turn only through the early telegraph; commit facing before the strike. Current prototype hit window is 30–48% of the clip, converted by playback rate; each swing damages at most once.
- Light hits interrupt attack and play the front/back reaction according to attacker position. Three light hits can provoke a dodge after the reaction, subject to four-second cooldown.
- Heavy hits play a short front/back reaction, then stun start, loop, end, recovery. Further damage during stun reduces health without resetting the stun indefinitely.
- Back/left/right dodges move the collision capsule 160 cm over 0.35 seconds with collision sweeps. They do not grant hidden invulnerability. Death clears pending action timers and stops movement/collision.

## Assets and verification

`Scripts/bind_enemy_combat.py` validates skeleton and duration, enables root extraction/locking, writes the 25 blueprint references and records `Saved/EnemyCombatBindings.json`.

`Scripts/test_enemy_combat.py` runs PIE over real frames, checking chase, six attacks, player skill -> stun -> recovery, three dodge displacements and death. Runtime-only player health override is restored afterward. Results are in `Saved/EnemyCombatRuntime.json`.

## Limits

Animation still uses single-node sequences, not an AnimBP blend graph. Hit windows need visual frame-by-frame tuning per clip. Chase uses direct movement with capsule collision, not navigation around obstacles. These are prototype limitations, not a finished commercial combat system.
