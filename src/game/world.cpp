#include "world.h"
#include <time.h>
#include "../system/timer.h"

static int world_thread(void *data)
{
    int cnt;

    for ( ;; ) {
        world *l_world = (world*)data;
        l_world->process_thrend();

        if( l_world->getDestory() )
            break;

        SDL_Delay(1);
    }

    return 0;
}

world::world( std::string Tileset, BlockList* B_List) {
    Seed = (int)time(NULL); // Seed
    p_buysvector = false;
    p_tilemap = new texture( Tileset);
    p_chunk_amount = 0;
    // Kein Chunk am anfang
    Chunks = NULL;
    p_world_tree_empty = true;
    // Blocklist link erstellen
    p_blocklist = B_List;

    p_destroy = false;
    p_thread = SDL_CreateThread(world_thread, "TestThread", (void *)this);
}

world::~world() {
    int l_return;
    p_destroy = true;
    SDL_WaitThread( p_thread, &l_return);

    // L�schen der Welt
    deleteChunks( Chunks);
    while( p_chunk_amount != 0) {
        SDL_Delay(1);
    }
    delete p_tilemap;
}

Tile* world::GetTile( int x, int y, int z) {
    Chunk *node = Chunks;
    for( ;; ) {
        if( node == NULL)
            break;
        int p_chunk_x = x-node->getX()*CHUNK_SIZE;
        int p_chunk_y = y-node->getY()*CHUNK_SIZE;
        int p_chunk_z = z-node->getZ()*CHUNK_SIZE;

        // X - Achse
        if(p_chunk_x < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_x >= CHUNK_SIZE ) {
            node = node->next;
            continue;
        }
        // Y - Achse
        if(p_chunk_y < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_y >= CHUNK_SIZE ) {
            node = node->next;
            continue;
        }
        // Z - Achse
        if(p_chunk_z < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_z >= CHUNK_SIZE) {
            node = node->next;
            continue;
        }
        //printf("%d %d %d | ", p_chunk_x, p_chunk_y, p_chunk_z);
        Tile *l_tile = node->getTile( p_chunk_x, p_chunk_y, p_chunk_z );
        if( l_tile && l_tile->ID == EMPTY_BLOCK_ID)
            break;
        return l_tile;
    }
    return NULL;
}

Chunk* world::getChunkWithPos( int x, int y, int z) {
    Chunk *node = Chunks;
    for( ;; ) {
        if( node == NULL)
            break;
        int p_chunk_x = x-node->getX()*CHUNK_SIZE;
        int p_chunk_y = y-node->getY()*CHUNK_SIZE;
        int p_chunk_z = z-node->getZ()*CHUNK_SIZE;

        // X - Achse
        if(p_chunk_x < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_x >= CHUNK_SIZE ) {
            node = node->next;
            continue;
        }
        // Y - Achse
        if(p_chunk_y < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_y >= CHUNK_SIZE ) {
            node = node->next;
            continue;
        }
        // Z - Achse
        if(p_chunk_z < 0 ) {
            node = node->next;
            continue;
        }
        if(p_chunk_z >= CHUNK_SIZE) {
            node = node->next;
            continue;
        }
        return node;
    }
    return NULL;
}

void world::SetTile( Chunk *chunk, int tile_x, int tile_y, int tile_z, int ID) {
    if( chunk == NULL) {
        printf( "SetTile: Cant set tile!\n");
        return;
    }
    int chunk_x = chunk->getX()*CHUNK_SIZE;
    int chunk_y = chunk->getY()*CHUNK_SIZE;
    int chunk_z = chunk->getZ()*CHUNK_SIZE;

    // Letzte pr�fung
    //chunk->CreateTile( tile_x-chunk_x, tile_y-chunk_y, tile_z-chunk_z, ID);
    chunk->set( tile_x-chunk_x, tile_y-chunk_y, tile_z-chunk_z, ID);
}

void world::process_thrend() {
    for( int i = 0; i < (int)p_creatingList.size(); i++)
    {
        Chunk *l_node = getChunk( p_creatingList[i].x, p_creatingList[i].y, p_creatingList[i].z);
        if( l_node == NULL) {
            l_node = createChunk( p_creatingList[i].x, p_creatingList[i].y, p_creatingList[i].z);
            p_updateNodeList.push_back( l_node);
            p_creatingList.erase( p_creatingList.begin()+ i);
            break;
        }
    }

    for( auto l_pos:p_deletingList)
    {
        Chunk *l_node = getChunk( l_pos.x, l_pos.y, l_pos.z);
        if( l_node != NULL)
            deleteChunk( l_node);
    }
    p_deletingList.clear();

    // be �nderung Updaten
    UpdateArray();
}

void world::process() {
    // Reset Idle time -> bis der Chunk sich selbst l�scht
    Chunk *node = Chunks;
    for( ;; ) {
        if( node == NULL)
            break;
        node->ResetTimeIdle();
        node = node->next;
    }
}

