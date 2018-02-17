#include "chunk.h"
#include "../system/timer.h"
#include <stdio.h>

Chunk::Chunk( int X, int Y, int Z, int Seed, BlockList* b_list) {
    // node reset
    next = NULL;
    // side reset
    front = NULL;
    back = NULL;
    right = NULL;
    left = NULL;
    up = NULL;
    down = NULL;
    p_time_idle = SDL_GetTicks();
    // falgs
    p_nomorevbo = false;
    p_updateonce = false;
    p_changed = false;
    p_elements = 0;
    p_createvbo = false;
    p_arraychange = false;
    p_updatevboonce = false;
    p_updatevbo = false;
    p_deleting = false;
    p_seed = Seed;
    // Set Position
    this->x = X;
    this->y = Y;
    this->z = Z;
    p_tile = NULL;

    int t = SDL_GetTicks();

    p_tile = (Tile *)malloc(sizeof(Tile) * CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH); //new (std::nothrow) Tile*[ CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH]

    if (p_tile == nullptr) {
      // error assigning memory. Take measures.
      printf( "Chunk::Chunk error\n");
      return;
    }


    for (int cz = 0; cz < CHUNK_DEPTH; cz++)
        for (int cx = 0; cx < CHUNK_WIDTH; cx++)
            for(int cy = 0; cy < CHUNK_HEIGHT; cy++) {
                int index = TILE_REGISTER( cx, cy, cz);
                p_tile[ index ].ID = EMPTY_BLOCK_ID;
                //p_tile[ index ].;
                //CreateTile( cx, cy, cz, 0);
            }
    printf( "Chunk::chunk %dms\n", SDL_GetTicks()-t);
    // http://www.blitzbasic.com/Community/posts.php?topic=93982
}

Chunk::~Chunk() {
    Timer timer;
    timer.Start();
    // node reset
    next = NULL;
    // side reset
    front = NULL;
    back = NULL;
    right = NULL;
    left = NULL;
    up = NULL;
    down = NULL;
    // L�schen p_tile
    for (int cz = 0; cz < CHUNK_DEPTH; cz++)
        for (int cx = 0; cx < CHUNK_WIDTH; cx++)
            for(int cy = 0; cy < CHUNK_HEIGHT; cy++) {
                    int Register = TILE_REGISTER( cx, cy, cz);
//                    delete p_tile[ Register];
//                    p_tile[ Register] = NULL;
                }
    delete[] p_tile;

    //printf( "~Chunk(): remove p_tile in %dms\n", timer.GetTicks());
    timer.Start();
    // vbo daten l�schen
/*    p_data.clear();
    p_vertex.clear();*/
    //p_normal.clear();
    printf( "~Chunk(): remove vbo data in %dms\n", timer.GetTicks());
}

void Chunk::CreateTile( int X, int Y, int Z, int ID) {
    Tile* tile;
    tile = GetTile( X, Y, Z);
    if( ID == 0) {
        if( tile != NULL) {
            p_changed = true;
//            delete p_tile[ TILE_REGISTER( X, Y, Z)];
//            p_tile[ TILE_REGISTER( X, Y, Z)] = NULL;
        }
        return;
    }

    // tile nehmen
    tile->ID = ID;
    // welt hat sich ver�ndert
    p_changed = true;
}

void Chunk::set( int X, int Y, int Z, int ID) {
    Tile* l_tile;
    l_tile = GetTile( X, Y, Z);
    if( l_tile == NULL)
        return;

    // tile nehmen
    l_tile->ID = ID;
    // welt hat sich ver�ndert
    p_changed = true;
}

Tile *Chunk::GetTile( int X, int Y, int Z) {
    if( X < 0)
        return NULL;
    if( Y < 0)
        return NULL;
    if( Z < 0)
        return NULL;
    // X Y Z
    // Z X Y -> https://www.youtube.com/watch?v=B4DuT61lIPU
    Tile *l_tile = &p_tile[ TILE_REGISTER( X, Y, Z)];
    return l_tile;
}

bool Chunk::CheckTile( int X, int Y, int Z) {
    if( p_tile[ TILE_REGISTER( X, Y, Z)].ID == EMPTY_BLOCK_ID)
        return false;
    return true;
}

//Helper method to go from a float to packed char
unsigned char ConvertChar(float value) {
  //Scale and bias
  value = (value + 1.0f) * 0.5f;
  return (unsigned char)(value*255.0f);
}

