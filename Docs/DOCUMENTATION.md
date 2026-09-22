# PityRoll — Bad-Luck Protection, Soft & Hard Pity

**A five percent drop rate means somebody goes eighty tries without it.**

That player does not think *variance*. That player thinks the game is broken, writes a post about it,
and is not wrong to feel what they feel. PityRoll is the layer that puts a floor under bad luck.

---

## 0. Supported engine and platforms

* Unreal Engine **5.8**
* **Win64.** The plugin's `PlatformAllowList` is Win64 only; Mac and Linux are not supported.
* One runtime C++ module, no third-party code, full source included.
* No dependency on GameplayAbilities, the loot system you already have, or any save framework. A
  chance is a float and a counter is an int.

---

## 1. The five-minute install

1. Add a **Pity Roll** component to whatever owns the luck — usually the player state.
2. Where your table rolls an entry, replace the roll with one line:

```
Result = RollFor("Legendary", 0.05, RandomValue)
```

3. If `Result.bGranted`, hand out the drop. If not, do what you did before.
4. Show `Result.EffectiveChance` in the UI. That is the number the roll used.

That is all of it. The component keeps one counter per key and nothing else.

---

## 2. One counter per entry, and why it is not negotiable

A key is **one entry of one table**: `"Legendary"`, `"BossMount"`, `"ExoticWeapon"`.

A single shared counter is the most common way this is got wrong, and it silently defeats the whole
feature: picking up a common resets the protection that was building towards the rare, so the rare
arrives no sooner than pure chance would have brought it. The player is protected on paper and
unprotected in fact.

`GetKnownKeys()` lists everything the component has seen, which is what a save game iterates.

---

## 3. Soft pity: the chance climbs

```
SoftPityAfter   misses before the chance starts climbing   (0 = from the first miss)
RampPerMiss     how much it climbs per miss                (0 = soft pity off)
MaxChance       never climb above this                     (1 = no cap)
```

Below `SoftPityAfter` the chance is **exactly** the base chance. That matters more than it looks: a
player who datamines your table should find the number they were told. A chance that quietly starts
drifting on attempt one makes every published rate a lie.

```
base 5%, SoftPityAfter 10, RampPerMiss 1%

 misses    0  1  2 ... 10   11   12   ...  20
 chance    5  5  5      5    6    7         15   (percent)
```

`EffectiveChance(BaseChance, Misses, Rules)` is a pure function, and `Roll` calls it rather than
repeating the arithmetic. There is no second copy of the formula to drift out of step with the first.

---

## 4. Hard pity: the guarantee lands on the attempt you named

`HardPityAt = 40` means **the fortieth attempt in a row without the drop gets it.** Not the
thirty-ninth, not the forty-first.

The off-by-one here is invisible in testing and obvious to a community spreadsheet. The plugin states
the rule as an assertion in its own tests: with thirty-nine misses behind it, the next attempt is
number forty, so `IsNextAttemptGuaranteed(39, Rules)` is true and `IsNextAttemptGuaranteed(38, Rules)`
is false.

`AttemptsUntilGuarantee` gives a UI the countdown. It returns a negative number when there is no
guarantee configured, rather than a large one — "no guarantee" and "a long way off" are different
things and a progress bar should show them differently.

---

## 5. The counter is consumed exactly once

A **natural** success inside the pity window resets the counter too.

This is the second most common mistake. Reset only on the guaranteed grant, and a player who gets
lucky on attempt thirty-eight keeps a nearly-full pity counter *and* the drop — so the next rare
arrives two attempts later. Nobody should get the drop and keep the protection.

`FPityResult::Outcome` tells the two apart:

| Outcome | Meaning |
|---|---|
| `Missed` | The roll failed; the counter went up. |
| `Granted` | The roll succeeded on its own, at the effective chance. |
| `GrantedByHardPity` | The guarantee stepped in. |
| `NotRolled` | Base chance of zero or less, with no pity configured. |

`State.HardPityGrants` counts the third case over a session. A high number is a finding, not a
statistic: it means the base chance is too low for the pace the game expects.

---

## 6. The random value is handed in

`RollFor(Key, BaseChance, RandomValue)` takes the number rather than drawing it.

That is what makes a roll reproducible across a server, a client prediction and a replay: the same
inputs give the same answer everywhere. Anything that owns a random stream can feed it —
`FRandomStream`, a seeded session generator, a recorded list in a test.

