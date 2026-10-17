"""
KrydenLootSystem - monta o espelho PUBLICO do repositorio (D-34, ADPL-SA).

O repositorio de desenvolvimento e privado: tem decisoes internas, o CLAUDE.md e a investigacao de trabalho
privado de cliente. O publico recebe so uma lista fechada de arquivos rastreados pelo git:

  KrydenLootSystem_Client/, KrydenLootSystem_Server/   codigo do mod
  template/<mapa>/                                      templates (sem o REPORT.md interno)
  tools/template/build_template.py                      o gerador
  tools/release/build_release.py, build_public.py       scripts de release
  workshop/                                             mod.cpp, logo, guia do servidor, texto do Workshop
  public/README.md, public/LICENSE.md, public/docs/     viram README.md, LICENSE.md e docs/ na raiz

Recusa se algum arquivo copiado citar uma palavra de tools/release/forbidden.txt (que nunca vai junto).
O destino e um clone do repositorio publico: tudo menos o .git e apagado e copiado de novo.

Uso: python tools/release/build_public.py --dest <clone do repositorio publico>
"""
import argparse, os, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FORBIDDEN_FILE = os.path.join(ROOT, "tools", "release", "forbidden.txt")
KEEP = ("KrydenLootSystem_Client/", "KrydenLootSystem_Server/", "template/", "workshop/",
        "tools/template/build_template.py", "tools/release/build_release.py", "tools/release/build_public.py")
RENAME = {"public/README.md": "README.md", "public/LICENSE.md": "LICENSE.md"}
SKIP_NAMES = {"REPORT.md"}
GITIGNORE = "build/\n*.pbo\n*.bisign\n*.bikey\n*.biprivatekey\n/release/\n__pycache__/\n"


def die(msg):
    print("ERRO:", msg)
    sys.exit(1)


def target_of(path):
    if path in RENAME:
        return RENAME[path]
    if path.startswith("public/docs/"):
        return "docs/" + path[len("public/docs/"):]
    if path.startswith(KEEP) and os.path.basename(path) not in SKIP_NAMES:
        return path
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dest", required=True)
    a = ap.parse_args()
    if not os.path.isdir(os.path.join(a.dest, ".git")):
        die("o destino nao e um clone git: %s" % a.dest)

    forbidden = []
    if os.path.isfile(FORBIDDEN_FILE):
        forbidden = [w.strip().lower() for w in open(FORBIDDEN_FILE, encoding="utf-8") if w.strip()]

    files = subprocess.run(["git", "-C", ROOT, "ls-files"], capture_output=True, text=True, check=True).stdout.split("\n")
    plan = [(f, target_of(f)) for f in files if f and target_of(f)]
    for src, _ in plan:
        if src.lower().endswith((".paa", ".png")):
            continue
        text = open(os.path.join(ROOT, src), encoding="utf-8", errors="replace").read().lower()
        for word in forbidden:
            if word.lower().rstrip("_") in text:
                die("%s cita uma palavra proibida" % src)

    for name in os.listdir(a.dest):
        if name == ".git":
            continue
        path = os.path.join(a.dest, name)
        shutil.rmtree(path) if os.path.isdir(path) else os.remove(path)
    for src, dst in plan:
        out = os.path.join(a.dest, dst)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        shutil.copy2(os.path.join(ROOT, src), out)
    with open(os.path.join(a.dest, ".gitignore"), "w", encoding="utf-8", newline="\n") as f:
        f.write(GITIGNORE)
    print("publico:", a.dest, "|", len(plan), "arquivos")


if __name__ == "__main__":
    main()
