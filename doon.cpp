#include "Pokitto.h"
#include <vector>
#include <stdint.h>

#include "tiles.h"
#include "doon_sprites.h"
#include "backgrounds.h"
#include "creatures.h"
#include "collectable_sprites.h"

#define TILE_SIZE 8
#define MAP_WIDTH 128
#define MAP_HEIGHT 28

#define FLOAT_MOVEMENT

#ifdef POK_SIM
    #define TARGET_FRAME_RATE 30
#else
    #define TARGET_FRAME_RATE 40
#endif // POK_SIM

Pokitto::Core game;
Pokitto::Display disp;
Pokitto::Buttons btn;
Pokitto::Sound snd;

using namespace std;

float lerp(float a, float b, float t) {
    return (1-t)*a + t*b;
}

float clip(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}
int clip(int n, int lower, int upper) {
  return std::max(lower, std::min(n, upper));
}

float dist(float x1, float y1, float x2, float y2) {
    float x = x1 - x2;
    float y = y1 - y2;
    float dist;
    dist = pow(x,2)+pow(y,2);
    dist = sqrt(dist);
    return dist;
}
float angle(float x1, float y1, float x2, float y2) {
    return atan2(y1 - y2, x1 - x2);
}

#define ID_PLAYER 0
#define ID_FLY 1
#define ID_MARCHER 2
#define ID_BEE 3
#define ID_STALKER 4
#define ID_PEACH 5

#define ID_PLUM 11
#define ID_PLATFORM 15
#define ID_LAVA 16
#define ID_KILL 25

#define STATE_MENU 0
#define STATE_SELECT 1
#define STATE_GAME 2
int state;

