

#define __AVR_ATtiny85__

#include "io_hw.h"
#include "twi/i2c.h"
#include "twi/uart.h"
#include "menu/menu.h"
#include "utils.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "ssd1306/ssd1306.h"

#define EEPROM_MAGIC_BYTE 0x42 
#define EEPROM_CONFIG_ADDR 1

const int8_t sin_table[64] PROGMEM = {
      0,   12,   25,   37,   49,   60,   71,   81,   90,   98,  106,  112,  117,  122,  125,  126,
    127,  126,  125,  122,  117,  112,  106,   98,   90,   81,   71,   60,   49,   37,   25,   12,
      0,  -12,  -25,  -37,  -49,  -60,  -71,  -81,  -90,  -98, -106, -112, -117, -122, -125, -126,
   -127, -126, -125, -122, -117, -112, -106,  -98,  -90,  -81,  -71,  -60,  -49,  -37,  -25,  -12
};

// 2. Osiem wierzchołków sześcianu (X, Y, Z) rozpiętych od -15 do +15
const int8_t cube_vertices[8][3] PROGMEM = {
    {-15, -15, -15}, { 15, -15, -15}, { 15,  15, -15}, {-15,  15, -15}, // Tył
    {-15, -15,  15}, { 15, -15,  15}, { 15,  15,  15}, {-15,  15,  15}  // Przód
};

const uint8_t tiles_4x4[3][4] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00}, // Indeks 0: Puste tło
    {0x0F, 0x09, 0x09, 0x0F}, // Indeks 1: Kwadrat (Ściana)
    {0x06, 0x0F, 0x0F, 0x06}  // Indeks 2: Kółko (Bohater)
};
struct config
{
  uint8_t work_time_m;
  uint8_t work_time_s;

  uint8_t break_time_m;
  uint8_t break_time_s;

  uint8_t pause;
  uint8_t is_break;
};

struct config c_config = {
    .work_time_m = 25,
    .work_time_s = 0,
    .break_time_m = 5,
    .break_time_s = 0,
    .pause = true,
    .is_break = false
};

typedef enum
{
  NONE = 0,
  ANY = 1,
  RIGHT = 2,
  LEFT = 3,
} BUTTONS;

volatile int awake_INT = false;
volatile uint16_t live_time_ms = 0;
volatile uint16_t live_time_s = 0;
int left_button_press = false;
int right_button_press = false;
int button_pressed = false;

volatile int is_awake = true;

BUTTONS buttons_management(void);
void display_menu(uint8_t index_menu);
static menu_item_t menu_list[MENU_LIST_SIZE];

void oled_draw_ack(void);
void oled_clear_sqares(uint8_t segment);
void draw_number(char number, int segment);
void oled_set_sqare(uint8_t x, uint8_t y);
void oled_clear_sqare(uint8_t x, uint8_t y);

void oled_draw_frame_16x16(uint8_t x, uint8_t y, const uint8_t* frame_progmem) {
    uint8_t ram_buffer[8]; // Zmniejszamy bufor do 8!
    
    // 1. Lewa górna ćwiartka (8x8 px)
    for(int i = 0; i < 8; i++) {
        ram_buffer[i] = pgm_read_byte(&frame_progmem[i]);
    }
    oled_set_pixels(x, y, 8, ram_buffer);

    // 2. Prawa górna ćwiartka (8x8 px) - Przesuwamy X o 8
    for(int i = 0; i < 8; i++) {
        ram_buffer[i] = pgm_read_byte(&frame_progmem[i + 8]);
    }
    oled_set_pixels(x + 8, y, 8, ram_buffer);

    // 3. Lewa dolna ćwiartka (8x8 px) - Przesuwamy Y o 1
    for(int i = 0; i < 8; i++) {
        ram_buffer[i] = pgm_read_byte(&frame_progmem[16 + i]);
    }
    oled_set_pixels(x, y + 1, 8, ram_buffer);

    // 4. Prawa dolna ćwiartka (8x8 px)
    for(int i = 0; i < 8; i++) {
        ram_buffer[i] = pgm_read_byte(&frame_progmem[24 + i]);
    }
    oled_set_pixels(x + 8, y + 1, 8, ram_buffer);
}

