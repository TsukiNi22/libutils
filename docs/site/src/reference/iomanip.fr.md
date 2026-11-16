+++
title = "IOManip"
summary = "Fonctions qui renvoient les séquences d'échappement ANSI (styles, couleurs, curseur, écran, modes, liens) et lecteurs des rapports du terminal."
keywords = "iomanip ansi séquence échappement couleur style gras souligné curseur écran effacer lien hypertexte souris rapport enable_ansi utils_iomanip"
+++

| Define | Groupe | Namespace | Headers |
|---|---|---|---|
| `_IOManip` (depuis `_Manip`) | `_Tools` | `utils::iomanip` | `utils/manip/iomanip/` |

Fonctions qui renvoient les séquences d'échappement ANSI (à écrire sur un terminal) et lecteurs des rapports du terminal. Les outils de chaînes du même groupe sont dans [SManip](page:smanip).

## Styles {#styles}

| Fonction | Séquence |
|---|---|
| `reset()` | `ESC[0m` |
| `strong()`, `dark()`, `italic()`, `underlined()` | `1`, `2`, `3`, `4` |
| `flashing_slow()`, `flashing_fast()` | `5`, `6` |
| `reversed()`, `hide()`, `bar()` | `7`, `8`, `9` |
| `monospace()`, `framed()`, `encircled()`, `overlined()` | `50`, `51`, `52`, `53` |
| `exposant()`, `indice()` | `73`, `74` (rarement pris en charge) |
| `<style>_reset()` | la réinitialisation correspondante (`strong_reset()` / `dark_reset()` = `22`, `flashing_*_reset()` = `25`...) |
| `set_style(Style)` / `set_style({Style, ...})` | une seule séquence pour plusieurs valeurs `Style` |
| `reset_style(ResetStyle)` / `reset_style({ResetStyle, ...})` | pareil pour les réinitialisations |

> [!NOTE]
> Les codes suivent ECMA-48 / xterm depuis `v3.0.0` (les clignotements lent / rapide étaient inversés, la réinitialisation du gras était `21`).

## Couleurs {#colors}

| Fonction | Description |
|---|---|
| `color(Color)` / `color(BackColor)` | 16 couleurs (`Black`... `BrightWhite`, `Default`) pour le texte / le fond |
| `color_id(id)`, `back_color_id(id)`, `underline_color_id(id)` | Palette de 256 couleurs |
| `color_rgb(r, g, b)`, `back_color_rgb(r, g, b)`, `underline_color_rgb(r, g, b)` | Couleurs vraies (true color) |

## Curseur et écran {#cursor-screen}

| Fonction | Description |
|---|---|
| `up(n)`, `down(n)`, `right(n)`, `left(n)` | Déplace le curseur |
| `next_line(n)`, `previous_line(n)`, `column(col)`, `pos(row, col)` | Va à une ligne / colonne / position |
| `save_cur()`, `load_cur()`, `show_cur()`, `hide_cur()` | État du curseur |
| `screen()`, `screen_start()`, `screen_end()`, `scrollback_buffer()` | Efface l'écran (tout / jusqu'au début / jusqu'à la fin / historique) |
| `line()`, `line_start()`, `line_end()` | Efface la ligne |
| `scroll_up(n)`, `scroll_down(n)` | Défilement |
| `save_screen()`, `load_screen()` | Écran alternatif |
| `hyperlink(display, link)`, `file_hyperlink(display, path)` | Liens cliquables (OSC 8) |
| `esc()`, `csi(code)` | `ESC` / `ESC[` brut + code |

## Modes {#modes}

| Fonctions | Description |
|---|---|
| `wrapping_enable()` / `wrapping_disable()` | Retour à la ligne automatique |
| `inverted_color_enable()` / `inverted_color_disable()` | Couleurs de l'écran inversées |
| `mouse_tracking_enable()` / `_disable()` | Rapports des clics de souris (classiques) |
| `mouse_move_tracking_enable()` / `_disable()` | Rapports des mouvements de souris |
| `mouse_adv_tracking_enable()` / `_disable()` | Rapports de souris SGR |
| `report_focus_enable()` / `report_focus_disabled()` | Prise / perte du focus (`ESC[I` / `ESC[O`) |
| `report_past_enable()` / `report_past_disable()` | Bracketed paste (`ESC[200~ ... ESC[201~`) |
| `enable_ansi()` | Windows : active le mode terminal virtuel de la console (les séquences sont interprétées), à appeler une fois au début ; `true` sous Unix (rien à faire) |

