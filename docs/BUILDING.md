# Building from source, and how it was made

## Build the PBOs

The mod is two PBOs. Each source folder has a `$PBOPREFIX$` file with its internal path:

```
KrydenLootSystem_Client   ->  Kryden\KrydenLootSystem\KrydenLootSystem_Client
KrydenLootSystem_Server   ->  Kryden\KrydenLootSystem\KrydenLootSystem_Server
```

Every path inside `config.cpp` (scripts, textures, models) starts with that prefix. Pack each folder with a PBO
tool that writes the prefix into the PBO: DayZ Tools Addon Builder, PBO Manager or Mikero's tools all do.
No binarization is needed (there are no models of our own).

If you publish your own version, sign it with **your own** key (DayZ Tools `DSCreateKey`, then sign each PBO)
and ship your `.bikey` in `keys/`. Never reuse someone else's key.

## Build the Workshop folder

`tools/release/build_release.py` puts together `release/@KrydenLootSystem/`: `mod.cpp`, the server guide, the
PBOs and their signatures, the public key and `ServerFiles/<map>/` (the template). It refuses to build if a
PBO is older than its source folder or is missing its signature.

```
python tools/release/build_release.py --stage <folder with the packed PBOs> --key <your .bikey> --map chernarusplus
```

## Test locally

1. Run a local DayZ server with the mod and the template installed (see the [server guide](../workshop/README.md)).
2. Set `"DebugLog": 1` in `Settings.json`: every search, every storage the CE creates and every storage it
   deletes writes a line in the `.RPT`.
3. A clean log is not proof that it works: check the storages in game.

## How it was made

- **Language:** Enforce Script, the scripting language of DayZ. The client side follows the vanilla patterns:
  the action is modeled on the vanilla tent and barrel actions, the "delete when empty" on the fireplace, the
  magazine in the gun on `Weapon_Base.SpawnAttachedMagazine`.
- **Loot data:** a Python 3 script (`tools/template/build_template.py`) reads the vanilla mission
  (`types.xml`, `cfgspawnabletypes.xml`, `mapgroupproto.xml`, `mapgrouppos.xml`, events, `areaflags.map`) and
  writes the template. The calibration is described in [How it works](HOW-IT-WORKS.md#how-the-loot-was-calibrated).
- **Textures and logo:** vanilla textures recolored, converted to `.paa` with DayZ Tools `ImageToPAA`.
- **Packing and signing:** Kryden's own PBO toolkit; any PBO tool that writes `$PBOPREFIX$` works the same.
- **Testing:** a local server with `DebugLog` on, in-game tests of every change, and a load test that searched
  600 storages of every class without a player (no errors, every lot within its Min/Max, about 7 ms per
  search).

## Contributing

Issues and pull requests are welcome. A fork or a modified version must follow the
[license](../LICENSE.md): credit with a link, no commercial use, the same license, and published as a separate
mod.