void draw_pixel(uint8_t x, uint8_t y) {
    // Zabezpieczenie przed wyjściem poza ekran
    if (x >= 128 || y >= 64) return;

    // Y skacze co 8, więc dzielimy przez 8 żeby poznać numer Paska (Page 0-7)
    uint8_t page = y / 8;
    
    // Reszta z dzielenia mówi nam, który to konkretnie piksel w pionie (Bit 0-7)
    uint8_t bit_pos = y % 8;

    // Przesuwamy jedynkę na odpowiednią pozycję
    uint8_t byte_to_send = (1 << bit_pos);

    // Wysyłamy ten 1 bajt na ekran. 
    // UWAGA: Pozostałe 7 pikseli w tym pionie zostanie wyczyszczonych!
    oled_set_pixels(x, page, 1, &byte_to_send);
}

void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) {
        draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

uint8_t tilemap[16][8];

// Funkcja pomocnicza do stawiania kafelka (abyś nie musiał myśleć o bitach przy projektowaniu planszy)
void set_tile(uint8_t x, uint8_t y_grid, uint8_t tile_id) {
    uint8_t page = y_grid / 2; // Dzielimy przez 2, by trafić w odpowiedni pasek RAM
    
    if (y_grid % 2 == 0) {
        // Zmieniamy górne 4 bity, dolne zostawiamy w spokoju
        tilemap[x][page] = (tilemap[x][page] & 0xF0) | (tile_id & 0x0F);
    } else {
        // Zmieniamy dolne 4 bity (przesuwamy w lewo o 4), górne zostawiamy w spokoju
        tilemap[x][page] = (tilemap[x][page] & 0x0F) | ((tile_id & 0x0F) << 4);
    }
}

void render_tilemap() {
    // 1. Pętla przez 8 pionowych pasków ekranu (Pages)
    for (uint8_t page = 0; page < 8; page++) {
        
        // 2. Pętla przez 32 kolumny kafelków (X)
        for (uint8_t x = 0; x < 16; x++) {
            
            // Pobieramy "kanapkę" z RAM
            uint8_t data = tilemap[x][page];
            
            // Maskujemy, żeby oddzielić indeksy
            uint8_t tile_top_idx = data & 0x0F;        // Dolne 4 bity
            uint8_t tile_bot_idx = (data >> 4) & 0x0F; // Przesuwamy i czytamy górne 4 bity

            // 3. Rysujemy 4 poziome piksele z których składa się nasz 4x4 kafelek
            for (uint8_t col = 0; col < 4; col++) {
                // Czytamy grafikę z PROGMEM dla obu kafelków
                uint8_t top_gfx = pgm_read_byte(&tiles_4x4[tile_top_idx][col]);
                uint8_t bot_gfx = pgm_read_byte(&tiles_4x4[tile_bot_idx][col]);

                // KLEIMY KANAPKĘ DLA OLED (Dolny przesuwamy o 4 bity w górę i łączymy OR z górnym)
                uint8_t oled_byte = (bot_gfx << 4) | (top_gfx & 0x0F);

                // Fizyczna kolumna ekranu (od 0 do 127) to nasz X * 4 + aktualny skrawek kafelka
                uint8_t x_oled = (x * 4) + col;
                
                // Wysyłamy finalny, złożony bajt na ekran!
                oled_set_pixels(x_oled, page, 1, &oled_byte);
            }
        }
    }
}

int8_t fast_sin(uint8_t angle) {
    return (int8_t)pgm_read_byte(&sin_table[angle % 64]);
}

int8_t fast_cos(uint8_t angle) {
    return fast_sin(angle + 16);
}

// Funkcja rysująca cały obrócony sześcian używając wektorów
void draw_wireframe_cube(uint8_t angle_x, uint8_t angle_z) {
    int16_t screen_pts[8][2]; // Tutaj zapiszemy pozycje 8 punktów po zrzutowaniu na ekran 2D
    
    // Obliczenia trygonometryczne stałoprzecinkowe (mnożymy przez sinus i dzielimy przez 128)
    int8_t sin_x = fast_sin(angle_x);
    int8_t cos_x = fast_cos(angle_x);
    int8_t sin_z = fast_sin(angle_z);
    int8_t cos_z = fast_cos(angle_z);

    // Przelicz każdy z 8 wierzchołków
    for (int i = 0; i < 8; i++) {
        // Czytamy oryginalne współrzędne 3D z pamięci Flash
        int8_t x = (int8_t)pgm_read_byte(&cube_vertices[i][0]);
        int8_t y = (int8_t)pgm_read_byte(&cube_vertices[i][1]);
        int8_t z = (int8_t)pgm_read_byte(&cube_vertices[i][2]);

        // 1. Obrót w osi X
        int16_t y_rot = (y * cos_x - z * sin_x) / 128;
        int16_t z_rot = (y * sin_x + z * cos_x) / 128;

        // 2. Obrót w osi Z (kręcenie się dookoła)
        int16_t x_rot = (x * cos_z - y_rot * sin_z) / 128;
        int16_t y_rot2 = (x * sin_z + y_rot * cos_z) / 128;

        // 3. Rzutowanie ortogonalne (Ignorujemy nowe Z) i przesunięcie na środek ekranu (64, 32)
        screen_pts[i][0] = 64 + x_rot;
        screen_pts[i][1] = 32 + y_rot2;
    }

    // Narysuj 12 krawędzi sześcianu łącząc obliczone punkty liniami Bresenhama
    // Tylna ściana
    draw_line(screen_pts[0][0], screen_pts[0][1], screen_pts[1][0], screen_pts[1][1]);
    draw_line(screen_pts[1][0], screen_pts[1][1], screen_pts[2][0], screen_pts[2][1]);
    draw_line(screen_pts[2][0], screen_pts[2][1], screen_pts[3][0], screen_pts[3][1]);
    draw_line(screen_pts[3][0], screen_pts[3][1], screen_pts[0][0], screen_pts[0][1]);
    // Przednia ściana
    draw_line(screen_pts[4][0], screen_pts[4][1], screen_pts[5][0], screen_pts[5][1]);
    draw_line(screen_pts[5][0], screen_pts[5][1], screen_pts[6][0], screen_pts[6][1]);
    draw_line(screen_pts[6][0], screen_pts[6][1], screen_pts[7][0], screen_pts[7][1]);
    draw_line(screen_pts[7][0], screen_pts[7][1], screen_pts[4][0], screen_pts[4][1]);
    // Krawędzie łączące przód z tyłem
    draw_line(screen_pts[0][0], screen_pts[0][1], screen_pts[4][0], screen_pts[4][1]);
    draw_line(screen_pts[1][0], screen_pts[1][1], screen_pts[5][0], screen_pts[5][1]);
    draw_line(screen_pts[2][0], screen_pts[2][1], screen_pts[6][0], screen_pts[6][1]);
    draw_line(screen_pts[3][0], screen_pts[3][1], screen_pts[7][0], screen_pts[7][1]);
}


void oled_set_sqare(uint8_t x, uint8_t y) {
  byte bit_arr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  oled_set_pixels(x, y, 8, bit_arr);
}

void oled_clear_sqare(uint8_t x, uint8_t y) {
  byte bit_arr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  oled_set_pixels(x, y, 8, bit_arr);
}

void oled_draw_number_0(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3); oled_set_sqare(col_end, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_start, 5); oled_set_sqare(col_end, 5);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_1(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2);
  oled_set_sqare(col_mid, 3); oled_set_sqare(col_mid, 4); oled_set_sqare(col_mid, 5);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_2(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_end, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_start, 5);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_3(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_end, 3); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 5); oled_set_sqare(col_end, 6);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_4(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3); oled_set_sqare(col_end, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_start, 5); oled_set_sqare(col_mid, 5); oled_set_sqare(col_end, 5);
  oled_set_sqare(col_end, 6);
}