namespace Map {
    int bg_id = 0;
    int bgcolor = 0;
    unsigned char data[MAP_WIDTH*MAP_HEIGHT] {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,5,0,0,11,0,0,0,0,0,0,0,11,11,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
5,0,6,0,0,0,4,0,0,0,0,0,0,0,5,6,5,8,6,5,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
3,4,3,4,0,0,15,40,0,0,0,0,0,3,4,4,4,3,4,4,4,0,11,11,0,0,0,0,7,7,7,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,1,1,0,0,0,0,0,0,0,0,0,0,0,1,2,1,1,2,1,0,0,11,11,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,11,0,0,0,0,0,11,11,11,0,0,0,0,0,11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,10,10,10,10,15,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,11,0,0,0,0,0,11,11,11,0,0,0,0,0,11,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,10,10,10,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,0,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,0,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,11,0,0,0,11,0,0,0,0,0,0,0,0,7,15,40,0,0,0,0,7,7,15,40,0,0,0,0,7,7,15,40,0,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,5,0,6,0,0,0,0,0,0,0,0,11,7,0,0,0,14,0,0,0,7,11,0,0,0,0,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,4,0,0,0,0,0,0,0,9,0,0,0,0,0,0,11,11,0,0,0,0,0,0,0,5,0,11,0,6,0,0,0,0,0,0,0,9,0,0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,3,4,4,4,0,0,0,0,0,0,0,11,7,7,0,0,0,0,0,0,0,7,7,11,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,2,4,6,0,5,0,6,5,8,13,0,0,0,5,0,0,0,0,5,0,6,5,0,6,4,1,11,2,4,0,6,13,5,0,6,0,8,6,0,0,0,5,5,0,6,0,10,10,0,0,0,6,4,1,2,2,2,4,6,5,0,0,0,11,7,7,7,0,0,0,0,0,0,0,7,7,7,11,0,0,7,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,1,2,4,3,4,3,4,3,4,3,4,3,4,4,4,3,4,3,4,3,4,3,4,3,2,1,11,2,1,3,4,3,4,3,4,3,4,3,4,3,4,4,4,3,4,3,3,3,4,3,4,3,2,1,2,2,1,1,4,3,4,3,4,3,4,3,4,0,0,0,0,0,0,0,4,3,4,3,4,4,3,4,3,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,3,4,4,4,
2,2,2,2,1,2,2,2,2,2,1,2,1,2,2,2,2,2,1,2,2,2,2,2,2,2,1,11,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,25,25,25,25,25,25,25,2,1,2,1,2,2,1,2,1,2,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,25,1,1,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,2,
35,32,33,32,32,32,35,35,35,32,32,33,33,35,32,32,35,32,32,32,32,32,32,32,33,32,0,0,0,35,32,32,33,35,35,35,35,35,35,35,35,33,33,32,33,33,33,33,33,33,33,32,35,35,35,35,32,33,32,32,32,32,33,32,33,33,33,32,32,32,32,32,33,32,33,35,33,32,33,32,32,32,32,32,32,32,32,32,32,33,32,33,33,33,33,33,33,32,32,32,33,32,33,32,33,33,35,35,33,35,35,35,33,33,33,32,33,35,33,33,33,32,35,32,33,35,33,33,
35,32,32,32,33,32,32,32,35,33,33,33,32,35,33,35,33,33,32,35,35,33,35,32,32,32,0,0,0,35,35,35,0,0,0,0,0,0,0,0,0,0,0,32,33,35,35,35,35,33,32,35,35,32,32,35,35,32,33,33,33,33,32,35,32,32,33,33,33,32,33,33,32,33,35,35,35,33,0,0,0,0,0,0,32,33,33,33,33,0,0,0,0,32,32,33,33,32,32,32,32,33,33,32,33,33,33,33,33,33,33,32,35,32,35,33,0,0,0,0,0,0,33,32,33,32,33,33,
35,32,32,35,32,32,35,32,35,32,33,32,33,32,32,35,32,35,32,32,32,35,35,33,32,33,0,0,0,32,35,0,0,0,0,0,0,0,0,0,0,0,0,0,33,35,33,33,35,0,35,35,32,32,33,32,11,33,32,32,33,0,0,35,32,0,33,0,32,0,0,32,32,33,35,35,35,0,0,0,0,0,0,0,0,35,33,33,0,0,0,0,0,33,32,0,33,33,33,33,33,33,32,0,33,35,32,33,33,35,33,33,33,32,33,0,0,0,0,0,0,0,0,32,0,33,33,35,
33,33,35,33,32,35,32,32,33,32,35,32,32,35,35,33,33,33,33,33,35,33,32,32,33,32,0,0,0,33,32,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,32,32,0,0,32,32,0,32,33,11,11,11,33,32,0,0,0,32,0,0,0,0,0,0,0,0,33,35,35,33,33,0,0,0,0,0,0,0,0,35,33,33,0,11,11,0,0,32,0,0,0,32,0,32,32,0,0,0,0,32,33,35,33,33,32,35,35,32,35,0,0,0,11,11,11,0,0,0,0,0,33,32,
32,32,32,35,35,32,32,32,32,32,35,32,33,32,35,35,35,32,32,35,32,33,32,35,32,32,0,0,0,33,33,0,0,0,33,33,33,35,33,35,11,0,0,0,32,33,33,0,0,0,0,33,0,0,33,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,35,35,35,0,0,0,32,33,0,0,0,35,33,32,0,11,11,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,35,33,35,33,35,0,35,35,0,0,0,0,11,11,11,0,0,0,0,0,35,33,
35,32,32,32,32,33,33,32,33,32,33,32,32,32,32,35,32,35,32,33,35,35,32,35,35,32,0,0,0,32,32,11,0,0,33,35,35,35,35,35,11,0,0,0,0,33,0,0,0,0,0,0,0,0,32,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,33,33,11,0,0,35,33,0,0,0,35,33,33,0,0,0,0,0,0,0,0,0,0,0,0,33,0,0,0,0,0,0,35,35,32,0,0,32,35,0,0,0,0,33,33,33,0,0,0,0,33,33,33,
32,32,35,33,33,35,32,35,33,32,32,33,32,33,35,33,32,33,33,33,33,33,32,35,32,32,0,0,0,32,32,11,0,0,32,33,33,35,35,33,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,32,11,0,0,33,33,0,0,0,33,35,35,35,35,35,0,0,0,0,35,35,0,0,0,0,0,0,0,0,0,0,0,33,0,0,0,33,35,0,0,0,0,0,0,0,0,0,0,0,32,33,33,
32,32,32,33,32,35,32,32,33,33,32,32,32,35,32,32,33,32,32,32,33,33,35,32,33,32,0,0,0,33,32,11,0,0,33,35,33,35,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,11,11,11,0,0,0,0,32,32,0,0,0,35,32,0,0,0,33,35,33,35,35,35,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,0,0,0,32,35,0,0,12,0,0,0,0,0,0,0,0,35,33,35,
33,32,33,35,33,32,35,32,32,32,33,32,35,35,32,32,35,32,35,32,35,33,32,32,32,35,0,0,0,33,33,0,0,11,35,35,35,11,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,32,32,0,0,0,0,33,32,0,0,11,35,35,0,0,0,33,35,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,0,0,0,0,0,0,0,0,0,0,0,33,33,33,
32,32,33,35,35,32,32,32,35,33,32,32,32,33,32,32,32,32,32,32,32,35,35,32,33,33,0,0,0,0,32,0,0,11,35,35,35,11,11,0,0,0,0,0,0,11,11,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,32,0,0,11,33,32,0,0,0,33,33,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,35,35,32,
32,35,35,32,35,32,35,32,32,32,32,32,32,35,35,32,32,35,32,35,32,33,32,32,35,35,0,0,0,0,0,0,0,11,33,33,33,33,32,33,16,16,16,16,16,32,35,35,35,35,35,32,15,40,0,0,0,0,0,0,0,33,35,35,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,32,32,33,32,0,0,0,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,35,32,35,
33,32,32,32,35,32,32,33,32,33,35,32,32,35,35,32,32,32,35,32,35,33,35,33,32,32,0,0,0,0,0,0,0,0,32,35,33,33,33,32,24,24,24,24,24,33,32,32,35,32,35,35,0,0,0,0,11,0,0,0,0,33,35,35,33,35,35,0,0,0,0,0,0,13,0,0,0,0,0,0,35,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,33,32,35,0,0,0,0,0,32,33,32,33,0,0,0,0,0,0,0,0,0,0,0,0,32,32,35,35,35,
32,32,32,33,32,33,32,35,32,35,35,33,32,35,32,32,32,33,32,32,33,35,32,35,33,32,32,33,32,32,35,35,35,32,32,35,33,33,32,33,24,24,24,24,24,33,32,35,32,32,32,32,0,0,0,0,11,0,0,0,0,35,33,35,35,35,33,32,32,32,33,33,35,35,35,35,33,32,32,35,32,32,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,33,0,0,0,0,0,0,32,32,32,33,32,32,33,32,35,32,35,35,32,35,32,32,32,32,32,32,32,
32,33,33,32,32,32,32,32,32,32,33,32,33,32,33,32,33,32,33,33,32,33,35,32,32,33,32,33,35,32,32,32,32,32,35,33,33,32,33,33,24,24,24,24,24,33,32,35,35,35,35,35,16,16,16,16,32,16,16,16,16,35,35,35,35,33,33,33,32,33,32,33,33,33,35,33,32,33,33,32,35,32,16,16,16,33,32,33,16,16,16,16,16,32,32,16,16,16,16,16,16,16,16,16,16,16,16,32,32,33,32,32,33,33,33,33,33,32,32,33,35,33,33,33,35,33,35,35,
};
    bool loadData(char * filename) {
        bool success = pokInitSD();
        fileOpen(filename, FILE_MODE_BINARY);
        //0x00 WIDTH (NOT STORED YET)
        int width = fileGetChar();
        //0x01 HEIGHT
        int height = fileGetChar();
        //0x02 BG_ID
        bg_id = fileGetChar();
        //0x03 BG_COLOR
        bgcolor = fileGetChar();
        //0x04 DATA
        fileSetPosition(4);
        fileReadBytes(data, MAP_WIDTH*MAP_HEIGHT);
        fileClose();
    }
    long start_time = game.getTime()/1000;
    int SPAWN_IDS[255];
    void initilizeSpawnIDS() {
        SPAWN_IDS[12] = ID_FLY;
        SPAWN_IDS[13] = ID_MARCHER;
        SPAWN_IDS[14] = ID_BEE;
        SPAWN_IDS[38] = ID_STALKER;
    }

