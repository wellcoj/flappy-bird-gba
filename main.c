// The Main HAM Library
#include <mygba.h>
#include <stdio.h>
#include <stdlib.h>

//Graphics Includes
//Backgrounds
#include "gfx/bg/bg.pal.c"
#include "gfx/bg/bg_day.raw.c"
#include "gfx/bg/bg_day.map.c"
//#include "gfx/bg/bg_night.raw.c"
//#include "gfx/bg/bg_night.map.c"
#include "gfx/bg/ground.raw.c"
#include "gfx/bg/ground.map.c"

//Sprites - Pallet
#include "gfx/sprites/sprites.pal.c"
//Sprites - YellowBird
#include "gfx/sprites/flappy_up.raw.c"
#include "gfx/sprites/flappy_mid.raw.c"
#include "gfx/sprites/flappy_down.raw.c"
//Sprites - Pipes
#include "gfx/sprites/pipe_up.raw.c"
#include "gfx/sprites/pipe_down.raw.c"
//Sprite - Ground
#include "gfx/sprites/ground.raw.c"
// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------
// Global Variables
u8 bird;// Sprite object number
u8 pipeUpSpt;// Sprite object number
u8 pipeDownSpt0, pipeDownSpt1, pipeDownSpt2;// Sprite object number
u8 pipeUpSpt0, pipeUpSpt1, pipeUpSpt2;// Sprite object number
u8 groundPart0, groundPart1, groundPart2, groundPart3;
u8 vbl_count = 0; // Keeps track of the number of VBLs
int bird_x = 61;  // X position of bird (column)
int bird_y = 0;   // Y position of bird (row)
int pipes_x = 464;   // X position of pipe (column)
int pipes_y = 0;     // Y position of pipe (row)
u32 frames = 0;   // Global frame counter
u32 animcnt = 0;  // Current frame of animation
u32 ground_x = 0;  // Ground X coordinate
u32 scrollx = 64; // X co-ordinate of scrolling center
u32 scrolly = 64; // Y co-ordinate of scrolling center
u32 zoomx = 256; // X co-ordinate of zoom center
u32 zoomy = 256; // Y co-ordinate of zoom center
int rand_pipe_y[3] = {0, -32, -16};
int rand_pipe_x[3] = {240, 240 + 90, 240 + 180};

map_fragment_info_ptr bg_day;
map_fragment_info_ptr bg_ground;


// Function Prototypes
void vbl_func();        // VBL function
void move_bird();       // Drop the block
void update_bird();     // Apply block's new position
void query_buttons();   // Query for input
void setBgDay();
void setBgNight();
void birdUp();
void birdMid();
void birdDown();
void setBackGrounds();
void setGround();
void moveGround();
void update_bird_gfx();
void pipesGenerator();
void setPipesBeforeGround();
void pipesMover();
int random();

// Function: main()
int main()
{
    // Initialize HAMlib
    ham_Init();

    //Set Background day
    setBackGrounds();

    //Create bird sprite
    // Initialize the sprites palette
    ham_LoadObjPal((void*)sprites_Palette, 256);
    
    // Create the sprite and store the object number
    bird = ham_CreateObj((void*)flappy_up_Bitmap, OBJ_SIZE_16X16 , OBJ_MODE_NORMAL,1,0,0,0,0,0,0,110,50);
    pipesGenerator();
    //ham_SetFxMode(BIT2, BIT4, FX_MODE_ALPHABLEND);

    //Set ground sprites
    //setGround();

    // Start the VBL interrupt handler
    ham_StartIntHandler(INT_TYPE_VBL,(void*)&vbl_func);

    // Infinite loop to keep the program running
    while(1) {}

    return 0;
} // End of main()

void vbl_func(){
    // Increment VBL counter
    ++vbl_count;

    // Copy sprites to hardware
    ham_CopyObjToOAM();
	
    // Process the following every VBL
    query_buttons(); // Query for input
    update_bird_gfx(); // Apply bird's new graphic
    move_bird(); // Drop the bird
    pipesMover(); // Move pipe from right to left
    update_bird(); // Apply bird's new position
    
    //Move Ground
    moveGround();
    
    // Rotate the background
	//ham_RotBgEx(0,frames%360,120,80,scrollx,scrolly,zoomx,zoomy);

    ++frames; // Increment the frame counter
    
    if(vbl_count == 60){
        vbl_count = 0; // Reset the VBL counter
    }
    
    return;
} // End of vbl_func()

void query_buttons(){
    
    // Query Pad to see in what direction
    // the map should be scrolled
    if(F_CTRLINPUT_LEFT_PRESSED)
    {
      ++scrollx;
    }
    else
    if(F_CTRLINPUT_RIGHT_PRESSED)
    {
      --scrollx;
    }

    if(F_CTRLINPUT_DOWN_PRESSED)
    {
      ++scrolly;
    }
    else
    if(F_CTRLINPUT_UP_PRESSED)
    {
      --scrolly;
    }
    if (F_CTRLINPUT_START_PRESSED) {
    	scrollx = 64;
    	scrolly = 64;
	}
    
    if(F_CTRLINPUT_A_PRESSED)
    {
        if (bird_y > 0){
            
            birdDown();
            
            bird_y = bird_y - 7;
            if (bird_y < 0) bird_y = 0;
        }else{
            bird_y = 0;
        }
    }

    return;
} // End of query_buttons()