void oled_draw_number_5(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3); oled_set_sqare(col_start, 4);
  oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 5); oled_set_sqare(col_end, 6);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_6(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_start, 5); oled_set_sqare(col_end, 5);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_7(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_end, 3); oled_set_sqare(col_end, 4); oled_set_sqare(col_end, 5); oled_set_sqare(col_end, 6);
}

void oled_draw_number_8(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3); oled_set_sqare(col_end, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_start, 5); oled_set_sqare(col_end, 5);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_number_9(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_set_sqare(col_start, 2); oled_set_sqare(col_mid, 2); oled_set_sqare(col_end, 2);
  oled_set_sqare(col_start, 3); oled_set_sqare(col_end, 3);
  oled_set_sqare(col_start, 4); oled_set_sqare(col_mid, 4); oled_set_sqare(col_end, 4);
  oled_set_sqare(col_end, 5); oled_set_sqare(col_end, 6);
  oled_set_sqare(col_start, 6); oled_set_sqare(col_mid, 6); oled_set_sqare(col_end, 6);
}

void oled_draw_ack(void) {
  oled_clear_all();
  uint8_t segment_pixels = 1 * 8;
  uint8_t col_start = segment_pixels + (0 * 8);
  uint8_t col_mid_1 = segment_pixels + (1 * 8);
  uint8_t col_mid_2 = segment_pixels + (2 * 8);
  uint8_t col_mid_3 = segment_pixels + (3 * 8);
  uint8_t col_mid_4 = segment_pixels + (4 * 8);
  uint8_t col_end = segment_pixels + (5 * 8);

  oled_set_sqare(col_start, 4);
  oled_set_sqare(col_mid_1, 5);
  oled_set_sqare(col_mid_2, 6);
  oled_set_sqare(col_mid_3, 5);
  oled_set_sqare(col_mid_4, 4);
  oled_set_sqare(col_end, 3);
}