    bool show_parallax =true;

    unsigned char invisiblecolor = 0;

    uint8_t SOLID_TILES[255] = {
        0,1,1,1,1,0,0,1,0,0,1,0
    };
    void setSolids() {
        pokInitSD();
        fileOpen("Doon/tiledata.dat", FILE_MODE_BINARY);
        for (int i = 0; i<255; i++) {
            SOLID_TILES[i] = fileGetChar();
        }
        fileClose();
    }

    bool TOP_SOLID_TILES[255];
    bool INV_TILES[255];
    void setInvisableTiles() {
        int tiles[] = {0, 39, 40, 41, 42};
        for (int i = 0; i<sizeof(tiles)/4; i++) {
            INV_TILES[tiles[i]]=1;
        }
    }
    int getID(int x, int y) {
        int addr = (y * MAP_WIDTH) + x;
        return data[addr];
    }
    void setID(int x, int y, int val) {
        int addr = (y * MAP_WIDTH) + x;
        data[addr] = val;
    }
    bool isCollectable(int id) {
        return id==11;
    }

    bool checkSolid(int x, int y) {
        return SOLID_TILES[getID(x/TILE_SIZE, y/TILE_SIZE)];
    }

    int cam_x, cam_y, shake_x, shake_y;
    uint32_t shake_time;
    void shake() {
        if (game.getTime()-shake_time>500) {
        shake_time = game.getTime();
        }
    }
    void update(int target_x, int target_y) {
        if (game.getTime()-shake_time<500) {
            shake_x = random(-2, 2);
            shake_y = random(-2, 2);
        }
        else {
            shake_x = 0;
            shake_y = 0;
        }
        float cam_y_max, cam_y_min;
        cam_y_max = target_y < (MAP_HEIGHT/2)*TILE_SIZE? .0 : -MAP_HEIGHT/2*TILE_SIZE;
        cam_y_min = target_y < (MAP_HEIGHT/2)*TILE_SIZE? MAP_HEIGHT/2 : MAP_HEIGHT;
        show_parallax = target_y < (MAP_HEIGHT/2) * TILE_SIZE;
        cam_x = clip(lerp(cam_x, 55 - target_x - 5, .2), (float)-((MAP_WIDTH*TILE_SIZE)-110), (float).0);
        cam_y = clip(lerp(cam_y, 44 - target_y - 5, .2), -(((cam_y_min)*TILE_SIZE)-88), cam_y_max);
        //cam_x = clip(55 - target_x, -((MAP_WIDTH*TILE_SIZE)-110), 0);
        //cam_y = clip(44 - target_y, -((MAP_HEIGHT*TILE_SIZE)-88), 0);
    }
    void drawFast() {
        disp.invisiblecolor = invisiblecolor;
        for (unsigned char tx = 0; tx<15; tx++) {
            for (unsigned char ty = 0; ty<12; ty++) {
                bool flip = false;
                unsigned char id = getID(tx-(cam_x/TILE_SIZE), ty-(cam_y/TILE_SIZE));
                if (id==ID_KILL) id = 0;
                if (id==ID_LAVA||id==ID_LAVA + 8) {
                    id = id + ((game.frameCount/2) &7);
                    flip = (tx - (cam_x/TILE_SIZE))%2==0;
                }
                if (isCollectable(id)) {
                    int spr = (game.frameCount/2)%6;
                    disp.drawBitmap(tx*TILE_SIZE + ((int)cam_x%(TILE_SIZE)) + shake_x, ty*TILE_SIZE + ((int)cam_y%TILE_SIZE) + shake_y, collectables[spr]);
                }
                else if (!INV_TILES[id]) {
                    disp.drawBitmap(tx*TILE_SIZE + ((int)cam_x%(TILE_SIZE)) + shake_x, ty*TILE_SIZE + ((int)cam_y%TILE_SIZE) + shake_y, sprites[id], 0, flip);
                }
            }
        }
    }
}

