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
  tilePlayerSE*16+4,    mapRampNW+3,    mapFlat+2,  mapFlat+1,   mapFlat+1,   mapRampNW+1, mapFlat,
  mapFlat+1,    mapFlat+1,    mapFlat+2,  mapFlat+1,   mapFlat+1,   mapRampNW+1, mapFlat,
  mapFlat+1,    mapFlat+1,   mapFlatE+2,  mapRampNW+2,   mapFlat+1,     mapRampNW+1,   mapFlat,
  mapRampNE+1,  mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlat,      mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlat,      mapFlat,      mapFlat,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
  mapFlat+1,   mapKey+2,     mapRampNW+1,    mapFlat,     mapFlat,     mapFlat,   mapFlat,
};

static const uint8_t soundFalling = 0;

static const uint8_t fixedPoint = 4;
static const uint8_t fixedPointOne = 1 << fixedPoint;
static const uint8_t fixedPointHalf = fixedPointOne/2;
static const uint8_t fixedPointMask = fixedPointOne - 1;

static const uint8_t speedWalking = 2;

int load;


int16_t playerX = 4 << fixedPoint;
int16_t playerY = 4 << fixedPoint;
int16_t playerZ = 1 << fixedPoint;

int8_t playerTile = tilePlayerSE;
int8_t playerTileX; 
int8_t playerTileY;
int16_t playerOffsetX;
int16_t playerOffsetY;


void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(60);
  Serial.begin(9600);
}


void loop() {
  if (!(arduboy.nextFrame()))
    return;

  // Set up
  arduboy.pollButtons();
  backGround();

  // Decide on player tile
  playerTileX = (playerX + fixedPointHalf) >> fixedPoint;
  playerTileY = (playerY + fixedPointHalf) >> fixedPoint;

  // Calc player offset once
  playerOffsetX = getPlayerOffsetX();
  playerOffsetY = getPlayerOffsetY();


  // Top triangle  
  for (int8_t row=0; row < imapHeight; row++) {
    for (int8_t col=0; col <= min(row,imapWidth-1); col++) {
      drawMap(col,row-col);
    }
  }

  // Bottom triangle
  for (int8_t row=1; row < imapWidth; row++) {
    for (int8_t col=0; col < imapWidth-row; col++) {
      drawMap(row+col,imapHeight-1-col);
    }
  }
  
  movement();

  // CPU Lading
  if (arduboy.everyXFrames(32)) {
    load = (load + arduboy.cpuLoad())/2;
  }
  arduboy.setCursor(0,0);
  arduboy.print(load);

  // Display screen
  arduboy.display();
}

void drawMap(uint8_t mx, uint8_t my) {
  uint8_t z = getHeight(mx,my);
  uint8_t tile = getTile(mx,my);
  uint8_t zmin = (my<imapHeight-1)  ? getCompareHeight(mx,my+1)+1 : 0;
  zmin = (mx<imapWidth-1) ? min(zmin,getCompareHeight(mx+1,my)+1) : zmin;
  zmin = isObject(tile) ? min(zmin,z-1) : zmin;

  for (int8_t mz=min(zmin,z); mz < z; mz++) {
    sprites.drawPlusMask(playerOffsetX+screenX(mx,my,mz),playerOffsetY+screenY(mx,my,mz),isoTiles_16x16,mx==0? tileFlatE: tileFlat);
  }
  uint8_t offset = isObject(tile) ? floatOffset() : 0;
  sprites.drawPlusMask(playerOffsetX+screenX(mx,my,z),playerOffsetY+screenY(mx,my,z) - offset,isoTiles_16x16,tile);

  if (isPlayerTile(mx,my)) {
    //sprites.drawPlusMask(screenFX(playerX,playerY,playerZ),screenFY(playerX,playerY,playerZ)-2,isoTiles_16x16,playerTile);   
    sprites.drawPlusMask((WIDTH-16)/2,(HEIGHT-16)/2,isoTiles_16x16,playerTile);   
  }
}

bool isPlayerTile(uint8_t mx, uint8_t my) {
  return (mx==playerTileX) && (my==playerTileY);
}

uint8_t getHeight(uint8_t mx, uint8_t my) {
  return imap[mx+my*imapWidth] & 0xf;
}

uint8_t getCompareHeight(uint8_t mx, uint8_t my) {
  return getHeight(mx,my) - isObject(getTile(mx,my));
}

bool isObject(uint8_t tile) {
  return (tile >= tileKey);
}

