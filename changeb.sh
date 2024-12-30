#!/usr/bin/env bash

# ========================================================
# Script Bash per scaricare un'immagine e impostarla
# come sfondo su Linux (GNOME, KDE, XFCE) o macOS.
#
# Uso:
#   ./imposta_sfondo.sh
# ========================================================

# URL dell'immagine da scaricare
URL_IMMAGINE="https://media.istockphoto.com/id/1352010441/it/foto/coppia-di-gattini-addormentati-innamorati-il-giorno-di-san-valentino-i-nasi-dei-gatti-si.jpg?s=612x612&w=0&k=20&c=bd60KCbS1ImSwQbdND-8c3uGUVGK9VLrDPMQ-ebAJ0Q="

# Nome del file immagine da salvare localmente
IMG_NAME="sfondo_gattini.jpg"

# Percorso completo dell'immagine (nella cartella corrente)
IMG_PATH="$(pwd)/$IMG_NAME"


# --------------------------------------------------------
# 1) Funzione per scaricare l'immagine (usa 'curl' o 'wget')
# --------------------------------------------------------
scarica_immagine() {
  local url="$1"
  local destinazione="$2"

  echo "Scarico immagine da: $url"
  # Prova con 'curl'
  if command -v curl >/dev/null 2>&1; then
    curl -L "$url" -o "$destinazione" || {
      echo "Errore durante il download con curl"
      exit 1
    }
  # In alternativa, prova con 'wget'
  elif command -v wget >/dev/null 2>&1; then
    wget -O "$destinazione" "$url" || {
      echo "Errore durante il download con wget"
      exit 1
    }
  else
    echo "Né 'curl' né 'wget' sono installati. Impossibile scaricare l'immagine."
    exit 1
  fi

  echo "Immagine salvata in: $destinazione"
}


# -----------------------------------------------
# 2) Funzioni specifiche per impostare lo sfondo
# -----------------------------------------------
set_bg_gnome() {
  local img="$1"
  echo "[GNOME] Imposto sfondo con gsettings: $img"

  gsettings set org.gnome.desktop.background picture-uri "file://$img"
  gsettings set org.gnome.desktop.background picture-uri-dark "file://$img"
}

set_bg_kde() {
  local img="$1"
  echo "[KDE/Plasma] Imposto sfondo con qdbus: $img"

  # Lancia uno script tramite shell che esegue qdbus
  script='
for i in $(qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.currentVirtualDesktop); do
  qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.setWallpaperForActivity "$i" "'"$img"'"
done
'
  bash -c "$script" || {
    echo "Errore durante l'impostazione dello sfondo su KDE."
  }
}

set_bg_xfce() {
  local img="$1"
  echo "[XFCE] Imposto sfondo con xfconf-query: $img"

  xfconf-query -c xfce4-desktop \
               -p /backdrop/screen0/monitor0/workspace0/last-image \
               -s "$img" || {
    echo "Errore durante l'impostazione dello sfondo su XFCE."
  }
}

set_bg_linux() {
  # Prova a rilevare il Desktop Environment
  local desktop_env
  desktop_env=$(echo "${XDG_CURRENT_DESKTOP:-}" | tr '[:upper:]' '[:lower:]')

  echo "Rilevato Desktop Environment: $desktop_env"

  if [[ "$desktop_env" == *"gnome"* ]]; then
    set_bg_gnome "$1"
  elif [[ "$desktop_env" == *"kde"* ]] || [[ "$desktop_env" == *"plasma"* ]]; then
    set_bg_kde "$1"
  elif [[ "$desktop_env" == *"xfce"* ]]; then
    set_bg_xfce "$1"
  else
    echo "Desktop Environment non supportato o non riconosciuto."
    echo "Aggiungi logica per LXDE, MATE, ecc. se necessario."
  fi
}

set_bg_macos() {
  local img="$1"
  echo "Imposto sfondo su macOS con osascript: $img"

  # Attenzione: su macOS, se l'immagine non è in un percorso leggibile
  # dal Finder, potrebbe non funzionare
  osascript <<EOF
tell application "System Events"
  set picture of every desktop to ("$img" as POSIX file)
end tell
EOF
}

# ---------------------------
# 3) MAIN
# ---------------------------
main() {
  # Scarica l'immagine
  scarica_immagine "$URL_IMMAGINE" "$IMG_PATH"

  # Verifica il sistema operativo
  local os
  os="$(uname -s | tr '[:upper:]' '[:lower:]')"

  if [[ "$os" == "linux" ]]; then
    set_bg_linux "$IMG_PATH"
  elif [[ "$os" == "darwin" ]]; then
    set_bg_macos "$IMG_PATH"
  else
    echo "Sistema operativo non supportato: $(uname -s)"
    exit 1
  fi
}

main "$@"