void oled_clear_sqares(uint8_t segment) {
  uint8_t segment_pixels = segment * 8;
  uint8_t padding = 4;
  uint8_t col_start = segment_pixels + (0 * 8) + padding;
  uint8_t col_mid = segment_pixels + (1 * 8) + padding;
  uint8_t col_end = segment_pixels + (2 * 8) + padding;

  oled_clear_sqare(col_start, 2); oled_clear_sqare(col_mid, 2); oled_clear_sqare(col_end, 2);
  oled_clear_sqare(col_start, 3); oled_clear_sqare(col_mid, 3); oled_clear_sqare(col_end, 3);
  oled_clear_sqare(col_start, 4); oled_clear_sqare(col_mid, 4); oled_clear_sqare(col_end, 4);
  oled_clear_sqare(col_start, 5); oled_clear_sqare(col_mid, 5); oled_clear_sqare(col_end, 5);
  oled_clear_sqare(col_start, 6); oled_clear_sqare(col_mid, 6); oled_clear_sqare(col_end, 6);
}

void draw_number(char number, int segment) {
  switch (number) {
    case '0': oled_draw_number_0(segment); break;
    case '1': oled_draw_number_1(segment); break;
    case '2': oled_draw_number_2(segment); break;
    case '3': oled_draw_number_3(segment); break;
    case '4': oled_draw_number_4(segment); break;
    case '5': oled_draw_number_5(segment); break;
    case '6': oled_draw_number_6(segment); break;
    case '7': oled_draw_number_7(segment); break;
    case '8': oled_draw_number_8(segment); break;
    case '9': oled_draw_number_9(segment); break;
    default: break;
  }
}

// --- FUNKCJE EEPROM ---

