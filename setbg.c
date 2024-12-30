#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <unistd.h>
  #include <sys/wait.h>
  // macOS specific includes
#else
  #include <unistd.h>
  #include <sys/wait.h>
  // Linux specific includes
#endif

/**
 * Struttura usata da libcurl per scrivere i dati su file.
 */
struct MemoryStruct {
    FILE *fp;
};

/**
 * Funzione di callback per libcurl, scrive i chunk di dati su file.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct MemoryStruct *mem = (struct MemoryStruct *)stream;
    return fwrite(ptr, size, nmemb, mem->fp);
}

/**
 * Scarica un'immagine da un URL e la salva sul disco alla destinazione specificata.
 */
bool download_image(const char *url, const char *dest_path)
{
    CURL *curl_handle = NULL;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.fp = fopen(dest_path, "wb");
    if (!chunk.fp) {
        fprintf(stderr, "Errore nell'apertura del file %s: %s\n", dest_path, strerror(errno));
        return false;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    if(curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl_handle);
        if(res != CURLE_OK) {
            fprintf(stderr, "Errore durante il download dell'immagine da %s: %s\n",
                    url, curl_easy_strerror(res));
            fclose(chunk.fp);
            curl_easy_cleanup(curl_handle);
            curl_global_cleanup();
            return false;
        }

        curl_easy_cleanup(curl_handle);
    }
    fclose(chunk.fp);
    curl_global_cleanup();

    printf("Immagine scaricata correttamente in: %s\n", dest_path);
    return true;
}

/**
 * Imposta lo sfondo su GNOME tramite gsettings.
 */
void set_bg_gnome(const char *img)
{
    printf("[GNOME] Imposto sfondo con gsettings: %s\n", img);

    // Esempio di comando: gsettings set org.gnome.desktop.background picture-uri file:///percorso
    // Per sicurezza, aggiungi escape corretti se l'immagine ha spazi o caratteri speciali
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "gsettings set org.gnome.desktop.background picture-uri \"file://%s\" && "
             "gsettings set org.gnome.desktop.background picture-uri-dark \"file://%s\"",
             img, img);

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Errore durante l'impostazione dello sfondo su GNOME.\n");
    }
}

/**
 * Imposta lo sfondo su KDE/Plasma tramite qdbus.
 */
void set_bg_kde(const char *img)
{
    printf("[KDE/Plasma] Imposto sfondo con qdbus: %s\n", img);
    // Script semplificato. L'originale Python esegue un for su tutti i desktop virtuali,
    // qui lo traduciamo in un singolo comando di esempio
    // Nota: potresti dover adattare la logica per più schermi/attività
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
        "for i in $(qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.currentVirtualDesktop); do "
        "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.setWallpaperForActivity \"$i\" \"%s\"; "
        "done",
        img);

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Errore durante l'impostazione dello sfondo su KDE.\n");
    }
}

/**
 * Imposta lo sfondo su XFCE tramite xfconf-query.
 */
void set_bg_xfce(const char *img)
{
    printf("[XFCE] Imposto sfondo con xfconf-query: %s\n", img);
    // Anche qui, potrebbe servire un loop su più monitor, parametri differenti, etc.
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitor0/workspace0/last-image -s \"%s\"",
             img);

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Errore durante l'impostazione dello sfondo su XFCE.\n");
    }
}

/**
 * Funzione che su Linux cerca di rilevare il desktop environment e applica
 * la corretta impostazione dello sfondo.
 */
void set_bg_linux(const char *img)
{
    const char *desktop_env = getenv("XDG_CURRENT_DESKTOP");
    const char *session_type = getenv("XDG_SESSION_TYPE");

    if (!desktop_env) {
        desktop_env = "";
    }
    if (!session_type) {
        session_type = "";
    }

    printf("Rilevato Desktop Environment: %s, Session Type: %s\n", desktop_env, session_type);

    // Convertiamo in minuscolo per semplificare i controlli
    char env_lower[256];
    strncpy(env_lower, desktop_env, sizeof(env_lower));
    for (size_t i = 0; i < strlen(env_lower); i++) {
        env_lower[i] = (char)tolower((unsigned char)env_lower[i]);
    }

    if (strstr(env_lower, "gnome")) {
        set_bg_gnome(img);
    } else if (strstr(env_lower, "kde") || strstr(env_lower, "plasma")) {
        set_bg_kde(img);
    } else if (strstr(env_lower, "xfce")) {
        set_bg_xfce(img);
    } else {
        printf("Desktop Environment non supportato o non riconosciuto.\n");
        printf("Potresti aggiungere qui altre logiche per LXDE, MATE, etc.\n");
    }
}

/**
 * Imposta lo sfondo su macOS con osascript.
 */
void set_bg_macos(const char *img)
{
    printf("Imposto sfondo su macOS con osascript: %s\n", img);
    // Comando AppleScript: set picture of every desktop ...
    // Poiché serve passare un comando multilinea, useremo un singolo snippet con `osascript -e`
    char cmd[2048];
    snprintf(cmd, sizeof(cmd),
             "osascript -e 'tell application \"System Events\" "
             "to set picture of every desktop to (\"%s\" as POSIX file)'",
             img);

    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Errore durante l'impostazione dello sfondo su macOS.\n");
    }
}

/**
 * Imposta lo sfondo su Windows tramite SystemParametersInfoW.
 */
void set_bg_windows(const char *img)
{
    printf("Imposto sfondo su Windows: %s\n", img);

    // Per Windows, usiamo la WinAPI.  
    // Ricorda di compilare con il flag -municode se necessario, oppure usare la wide-char API correttamente.
    // Qui usiamo la versione wide (SystemParametersInfoW).
    // Convertiamo il percorso in wchar_t.  
    wchar_t wpath[MAX_PATH];
    memset(wpath, 0, sizeof(wpath));

    // Semplice conversione ASCII -> Unicode (senza supporto internazionale completo).
    mbstowcs(wpath, img, strlen(img));

    int result = SystemParametersInfoW(
        SPI_SETDESKWALLPAPER, 0, (void *)wpath,
        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE
    );

    if(!result) {
        fprintf(stderr, "Errore durante l'impostazione dello sfondo su Windows (codice: %d).\n",
                GetLastError());
    }
}

/**
 * Programma principale.
 */
int main(void)
{
    // URL dell'immagine
    const char *url_immagine = "https://media.istockphoto.com/id/1352010441/it/foto/coppia-di-gattini-addormentati-innamorati-il-giorno-di-san-valentino-i-nasi-dei-gatti-si.jpg?s=612x612&w=0&k=20&c=bd60KCbS1ImSwQbdND-8c3uGUVGK9VLrDPMQ-ebAJ0Q=";

    // Nome e percorso dove salvare l’immagine
    const char *img_name = "sfondo_gattini.jpg";
    
    // Scarichiamo l'immagine
    if (!download_image(url_immagine, img_name)) {
        fprintf(stderr, "Impossibile scaricare l'immagine, uscita...\n");
        return 1;
    }

    // Rileviamo il sistema operativo
#if defined(_WIN32) || defined(_WIN64)
    set_bg_windows(img_name);
#elif defined(__APPLE__)
    set_bg_macos(img_name);
#elif defined(__linux__)
    set_bg_linux(img_name);
#else
    printf("Sistema operativo non supportato.\n");
#endif

    return 0;
}
