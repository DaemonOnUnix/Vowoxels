#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"
#include "voxelengine/data.h"
#include "metaprog/utils.h"
#include "linear_algebra/perlinnoise.h"

CREATE_COMMON(chunk_manager_pos);

Chunk_manager* initChunkManager(){
    EngineData* data = getEngineData();
    data->chunkM = malloc(sizeof(Chunk_manager));
    data->chunkM->actual_chunk_x = 0;
    data->chunkM->actual_chunk_y = 0;
    data->chunkM->actual_chunk_z = 0;
    data->chunkM->chunks = NULL;
    data->chunkM->need_update = 0;
    pthread_rwlock_init(&data->chunkM->chunkslock, NULL);
    // Create newx thread
    pthread_create(&data->chunkM->working_th, NULL, thread_loading_chunks, NULL);
    pthread_detach(data->chunkM->working_th);

    return data->chunkM;
}

#define abs(x) (((x) >= 0) ? (x) : (x)*(-1))

void insertChunkToChunklist(Chunk *chunk){
    EngineData* data = getEngineData();

    // Malloc new node
    struct chunk_list* new_chunk_list = malloc(sizeof(struct chunk_list));
    new_chunk_list->chunk = chunk;
    new_chunk_list->next = NULL;

    // Add to the start of the list
    pthread_rwlock_wrlock(&data->chunkM->chunkslock);
    if(data->chunkM->chunks){
        new_chunk_list->next = data->chunkM->chunks;
    }
    data->chunkM->chunks = new_chunk_list;
    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    return;
}

Chunk* deleteNextChunklist(struct chunk_list* ch_list_prev){
    EngineData* data = getEngineData();
    struct chunk_list* deleted_chunk_list = NULL;
    Chunk* chunk_to_return = NULL;
    pthread_rwlock_wrlock(&data->chunkM->chunkslock);

    // Take deleted chunk list
    if(!ch_list_prev){
        if(!data->chunkM->chunks)
            return NULL;
        // Delete the first one
        deleted_chunk_list = data->chunkM->chunks;
        // Skip deleted value
        data->chunkM->chunks = data->chunkM->chunks->next;
    }else
    {
        // Delete the next to ch_list_prev
        deleted_chunk_list = ch_list_prev->next;
        // Skip deleted value
        ch_list_prev->next = ch_list_prev->next->next;
    }

    // Keep his chunk
    chunk_to_return = deleted_chunk_list->chunk;   

    // Destroy link to the list
    deleted_chunk_list->next = NULL;
    free(deleted_chunk_list);

    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    return chunk_to_return;
}

void removeChunks(){
    EngineData* data = getEngineData();
    int howmany = 0;
    int howmany2 = 0;
    
    // Remove chunks to far away from Camera
    struct chunk_list* prev = NULL;
    pthread_rwlock_rdlock(&data->chunkM->chunkslock);
    for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL;) {
        pthread_rwlock_unlock(&data->chunkM->chunkslock);
        //Distance chnk<>player_chnk > dist_affichage ?
        get_lock_chunk_manager_pos();
        if (
            abs(chnk->chunk->chunk_x - data->chunkM->actual_chunk_x) > VIEW_DIST+1 ||
            abs(chnk->chunk->chunk_z - data->chunkM->actual_chunk_z) > VIEW_DIST+1
            ) {
                set_lock_chunk_manager_pos();
                howmany++;
                
                // Remove from chunks list
                Chunk* chunk_to_free = deleteNextChunklist(prev);
                freeChunk(chunk_to_free);

                // Set chnk
                chnk = data->chunkM->chunks;
                if (prev)
                    chnk = prev->next;
                
        }else
        {
            set_lock_chunk_manager_pos();
            prev = chnk;
            chnk = chnk->next;
        }
        howmany2++;
        pthread_rwlock_rdlock(&data->chunkM->chunkslock);
    }
    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    LOG_INFO("%u freed on %u!", howmany, howmany2)
}
#include "collections/string.h"

void updateChunks(Vec3 camera_pos){
    EngineData* data = getEngineData();
    // Check if camera change chunk
    if(
        ((int64_t)camera_pos.x)/CHUNK_DIMENSION != data->chunkM->actual_chunk_x ||
        ((int64_t)camera_pos.y)/CHUNK_DIMENSION != data->chunkM->actual_chunk_y ||
        ((int64_t)camera_pos.z)/CHUNK_DIMENSION != data->chunkM->actual_chunk_z ){
        // ICI
        LOG_INFO("Chunks need update")
        data->chunkM->actual_chunk_x = ((int64_t)camera_pos.x)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_y = ((int64_t)camera_pos.y)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_z = ((int64_t)camera_pos.z)/CHUNK_DIMENSION;
        smartstr pogstr pikalul = string("", 40 );
        sprintf(pikalul, "x: %li; y: %li; z: %li", data->chunkM->actual_chunk_x, data->chunkM->actual_chunk_y, data->chunkM->actual_chunk_z);
        glfwSetWindowTitle(data->window, pikalul);
        if (!data->chunkM->need_update)
            removeChunks();
        data->chunkM->need_update = 1;
    }
}

