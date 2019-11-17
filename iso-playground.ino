#include <Arduboy2.h>
#include <Sprites.h>
#include <ArduboyTones.h>
#include "tiles.h"

Arduboy2 arduboy;
Sprites  sprites;
ArduboyTones sound(arduboy.audio.enabled);

static const uint8_t tileWidth = 16;
static const uint8_t tileThickness = 8;

// static const uint8_t tileVoid=  0;
static const uint8_t tileFlat     = 0;
static const uint8_t tileFlatE    = 1;
static const uint8_t tileRampNE   = 2;
static const uint8_t tileRampNW   = 3;
static const uint8_t tileDoorNE   = 4;
static const uint8_t tileDoorNW   = 5;
static const uint8_t tilePlayerNE = 6;
static const uint8_t tilePlayerNW = 7;
static const uint8_t tilePlayerSE = 8;
static const uint8_t tilePlayerSW = 9;
static const uint8_t tileKey      = 10;

static const uint8_t mapFlat     = 16*tileFlat;
static const uint8_t mapFlatE    = 16*tileFlatE;
static const uint8_t mapRampNE   = 16*tileRampNE;
static const uint8_t mapRampNW   = 16*tileRampNW;
static const uint8_t mapDoorNE   = 16*tileDoorNE;
static const uint8_t mapDoorNW   = 16*tileDoorNW;
static const uint8_t mapKey      = 16*tileKey;

static const uint8_t imapWidth=7;
static const uint8_t imapHeight=7;

uint8_t imap[] = {
  mapFlat+1,    mapFlat+1,    mapFlat+1,  mapFlat+1,   mapFlat+1,   mapRampNW+1, mapFlat,
  mapFlat+1,    mapFlat+1,    mapFlat+1,  mapFlat+1,   mapFlat+1,   mapRampNW+1, mapFlat,
  mapFlat+1,    mapFlat+1,   mapFlat+1,  mapFlat+1,   mapFlat,     mapFlat,   mapFlat,
  mapRampNE+1,  mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlat,      mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlat,      mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlatE+1,   mapKey+2,     mapRampNW+1,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
};

static const uint8_t soundFalling = 0;

int load;

int8_t playerX = 4 << 2;
int8_t playerY = 4 << 2;
int8_t playerZ = 1 << 2;

int8_t playerTile = tilePlayerSE;


void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(30);
  Serial.begin(9600);
}


void loop() {
  if (!(arduboy.nextFrame()))
    return;

  // Set up
  arduboy.pollButtons();
  backGround();
  
  for (int8_t mx=0; mx < imapWidth; mx++) {
    for (int8_t my=0; my < imapHeight; my++) {

      uint8_t z = getHeight(mx,my);
      uint8_t tile = getTile(mx,my);

      uint8_t zmin = (my<imapHeight-1)  ? getHeight(mx,my+1)+1 : 0;
      zmin = (mx<imapWidth-1) ? min(zmin,getHeight(mx+1,my)+1) : zmin;

      for (int8_t mz=min(zmin,z); mz < z; mz++) {
        sprites.drawPlusMask(screenX(mx,my,mz),screenY(mx,my,mz),isoTiles_16x16,mx==0? tileFlatE: tileFlat);
      }
      uint8_t offset = (tile >= tileKey) ? floatOffset() : 0;
      sprites.drawPlusMask(screenX(mx,my,z),screenY(mx,my,z) - offset,isoTiles_16x16,tile);

      if (coordinatesEqual(playerX+2,playerY+2,mx,my)) {
        sprites.drawPlusMask(screenFX(playerX,playerY,playerZ),screenFY(playerX,playerY,playerZ)-2,isoTiles_16x16,playerTile);        
      }
    }
  }

  // Main Character

  if (arduboy.everyXFrames(32)) {
    load = (load + arduboy.cpuLoad())/2;
  }

  movement();

  arduboy.setCursor(0,0);
  arduboy.print(load);
  arduboy.display();
}

uint8_t getHeight(uint8_t mx, uint8_t my) {
  return imap[mx+my*imapWidth] & 0xf;
}

uint8_t floatOffset() {
  uint8_t frame = (arduboy.frameCount >> 4) & 0x7;
  return (frame >= 4) ? 7-frame : frame;
}

