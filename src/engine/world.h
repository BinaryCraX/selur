#ifndef WORLD_H
#define WORLD_H 1

#include "../graphic/graphic.h"
#include "chunk.h"
#include "block.h"
#include "landscape_generator.h"
#include "../engine/config.h"
//#include "../graphic/shadowmap.h"

#define WORLD_TILE_IDLE_TIME 2*1000 //2s

#define WORLD_TEST_FACTOR 0

Uint32 thrend_worldGenerator( Uint32 interval, void *Paramenter);

class world_data_list {
    public:
        glm::vec3 position;
        bool landscape;
};

class world {
public:
    world( texture *image, block_list* B_List);
    virtual ~world();

    tile *GetTile( int x, int y, int z);
    Chunk *getChunkWithPos( int x, int y, int z);
    void SetTile( Chunk *chunk, int tile_x, int tile_y, int tile_z, int ID);

    void process_thrend_handle();
    void process_thrend_update();
    void process( btDiscreteDynamicsWorld *world);

    void deleteChunks( Chunk* chunk);
    void deleteChunk( Chunk* node);



    Chunk *createChunk( int pos_x, int pos_y, int pos_z, bool generateLandscape = false);
    void destoryChunk( int pos_x, int pos_y, int pos_z);
    bool CheckChunk( int pos_x, int pos_y, int pos_z);
    Chunk* getChunk( int X, int Y, int Z);
    void addChunk( glm::tvec3<int> pos, bool generateLandscape);
    void addDeleteChunk( glm::tvec3<int> pos );

    void draw( graphic *graphic, config *config, glm::mat4 viewProjection) ;
    void drawTransparency( Shader* shader, glm::mat4 viewProjection, bool alpha_cutoff, glm::mat4 aa = glm::mat4(1));
    void drawNode( Shader* shader, glm::mat4 viewProjection,  glm::mat4 aa =  glm::mat4(1));

    void updateArray();

    bool getDestory() { return p_destroy; }
    inline int GetAmountChunks() const { return p_chunk_amount; }
    Chunk *getNode() { return Chunks; }
protected:
private:
    int p_seed;
    bool p_buysvector;
    bool p_world_tree_empty;

    int p_chunk_amount;
    Chunk* Chunks;
    block_list *p_blocklist;
    bool p_destroy;
    texture *p_image;

    SDL_Thread *p_thread_update;
    SDL_Thread *p_thread_handle;

    std::vector<world_data_list> p_creatingList;
    std::vector<world_data_list> p_deletingList;

    //ShadowMap Shadow;
};

#endif // WORLD_H