struct Platform {
    float x, y, vx, vy;
    int w, h;
    int index;
    Platform() {
        x = 0;
        y = 0;
        vx = 0;
        vy = 0;
        w = 0;
        h = 0;
        vx = 0;
        vy = 0;
        index = 0;
    }
    Platform(int _x, int _y) {
        x = _x;
        y = _y;
        vx = 1;
        vy = 0;
        w = 16;
        h = 8;
    };
    Platform(int _x, int _y, int _vx, int _vy, int _w) {
        x = _x;
        y = _y;
        h = 8;
        vx = _vx;
        vy = _vy;
        w = _w;
    };
    void update(int _index) {
        using namespace Map;
        index = _index;
        if (checkSolid(x + vx, y)||checkSolid(x + w + vx, y)) {
            vx = -vx;
        }

        unsigned char tile = getID(x/8, y/8);
        if (tile==39) {
            vx = 0;
            vy = -1;
        }
        else if (tile==41) {
            vx = 0;
            vy = 1;
        }
        else if (tile==40) {
            vx = 1;
            vy = 0;
        }
        else if (tile==42) {
            vx = -1;
            vy = 0;
        }
        x += vx;
        y += vy;
    }
    void draw() {
        using namespace Map;
        for (int tx = 0; tx<w; tx+=8) {
            for (int ty = 0; ty<h; ty+=8) {
                disp.drawBitmap(x + tx + Map::cam_x + shake_x, y + ty + Map::cam_y + shake_y, sprites[ID_PLATFORM]);
            }
        }
    }
};
vector<Platform> plats;
struct Actor {
    char id;
    float x, y, vx, vy;
    uint8_t w, h;
    int spr;
    bool flip;
    bool jumping;
    bool alive;
    bool onPlatform;
    int death_time;
    Platform parent_platform;
    Actor(int _x, int _y, char _id) {
        id = _id;
        x = _x;
        y = _y;
        vx = 0;
        vy = 0;
        w = 7;
        h = 7;
        switch(id){
            case ID_PLAYER:
                w = 6;
                h = 11;
            break;
            case ID_MARCHER:
                vx = 1;
            break;
            case ID_BEE:
                vx = 1;
            break;
            case ID_STALKER:
                vx = 1;
            break;
            case ID_PEACH:
                w = 4;
                h = 4;
                vx = 4;
                vy = -1;
            break;
        }
        spr = 0;
        flip = false;
        jumping = false;
        onPlatform = false;
        alive = true;
    }
    bool isEnemy() {
        return id!=ID_PLAYER;
    }
    bool isBody() {
        return true;
    }
    void control(vector<Actor> &_ents) {
        bool walking = false;
        bool run = btn.bBtn();
        if (btn.leftBtn()) {
            #ifdef FLOAT_MOVEMENT
                vx = std::max(vx-.2, run? -2.0 : -1.0);
            #else
                vx = -1;
            #endif // FLOAT_MOVEMENT
            walking = true;
            flip = true;
        }
        else if (btn.rightBtn()) {
            #ifdef FLOAT_MOVEMENT
                vx = std::min(vx+.2, run? 2.0 : 1.0);
            #else
                vx = 1;
            #endif // FLOAT_MOVEMENT
            walking = true;
            flip = false;
        }
        else {
            #ifdef FLOAT_MOVEMENT
                vx = lerp(vx, 0, .2);
            #else
                vx = 0;
            #endif // FLOAT_MOVEMENT
            spr = (game.frameCount/2)%5;
        }
        if (walking) {
            spr = 5 + ((game.frameCount/2)%8);
        }
        if (walking&&btn.bBtn()) {
            spr = 13 + ((game.frameCount/2)%8);
        }

        if (!walking&&btn.downBtn()) {
            spr = 24;
        }

        if (!onPlatform&&!(Map::checkSolid(x, (y + h+1) + vy)||(Map::checkSolid(x + w, (y+h+1)+vy)))) {
            spr = vy>0 ? 21 : 22;
        }

        int wall_hug = 0;

        if ((btn.leftBtn()&&Map::checkSolid(x-1, y)) || (btn.rightBtn()&&Map::checkSolid(x + w + 1, y))) {
            if (!Map::checkSolid(x + (w/2), y + h + 1 + vy)) {
                spr = 23;
                if (btn.leftBtn()) wall_hug = 1;
                else wall_hug = -1;
                vy/=1.5;
            }
        }
        bool canJump = false;
        if (btn.pressed(BTN_A)) {
            if (Map::checkSolid(x, y + h + 1)||(Map::checkSolid(x + w, y + h + 1))||wall_hug!=0||onPlatform) {
                canJump = true;
            }
        }

        if ((canJump)||(jumping&&btn.aBtn()&&btn.timeHeld(BTN_A)<7)) {
            vx += wall_hug*2;
            vy = -3;
            jumping = true;
        }
        else {
            jumping = false;
        }
        if (btn.pressed(BTN_UP)) {
            _ents.push_back(Actor(x, y-12, ID_PEACH));
            _ents[_ents.size()].vx=16;
            _ents[_ents.size()].vy=-2;
        }
    };
    void die() {
        alive = false;
        death_time = game.frameCount;
    };
    bool updateParentPlatform() {
        onPlatform = false;
        for (int i = 0; i<plats.size(); i++) {
            Platform * plat = &plats[i];
            if (game.collidePointRect(x + (w/2), y+vy + h + 1, plat->x, plat->y, plat->w, plat->h)) {
                if (!game.collidePointRect(x + (w/2), y + vy + h, plat->x, plat->y, plat->w, plat->h)) {
                    parent_platform = *plat;
                    onPlatform = true;
                }
            }
        }
        return onPlatform;
    };
    void updateRelativePlatformPosition() {
        if (onPlatform) {
            int inx = parent_platform.index;
            float offset_x = plats[inx].x - parent_platform.x;
            float offset_y = plats[inx].y - parent_platform.y;
            x = x + offset_x;
            y = y + offset_y;
        }
    }
    void update(vector<Actor> &_ents, int index) {

        using namespace Map;

        if (!alive) {
            int base_spr;
            switch (id) {
            case ID_FLY:
                base_spr = 8;
                spr = base_spr + ( (game.frameCount - death_time) / 2 ) % 6;
                break;
            case ID_MARCHER:
                base_spr = 22;
                spr = base_spr + ( (game.frameCount - death_time) / 2 ) % 8;
                break;
            case ID_BEE:
                base_spr = 36;
                spr = base_spr + ( (game.frameCount - death_time) / 2 ) % 7;
            case ID_STALKER:
                base_spr = 42;

            }
            if (base_spr-spr==0&&game.frameCount - death_time>1) {
                _ents.erase(_ents.begin()+index);
            }

            return;
        }

        updateRelativePlatformPosition();

        if (checkSolid(x + vx, y)||checkSolid(x + vx + w, y)||checkSolid(x + vx, y + h)||checkSolid(x + vx + w, y + h)) {
            if (isEnemy()) {
                vx=-vx;
                flip = vx<0&&1;
            }
            else vx = 0;
        }

        if (id==ID_STALKER) {
            if (_ents[0].x<x) {
                vx = max(vx-.1, -2.0);
            }
            else {
                vx = min(vx+.1, 2.0);
            }
        }

        if (w>7||h>7) {
            if (checkSolid(x + vx, y + (h/2))||checkSolid(x + vx + w, y + (h/2))) {
                vx = 0;
            }
        }

        for (int i = 0; i<plats.size(); i++) {
            Platform * plat = &plats[i];
            if (game.collideRectRect(x+vx, y, w, h, plat->x + plat->vx, plat->y, plat->w, plat->h)) {
                vx = plat->vx;
            }
        }

        x += vx;

        if (id!=ID_FLY&&id!=ID_BEE&&!jumping) {
            vy=vy+.5;
        }


        if (id==ID_FLY) {
            vy = sin( (double) (game.frameCount/4))*2;
        }
        else if (id==ID_BEE) {
            vy = cos( (double) (game.frameCount/6));
        }

        if (checkSolid(x, y + h + vy)||checkSolid(x + w, y + h + vy)) {
            vy = 0;
        }
        else if (checkSolid(x, y + vy)||checkSolid(x + w, y + vy)) {
            vy = 0;
        }
        for (int i = 0; i<plats.size(); i++) {
            Platform * plat = &plats[i];
            if (game.collideRectRect(x, y+vy, w, h+1, plat->x, plat->y+plat->vy, plat->w, plat->h)) {
                vy = plat->vy;
            }
        }


        for (int i = 0; i<_ents.size(); i++) {
            Actor * other = &_ents.at(i);
            if (i!=index) {
                if (other->alive&&index==0&&vy>0&&game.collideRectRect(x, y + vy, w, h, other->x, other->y, other ->w, other->h)) {
                    if (!(checkSolid(x, y-4)||checkSolid(x+w, y-4))) {
                        vy = -4;
                    }
                    other->die();
                }
                else if (other->alive&&index==0&&game.collideRectRect(x + vx, y + vy, w, h, other->x, other->y, other->w, other->h)) {
                    //state = STATE_SELECT;
                    Map::shake();
                    vy += -2;
                    if (other->x>x) {vx-=2.5;}
                    else if (other->x<x) {vx+=2.5;}
                }
            }
        }
        //ANIMATIONS
        if (isEnemy()) {
            if (id==ID_MARCHER) {
                spr = 14 + ((game.frameCount/2)%8);
            }
            else if (id==ID_FLY) {
                spr = (game.frameCount/2)%8;
            }
            else if (id==ID_BEE) {
                spr = 30 + (game.frameCount)%6;
            }
            else if (id == ID_STALKER && vx!=0) {
                spr = 43 + (game.frameCount/2)%8;
            }
        }

        if (index==0) {

            if (getID( (x) / TILE_SIZE, (y) / TILE_SIZE)==ID_PLUM) {
                setID( (x) / TILE_SIZE, (y) / TILE_SIZE, 0);
            }

            if (getID( ( x + (w) ) / TILE_SIZE, (y) / TILE_SIZE)==ID_PLUM) {
                setID( ( x + (w) ) / TILE_SIZE, (y) / TILE_SIZE, 0);
            }

            if (getID( ( x + (w) ) / TILE_SIZE, (y + (h) ) / TILE_SIZE)==ID_PLUM) {
                setID( ( x + (w) ) / TILE_SIZE, (y + (h) ) / TILE_SIZE, 0);
            }

            if (getID( (x) / TILE_SIZE, (y + h) / TILE_SIZE)==ID_PLUM) {
                setID( (x) / TILE_SIZE, (y + h) / TILE_SIZE, 0);
            }
        }

        y += vy;
    };
    void draw() {
        using namespace Map;
        if (isEnemy()) {
            disp.invisiblecolor = 0;
            disp.drawBitmap(x + Map::cam_x + shake_x, y + Map::cam_y + shake_y, creatures[spr], 0, flip);
        }
        else {
            disp.invisiblecolor = 1;
            disp.drawBitmap(x - 2 + Map::cam_x + shake_x, y + Map::cam_y + shake_y, Doon[spr], 0, flip);
        }
        disp.invisiblecolor = Map::invisiblecolor;
    }

};