void world::deleteChunks( Chunk* chunk) {
    if( chunk == NULL )
        return;
    if( chunk->next )
        deleteChunks( chunk->next );
    deleteChunk( chunk);
}

void world::deleteChunk( Chunk* node) {
    if( node == NULL)
        return;
    if( node->GetVbo() )
        node->DestoryVbo();
    // l�schen des chunks
    destoryChunk( node->getX(), node->getY(), node->getZ());
    //DeletingChunkList.push_back( Vector);
}

Chunk *world::createChunk( int pos_x, int pos_y, int pos_z) {
    Chunk *node;

    // Chunk erstellen
    Timer timer;
    timer.Start();
    node = new Chunk( pos_x, pos_y, pos_z, 102457, p_blocklist);

    // Landscape erstellen

    Landscape_Generator( node, p_blocklist);
    //printf( "Create Chunk %dms %dkb\n", timer.GetTicks(), CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_DEPTH*sizeof( Tile)/1024);

    // seiten finden
    Chunk *snode;
    if( CheckChunk( pos_x+1, pos_y, pos_z)) {
        snode = getChunk( pos_x+1, pos_y, pos_z);
        snode->back = node;
        node->front = snode;
    }
    if( CheckChunk( pos_x-1, pos_y, pos_z)) {
        snode = getChunk( pos_x-1, pos_y, pos_z);
        snode->front = node;
        node->back = snode;
    }
    if( CheckChunk( pos_x, pos_y+1, pos_z)) {
        snode = getChunk( pos_x, pos_y+1, pos_z);
        snode->down = node;
        node->up = snode;
    }
    if( CheckChunk( pos_x, pos_y-1, pos_z)) {
        snode = getChunk( pos_x, pos_y-1, pos_z);
        snode->up = node;
        node->down = snode;
    }
    if( CheckChunk( pos_x, pos_y, pos_z+1)) {
        snode = getChunk( pos_x, pos_y, pos_z+1);
        snode->left = node;
        node->right = snode;
    }
    if( CheckChunk( pos_x, pos_y, pos_z-1)) {
        snode = getChunk( pos_x, pos_y, pos_z-1);
        snode->right = node;
        node->left = snode;
    }
    //printf( "Add Chunk %d %d %d %d\n", p_chunk_amount+1, node.chunk->GetX(), node.chunk->GetY(), node.chunk->GetZ());
    //printf( "Add Chunk %d: %d %d %d\n", p_chunk_amount+1, node->GetX(), node->GetY(), node->GetZ());
    p_chunk_amount++; // Chunks mitz�hlen

    Chunk *tmp = Chunks;
    int z = 0;
    // Falls hauptzweik nicht exestiert erstllen
    if( Chunks == NULL) {
        Chunks = node;
    } else {
        while( tmp->next != NULL ) {
            tmp = tmp->next;
            z++;
        }
        tmp->next = node;
    }
    return node;
}

void world::destoryChunk( int pos_x, int pos_y, int pos_z) {
    Timer timer;
    timer.Start();
    Chunk *tmp = Chunks;
    Chunk *tmpOld = NULL;
    for( ;; ) {
        if( tmp == NULL)
            break;
         if( tmp->getX() == pos_x &&
             tmp->getY() == pos_y &&
             tmp->getZ() == pos_z ) {
            if( tmpOld == NULL)
                Chunks = tmp->next;
            else tmpOld->next = tmp->next;

            // chunk seiten l�schen
            Chunk *snode;
            if( CheckChunk( pos_x+1, pos_y, pos_z)) {
                snode = getChunk( pos_x+1, pos_y, pos_z);
                snode->back = NULL;
            }
            if( CheckChunk( pos_x-1, pos_y, pos_z)) {
                snode = getChunk( pos_x-1, pos_y, pos_z);
                snode->front = NULL;
            }
            if( CheckChunk( pos_x, pos_y+1, pos_z)) {
                snode = getChunk( pos_x, pos_y+1, pos_z);
                snode->down = NULL;
            }
            if( CheckChunk( pos_x, pos_y-1, pos_z)) {
                snode = getChunk( pos_x, pos_y-1, pos_z);
                snode->up = NULL;
            }
            if( CheckChunk( pos_x, pos_y, pos_z+1)) {
                snode = getChunk( pos_x, pos_y, pos_z+1);
                snode->left = NULL;
            }
            if( CheckChunk( pos_x, pos_y, pos_z-1)) {
                snode = getChunk( pos_x, pos_y, pos_z-1);
                snode->right = NULL;
            }
            // l�schen
            delete tmp;
            // runterz�hlen
            p_chunk_amount--;
            break;
        }
        tmpOld = tmp;
        tmp = tmp->next;
    }
}

