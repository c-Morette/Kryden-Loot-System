# Editing the loot

Everything the storages give comes from JSON files in your server profile folder:

```
<profiles>/Kryden/KrydenLootSystem/
  Settings.json    general options
  Storages.json    each storage class: how many items, from which pools
  Pools/*.json     lists of items with a weight (one file per pool)
  Kits.json        the default attachments, contents, damage and quantity of each item
```

The files are read when the server starts: **restart the server after editing**. A broken file is reported
in the `.RPT` log and skipped; the mod never overwrites your files. The format of every field is in the
[server guide](../workshop/README.md).

## How one search works

1. The server picks how many items the storage gets: a number between `Min` and `Max` of its entry in
   `Storages.json`.
2. For each item it picks a pool from that entry's `Pools` list, by the pool `Weight`.
3. Inside the pool it picks one item, by the item `Weight`.
4. The item is created with its kit from `Kits.json`: attachments, things inside, damage, quantity.

A weight only means something next to the other weights of the same list. In a pool with `AKM: 2000` and
`SKS: 1000`, the AKM comes twice as often as the SKS.

## See what is coming out

Set `"DebugLog": 1` in `Settings.json`. Every search writes one line in the `.RPT`:

```
Survivor searched KLS_WoodenCrate_Military_T4 at <...>: 9 item(s) [AliceBag_Green(28800) SmershVest(14400) ...]
```

`+Mag_...` after a gun is the gun's own magazine or ammo. Turn it back to `0` when you are done.

## Common changes

**Fewer or more guns.** In `Storages.json`, each storage has pools named `<Area>_T<tier>_Guns` in its `Pools`
list. Lower or raise their `Weight`. `MaxGunsPerStorage` in `Settings.json` caps guns per storage (default 2).
`AmmoChance` (0 to 1) is the chance that a gun comes with its own magazine or ammo.

**More or fewer items per storage.** Change `Min` and `Max` of the storage in `Storages.json`. The cargo of a
storage is 10x10, so very high numbers just fail to fit.

**One item rarer or more common in an area.** Open the pool of that area, for example
`Pools/Military_T4.json`, find the line and change its `Weight`. Halve it to make it about half as common.

**Remove an item from the storages.** Delete its line from the pools. To have it back on the floor as in
vanilla, also delete its `<type>` entry from `kls/kls_types.xml` in the mission (that entry is what stops the
CE from spawning it on the floor).

**Add an item from another mod.**
1. Add a line to the pools where it should appear:
   `{"Item": "MyMod_Rifle", "Weight": 500, "Category": "guns"}`
2. If it needs attachments or contents, add an entry to `Kits.json` (copy the entry of a similar vanilla item).
3. To stop it from also spawning on the floor, add to the mission `kls/kls_types.xml`:
   ```xml
   <type name="MyMod_Rifle">
       <nominal>0</nominal>
       <min>0</min>
   </type>
   ```
4. Classnames that don't exist are logged once at start and skipped, so a typo shows up in the `.RPT`.

**Change what comes with a gun.** Edit its entry in `Kits.json`. Each group in `Attachments` spawns with its
`Chance` (0 to 1), and one item of the group is picked by `Weight`. To change it in only one pool, write the
field on the pool line instead: whatever the line has wins over the kit, field by field.

**More or fewer storages in the world.** That is the CE, in the mission `kls/kls_types.xml`: the `nominal`
and `min` of each `KLS_...` type. `lifetime` (4 hours by default) is how long an unsearched storage stays
before it respawns elsewhere.

## Categories

Pools mark each item with a `Category`, used by the themed storages (the medical kit only takes `medical`, the
food sack only `food` and `drink`...). Values used by the template: `guns`, `ammo`, `attachments`, `medical`,
`tools`, `food`, `drink`, `clothes`, `containers`, `explosives`.
