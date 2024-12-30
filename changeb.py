#!/usr/bin/env python3

import sys
import os
import platform
import subprocess

try:
    import requests
except ImportError:
    print("Installazione del modulo 'requests'...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "requests"])
    import requests


def scarica_immagine(url, percorso_destinazione):
    try:
        response = requests.get(url, stream=True)
        response.raise_for_status()
        with open(percorso_destinazione, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                f.write(chunk)
        print(f"Immagine scaricata correttamente in: {percorso_destinazione}")
    except Exception as e:
        print(f"Errore durante il download dell'immagine: {e}")
        sys.exit(1)


def set_bg_gnome(img):
    try:
        print(f"[GNOME] Imposto sfondo con gsettings: {img}")
        subprocess.run([
            "gsettings", "set",
            "org.gnome.desktop.background",
            "picture-uri",
            f"file://{img}"
        ], check=True)
        subprocess.run([
            "gsettings", "set",
            "org.gnome.desktop.background",
            "picture-uri-dark",
            f"file://{img}"
        ], check=True)
    except Exception as e:
        print(f"Errore durante l'impostazione dello sfondo su GNOME: {e}")


def set_bg_kde(img):
    try:
        print(f"[KDE/Plasma] Imposto sfondo con qdbus: {img}")
        script = (
            'for i in $(qdbus org.kde.plasmashell /PlasmaShell '
            'org.kde.PlasmaShell.currentVirtualDesktop); do '
            f'qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.setWallpaperForActivity "$i" "{img}"; done'
        )
        subprocess.run(script, shell=True, check=True)
    except Exception as e:
        print(f"Errore durante l'impostazione dello sfondo su KDE: {e}")


def set_bg_xfce(img):
    try:
        print(f"[XFCE] Imposto sfondo con xfconf-query: {img}")
        subprocess.run([
            "xfconf-query", "-c", "xfce4-desktop",
            "-p", "/backdrop/screen0/monitor0/workspace0/last-image",
            "-s", img
        ], check=True)
    except Exception as e:
        print(f"Errore durante l'impostazione dello sfondo su XFCE: {e}")


def set_bg_linux(img):
    desktop_env = os.environ.get('XDG_CURRENT_DESKTOP', '').lower()
    session_type = os.environ.get('XDG_SESSION_TYPE', '').lower()

    print(f"Rilevato Desktop Environment: {desktop_env}, Session Type: {session_type}")

    if 'gnome' in desktop_env:
        set_bg_gnome(img)
    elif 'kde' in desktop_env or 'plasma' in desktop_env:
        set_bg_kde(img)
    elif 'xfce' in desktop_env:
        set_bg_xfce(img)
    else:
        print("Desktop Environment non supportato o non riconosciuto.")
        print("Potresti aggiungere qui altre logiche per LXDE, MATE, etc.")


def set_bg_macos(img):
    try:
        print(f"Imposto sfondo su macOS con osascript: {img}")
        script = f'''
        tell application "System Events"
            set picture of every desktop to ("{img}" as POSIX file)
        end tell
        '''
        subprocess.run(["osascript", "-e", script], check=True)
    except Exception as e:
        print(f"Errore durante l'impostazione dello sfondo su macOS: {e}")


def set_bg_windows(img):
    try:
        print(f"Imposto sfondo su Windows: {img}")
        import ctypes
        SPI_SETDESKWALLPAPER = 20
        SPIF_UPDATEINIFILE = 0x1
        SPIF_SENDWININICHANGE = 0x2
        img_assoluto = os.path.abspath(img)
        ctypes.windll.user32.SystemParametersInfoW(
            SPI_SETDESKWALLPAPER, 0, img_assoluto,
            SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE
        )
    except Exception as e:
        print(f"Errore durante l'impostazione dello sfondo su Windows: {e}")


def main():
    url_immagine = (
        "https://media.istockphoto.com/id/1352010441/it/foto/"
        "coppia-di-gattini-addormentati-innamorati-il-giorno-di-san-valentino"
        "-i-nasi-dei-gatti-si.jpg?s=612x612&w=0&k=20&c=bd60KCbS1ImSwQbdND-8c3uGUVGK9VLrDPMQ-ebAJ0Q="
    )

    img_name = "sfondo_gattini.jpg"
    img = os.path.join(os.getcwd(), img_name)

    scarica_immagine(url_immagine, img)
    osystem = platform.system().lower()

    if osystem.startswith('linux'):
        set_bg_linux(img)
    elif osystem.startswith('darwin'):
        set_bg_macos(img)
    elif osystem.startswith('windows'):
        set_bg_windows(img)
    else:
        print(f"Sistema operativo non supportato: {platform.system()}")


if __name__ == "__main__":
    main()