## Rapports {#reports}

| Fonction | Description |
|---|---|
| `get_pos()` | Demande la position du curseur (le terminal répond `ESC[row;colR`) |
| `read_cursor_position()` | Lit cette réponse sur stdin, `{row, col}` (`{-1, -1}` si invalide) |
| `read_mouse_event()` | Lit un rapport de souris classique (`ESC[M` + 3 octets), renvoie `MouseEvent{button, x, y}` |
| `read_advanced_mouse_event()` | Lit un rapport SGR (`ESC[<b;x;yM`), renvoie `AdvancedMouseEvent{button, x, y, pressed}` |

`MouseButton` : `Left`, `Right`, `Middle`, `Release`, `Unknown`. Les rapports invalides lèvent `ANSIMouseEvent`.

## Caractères spéciaux {#special-characters}

`utils::iomanip::Char` (`enum class: char`) : les caractères de contrôle ASCII `NUL`, `SOH`... `ESC`, `DEL` et `EOF` (`static_cast<char>(utils::iomanip::Char::ESC)`).

## Exemple {#example}

```cpp
#define _IOManip
#include <utils/utils.hpp>

std::cout << utils::iomanip::set_style({utils::iomanip::Style::Strong, utils::iomanip::Style::Underlined})
          << utils::iomanip::color(utils::iomanip::Color::Red) << "error" << utils::iomanip::reset()
          << " see " << utils::iomanip::hyperlink("the docs", "https://tsukini22.github.io/libutils/docs/") << std::endl;
```

## En C {#in-c}

Chaque séquence est une chaîne : `utils_iomanip_<name>` avec le nom C++. Une séquence sans argument est une chaîne statique ; une séquence avec arguments est gardée dans un buffer thread local valide jusqu'au prochain appel de la même fonction dans le thread, et sa version `_r` la copie dans un buffer à vous (renvoie la longueur complète comme `snprintf`). Les enums sont `utils_iomanip_color_t` (`C_API_IOMANIP_COLOR_*`), `utils_iomanip_back_color_t`, `utils_iomanip_style_t` (`C_API_IOMANIP_STYLE_*`), `utils_iomanip_reset_style_t`, `utils_iomanip_char_t` (`C_API_IOMANIP_CHAR_*`), `utils_iomanip_mouse_button_t`.

| C++ | C |
|---|---|
| `set_style({...})` | `utils_iomanip_set_styles(styles, count)` (`utils_iomanip_set_style` pour un seul) |
| `color(Color::Red)` | `utils_iomanip_color(C_API_IOMANIP_COLOR_RED)` |
| `pos(row, col)` | `utils_iomanip_pos(row, col)` / `utils_iomanip_pos_r(row, col, buf, size)` |
| `read_cursor_position()` | `utils_iomanip_read_cursor_position(&row, &col)` |
| `read_mouse_event()` | `utils_iomanip_read_mouse_event(&event)` (`utils_iomanip_mouse_event_t`) |

```c
#define _IOManip
#include <utils.h>
#include <stdio.h>

int main(void)
{
    utils_iomanip_style_t styles[] = {C_API_IOMANIP_STYLE_STRONG, C_API_IOMANIP_STYLE_UNDERLINED};
    char link[256];

    utils_iomanip_enable_ansi(); // Windows console: interpret the sequences
    utils_iomanip_hyperlink_r("the docs", "https://tsukini22.github.io/libutils/docs/", link, sizeof(link));
    printf("%s%serror%s see %s\n", utils_iomanip_set_styles(styles, 2), utils_iomanip_color(C_API_IOMANIP_COLOR_RED),
        utils_iomanip_reset(), link);
    return 0;
}
```