void eeprom_set_byte(uint16_t address, uint8_t data)
{
  while (EECR & (1 << EEPE));
  EEAR = address;
  EEDR = data;
  uint8_t old_SREG = SREG;
  cli();
  EECR |= (1 << EEMPE);
  EECR |= (1 << EEPE);
  SREG = old_SREG;
}

uint8_t eeprom_read_byte(uint16_t adres)
{
  while (EECR & (1 << EEPE));
  EEAR = adres;
  EECR |= (1 << EERE);
  return EEDR;
}

void set_eeprom_struct(uint16_t address, struct config *data)
{
  uint8_t *ptr = (uint8_t *)data;
  uint8_t size = sizeof(struct config);
  for (int i = 0; i < size; i++) {
    eeprom_set_byte(address + i, ptr[i]);
  }
}

void read_eeprom_struct(uint16_t address, struct config *data)
{
  uint8_t *ptr = (uint8_t *)data;
  uint8_t size = sizeof(struct config);
  for (int i = 0; i < size; i++) {
    ptr[i] = eeprom_read_byte(address + i);
  }
}

// --- FUNKCJE POMOCNICZE ---

void print_uart_binary(uint8_t data)
{
  for (int i = 7; i >= 0; i--) {
    if ((data >> i) & 1) uart_send("1");
    else uart_send("0");
  }
  uart_send("\n\r");
}

void format_uint16_manual(uint16_t data, char *buffer)
{
  buffer[5] = '\0';
  buffer[4] = (data % 10) + '0'; data /= 10;
  buffer[3] = (data % 10) + '0'; data /= 10;
  buffer[2] = (data % 10) + '0'; data /= 10;
  buffer[1] = (data % 10) + '0'; data /= 10;
  buffer[0] = (data % 10) + '0';
}

void merge_strings_safe(const char *a, const char *b, const char *c, char *buf, uint8_t max_len)
{
  uint8_t i = 0;
  if(a) while(*a && i < max_len - 1) buf[i++] = *a++;
  if(b) while(*b && i < max_len - 1) buf[i++] = *b++;
  if(c) while(*c && i < max_len - 1) buf[i++] = *c++;
  buf[i] = '\0';
}

// --- PRZERWANIA I INICJALIZACJA SPRZĘTU ---

void init_PCINT(void)
{
  CLR_BIT(DDRB, PIN4);
  SET_BIT(PORTB, PIN4);
  SET_BIT(GIMSK, PCIE);
  SET_BIT(PCMSK, PIN4);
  sei();
}

void init_timer_20ms(void)
{
  TCCR0A |= (1 << WGM01);
  TCCR0B |= (1 << CS02) | (1 << CS00);
  TCCR0B &= ~(1 << CS01);
  OCR0A = 155;
  TIMSK |= (1 << OCIE0A);
}

ISR(PCINT0_vect)
{
  if (!is_awake && !(PINB & (1 << PB4))) {
    awake_INT = true;
    is_awake = true;
  }
}

ISR(TIM0_COMPA_vect)
{
  if (!c_config.pause) {
    live_time_ms += 20;
    if (live_time_ms >= 1000) {
      live_time_ms -= 1000;
      live_time_s++;
    }
  }
}

uint16_t get_life_time_s()
{
  uint16_t cp_time;
  uint8_t old_SREG = SREG;
  cli();
  cp_time = live_time_s;
  SREG = old_SREG;
  return cp_time;
}

void sleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}
// --- LOGIKA CALLBACKÓW MENU ---