//Pack 3 values into 1 float
float PackToFloat(unsigned char x, unsigned char y, unsigned char z)
{
  unsigned int packedColor = (x << 16) | (y << 8) | z;
  float packedFloat = (float) ( ((double)packedColor) / ((double) (1 << 24)) );

  return packedFloat;
}

void Chunk::UpdateArray( BlockList *List, Chunk *Back, Chunk *Front, Chunk *Left, Chunk *Right, Chunk *Up, Chunk *Down) {
    int i = 0;
    glm::vec2 Side_Textur_Pos;
    Timer timer;
    timer.Start();

    // wird gel�scht
    if( p_deleting)
        return;

    bool b_visibility = false;

    Tile * l_tile = NULL;

    // View from negative x
    for(int z = 0; z < CHUNK_DEPTH; z++) {
        for(int x = CHUNK_WIDTH - 1; x >= 0; x--) {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                l_tile = GetTile( x, y, z);
                if( !l_tile)
                    continue;

                if( l_tile->ID == EMPTY_BLOCK_ID) {// Tile nicht vorhanden
                    b_visibility = false;
                    continue;
                }
                unsigned int type = l_tile->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                // Line of sight blocked?
                if(  x != 0 && CheckTile(x-1, y, z) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x-1, y, z)->ID )->getAlpha() )
                {
                    b_visibility = false;
                    continue;
                }
                // View from negative x
                if( x == 0 && Back != NULL && Back->CheckTile(CHUNK_WIDTH-1, y, z) && Back->GetTile( CHUNK_WIDTH-1, y, z)->ID) {
                    b_visibility = false;
                    continue;
                }
                if( 0&& b_visibility && y != 0 && CheckTile(x, y-1, z) && ( l_tile->ID == GetTile( x, y-1, z)->ID) ) {
                    p_vertex[i - 4] = block_vertex( x,     y+1,     z, 0);
                    p_vertex[i - 3] = block_vertex( x, y+1, z, 0);
                    p_vertex[i - 1] = block_vertex( x, y+1, z+1, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 0);

                    p_data[i] = block_data( 0.f, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0.f);
                    p_vertex[i++] = block_vertex(x, y + 1, z + 1, 0);
                }

                b_visibility = true;
            }
        }
    }
    // View from positive x
    for(int z = 0; z < CHUNK_DEPTH; z++) {
        for(int x = 0; x < CHUNK_WIDTH; x++)  {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                if( GetTile( x, y, z) == NULL) // Tile nicht vorhanden
                    continue;
                uint8_t type = GetTile( x, y, z)->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                if(  x != CHUNK_WIDTH-1 && CheckTile(x+1, y, z) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x+1, y, z)->ID)->getAlpha() ) {
                    b_visibility = false;
                    continue;
                }
                // View from positive x
                if( x == CHUNK_WIDTH-1 && Front != NULL && Front->CheckTile( 0, y, z) && Front->GetTile( 0, y, z)->ID) {
                    b_visibility = false;
                    continue;
                }
                if(b_visibility && y != 0 && CheckTile( x, y-1, z) && GetTile( x, y, z)->ID == GetTile( x, y-1, z)->ID) {
                    p_vertex[i - 5] = block_vertex(x + 1, y+1, z, 0);
                    p_vertex[i - 3] = block_vertex(x +1, y+1, z, 0);
                    p_vertex[i - 2] = block_vertex(x +1, y+1, z+1, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 1);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z + 1, 0);
                }
                b_visibility = true;
            }
        }
    }

    // View from negative y
    for(int z = 0; z < CHUNK_DEPTH; z++) {
        for(int y = CHUNK_HEIGHT - 1; y >= 0; y--)
            for(int x = 0; x < CHUNK_WIDTH; x++) {
              {
                if( GetTile( x, y, z) == NULL) // Tile nicht vorhanden
                    continue;
                uint8_t type = GetTile( x, y, z)->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                if( y != 0 && CheckTile(x, y-1, z) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x, y-1, z)->ID)->getAlpha() ) {
                    b_visibility = false;
                    continue;
                }
                if( y == 0 && Down != NULL && Down->CheckTile(x, CHUNK_HEIGHT-1, z) && Down->GetTile( x, CHUNK_HEIGHT-1, z)->ID) {
                    b_visibility = false;
                    continue;
                }
                if(b_visibility && x != 0 && CheckTile(x-1, y, z) && GetTile( x-1, y, z)->ID == GetTile( x, y, z)->ID) {
                    p_vertex[i - 5] = block_vertex(x+1    , y, z, 0);
                    p_vertex[i - 3] = block_vertex(x + 1, y, z, 0);
                    p_vertex[i - 2] = block_vertex(x + 1, y, z+1, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 5);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y, z + 1, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z + 1, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y, z + 1, 0);
                }
                b_visibility = true;
            }
        }
    }
    // View from positive y
    for(int z = 0; z < CHUNK_DEPTH; z++) {
         for(int y = 0; y < CHUNK_HEIGHT; y++){
             for(int x = 0; x < CHUNK_WIDTH; x++) {
                if( GetTile( x, y, z) == NULL) // Tile nicht vorhanden
                    continue;
                uint8_t type = GetTile( x, y, z)->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                if(  y != CHUNK_HEIGHT-1 && CheckTile(x, y+1, z) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x, y+1, z)->ID)->getAlpha() ) {
                    b_visibility = false;
                    continue;
                }
                if( y == CHUNK_HEIGHT-1 && Up != NULL && Up->CheckTile(x, 0, z) && Up->GetTile( x, 0, z)->ID) {
                    b_visibility = false;
                    continue;
                }
                if(b_visibility && x != 0 && CheckTile(x-1, y, z) && GetTile( x-1, y, z)->ID == GetTile( x, y, z)->ID) {
                    p_vertex[i - 4] = block_vertex( x+1,     y+1,     z, 0);
                    p_vertex[i - 3] = block_vertex( x+1, y+1, z, 0);
                    p_vertex[i - 1] = block_vertex( x+1, y+1, z+1, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 4);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z + 1, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z + 1, 0);

                    p_data[i] = block_data( 1, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z + 1, 0);
                }
                b_visibility = true;
            }
        }
    }
    // View from negative z
    for(int z = CHUNK_DEPTH - 1; z >= 0; z--) {
         for(int x = 0; x < CHUNK_WIDTH; x++){
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                if( GetTile( x, y, z) == NULL) // Tile nicht vorhanden
                    continue;
                uint8_t type = GetTile( x, y, z)->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                if(  z != 0 && CheckTile(x, y, z-1) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x, y, z-1)->ID)->getAlpha() ) {
                    b_visibility = false;
                    continue;
                }
                if( z == 0 && Left != NULL && Left->CheckTile(x, y, CHUNK_DEPTH-1) && Left->GetTile( x, y, CHUNK_DEPTH-1)->ID) {
                    b_visibility = false;
                    continue;
                }
                if(b_visibility && y != 0 && CheckTile(x, y-1, z) && GetTile( x, y, z)->ID == GetTile( x, y-1, z)->ID) {
                    p_vertex[i - 5] = block_vertex(x, y+1, z, 0);
                    p_vertex[i - 2] = block_vertex(x+1, y+1, z, 0);
                    p_vertex[i - 3] = block_vertex(x, y+1, z, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 2);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z, 0);
                }
                b_visibility = true;
            }
        }
    }
    // View from positive z
    for(int z = 0; z < CHUNK_DEPTH; z++) {
        for(int x = 0; x < CHUNK_WIDTH; x++) {
            for(int y = 0; y < CHUNK_HEIGHT; y++) {
                if( GetTile( x, y, z) == NULL) // Tile nicht vorhanden
                    continue;
                uint8_t type = GetTile( x, y, z)->ID;
                if( type == 0) {
                    b_visibility = false;
                    continue;
                }
                if( z != CHUNK_DEPTH-1 && CheckTile(x, y, z+1) && List->GetBlock( type)->getAlpha() == List->GetBlock( GetTile( x, y, z+1)->ID)->getAlpha() ) {
                    b_visibility = false;
                    continue;
                }
                // View from positive z
                if( z == CHUNK_DEPTH-1 && Right != NULL && Right->CheckTile(x, y, 0) && Right->GetTile( x, y, 0)->ID) {
                    b_visibility = false;
                    continue;
                }
                if(b_visibility && y != 0 && CheckTile(x, y-1, z) && GetTile( x, y-1, z)->ID == GetTile( x, y, z)->ID) {
                    p_vertex[i - 4] = block_vertex( x,     y+1,     z+1, 0);
                    p_vertex[i - 3] = block_vertex( x, y+1, z+1, 0);
                    p_vertex[i - 1] = block_vertex( x+1, y+1, z+1, 0);
                } else {
                    Side_Textur_Pos = List->GetTexturByType( type, 3);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x, y + 1, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y, z + 1, 0);

                    p_data[i] = block_data( 0, Side_Textur_Pos.x, Side_Textur_Pos.y, 0);
                    p_vertex[i++] = block_vertex(x + 1, y + 1, z + 1, 0);
                }
                b_visibility = true;
            }

        }
    }
    // normal errechnen
    int v;
    //p_normal.resize( p_vertex.size());
    for( v = 0; v+3 <= i; v+=3) {
        glm::vec3 a(p_vertex[v].x, p_vertex[v].y, p_vertex[v].z);
        glm::vec3 b(p_vertex[v+1].x, p_vertex[v+1].y, p_vertex[v+1].z);
        glm::vec3 c(p_vertex[v+2].x, p_vertex[v+2].y, p_vertex[v+2].z);
        glm::vec3 edge1 = b-a;
        glm::vec3 edge2 = c-a;
        glm::vec3 normal = glm::normalize( glm::cross( edge1, edge2));

        p_vertex[v+0].w = PackToFloat ( ConvertChar( normal.x) , ConvertChar( normal.y), ConvertChar( normal.z));
        p_vertex[v+1].w = PackToFloat ( ConvertChar( normal.x) , ConvertChar( normal.y), ConvertChar( normal.z));
        p_vertex[v+2].w = PackToFloat ( ConvertChar( normal.x) , ConvertChar( normal.y), ConvertChar( normal.z));

        /*p_normal[v].x = normal.x;
        p_normal[v].y = normal.y;
        p_normal[v].z = normal.z;

        p_normal[v+1].x = normal.x;
        p_normal[v+1].y = normal.y;
        p_normal[v+1].z = normal.z;

        p_normal[v+2].x = normal.x;
        p_normal[v+2].y = normal.y;
        p_normal[v+2].z = normal.z;*/
    }
    p_elements = i;
    p_changed = false;
    p_updateonce = true;
    if( p_elements == 0) {// Kein Speicher resavieren weil leer
        return;
    }
    p_arraychange = true;
    printf( "UpdateArray %dms %d %d %d\n", timer.GetTicks(), p_elements, GetAmount());
}

