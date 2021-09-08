#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"

static Chunk_manager chunk_manager;

CREATE_COMMON(chunk_manager_pos);

void initChunkManager(){
    // Create newx thread
    pthread_create(&chunk_manager.working_th, NULL, thread_loading_chunks, NULL);
    pthread_detach(chunk_manager.working_th);
}

#define abs(x) (((x) >= 0) ? (x) : (x)*(-1))

void removeChunks(){
    struct chunk_list* prev = NULL;
    struct chunk_list* next = NULL;
    for (struct chunk_list* chnk = chunk_manager.chunks; chnk != NULL; chnk = next) {
        //Distance chnk<>player_chnk > dist_affichage ?
        get_lock_chunk_manager_pos();
        if (
            abs(chnk->chunk_x - chunk_manager.actual_chunk_x) > VIEW_DIST || 
            abs(chnk->chunk_y - chunk_manager.actual_chunk_y) > VIEW_DIST ||
            abs(chnk->chunk_z - chunk_manager.actual_chunk_z) > VIEW_DIST
            ) {
                set_lock_chunk_manager_pos();
                //B E  G O N E
                prev->next = chnk->next;
                next = chnk->next;
                free(chnk);
        }
        else {
            set_lock_chunk_manager_pos();
            prev = chnk;
            next = chnk->next;
        }
    }
}

void updateChunks(Vec3 camera_pos){
    // Check if camera change chunk
    if(
        ((int)camera_pos.x)/CHUNK_DIMENSION != chunk_manager.actual_chunk_x ||
        ((int)camera_pos.y)/CHUNK_DIMENSION != chunk_manager.actual_chunk_y ||
        ((int)camera_pos.z)/CHUNK_DIMENSION != chunk_manager.actual_chunk_z ){
        // ICI
        chunk_manager.need_update = 1;
    }
}

#include "metaprog/utils.h"

void updateCoord(int dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz){
    switch (dir)
    {
    case 0:
        (*x)++;
        if (*x == camx+(VIEW_DIST/2))
            dir++;
        break;
    case 1:
        (*y)++;
        if (*y == camy+(VIEW_DIST/2))
            dir++;
        break;
    case 2:
        (*z)++;
        if (*z == camz+(VIEW_DIST/2))
            dir++;
        break;
    case 3:
        (*x)--;
        if (*x == camx-(VIEW_DIST/2))
            dir++;
        break;
    case 4:
        (*y)--;
        if (*y == camy-(VIEW_DIST/2))
            dir++;
        break;
    case 5:
        (*z)--;
        if (*z == camz-(VIEW_DIST/2))
            dir++;
        break;
    case 6:
        while(!__sync_bool_compare_and_swap(&(chunk_manager.need_update), 0, 2));
        __sync_synchronize();
        break;
    default:
        PANIC("How did you get there ?");
        break;
    }
}

void* thread_loading_chunks(void *args){
    
    UNUSED(args);

    int dir = 0;
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    
    get_lock_chunk_manager_pos();
    int32_t camx = chunk_manager.actual_chunk_x;
    int32_t camy = chunk_manager.actual_chunk_y;
    int32_t camz = chunk_manager.actual_chunk_z;
    set_lock_chunk_manager_pos();

    while (1)
    {
        // Check if update
        if(chunk_manager.need_update)
        {
            // Remove far chunks
            removeChunks();

            // Reset var
            dir = 0;
            x = 0;
            y = 0;
            z = 0;

            get_lock_chunk_manager_pos();    
            camx = chunk_manager.actual_chunk_x;
            camy = chunk_manager.actual_chunk_y;
            camz = chunk_manager.actual_chunk_z;
            set_lock_chunk_manager_pos();

        }
        // Shearch next chunk to load
        unsigned char already_exist = 0;
        for (struct chunk_list* chnk = chunk_manager.chunks; !chunk_manager.need_update && chnk != NULL; chnk = chnk->next) {
            if(chnk->chunk_x == camx+x && chnk->chunk_y == camy+y && chnk->chunk_z == camz+z) {
                updateCoord(dir, &x, &y, &z, camx, camy, camz);
                already_exist++;
            }
        }

        // If don't need to load chunk
        if (chunk_manager.need_update || already_exist)
            continue;
        
        // Try to load the chunk
        Chunk* chunk = loadChunkFromFile("", camx+x, camy+y, camz+z);
        
        // Failed to load existing chunk
        if(!chunk){
            // TODO: Generate Chunk
            chunk = malloc(sizeof(Chunk));
        }

        // Create mesh
        updateChunkVertex(chunk);

        // Adding the new chunk to the linked list
        struct chunk_list* chunktmp = chunk_manager.chunks;
        chunk_manager.chunks = malloc((sizeof(struct chunk_list)));
        chunk_manager.chunks->next = chunktmp;
        chunk_manager.chunks->chunk = chunk;
    }

    pthread_exit(NULL);
}