void *start_menu_callback(const int arv)
{
  uart_send("start pomodoro\n\r");
  
  uint16_t total_seconds;
  
  if (c_config.is_break) {
      total_seconds = (c_config.break_time_m * 60) + c_config.break_time_s;
  } else {
      total_seconds = (c_config.work_time_m * 60) + c_config.work_time_s;
  }

  uint16_t last_drawn_seconds = 0xFFFF;
  uint16_t elapsed_s = get_life_time_s(); 
  
  c_config.pause = false;
  oled_clear_all();

  while (true)
  {
    uint16_t current_s = get_life_time_s();
    
    if (current_s != elapsed_s) {
        elapsed_s = current_s;
        
        if (total_seconds > 0) {
            total_seconds--;
        } else {
            c_config.is_break = !c_config.is_break;
            c_config.pause = true; 
            
            oled_draw_ack();
            _delay_ms(2000);
            
            display_menu(0);
            break;
        }
    }

    if (total_seconds != last_drawn_seconds) {
        last_drawn_seconds = total_seconds;
        
        uint16_t m = total_seconds / 60;
        uint16_t s = total_seconds % 60;
        char buf[6];
        
        format_uint16_manual(m, buf);
        oled_clear_sqares(0);
        oled_clear_sqares(4);
        draw_number(buf[3], 0);
        draw_number(buf[4], 4);

        byte bit_arr[] = {0xff, 0xff, 0xff, 0xff};
        oled_set_pixels(62, 3, 4, bit_arr);
        oled_set_pixels(62, 5, 4, bit_arr);

        format_uint16_manual(s, buf);
        oled_clear_sqares(8);
        oled_clear_sqares(12);
        draw_number(buf[3], 8);
        draw_number(buf[4], 12);
    }

    BUTTONS button = buttons_management();

    if (button == RIGHT) {
      c_config.pause = !c_config.pause;
    }
    else if (button == LEFT) {
      c_config.pause = true;
      display_menu(0);
      break;
    }
  }
}

void *stop_menu_callback(const int arv)
{
  c_config.pause = true;
  oled_draw_ack();
  _delay_ms(2000);
  display_menu(0);
}

void *reset_menu_callback(const int arv)
{
  c_config.pause = true;
  c_config.is_break = false; 
  oled_draw_ack();
  _delay_ms(2000);
  display_menu(0);
}

void *sleep_menu_callback(const int arv)
{
  menu_item_t menu_item = menu_list[arv];
  oled_clear_all();
  
  char buf[32]; 
  merge_strings_safe("<", menu_item.menu_name, ">", buf, sizeof(buf));
  oled_write_text(15, 4, buf, OLED_WHITE);

  sleep(); 
}

void *set_work_menu_callback(const int arv)
{
  uint8_t current_val = c_config.work_time_m;
  uint8_t last_val = 0xFF;
  oled_clear_all();

  while (true)
  {
    if (last_val != current_val) {
      last_val = current_val;
      char tmp_buffer[6];
      format_uint16_manual((uint16_t)current_val, tmp_buffer);
      
      oled_clear_sqares(4);
      oled_clear_sqares(8);
      draw_number(tmp_buffer[3], 4);
      draw_number(tmp_buffer[4], 8);
    }

    BUTTONS button = buttons_management();

    if (button == RIGHT) {
      current_val = (current_val + 1) % 60;
    }
    else if (button == LEFT) {
      c_config.work_time_m = current_val;
      set_eeprom_struct(EEPROM_CONFIG_ADDR, &c_config); 
      oled_draw_ack();
      _delay_ms(2000);
      display_menu(0);
      break;
    }
  }
}

void *set_break_menu_callback(const int arv)
{
  uint8_t current_val = c_config.break_time_m;
  uint8_t last_val = 0xFF;
  oled_clear_all();

  while (true)
  {
    if (last_val != current_val) {
      last_val = current_val;
      char tmp_buffer[6];
      format_uint16_manual((uint16_t)current_val, tmp_buffer);
      
      oled_clear_sqares(4);
      oled_clear_sqares(8);
      draw_number(tmp_buffer[3], 4);
      draw_number(tmp_buffer[4], 8);
    }

    BUTTONS button = buttons_management();

    if (button == RIGHT) {
      current_val = (current_val + 1) % 60;
    }
    else if (button == LEFT) {
      c_config.break_time_m = current_val;
      set_eeprom_struct(EEPROM_CONFIG_ADDR, &c_config); 
      oled_draw_ack();
      _delay_ms(2000);
      display_menu(0);
      break;
    }
  }
}

