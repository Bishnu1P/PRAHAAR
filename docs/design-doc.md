# PRAHAAR — Core Design Doc (v0.1)

Purpose: lock in the numbers and rules so all three of you build against the
same source of truth instead of guessing independently. Update this file as
values change during playtesting — it's the contract between subsystems.

## Player

| Stat            | Starting Value | Notes                              |
|-----------------|-----------------|-------------------------------------|
| Health          | 100             | Game over at 0                     |
| Move speed      | 200 px/s        | Tune after arena size is finalized |
| Pickup radius   | 40 px           | Distance to auto-collect XP orbs   |

## Enemy Types (initial set — expand later)

| Type     | Health | Speed (px/s) | Damage | Spawn weight over time |
|----------|--------|--------------|--------|--------------------------|
| Grunt    | 20     | 80           | 5      | High early, tapers off  |
| Runner   | 10     | 150          | 3      | Appears after ~1 min    |
| Brute    | 60     | 50           | 15     | Appears after ~3 min    |

Difficulty scaling formula (placeholder — tune during testing):
```
spawn_interval(t) = max(0.3, 2.0 - 0.01 * t_seconds)
enemy_hp_multiplier(t) = 1.0 + 0.02 * (t_seconds / 60)
```

## Weapons (3–5 for in-scope target)

| Weapon        | Base Damage | Fire Rate | Behavior                     |
|---------------|-------------|-----------|-------------------------------|
| Dagger        | 8           | 1.5/s     | Fires at nearest enemy       |
| Whip          | 6           | 1/s       | Hits all enemies in an arc   |
| Orbiting Axe  | 10          | n/a       | Continuously circles player  |

Each weapon needs at least one upgrade tier (e.g., +damage, +fire rate, or
+projectile count) — define these once base weapons are implemented.

## Progression

```
xp_to_next_level(level) = 5 + (level * 3)
```
- XP orbs drop from defeated enemies (value scales loosely with enemy tier).
- On level-up: pause game, present 3 random upgrade choices (new weapon or
  passive bonus), resume on selection.

## Open questions for the team to settle before Step 2

- [ ] Exact arena dimensions (affects move speed tuning)
- [ ] Passive ability list (e.g., +max HP, +move speed, +pickup radius)
- [ ] Whether enemies use simple direct pursuit or basic pathing around obstacles