uint8_t getTile(uint8_t mx, uint8_t my) {
  return imap[mx+my*imapWidth] >> 4;
}

uint8_t playerMX() {
  return (playerX + 2) >> 2;
}

uint8_t playerMY() {
  return (playerY + 2) >> 2;
}

bool coordinatesEqual(uint8_t fx, uint8_t fy, uint8_t mx, uint8_t my) {
  return ((fx>>2) == mx) && ((fy>>2) == my);  
}

void backGround() {
  uint16_t offset = 0;
  for(uint16_t offset=0; offset < WIDTH*HEIGHT/8;) {
    arduboy.sBuffer[offset++] = 0x55;
    arduboy.sBuffer[offset++] = 0xAA;    
  }
}

int8_t screenX(int8_t mx, int8_t my, int8_t mz) {
  return (WIDTH-16)/2 + (mx-my)*tileWidth/2;
}

int8_t screenY(uint8_t mx, uint8_t my, int8_t mz) {
  return (mx+my)*tileWidth/4 + tileThickness*(0-mz);
}


int8_t screenFX(int8_t mx, int8_t my, int8_t mz) {
  return (WIDTH-16)/2 + (mx-my)*tileWidth/8;
}

int8_t screenFY(uint8_t mx, uint8_t my, int8_t mz) {
  return (mx+my)*tileWidth/16 + tileThickness/4*(0-mz);
}

void movement() {
  bool move = false;
  if (arduboy.pressed(UP_BUTTON + RIGHT_BUTTON)) {
    playerTile = tilePlayerNE;
    move = true;
  }
  if (arduboy.pressed(UP_BUTTON + LEFT_BUTTON)) {
    playerTile = tilePlayerNW;
    move = true;
  }
  if (arduboy.pressed(DOWN_BUTTON + RIGHT_BUTTON)) {
    playerTile = tilePlayerSE;
    move = true;
  }
  if (arduboy.pressed(DOWN_BUTTON + LEFT_BUTTON)) {
    playerTile = tilePlayerSW;
    move = true;
  }

  uint8_t prevX = playerX;
  uint8_t prevY = playerY;
  uint8_t prevH = heightAtPlayer();
    
//  if (arduboy.justPressed(B_BUTTON)) {
  if (move) {
      switch(playerTile) {
      case tilePlayerNE:
        playerY = max(0,playerY-1);
        break;
      case tilePlayerNW:
        playerX = max(0,playerX-1);
        break;
      case tilePlayerSE:
        playerX = min((imapWidth-1)*4,playerX+1);
        break;
      case tilePlayerSW:
        playerY = min((imapHeight-1)*4,playerY+1);
        break;      
    }
//    Serial.print(playerX);
//    Serial.print(",");
//    Serial.print(playerY);
//    Serial.print(",");
//    Serial.print(playerZ);
//    Serial.print(",");
//    Serial.print(playerMX());
//    Serial.print(",");
//    Serial.print(playerMY());
  }

  uint8_t fh = heightAtPlayer();

  // Check if blocked
  if (fh >= prevH+4) {
    playerX = prevX;
    playerY = prevY;
    fh = heightAtPlayer();
  }

  if (playerZ < fh) {
    playerZ++;
  }
  else if (playerZ > fh) {
    if (playerZ - fh > 2) {
      playSound(soundFalling);
    }
    // Fall slower
    if (arduboy.everyXFrames(2)) {
      playerZ--;
    }
  }
}

uint8_t heightAtPlayer() {
  uint8_t pmx = playerMX();
  uint8_t pmy = playerMY();
  uint8_t tile = getTile(pmx,pmy);
  uint8_t fh = (getHeight(pmx,pmy)+ (tile < tileKey))*4;
  return isRamp(pmx,pmy) ? fh-2 : fh;
}

bool isRamp(uint8_t mx, uint8_t my) {
  uint8_t tile = getTile(mx,my);
  return (tile >= tileRampNE) && (tile <= tileRampNW);
}

void playSound(uint8_t soundEffect) {
  switch(soundEffect) {
    case soundFalling:
        sound.tone(NOTE_A4H,35, NOTE_B4H,35);
        break;
  }
}