bool world::CheckChunk( int X, int Y, int Z) {
    Chunk *tmp = Chunks;
    for( ;; ) {
        if( tmp == NULL)
            break;
         if( tmp->getX() == X &&
             tmp->getY() == Y &&
             tmp->getZ() == Z )
             return true;
        tmp = tmp->next;
    }
    return false;
}



Chunk* world::getChunk( int X, int Y, int Z) {
    Chunk *tmp = Chunks;
    for( ;; ) {
        if( tmp == NULL)
            break;
         if( tmp->getX() == X &&
             tmp->getY() == Y &&
             tmp->getZ() == Z )
             return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void world::addChunk( glm::tvec3<int> pos ) {
    p_creatingList.push_back( pos);
}
void world::addDeleteChunk( glm::tvec3<int> pos ) {
    p_deletingList.push_back( pos);
}

float p = 0.0f;
bool b = true;

Camera *cam;

#define ota_size 200.0f

void world::draw( graphic *graphic, Config *config, glm::mat4 viewProjection) {
    int g_width = graphic->getWidth();
    int g_height = graphic->getHeight();

    // get shader
    Shader* l_shader = graphic->getVoxelShader();

    int i = 0;
    for( auto l_node:p_updateNodeList)
    {
        if( l_node->GetArrayChange())
            l_node->updateVbo( l_shader);
    }
    p_updateNodeList.clear();

    // einstellung des shaders
    l_shader->Bind();
    l_shader->SetSize( (graphic->getDisplay()->getTilesetHeight()/16), ( graphic->getDisplay()->getTilesetWidth()/16) );

    p_tilemap->Bind();

    // How to not code a draw function... well played alex
    if( config->GetSupersampling() ) {
        for( int alpha_cut = 0; alpha_cut < 1; alpha_cut++) {
            for(int i = 0; i < 4; i++) {
                glm::vec3 shift = glm::vec3((i % 4) * 0.5 / g_width, (i / 2) * 0.5 / g_height, 0);
                glm::mat4 aa = glm::translate(glm::mat4(1.0f), shift);

                // Zeichnen
                drawTransparency( l_shader, viewProjection, alpha_cut == 1 ? false : true, aa);
                glAccum(i ? GL_ACCUM : GL_LOAD, 0.25f);
            }
            // zusammenrechnen
            glAccum(GL_RETURN, 1);
        }
        // Zeichne Transperenz wenn gew�nscht
        if( config->GetTransparency())
            drawTransparency( l_shader, viewProjection, false);
    } else {
        if( config->GetTransparency() ) {
            // Transparent zeichnen
            drawTransparency( l_shader, viewProjection, true);
            drawTransparency( l_shader, viewProjection, false);
        } else {
            // Ohne Transperenz -> Sehr schnell
            graphic->getVoxelShader()->SetAlpha_cutoff( 0.0f);
            glDisable( GL_BLEND);
            drawNode( l_shader, viewProjection);
            glEnable( GL_BLEND);
        }
    }
    // disable all Vertex arrays
    l_shader->disableArray( 0);
    l_shader->disableArray( 1);
    glUseProgram( 0 );
}

void world::drawTransparency( Shader* shader, glm::mat4 viewProjection, bool alpha_cutoff, glm::mat4 aa) {
    // shader einstellen
    if( alpha_cutoff) {
        glDepthMask(true);
        shader->SetAlpha_cutoff( 0.99f);
    } else {
        glDepthMask(false);
        shader->SetAlpha_cutoff( 1.10f);
    }
    // Draw node
    drawNode( shader,viewProjection, aa);
    glDepthMask(true);
}

void world::drawNode( Shader* shader, glm::mat4 viewProjection, glm::mat4 aa) {
    Chunk *node = Chunks;
    for( ;; ) {
        if( node == NULL)
            break;
        // Update Chunk if Change
        if( node->GetArrayChange())
            p_updateNodeList.push_back(node); // for later now we draw
        if( node->GetUpdateOnce() && node->getAmount() != 0 && node->GetUpdateVboOnce()) {
            if( SDL_GetTicks() - node->GetTimeIdle() > WORLD_TILE_IDLE_TIME ) {
                deleteChunk( node );
            } else {
                node->draw( shader, viewProjection, aa);
            }
        }
        node = node->next;
    }
}
void world::UpdateArray() {
    UpdateArrayNode();
}

void world::UpdateArrayNode() {
    Chunk *node = Chunks;
    for( ;; ) {
        if( node == NULL)
            break;
        // Update Chunk if Change
        if( (node->GetChanged() || !node->GetUpdateOnce() ) && node->IsDrawable()) {// Update Chunk -> es hat sich was ver�ndert
            node->UpdateArray( p_blocklist); //, node->back, node->front, node->left, node->right, node->up, node->down);
        }
        node = node->next;
    }
}
