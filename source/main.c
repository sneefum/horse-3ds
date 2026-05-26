#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>
#include <citro2d.h>

#include "data.h"
#include "font.h"

int drawToBottomFB(u8* fb, int x, int y, int col) {
    if (x < 0 || x >= 320 || y < 0 || y >= 240) {    
        return 1;
    }

    int pos = (x * 240 + (239 - y)) * 3;

    u8 r = col & 0xFF;
    u8 g = (col >> 8) & 0xFF;
    u8 b = (col >> 16) & 0xFF;

    fb[pos]     = b;
    fb[pos + 1] = g;
    fb[pos + 2] = r;

    return 0;
}

int drawToTopFB(u8* fb, int x, int y, int col) {
    if (x < 0 || x >= 400 || y < 0 || y >= 240) {
        return 1;
    }

    int pos = (x * 240 + (239 - y)) * 3;

    u8 r = col & 0xFF;
    u8 g = (col >> 8) & 0xFF;
    u8 b = (col >> 16) & 0xFF;

    fb[pos]     = b;
    fb[pos + 1] = g;
    fb[pos + 2] = r;

    return 0;
}

int vecMag(int x, int y) {
    return sqrt((x*x)+(y*y));
}

int vecMagFloat(int x, int y) {
    return sqrt((x * x) + (y * y));
}

int distance(float x1, float y1, float x2, float y2) {
    return vecMagFloat(x2 - x1, y2 - y1);
}

void drawChar(u8* fb, int x, int y, char c, int scale) {
    for (int i = 0; i < 64; i++) {
        if (((charset[(int)c] >> (63 - i)) & 1) == 1) {
            for (int j = 0; j < scale; j++) {
                for (int k = 0; k < scale; k++) {
                    drawToTopFB(fb, (i % 8) * scale + j + x, (i / 8) * scale + k + y, 16777215);
                }
            } 
        }
    }
}

void drawString(u8* fb, int x, int y, char* s, int scale) {
    int cursorX = x;

    for (int i = 0; i < strlen(s); i++) {
        drawChar(fb, cursorX, y, s[i], scale);
        cursorX += 8 * scale;
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

	gfxInitDefault();

    float posX = 0;
    float posY = 0;

    int dir = 1; // -1 for left, 1 for right
    
    float dx = 0;
    float dy = 0;

    float appleX = rand() % 290;
    float appleY = rand() % 210;

    int score = 0;
    
	while (aptMainLoop()) {	
        gspWaitForVBlank();
		hidScanInput(); 
    
        u8* fbBottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
        u8* fbTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

		u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START)
			break;

        if (kDown & KEY_LEFT)
            dir = -1;
        if (kDown & KEY_RIGHT)
            dir = 1; 
        
        // Directional input

        dx = 0;
        dy = 0;

        circlePosition pos;
        hidCircleRead(&pos);

        dx = pos.dx;
        dy = -pos.dy;

        if (vecMag(dx, dy) < 30) {
            dx = 0;
            dy = 0;
            if (kHeld & KEY_LEFT)
                dx--;
            if (kHeld & KEY_RIGHT)
                dx++;
            if (kHeld & KEY_UP)
                dy--;
            if (kHeld & KEY_DOWN)
                dy++;
            if (dx != 0 && dy != 0) {
                float dpadLen = sqrt(2.0);
                dx /= dpadLen;
                dy /= dpadLen;
            }
            dx *= 2;
            dy *= 2;
        } else {
            // Normalize to length 2
        
            float length = vecMag(dx, dy);
            float maxLength = 154;
            float modifier = fminf(length / maxLength, 1);

            if (length > 0) {
                dx = (dx / length) * modifier * 2;
                dy = (dy / length) * modifier * 2;
            }
        }

        // Finish movement
        
        posX += dx;
        posY += dy;

        if (distance(posX + 35, posY + 50, appleX, appleY) < 60) {
            score++;
            
            while (distance(posX + 35, posY + 50, appleX, appleY) < 100) {
                appleX = rand() % 290;
                appleY = rand() % 210;
            }
        }

        if (posX < 0)
            posX = 0;
        if (posX > 320 - 100)
            posX = 320 - 100;
        if (posY < 0)
            posY = 0;
        if (posY > 240 - 100)
            posY = 240 - 100;

        // Graphics

        memset(fbBottom, 0, 240 * 320 * 3);
        memset(fbTop, 0, 240 * 400 * 3);

        for (int y = 0; y < 207; y++) {
            for (int x = 0; x < 264; x++) {
                int pos = 3 * ((264 * y) + x);
                int col = (title[pos + 2] << 16)
                        | (title[pos + 1] << 8)
                        | (title[pos]);
                drawToTopFB(fbTop, x + 68, y + 32, col);
            }
        }
        
        char scoreBuf[64];
        sprintf(scoreBuf, "Score: %d", score);
        drawString(fbTop, 0, 0, scoreBuf, 3);

        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                int pos = 3 * ((100 * y) + (dir == 1 ? x : 99 - x));
                int col = (horse[pos + 2] << 16)
                        | (horse[pos + 1] << 8)
                        | (horse[pos]);
                drawToBottomFB(fbBottom, x + (int)posX, y + (int)posY, col);
            }
        }

        for (int y = 0; y < 30; y++) {
            for (int x = 0; x < 30; x++) {
                int pos = 3 * ((30 * y) + x);
                int col = (apple[pos + 2] << 16)
                        | (apple[pos + 1] << 8)
                        | (apple[pos]);
                if (col != 0)
                    drawToBottomFB(fbBottom, x + appleX, y + appleY, col);
            }
        }
        
        gfxFlushBuffers();
		gfxSwapBuffers();
	}

	gfxExit();
	return 0;
}
