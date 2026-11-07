/* SPDX-License-Identifier: GPL-3.0-only */
/* LILA's start-up screen: the name in big bouncing letters, and ten dots that light up while it starts.
 * Drawn with the UI's canvases, so the same code runs on the FM-1 and in the emulator. */
#pragma once

static void splash_draw(int lit)
{
    static const uint32_t LC[4] = {0xFF5060, 0xFFC830, 0x50E070, 0x50B0FF};
    static const char *const LT[4] = {"L", "I", "L", "A"};
    int band, i;
    uint32_t t = plat_ms();
    look.bri = 256; look.sat = 256; look.warm = 0; look.hue = 0; look.fade = 0; look.night = 0; look.flash = 0;
    for (band = 0; band < 2; band++) {
        cv_px = cv_buf[band];
        cv_begin(240, 120, C_BLACK);
        cv_oy = -120 * band;
        vgrad(0, 240, RGB(30, 20, 90), RGB(120, 40, 140));
        for (i = 0; i < 40; i++) {
            int x = (i * 61) % 240, y = (i * 37) % 240, tw = isin((int)(t / 9) + i * 23);
            if (tw > 20)
                sparkle(x, y, 1 + (tw > 100), RGB(255, 245, 190));
        }
        for (i = 0; i < 4; i++) {
            int x = 24 + i * 48, y = 46 + (isin((int)(t / 5) + i * 48) * 8 >> 7);
            text_big(x + 24, y + 6, &FONT_M, LT[i], RGB(40, 10, 60), 4);
            text_big(x + 24, y, &FONT_M, LT[i], rgb24(LC[i]), 4);
        }
        text_c(120, 160, &FONT_B, "MUSIC LAND", RGB(255, 255, 255));
        for (i = 0; i < 10; i++) {
            uint16_t c = i < lit ? rgb24(LC[i & 3]) : RGB(70, 60, 120);
            fill_circle(30 + i * 20, 196, i < lit ? 7 : 4, c);
        }
        cv_blit(0, (uint32_t)(band * 120));
    }
    lcd_sync();
}
static void splash_static(void) { splash_draw(0); }
static void splash_squares(int lit) { splash_draw(lit); }