uint8_t floatOffset() {
  uint8_t frame = (arduboy.frameCount >> 4) & 0x7;
  return (frame >= 4) ? 7-frame : frame;
}

uint8_t getTile(uint8_t mx, uint8_t my) {
  return imap[mx+my*imapWidth] >> 4;
}

uint8_t playerMX() {
  return (playerX + fixedPointHalf) >> fixedPoint;
}

uint8_t playerMY() {
  return (playerY + fixedPointHalf) >> fixedPoint;
}

bool coordinatesEqual(uint8_t fx, uint8_t fy, uint8_t mx, uint8_t my) {
  return ((fx>>fixedPoint) == mx) && ((fy>>fixedPoint) == my);  
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
  return (mx+my)*tileWidth/4 - mz*tileThickness;
}

int16_t getPlayerOffsetX() {
  return (WIDTH-16)/2 - screenFX(playerX,playerY,playerZ);
}

int16_t getPlayerOffsetY() {
  return (HEIGHT-16)/2 - screenFY(playerX,playerY,playerZ);
}


int16_t screenFX(int16_t fx, int16_t fy, int16_t fz) {
  return (WIDTH-16)/2 + (((fx-fy)*tileWidth/2)>>fixedPoint);
}

int16_t screenFY(uint8_t fx, uint8_t fy, int8_t fz) {
  return (((fx+fy)*tileWidth/4 - fz*tileThickness)>>fixedPoint);
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
    Serial.print("Before:");
    printFixedPoint(playerX);
    Serial.print(",");
    printFixedPoint(playerY);
    Serial.print(",");
    printFixedPoint(playerZ);
    Serial.print(" : ");
    Serial.print(playerTileX);
    Serial.print(",");
    Serial.print(playerTileY);
    Serial.print(" : ");
    Serial.print(screenFX(playerX,playerY,playerZ));
    Serial.print(",");
    Serial.println(screenFY(playerX,playerY,playerZ));

    switch(playerTile) {
      case tilePlayerNE:
        playerY = max(0,playerY-speedWalking);
        break;
      case tilePlayerNW:
        playerX = max(0,playerX-speedWalking);
        break;
      case tilePlayerSE:
        playerX = min((imapWidth-1)<<fixedPoint,playerX+speedWalking);
        break;
      case tilePlayerSW:
        playerY = min((imapHeight-1)<<fixedPoint,playerY+speedWalking);
        break;      
    }
    
    Serial.print("After:");
    printFixedPoint(playerX);
    Serial.print(",");
    printFixedPoint(playerY);
    Serial.print(",");
    printFixedPoint(playerZ);
    Serial.print(" : ");
    Serial.print(playerTileX);
    Serial.print(",");
    Serial.print(playerTileY);
    Serial.print(" : ");
    Serial.print(screenFX(playerX,playerY,playerZ));
    Serial.print(",");
    Serial.println(screenFY(playerX,playerY,playerZ));
  }

  uint8_t fh = heightAtPlayer();

  // Check if blocked
  if (fh >= prevH+fixedPointOne) {
    if (playerTile == tilePlayerNE || playerTile == tilePlayerSW) {
      playerY = ((prevY + fixedPointHalf) & ~fixedPointMask) ;
    }
    else {
      playerX = ((prevX + fixedPointHalf) & ~fixedPointMask) ;
    }
    fh = heightAtPlayer();

    Serial.print("Block:");
    printFixedPoint(playerX);
    Serial.print(",");
    printFixedPoint(playerY);
    Serial.print(",");
    printFixedPoint(playerZ);
    Serial.print(" : ");
    Serial.print(playerTileX);
    Serial.print(",");
    Serial.print(playerTileY);
    Serial.print(" : ");
    Serial.print(screenFX(playerX,playerY,playerZ));
    Serial.print(",");
    Serial.println(screenFY(playerX,playerY,playerZ));  
  }

  if (playerZ < fh) {
    playerZ++;
  }
  else if (playerZ > fh) {
    if (playerZ - fh > fixedPointHalf) {
      playSound(soundFalling);
    }
    playerZ--;
  }
}

void printFixedPoint(uint16_t value) {
  Serial.print( ((float) value) / 16.0,4);
}

uint8_t heightAtPlayer() {
  uint8_t pmx = playerMX();
  uint8_t pmy = playerMY();
  uint8_t tile = getTile(pmx,pmy);
  uint8_t fh = (1+getCompareHeight(pmx,pmy))<<fixedPoint;
  return isRamp(pmx,pmy) ? fh-fixedPointHalf : fh;
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