vector<Actor> ents;

void Transition(char id) {
    int dirs[][2] {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    bool complete = 0;
    disp.persistence = true;
    char last_color = disp.color;
    int frame = 0;
    switch (id) {
        disp.setColor(3);
    }
    while (!complete) {
        switch (id) {
            case 0:
                disp.fillCircle(55, 44, frame*2);
                disp.update();
                complete = frame>55? true : false;
            break;
            case 1:
                    for (int x = 0; x < 110; x++) {
                        for (int y = 0; y < 88; y++) {
                            int dir = random(3);
                            char color = disp.getPixel(x, y);
                            int dx = x + dirs[dir][0]%110;
                            int dy = y + dirs[dir][1]%88;
                            disp.screenbuffer[dy*55+dx/2] = color;
                        }
                    }
                    disp.setColor(2);
                    //disp.print(0, 0, "YOU DIED!");
                    disp.update();
                    complete = frame > 100? 1 : 0;
            break;
        }
        frame++;
    }
    disp.persistence = false;
    disp.color = last_color;
}

void initializeMapActors() {
    int dirs[][2] {
        {-1, 0},
        {0, 1},
        {1, 0},
        {0, -1},
    };
    Map::initilizeSpawnIDS();
    for (int i = 0; i<(MAP_WIDTH*MAP_HEIGHT); i++) {
        int id = Map::data[i];
        int x, y;
        if (Map::SPAWN_IDS[id]) {
            x = i%MAP_WIDTH;
            y = (i-x)/MAP_WIDTH;
            ents.push_back(Actor(x*TILE_SIZE, y*TILE_SIZE, Map::SPAWN_IDS[id]));
            Map::data[i] = 0;
        }
        else if (id == ID_PLATFORM) {
            x = i%MAP_WIDTH;
            y = (i-x)/MAP_WIDTH;
            uint8_t w = 16;
            if (Map::getID(x, y-1)!=0) {
                w = Map::getID(x, y-1)*8;
                Map::setID(x, y-1, 0);
            }
            plats.push_back(Platform(x*TILE_SIZE, y*TILE_SIZE, 1, 0, w));
            Map::setID(x, y, 0);
        }
    }
}

struct Menu {
    void update() {
        if (btn.pressed(BTN_A)) {
            state = STATE_SELECT;
        }
    };
    void draw() {
        disp.setCursor(0, 0);
        disp.println("DOON");
        disp.println("trelemar");
    };
}mainmenu;

struct LevelSelect {
    int current_level;
    LevelSelect() {
        current_level = 1;
    };
    void update() {
        if (btn.pressed(BTN_RIGHT)) {
            current_level++;
        }
        else if (btn.pressed(BTN_LEFT)) {
            current_level--;
        }
        if (btn.pressed(BTN_DOWN)&&current_level+6<=30) {
            current_level+=6;
        }
        else if (btn.pressed(BTN_UP)&&current_level-6>=1) {
            current_level-=6;
        }
        if (btn.pressed(BTN_A)) {
            char file[10];
            sprintf(file, "Doon/map%i.dat", current_level);
            Map::loadData(file);
            snd.playMusicStream("Doon/doon11.snd");
            state = STATE_GAME;
            ents.clear();
            plats.clear();
            ents.push_back(Actor(8, 16, ID_PLAYER));
            initializeMapActors();
        }
        if (btn.pressed(BTN_B)) {
            state = STATE_MENU;
        }
        current_level = clip(current_level, 1, 30);
    }

    void draw() {
        disp.bgcolor = 12;
        disp.setFont(font3x5);
        disp.setColor(14, 0);
        disp.invisiblecolor = 0;
        disp.setCursor(55 - disp.fontWidth * 4, 4);

        //disp.setColor(0);
        for (int x = 0; x < 6; x++) {
            for (int y = 0; y < 5; y++) {
                int dx, dy, i;
                i = 6 * y + x + 1;
                dx = 11 + x * 15;
                dy = 6 + y * 16;

                disp.setColor(i == current_level ? 13 : 2);
                disp.fillRoundRect(dx-1, dy-1, 14, 15, 2);

                disp.setColor(i == current_level ? 15 : 12);
                disp.fillRoundRect(dx, dy+1, 12, 12, 2);

                disp.setColor(i == current_level ? 2 : 13);
                disp.fillRoundRect(dx, dy, 12, 12, 2);

                disp.setColor(i == current_level ? 15 : 12);
                disp.print(dx + 1, dy + 2, i);

                disp.setColor(i == current_level ? 13 : 14);
                disp.print(dx + 1, dy + 1, i);

                if (i<8) {
                    disp.drawBitmap(dx+6, dy +7, sprites[43]);
                }
            }
        }
        disp.bgcolor = 12;
    }
}levelselect;

int main() {
    state = STATE_MENU;
    game.begin();
    game.setFrameRate(TARGET_FRAME_RATE);
    disp.setFont(font3x5);
    disp.load565Palette(Doon_pal);
    disp.setColor(0, 14);
    disp.invisiblecolor = 13;
    //Map::loadData("map1.dat");
    ents.push_back(Actor(8,16,ID_PLAYER));
    initializeMapActors();
    Actor * player = &ents[0];

    Map::setSolids();
    Map::setInvisableTiles();
    Map::shake_x = 0;
    Map::shake_y = 0;
    Map::shake_time = game.getTime();
    while (game.isRunning()) {
        //player->control();
        if (game.update()) {
            switch (state) {
                case STATE_MENU:
                    mainmenu.update();
                    mainmenu.draw();
                break;
                case STATE_SELECT:
                    levelselect.update();
                    levelselect.draw();
                break;
                case STATE_GAME:
                    if (btn.pressed(BTN_C)) {
                        state = STATE_SELECT;
                        ents.clear();
                        plats.clear();
                    }
                    disp.invisiblecolor = Map::bgcolor;
                    if (Map::show_parallax) {
                        disp.bgcolor = Map::bgcolor;
                        //disp.drawBitmap((game.frameCount/8)%112+(Map::cam_x%112), Map::cam_y, parallax[1]);
                        //disp.drawBitmap(112+(game.frameCount/8)%112+(Map::cam_x%112), Map::cam_y, parallax[1]);
                        disp.drawBitmap((Map::cam_x/2)%112, 44, parallax[Map::bg_id]);
                        disp.drawBitmap(((Map::cam_x/2)%112) + 112, 44, parallax[Map::bg_id]);
                    }
                    else {
                        disp.bgcolor = 12;
                    }
                    //disp.bgcolor = 0;
                    disp.invisiblecolor = 0;
                    for (int i = 0; i<ents.size(); i++) {
                        ents[i].updateParentPlatform();
                    }

                    for (int i = 0; i<plats.size(); i++) {
                        plats[i].update(i);
                    }

                    player->control(ents);
                    player->update(ents, 0);
                    for (int i = 1; i<ents.size(); i++) {
                        ents[i].update(ents, i);
                        ents[i].draw();
                    }
                    for (int i = 0; i<plats.size(); i++) {
                        plats[i].draw();
                    }
                    player->draw();
                    Map::drawFast();
                    Map::update(player->x + (player->vx*8), player->y);
                    if (Map::show_parallax) {
                        disp.bgcolor = Map::bgcolor;
                    }
                break; //STATE_GAME
            }
        }
    }
    return 1;
}