void setBackGrounds(){
    // Setup the background mode
    ham_SetBgMode(1);

    // Initialize the background palettes
    ham_LoadBGPal((void*)bg_Palette,256);

    // Setup the tileset for our image
    ham_bg[0].ti = ham_InitTileSet(
            (void*)bg_day_Tiles,
            SIZEOF_16BIT(bg_day_Tiles),1,1);
    ham_bg[1].ti = ham_InitTileSet(
            (void*)ground_Tiles,
            SIZEOF_16BIT(ground_Tiles),1,1);

    // Setup the map for our image
    ham_bg[0].mi = ham_InitMapEmptySet(3,0);
    ham_bg[1].mi = ham_InitMapEmptySet(3,0);

    bg_day = ham_InitMapFragment(
        (void*)bg_day_Map,30,20,0,0,30,20,0);
    bg_ground = ham_InitMapFragment(
        (void*)ground_Map,30,20,0,0,30,20,0);

    ham_InsertMapFragment(bg_day,0,0,0);
    ham_InsertMapFragment(bg_ground,1,0,0);

    // Display the background
    ham_InitBg(0,1,3,1);
    ham_InitBg(1,1,0,1);
}

void moveGround(){
    ground_x += 1;
    ham_InsertMapFragment(bg_ground,1,ground_x,0);
    return;
}

void update_bird(){
    ham_SetObjXY(bird,bird_x,bird_y);
    return;
}
void move_bird(){
    // Check if 5 VBLs have passed
    if (frames == 1) {
        if (bird_y < 106) bird_y = bird_y + 5;// Drop
    }
    update_bird();
    return;
}
void pipesGenerator(){
    // Create the sprite and store the object number
    pipeDownSpt0 = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL ,1,0,0,0,0,1,0,rand_pipe_x[0], 0);
    pipeDownSpt1 = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL ,1,0,0,0,0,1,0,rand_pipe_x[1], 0);
    pipeDownSpt2 = ham_CreateObj((void*)pipe_down_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[2], 0);
    
    pipeUpSpt0 = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[0], 96 );
    pipeUpSpt1 = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[1], 96 );
    pipeUpSpt2 = ham_CreateObj((void*)pipe_up_Bitmap, OBJ_SIZE_32X64 , OBJ_MODE_NORMAL,1,0,0,0,0,1,0,rand_pipe_x[2], 96 );
    
    return;
}



void pipesMover(){
    
    if (frames == 1) {
        
        if(rand_pipe_x[0] <= -32){
            rand_pipe_x[0] = 240;
            rand_pipe_y[0] = random(-37, 0);
        }
        if(rand_pipe_x[1] <= -32){
             rand_pipe_x[1] = 240;
             rand_pipe_y[1] = random(-37, 0);
        }
        if(rand_pipe_x[2] <= -32){
             rand_pipe_x[2] = 240;
             rand_pipe_y[2] = random(-37, 0);
        }

        ham_SetObjXY(pipeDownSpt0,rand_pipe_x[0], rand_pipe_y[0]);
        ham_SetObjXY(pipeUpSpt0,rand_pipe_x[0], rand_pipe_y[0] + 110);
            
        ham_SetObjXY(pipeDownSpt1, rand_pipe_x[1], rand_pipe_y[1]);
        ham_SetObjXY(pipeUpSpt1, rand_pipe_x[1], rand_pipe_y[1] + 110 );
        
        ham_SetObjXY(pipeDownSpt2, rand_pipe_x[2], rand_pipe_y[2]);
        ham_SetObjXY(pipeUpSpt2, rand_pipe_x[2],rand_pipe_y[2] + 110 );
        
        rand_pipe_x[0] -= 2 ;
        rand_pipe_x[1] -= 2 ;
        rand_pipe_x[2] -= 2 ;
    }
    return;
}

void birdUp(){
    ham_UpdateObjGfx(bird, (void*)&flappy_up_Bitmap[0]);
    return;
}

void birdMid(){
    ham_UpdateObjGfx(bird, (void*)&flappy_mid_Bitmap[0]);
    return;
}

void birdDown(){
    ham_UpdateObjGfx(bird, (void*)&flappy_down_Bitmap[0]);
    return;
}

void update_bird_gfx()
{
    // We'll only update the animation every 5th frame
    if (frames > 2) {
        // Reset the frame counter
        frames = 0;
        // Figure out where to load the image from and update it
        if( animcnt == 0 ) birdUp();
        if( animcnt == 1 ) birdMid();
        if( animcnt == 2 ) birdDown();
        // Increment the animation counter
        if (animcnt == 2) {
            animcnt = 0;
        } else {
            animcnt++;
        }
    }
    return;
}

int random(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}