void *show_config_menu_callback(const int arv)
{
  oled_clear_all();
  char buffer[6];
  char buf[32]; 

  format_uint16_manual(c_config.work_time_m, buffer);
  merge_strings_safe("WORK: ", buffer, " m", buf, sizeof(buf));
  oled_write_text(5, 0, buf, OLED_WHITE);

  format_uint16_manual(c_config.break_time_m, buffer);
  merge_strings_safe("BREAK: ", buffer, " m", buf, sizeof(buf));
  oled_write_text(5, 1, buf, OLED_WHITE);

  merge_strings_safe("IS BRK: ", c_config.is_break ? "YES" : "NO", "", buf, sizeof(buf));
  oled_write_text(5, 2, buf, OLED_WHITE);

  while (true) {
    BUTTONS button = buttons_management();
    if (button == LEFT || button == RIGHT) {
      oled_draw_ack();
      _delay_ms(1000);
      display_menu(0);
      break;
    }
  }
}
void *show_reg_menu_callback(const int arv)
{
  oled_clear_all();
  char buffer[6];
  char buf[32];
  
  while (true) {
  volatile uint8_t *ptr =  ((volatile uint8_t *)(0x0000));

  for(uint8_t addr = 0x0000; addr <= 0x005F; addr++){
    byte arr[1] = {addr};
    byte arr_[1] = {(*(ptr+addr))};
    oled_set_pixels(addr, 3, 1, arr);
    oled_set_pixels(addr, 2, 1, arr_);
  }

  for(uint8_t addr = 0x0000; addr <= 0x005F; addr++){
    byte arr[1] = {addr};
    byte arr_[1] = {(*(ptr+addr))};
    oled_set_pixels(addr, 6, 1, arr);
    oled_set_pixels(addr, 5, 1, arr_);
  }

  *(ptr+(0x02)) += 1;

    BUTTONS button = buttons_management();
    if (button == LEFT || button == RIGHT) {
      oled_draw_ack();
      _delay_ms(1000);
      display_menu(0);
      break;
    }
    _delay_ms(150);
  }
}

void *show_cube_menu_callback(const int arv)
{
    oled_clear_all(); 

    for (uint8_t x = 0; x < 16; x++) {
        for (uint8_t page = 0; page < 8; page++) {
            tilemap[x][page] = 0x00; // Indeks 0 (Puste tło) wszędzie
        }
    }

    // 2. Narysuj ramkę areny (Indeks 1 - Ściana)
    for (uint8_t x = 0; x < 16; x++) {
        set_tile(x, 0, 1);  // Sufit
        set_tile(x, 15, 1); // Podłoga (Y się nie zmienia)
    }
    for (uint8_t y = 0; y < 16; y++) {
        set_tile(0, y, 1);  // Lewa ściana
        set_tile(15, y, 1); // Prawa ściana (Skurczyliśmy z 31 na 15)
    }

    // 3. Dodaj lewitującą platformę na środku (Zwężona i przesunięta)
    for (uint8_t x = 4; x <= 11; x++) {
        set_tile(x, 11, 1); 
    }
    
    // 4. Dodaj dwie przeszkody (słupki) po bokach platformy
    set_tile(4, 10, 1); 
    set_tile(11, 10, 1); 

    // 5. Postaw "Bohatera" (Indeks 2 - Kółko) na środku nowej platformy
    set_tile(7, 10, 2);
    
    render_tilemap();
    while (true)
    {
        // Czyszczenie jest wymagane, inaczej nowy sześcian narysuje się NA starym
        
        // Rysujemy na ekranie. Z każdym obiegiem pętli angle rośnie.
        // angle/2 dla X sprawia, że sześcian "kiwa się" w górę/dół wolniej niż obraca się wokół Z.
        // draw_wireframe_cube(angle / 2, angle);
        
        // angle++; // Zwiększ kąt na następną klatkę
        

        _delay_ms(10); // Maleńkie opóźnienie, aby uniknąć totalnego rozerwania I2C

        // Wyjście z menu
        BUTTONS button = buttons_management();
        if (button == LEFT || button == RIGHT)
        {
            oled_draw_ack();
            _delay_ms(1000);
            display_menu(0);
            break;
        }
    }
}
void *bt_menu_callback(const int arv)
{
    uint8_t angle = 0;

    while (true)
    {
        
        BUTTONS button = buttons_management();
        if (button == LEFT || button == RIGHT)
        {
            oled_draw_ack();
            _delay_ms(1000);
            display_menu(0);
            break;
        }
    }
}