void Chunk::DestoryVbo() {
    p_nomorevbo = true;
    while( p_updatevbo);

    if( p_createvbo ) {
        p_createvbo = false;
        glDeleteBuffers(1, &p_vboVertex);
//        glDeleteBuffers(1, &p_vboNormal);
        glDeleteBuffers(1, &p_vboData);
    }
}

void Chunk::UpdateVbo() {
    Timer timer;
    timer.Start();

    // Nicht bearbeiten falls es anderweilig bearbeitet wird
    if( p_deleting)
        return;
    if( p_elements == 0 || p_updateonce == false || p_nomorevbo == true)
        return;

    // VBO erstellen falls dieser fehlt
    if(p_createvbo == false) {
        // Create vbo
        glGenBuffers(1, &p_vboVertex);
//        glGenBuffers(1, &p_vboNormal);
        glGenBuffers(1, &p_vboData);
        p_createvbo = true;
    }

    // flags �ndern
    p_arraychange = false;
    p_updatevboonce = true;

    // anderweilig besch�ftigt?
    while( p_updatevbo);
    p_updatevbo = true;
    // vbo updaten
    glBindBuffer(GL_ARRAY_BUFFER, p_vboVertex);
    glBufferData(GL_ARRAY_BUFFER, p_elements * sizeof *p_vertex, p_vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, p_vboData);
    glBufferData(GL_ARRAY_BUFFER, p_elements * sizeof *p_data, p_data, GL_STATIC_DRAW);
    p_updatevbo = false;
    printf( "UpdateVbo %dms %d * %d = %d\n", timer.GetTicks(), sizeof(block_vertex), GetAmount(), GetAmount() * sizeof(block_data));
}

void Chunk::Draw( Graphic* graphic, Shader* shader, Camera* camera, Camera* shadow, glm::mat4 aa) {
    // Position errechnen
    Transform f_form;
    f_form.GetPos().x = x*CHUNK_WIDTH;
    f_form.GetPos().y = y*CHUNK_HEIGHT;
    f_form.GetPos().z = z*CHUNK_DEPTH;

    // chunk wird grad gel�scht
    if( p_nomorevbo == true)
        return;
    // wird gel�scht
    if( p_deleting)
        return;
    if( p_updatevbo)
        return;
    p_updatevbo = true;

    // Shader einstellen
    shader->Update( f_form, camera, shadow, aa);

    // vbo pointer auf array setzen
    shader->BindArray( p_vboVertex, 0, GL_FLOAT);
    shader->BindArray( p_vboData, 1);

    // Dreiecke zeichnen
    // Debug GL_LINES, GL_TRIANGLES
    glDrawArrays( GL_TRIANGLES, 0, (int)p_elements);

    // Update war erfolgreich
    p_updatevbo = false;
}