#include <time.h>
void updateCoord(int* dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz){
    UNUSED(camy);
    if(*y<0){
        LOG_INFO("UNDER THE MAP")
        *dir = 5;
    }
    switch (*dir)
    {
    case 0:
        (*y) = camy+VIEW_DIST;
        (*x)++;
        (*dir)++;
        break;
    case 1:
        (*y)--;
        if(*y<=0){
            (*y) = camy+VIEW_DIST;
            (*dir)++;
        }
        break;
    case 2:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*z)++;
            if (*z - camz == *x - camx)
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 3:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*x)--;
            if (*x - camx == -(*z - camz))
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 4:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*z)--;
            if (*x - camx == *z - camz)
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 5:
        if((*y) == 0){
            (*y) = camy+VIEW_DIST;
            (*x)++;
            if (*x - camx == -(*z - camz))
                (*dir)++;
        }else{
            (*y)--;
        }
        break;
    case 6:
        (*y)--;
        if((*y) == 0){
            if(VIEW_DIST > *x - camx)
                (*dir)=0;
            else
                (*dir)++;
        }
        break;
    case 7:
        LOG_INFO("Finish to load all CHUNKS")
        while(!__sync_bool_compare_and_swap(&(data->chunkM->need_update), 1, 2));
        __sync_synchronize();
        break;
    default:
        PANIC("How did you get there ?");
        break;
    }
    LOG_INFO("Updating Coord x: %i; y: %i; z: %i, dir: %i", *x, *y, *z, *dir)
}

void* thread_loading_chunks(void *args){
    EngineData* data = getEngineData();
    UNUSED(args);

    int dir = 0;
    int32_t x = data->chunkM->actual_chunk_x;
    int32_t y = data->chunkM->actual_chunk_y;
    int32_t z = data->chunkM->actual_chunk_z;
    
    get_lock_chunk_manager_pos();
    int32_t camx = data->chunkM->actual_chunk_x;
    int32_t camy = data->chunkM->actual_chunk_y;
    int32_t camz = data->chunkM->actual_chunk_z;
    set_lock_chunk_manager_pos();

    while (1)
    {
        // Check if update
        if(data->chunkM->need_update)
        {
            data->chunkM->need_update = 0;
            LOG_INFO("Chunks updating")
            // Reset var
            dir = 0;
            x = data->chunkM->actual_chunk_x;
            y = data->chunkM->actual_chunk_y;
            z = data->chunkM->actual_chunk_z;

            get_lock_chunk_manager_pos();    
            camx = data->chunkM->actual_chunk_x;
            camy = data->chunkM->actual_chunk_y;
            camz = data->chunkM->actual_chunk_z;
            set_lock_chunk_manager_pos();

        }
        // Shearch next chunk to load
        pthread_rwlock_rdlock(&data->chunkM->chunkslock);
        for (struct chunk_list* chnk = data->chunkM->chunks; !data->chunkM->need_update && chnk != NULL; chnk = chnk->next) {
            if(chnk->chunk->chunk_x == x && chnk->chunk->chunk_y == y && chnk->chunk->chunk_z == z) {
                pthread_rwlock_unlock(&data->chunkM->chunkslock);
                updateCoord(&dir, &x, &y, &z, camx, camy, camz);
                pthread_rwlock_rdlock(&data->chunkM->chunkslock);
            }
        }
        pthread_rwlock_unlock(&data->chunkM->chunkslock);

        if (data->chunkM->need_update){
            continue;
        }
        
        // Try to load the chunk
        Chunk* chunk = loadChunkFromFile("", x, y, z);
        
        // Failed to load existing chunk
        if(!chunk){
            // TODO: Generate Chunk
            // TEST CHUNK
            chunk = newChunk(x, y, z);
            for (size_t _x = 0; _x < CHUNK_DIMENSION; _x++)
            {
                for (size_t _z = 0; _z < CHUNK_DIMENSION; _z++)
                {
                    float p = perlin2d((float)x*CHUNK_DIMENSION + _x,(float)z*CHUNK_DIMENSION + _z, 0.01, 4);
                    
                    float h = mapfloat(p, 0.0f, 1.0f, 0.0f, MAX_HEIGHT);
                    for (size_t _y = 0; _y < CHUNK_DIMENSION && y*CHUNK_DIMENSION+_y < h; _y++)
                    {
                        chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 1;
                    }
                    
                }
                
            }
        }

        // Create mesh
        updateChunkVertex(chunk);

        // Add chunk to render
        insertChunkToChunklist(chunk);

        updateCoord(&dir, &x, &y, &z, camx, camy, camz);
    }

    pthread_exit(NULL);
}

