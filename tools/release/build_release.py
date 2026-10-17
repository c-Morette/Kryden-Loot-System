"""
KrydenLootSystem - monta a pasta do Workshop (D-30): release/@KrydenLootSystem/

  mod.cpp, README.md                    de workshop/
  addons/*.pbo + .bisign               do staging (empacotado antes com o KrydenPboToolkit `full`)
  keys/Morette.bikey                   chave publica
  ServerFiles/<mapa>/mission|profile   o template do mapa (sem o REPORT.md interno)

Recusa montar se: o PBO for mais velho que o fonte (o empacotador sai com exit 0 sem empacotar), faltar a
.bisign, sobrar codigo de teste (KLS_StressTest) ou palavra proibida no PBO (lista extra opcional em
tools/release/forbidden.txt, uma por linha), ou o Settings.json do template vier com DebugLog ligado.

Uso: python tools/release/build_release.py [--stage P:\\_kls_stage] [--key P:\\Morette.bikey] [--map chernarusplus]
"""
import argparse, json, os, shutil, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PBOS = ("KrydenLootSystem_Client", "KrydenLootSystem_Server")
FORBIDDEN_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "forbidden.txt")
FORBIDDEN = [b"KLS_StressTest"]
if os.path.isfile(FORBIDDEN_FILE):
    FORBIDDEN += [w.strip().encode() for w in open(FORBIDDEN_FILE, encoding="utf-8") if w.strip()]


def die(msg):
    print("ERRO:", msg)
    sys.exit(1)


def newest_source(folder):
    newest = 0
    for base, _, files in os.walk(folder):
        for name in files:
            newest = max(newest, os.path.getmtime(os.path.join(base, name)))
    return newest


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stage", default=r"P:\_kls_stage")
    ap.add_argument("--key", default=r"P:\Morette.bikey")
    ap.add_argument("--map", default="chernarusplus")
    ap.add_argument("--out", default=os.path.join(ROOT, "release"))
    a = ap.parse_args()

    mod = os.path.join(a.out, "@KrydenLootSystem")
    template = os.path.join(ROOT, "template", a.map)
    if not os.path.isfile(a.key):
        die("chave publica nao encontrada: %s" % a.key)
    if not os.path.isdir(template):
        die("template nao encontrado: %s" % template)

    settings = json.load(open(os.path.join(template, "profile", "Kryden", "KrydenLootSystem", "Settings.json"), encoding="utf-8"))
    if settings.get("DebugLog"):
        die("Settings.json do template com DebugLog ligado")

    for pbo in PBOS:
        path = os.path.join(a.stage, pbo + ".pbo")
        if not os.path.isfile(path) or not os.path.isfile(path + ".Morette.bisign"):
            die("falta %s ou a .bisign dele" % path)
        if os.path.getmtime(path) < newest_source(os.path.join(ROOT, pbo)):
            die("%s e mais velho que o fonte: reempacote" % path)
        if os.path.getmtime(path + ".Morette.bisign") < os.path.getmtime(path):
            die("%s.Morette.bisign e mais velha que o PBO: reassine" % path)
        data = open(path, "rb").read()
        for word in FORBIDDEN:
            if word in data:
                die("%s contem %s" % (path, word.decode()))

    # meta.cpp e do launcher/Publisher (id do Workshop): sobrevive a remontagem.
    meta = None
    if os.path.isfile(os.path.join(mod, "meta.cpp")):
        meta = open(os.path.join(mod, "meta.cpp"), encoding="utf-8", errors="replace").read()
    if os.path.isdir(mod):
        shutil.rmtree(mod)
    os.makedirs(os.path.join(mod, "addons"))
    os.makedirs(os.path.join(mod, "keys"))
    for name in ("mod.cpp", "README.md"):
        shutil.copy2(os.path.join(ROOT, "workshop", name), mod)
    for pbo in PBOS:
        for ext in (".pbo", ".pbo.Morette.bisign"):
            shutil.copy2(os.path.join(a.stage, pbo + ext), os.path.join(mod, "addons"))
    shutil.copy2(a.key, os.path.join(mod, "keys"))
    if meta is not None:
        with open(os.path.join(mod, "meta.cpp"), "w", encoding="utf-8", newline="\n") as f:
            f.write(meta)
    for part in ("mission", "profile"):
        shutil.copytree(os.path.join(template, part), os.path.join(mod, "ServerFiles", a.map, part))

    count = sum(len(files) for _, _, files in os.walk(mod))
    print("release:", mod, "|", count, "arquivos")


if __name__ == "__main__":
    main()