`RollForRandom(Key, BaseChance)` draws from the engine's stream instead. It is convenient, it is
correct, and it is not replayable. Use it for single-player chests and nothing that has to agree with
another machine.

---

## 7. Saving and loading

`FPityState` is plain data — four integers, no pointers, no objects. It serialises into whatever your
project already uses.

```
for Key in GetKnownKeys():   save[Key] = GetState(Key)
...
for Key, S in save:          RestoreState(Key, S)
```

`ResetKey` and `ResetAll` are for a new season, a prestige, a fresh character.

---

## 8. What PityRoll is not

* **It is not a loot table.** It does not know what a drop is or what is in it. It answers one
  question — did this attempt succeed — and your table does the rest.
* **It is not a random number generator.** It takes one.
* **It is not replicated.** Roll on the server and replicate the result. A pity counter on a client
  is a pity counter the client can edit.
* **It does not decide fairness for you.** `MaxChance`, the ramp and the guarantee are three dials
  and they belong to your design.

---

## 9. Console commands

| Command | What it does |
|---|---|
| `PityRoll.Dump` | Every pity counter in the level: misses, attempts, grants, how many came from the guarantee, and how far the guarantee is. |

---

## 10. API reference

### `UPityRollComponent`

`RollFor(Key, BaseChance, RandomValue)`, `RollForRandom(Key, BaseChance)`,
`GetEffectiveChance(Key, BaseChance)`, `GetAttemptsUntilGuarantee(Key)`, `GetState(Key)`,
`SetRulesFor(Key, Rules)`, `GetRulesFor(Key)`, `ResetKey(Key)`, `ResetAll()`, `GetKnownKeys()`,
`RestoreState(Key, State)`.

Delegates: `OnGranted(Key, bByHardPity, Attempt)`, `OnMissed(Key, Misses)`.

The component does not tick. Pity counts attempts, never seconds.

### `UPityRollStatics` — the rules, on their own

`NormaliseRules`, `EffectiveChance`, `IsNextAttemptGuaranteed`, `Roll`, `AttemptsUntilGuarantee`,
`ObservedRate`.

No world, no actor, no random stream. The component calls exactly these and so do the tests, which is
the only way the chance on screen and the chance in the roll cannot drift apart.

### Project Settings > Plugins > PityRoll

`DefaultRules` applies to any entry that has not been given its own with `SetRulesFor`.
`bLogHardPity` writes a line whenever the guarantee had to step in.

---

## 11. The demo level

`Content/PityRoll/Maps/L_PityRollDemo` — two columns at the same five percent, fed **the same**
sequence of random numbers from a fixed seed. The left column has no protection; the right one has
soft pity from ten misses and a guarantee at forty.

Both columns are identical for the first few seconds — that is the proof that the sequence is shared
and the difference later is the protection and nothing else. Then the left column goes forty-one
attempts dry while the right one, having had its chance climb to twelve percent, has long since
delivered. The row of small bars along the floor is every dry run so far, in order: the shape of the
run, not just its worst moment.

The director drives `UPityRollStatics::Roll` directly, so every number on the board is the plugin's
own output.

**If the level looks frozen**, the viewport is not set to realtime. Either switch realtime on, or
call `StepDemo(Seconds)` on the director yourself — that is what the screenshot run does.

---

## 12. Troubleshooting

**The chance never climbs.** `RampPerMiss` is zero, or `SoftPityAfter` is above the misses you are
reaching. `PityRoll.Dump` prints the misses.

**The guarantee never fires.** `HardPityAt` is zero, which means no guarantee. Or the base chance is
high enough that the counter never gets there — check `HardPityGrants` against `Grants`.

**The guarantee fires one attempt late.** You are counting misses where the plugin counts attempts.
`HardPityAt = 40` is the fortieth *attempt*, which is thirty-nine misses.

**The UI shows a different chance than the drops suggest.** Something is computing the chance a second
time. Show `Result.EffectiveChance` or call `GetEffectiveChance` — both are the same function the roll
uses.

**A common drop resets the rare's protection.** Both are going through the same key. Give each entry
its own.

**The counter is empty after a reload.** `FPityState` has to be saved and restored; the component
keeps it in memory only. See section 7.