static menu_item_t menu_list[] = {
    {0, "START", 5, start_menu_callback},
    {1, "STOP", 4, stop_menu_callback},
    {2, "RESET", 5, reset_menu_callback},
    {3, "SLEEP", 5, sleep_menu_callback},
    {4, "SET WORK", 8, set_work_menu_callback},
    {5, "SET BREAK", 9, set_break_menu_callback},
    {6, "SHOW", 4, show_config_menu_callback},
    {7, "SHOW REG", 8, show_reg_menu_callback},
    {8, "SHOW CUB", 8, show_cube_menu_callback},
    {9, "BT", 2, bt_menu_callback}
};

void display_menu(uint8_t index_menu)
{
  menu_item_t menu_item = menu_list[index_menu];
  menu_item_t post_menu_item = menu_list[(index_menu + 1) % MENU_LIST_SIZE];
  menu_item_t post_post_menu_item = menu_list[(index_menu + 2) % MENU_LIST_SIZE];

  oled_clear_all();
  oled_write_text(15, 3, menu_item.menu_name, OLED_WHITE);
  oled_write_text(15, 4, "-------", OLED_WHITE);
  oled_write_text(15, 5, post_menu_item.menu_name, OLED_WHITE);
  oled_write_text(15, 6, post_post_menu_item.menu_name, OLED_WHITE);
}

void execute_menu_item(int index_menu)
{
  menu_list[index_menu].callback(index_menu);
}

void init_buttons()
{
  CLR_BIT(DDRB, PIN3);
  SET_BIT(PORTB, PIN3);
}

BUTTONS buttons_management(void)
{
  if (button_pressed) {
    if (!(PINB & (1 << PIN3)) || !(PINB & (1 << PIN4))) {
      return NONE;
    } else {
      button_pressed = false;
    }
  }

  if (!button_pressed && !(PINB & (1 << PIN3))) {
    button_pressed = true;
    return RIGHT;
  }

  if (!button_pressed && !(PINB & (1 << PIN4))) {
    button_pressed = true;
    return LEFT;
  }

  return NONE;
}

int main(void)
{
  init_uart();
  init_buttons();
  init_PCINT();
  init_timer_20ms();
  i2c_init();

  oled_init();
  oled_clear_all();

  // --- ŁADOWANIE Z EEPROM ---
  if (eeprom_read_byte(0) == EEPROM_MAGIC_BYTE) {
      read_eeprom_struct(EEPROM_CONFIG_ADDR, &c_config);
  } else {
      eeprom_set_byte(0, EEPROM_MAGIC_BYTE);
      set_eeprom_struct(EEPROM_CONFIG_ADDR, &c_config);
  }

  uint8_t menu_index = 0;
  display_menu(menu_index);

  while (true)
  {
    if (awake_INT) awake_INT = false;

    if (is_awake) {
      BUTTONS button = buttons_management();

      if (button == RIGHT) {
        execute_menu_item(menu_index);
      }
      else if (button == LEFT) {
        menu_index = (menu_index + 1) % MENU_LIST_SIZE;
        display_menu(menu_index);
      }
    }
    _delay_ms(15);
  }
}
