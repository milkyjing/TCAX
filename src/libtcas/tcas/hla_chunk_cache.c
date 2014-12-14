/*
 *  Copyright (C) 2011 milkyjing <milkyjing@gmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  milkyjing
 *
 */

#include "hla_chunk_cache.h"


/* Cache normal frame chunks */

static int _rb_compare(const TCAS_pNormalFrameChunk a, const TCAS_pNormalFrameChunk b, const void *param) {
    if (a->offset < b->offset)
        return -1;
    else if (a->offset > b->offset)
        return 1;
    else
        return 0;
}

static void _rb_clean(TCAS_pNormalFrameChunk p, const void *param) {
    free(p->chunk.pos_and_color);
    free(p);
}

static void _libtcas_parse_linearized_index_streams_cache_normal(TCAS_pFileCache pFileCache, TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, RbTreePtr cacheNormal, TCAS_pQueuedFrameChunks pQfc) {
    tcas_u32 i, count;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    long result;
    TCAS_OrderedIndex key;
    TCAS_pOrderedIndex pOrderedIndex;
    TCAS_NormalFrameChunk fChunkKey;
    TCAS_pNormalFrameChunk pFChunk;   /* pointer to the element of the cacheNormal */
    TCAS_FrameChunk fc;     /* element of the vector */
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {    /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        key.first = n;
        key.last = n;
        result = vector_binary_search(linearIndexsTemp, &key, (vector_compare_func)libtcas_check_normal_index_validity, NULL);
        if (-1 != result) {  /* we find it */
            pOrderedIndex = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, result, NULL);
            fChunkKey.offset = pOrderedIndex->offset;   /* set the key value */
            if (!(pFChunk = (TCAS_pNormalFrameChunk)rbtree_find(cacheNormal, &fChunkKey))) {      /* the chunk hasn't been cached yet */
                pFChunk = (TCAS_pNormalFrameChunk)malloc(sizeof(TCAS_NormalFrameChunk));
                pFChunk->offset = pOrderedIndex->offset;
                pFChunk->ref_count = 1; /* initial reference count */
                if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pHeader->flag))
                    //libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndex->offset) << 2, &pFChunk->chunk);
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndex->offset) << 2, &pFChunk->chunk);
                else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pHeader->flag))
                    //libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndex->offset) << 2, &pFChunk->chunk);
                    libtcas_read_specified_chunk_ex_z(pFileCache, ((tcas_u64)pOrderedIndex->offset) << 2, &pFChunk->chunk);
                pFChunk->chunk.startTime = GETINDEXID(pOrderedIndex->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
                pFChunk->chunk.endTime = GETLAYER(pFChunk->chunk.cltp);
                rbtree_insert(cacheNormal, pFChunk);       /* pFChunk is rb_entry_ptr, since we store FChunk in the tree */
            } else
                pFChunk->ref_count++;
            fc.is_intermediate = 0;
            fc.offset = pOrderedIndex->offset;
            fc.pChunk = &pFChunk->chunk;    /* just a reference */
            fc.pRef = &pFChunk->ref_count;
            vector_push_back(pQfc->chunks, &fc);  /* we store TCAS_FrameChunk in the vector */
        }
    }
}

/* Cache key frame chunks, the vector stores references to normal frame chunks, but stores real content of intermediate frame chunks */

static int _avl_compare(const TCAS_pKeyFrameChunk a, const TCAS_pKeyFrameChunk b, const void *param) {
    if (a->offset < b->offset)
        return -1;
    else if (a->offset > b->offset)
        return 1;
    else
        return 0;
}

static void _avl_clean(TCAS_pKeyFrameChunk p, const void *param) {
    free(p->chunkPair.start.pos_and_color);
    free(p->chunkPair.end.pos_and_color);
    free(p);
}

static void _libtcas_parse_linearized_index_streams_cache_key(TCAS_pFileCache pFileCache, TCAS_pHeader pHeader, const TCAS_IndexStreamsPtr indexStreams, tcas_u32 n, AvlTreePtr cacheKey, TCAS_pQueuedFrameChunks pQfc) {
    tcas_u32 i, count;
    TCAS_LinearIndexsPtr linearIndexsTemp;
    long result;
    TCAS_OrderedIndex key;
    TCAS_pOrderedIndex pOrderedIndexStart;
    TCAS_pOrderedIndex pOrderedIndexEnd;
    TCAS_KeyFrameChunk fChunkKey;
    TCAS_pKeyFrameChunk pFChunk;   /* pointer to the element of the cacheKey */
    TCAS_FrameChunk fc;     /* element of the vector */
    count = vector_get_count(indexStreams);
    for (i = 0; i < count; i++) {    /* check every index stream */
        vector_retrieve(indexStreams, i, &linearIndexsTemp);  /* get the ith index stream, low copy, do not free linearIndexsTemp */
        key.first = n;
        key.last = n;
        result = vector_binary_search_interval(linearIndexsTemp, &key, (vector_compare_interval_func)libtcas_check_key_index_validity);
        if (-1 != result) {  /* we find it */
            pOrderedIndexStart = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, result, NULL);
            pOrderedIndexEnd = (TCAS_pOrderedIndex)vector_retrieve(linearIndexsTemp, result + 1, NULL);
            fChunkKey.offset = pOrderedIndexStart->offset;   /* set the key value */
            if (!(pFChunk = (TCAS_pKeyFrameChunk)avltree_find(cacheKey, &fChunkKey))) {      /* the chunk hasn't been cached yet */
                pFChunk = (TCAS_pKeyFrameChunk)malloc(sizeof(TCAS_KeyFrameChunk));
                pFChunk->offset = pOrderedIndexStart->offset;
                pFChunk->ref_count = 1; /* initial reference count */
                if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pHeader->flag)) {
                    //libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexStart->offset) << 2, &pFChunk->chunkPair.start);
                    //libtcas_read_specified_chunk(pFile, ((tcas_u64)pOrderedIndexEnd->offset) << 2, &pFChunk->chunkPair.end);
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexStart->offset) << 2, &pFChunk->chunkPair.start);
                    libtcas_read_specified_chunk_ex(pFileCache, ((tcas_u64)pOrderedIndexEnd->offset) << 2, &pFChunk->chunkPair.end);
                } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pHeader->flag)) {
                    //libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexStart->offset) << 2, &pFChunk->chunkPair.start);
                    //libtcas_read_specified_chunk_z(pFile, ((tcas_u64)pOrderedIndexEnd->offset) << 2, &pFChunk->chunkPair.end);
                    libtcas_read_specified_chunk_ex_z(pFileCache, ((tcas_u64)pOrderedIndexStart->offset) << 2, &pFChunk->chunkPair.start);
                    libtcas_read_specified_chunk_ex_z(pFileCache, ((tcas_u64)pOrderedIndexEnd->offset) << 2, &pFChunk->chunkPair.end);
                }
                avltree_insert(cacheKey, pFChunk);       /* pFChunk is rb_entry_ptr, since we store FChunk in the tree */
            } else
                pFChunk->ref_count++;
            /* generate intermediate frame chunk and also put it in the vector */
            libtcas_key_frame_chunk_add_dips(&pFChunk->chunkPair.start, &pFChunk->chunkPair.end);
            if (1 == GETTYPE(pFChunk->chunkPair.start.cltp))
                libtcas_generate_intermediate_frame_chunk_fast(&pFChunk->chunkPair.start, &pFChunk->chunkPair.end, pOrderedIndexStart->first, pOrderedIndexEnd->last, n, &fc.pChunk);
            else if (2 == GETTYPE(pFChunk->chunkPair.start.cltp))
                libtcas_generate_intermediate_frame_chunk(&pFChunk->chunkPair.start, &pFChunk->chunkPair.end, pOrderedIndexStart->first, pOrderedIndexEnd->last, n, &fc.pChunk);
            fc.pChunk->startTime = GETINDEXID(pOrderedIndexStart->id_pair);  /* note that with the help of indexs startTime and endTime are no longer needed, we can use them to do some other business */
            fc.pChunk->endTime = GETLAYER(fc.pChunk->cltp);
            fc.is_intermediate = 1;
            fc.offset = pOrderedIndexStart->offset;
            fc.pRef = &pFChunk->ref_count;
            vector_push_back(pQfc->chunks, &fc);  /* we store TCAS_FrameChunk in the vector */
        }
    }
}

static TCAS_Error_Code _libtcas_frame_chunks_cache_proc_args_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, int fileCacheSize, TCAS_pFrameChunksCacheProcArgs pArgs) {
    TCAS_Error_Code error;
    TCAS_pOrderedIndex pOrderedIndexs;
    error = libtcas_open_file(&pArgs->file, filename, tcas_file_open_existing);
    if (tcas_error_success != error) {
        printf("Error: can not open the TCAS file.\n");
        return error;
    }
    error = libtcas_read_header(&pArgs->file, &pArgs->header, 0);
    if (tcas_error_success != error) {
        libtcas_close_file(&pArgs->file);
        printf("Error: can not read the TCAS file.\n");
        return error;
    }
    if (TCAS_FILE_TYPE_COMPRESSED == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_create_ordered_index(&pArgs->file, &pArgs->header, fpsNumerator, fpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, step1.\n");
            return error;
        }
    } else if (TCAS_FILE_TYPE_COMPRESSED_Z == GETHI16B(pArgs->header.flag)) {
        if (0 == pArgs->header.chunks)
            libtcas_get_min_max_time_and_chunks_z(&pArgs->file, &pArgs->header.minTime, &pArgs->header.maxTime, &pArgs->header.chunks);
        error = libtcas_create_ordered_index_z(&pArgs->file, &pArgs->header, fpsNumerator, fpsDenominator, &pOrderedIndexs, NULL);
        if (tcas_error_success != error) {
            libtcas_close_file(&pArgs->file);
            printf("Error: can not parse the TCAS file, step1.\n");
            return error;
        }
    } else {
        libtcas_close_file(&pArgs->file);
        printf("Error: tcasfilter does no support this TCAS file type yet.\n");
        return tcas_error_file_type_not_support;
    }
    pArgs->indexStreams = libtcas_linearize_ordered_indexs(pOrderedIndexs, pArgs->header.chunks, NULL);
    free(pOrderedIndexs);
    libtcas_init_file_cache(&pArgs->fileCache, fileCacheSize, &pArgs->file);
    pArgs->width = width;
    pArgs->height = height;
    return tcas_error_success;
}

static void _vector_clean(TCAS_pFrameChunk pFrameChunk) {
    if (pFrameChunk->is_intermediate) {
        free(pFrameChunk->pChunk->pos_and_color);
        free(pFrameChunk->pChunk);
    }
}

DWORD WINAPI _libtcas_create_frame_chunks_with_linearized_index_worker_proc(LPVOID lpParam) {
    TCAS_pFrameChunksCache pFcc;
    TCAS_pFrameChunksCacheProcArgs pArgs;
    TCAS_QueuedFrameChunks qfc;
    pFcc = (TCAS_pFrameChunksCache)lpParam;
    pArgs = &pFcc->fccpArgs;    /* a shortcut */
    while (pFcc->active) {
        WaitForSingleObject(pFcc->semQueue, INFINITE);    /* wait for room to store the frame */
        if (pFcc->active) {    /* still active */
            EnterCriticalSection(&pFcc->lock);
            if (queue_get_count(pFcc->qFrameChunks) == 0)
                qfc.id = pArgs->n;
            else {
                queue_retrieve_front(pFcc->qFrameChunks, &qfc);
                if (qfc.id == pArgs->n)
                    qfc.id += queue_get_count(pFcc->qFrameChunks);
                else
                    qfc.id = pArgs->n;
            }
            vector_create(&qfc.chunks, sizeof(TCAS_FrameChunk), 0, NULL, (vector_cleanup_func)_vector_clean);
            _libtcas_parse_linearized_index_streams_cache_normal(&pArgs->fileCache, &pArgs->header, pArgs->indexStreams, qfc.id, pArgs->cacheNormal, &qfc);
            _libtcas_parse_linearized_index_streams_cache_key(&pArgs->fileCache, &pArgs->header, pArgs->indexStreams, qfc.id, pArgs->cacheKey, &qfc);
            queue_push_rear(pArgs->qFrameChunks, &qfc);
            LeaveCriticalSection(&pFcc->lock);
            ReleaseSemaphore(pFcc->semFrames, 1, NULL);    /* add an available frame */
        }
    }
    return 0;
}

TCAS_Error_Code libtcas_frame_chunks_cache_init(const char *filename, tcas_u32 fpsNumerator, tcas_u32 fpsDenominator, tcas_u16 width, tcas_u16 height, int maxFrameCount, int fileCacheSize, TCAS_pFrameChunksCache pFcc) {
    TCAS_Error_Code error;
    error = _libtcas_frame_chunks_cache_proc_args_init(filename, fpsNumerator, fpsDenominator, width, height, fileCacheSize, &pFcc->fccpArgs);
    if (tcas_error_success != error)
        return error;
    pFcc->minFrame = (tcas_u32)((tcas_u64)pFcc->fccpArgs.header.minTime * fpsNumerator / (fpsDenominator * 1000)) + 1;
    pFcc->maxFrame = (tcas_u32)((tcas_u64)pFcc->fccpArgs.header.maxTime * fpsNumerator / (fpsDenominator * 1000)) + 1;
    pFcc->maxFrameCount = maxFrameCount;
    pFcc->cacheNormal = rbtree_create((rb_compare_func)_rb_compare, NULL, (rb_cleanup_func)_rb_clean, NULL);
    pFcc->fccpArgs.cacheNormal = pFcc->cacheNormal;
    pFcc->cacheKey = avltree_create((avl_compare_func)_avl_compare, NULL, (avl_cleanup_func)_avl_clean, NULL);
    pFcc->fccpArgs.cacheKey = pFcc->cacheKey;
    queue_create(&pFcc->qFrameChunks, sizeof(TCAS_QueuedFrameChunks), pFcc->maxFrameCount, NULL, NULL);    /* stores pointers to frame buffers */
    pFcc->fccpArgs.qFrameChunks = pFcc->qFrameChunks;
    pFcc->semQueue = CreateSemaphore(NULL, 0, pFcc->maxFrameCount, NULL);
    pFcc->semFrames = CreateSemaphore(NULL, 0, pFcc->maxFrameCount, NULL);
    InitializeCriticalSection(&pFcc->lock);
    pFcc->active = 1;
    pFcc->tdWorker = CreateThread(NULL, 0, _libtcas_create_frame_chunks_with_linearized_index_worker_proc, pFcc, 0, &pFcc->threadID);
    return tcas_error_success;
}

void libtcas_frame_chunks_cache_run(TCAS_pFrameChunksCache pFcc) {
    pFcc->fccpArgs.n = pFcc->minFrame;
    ReleaseSemaphore(pFcc->semQueue, pFcc->maxFrameCount, NULL);
}

int libtcas_frame_chunks_cache_get(TCAS_pFrameChunksCache pFcc, tcas_u32 n, VectorPtr *pv) {
    TCAS_QueuedFrameChunks qfc;
    WaitForSingleObject(pFcc->semFrames, INFINITE);    /* wait for available frames */
    EnterCriticalSection(&pFcc->lock);
    queue_pop_front(pFcc->qFrameChunks, &qfc);
    if (qfc.id == n) {    /* the returned frame is just what we want, i.e., the playback moves in a smooth progress */
        pFcc->fccpArgs.n = qfc.id + 1;    /* require for next frames */
        LeaveCriticalSection(&pFcc->lock);
        *pv = qfc.chunks;
        ReleaseSemaphore(pFcc->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 1;
    } else {    /* we should drop all the frames cached, by calling this function in a loop */
        pFcc->fccpArgs.n = n;
        LeaveCriticalSection(&pFcc->lock);
        libtcas_frame_chunks_cache_free_frame(pFcc, qfc.chunks);        /* Note that, there is already a lock in this function */
        *pv = NULL;
        ReleaseSemaphore(pFcc->semQueue, 1, NULL);    /* we consumed one frame, so there is an available room we left */
        return 0;
    }
}

static int _libtcas_compare_order_layer_fc(const TCAS_pFrameChunk src, const TCAS_pFrameChunk key) {
    if (src->pChunk->endTime > src->pChunk->endTime) return 1;  /* compare layer, in fact endTime here contains layer info, since with the help of indexs we no longer use startTime and endTime data members we can use them to contain some other helpful information */
    else if (src->pChunk->endTime < src->pChunk->endTime) return -1;
    else {
        if (src->pChunk->startTime > src->pChunk->startTime) return 1;  /* compare id, note that id can not be the same */
        else return -1;
    }
}

TCAS_Error_Code libtcas_create_frame_with_chunks_cached(TCAS_pFrameChunksCache pFcc, VectorPtr vFrameChunks, tcas_u16 targetWidth, tcas_u16 targetHeight, tcas_byte **pBuf) {
    tcas_u16 width, height;
    tcas_u32 i, pitch, size;
    tcas_byte *rgba;
    tcas_u32 chunkCount;  /* number of effective chunks */
    TCAS_pFrameChunk pFChunk;
    width = GETPOSX(pFcc->fccpArgs.header.resolution);
    height = GETPOSY(pFcc->fccpArgs.header.resolution);
    pitch = width * (sizeof(tcas_byte) << 2);
    size = height * pitch;
    rgba = (tcas_byte *)malloc(size);  /* create frame buffer */
    memset(rgba, 0, size);  /* we must clear the bits of the frame buffer */
    chunkCount = vector_get_count(vFrameChunks);
    vector_quicksort(vFrameChunks, 0, chunkCount - 1, (vector_compare_func)_libtcas_compare_order_layer_fc);
    for (i = 0; i < chunkCount; i++) {
        pFChunk = (TCAS_pFrameChunk)vector_retrieve(vFrameChunks, i, NULL);
        libtcas_convert_chunks_to_rgba(pFChunk->pChunk, width, height, rgba);
    }
    libtcas_frame_chunks_cache_free_frame(pFcc, vFrameChunks);
    if (!(width == targetWidth && height == targetHeight)) {
        tcas_byte *src = rgba;
        libtcas_resample_rgba(src, width, height, &rgba, targetWidth, targetHeight);
        free(src);
    }
    *pBuf = rgba;
    return tcas_error_success;
}

void libtcas_frame_chunks_cache_free_frame(TCAS_pFrameChunksCache pFcc, VectorPtr vFrameChunks) {
    tcas_u32 i, count;
    TCAS_pFrameChunk pFChunk;   /* pointer to the element of the treeCache */
    TCAS_NormalFrameChunk nfcKey;
    TCAS_KeyFrameChunk kfcKey;
    count = vector_get_count(vFrameChunks);
    EnterCriticalSection(&pFcc->lock);
    for (i = 0; i < count; i++) {
        pFChunk = (TCAS_pFrameChunk)vector_retrieve(vFrameChunks, i, NULL);
        (*pFChunk->pRef)--;
        if (0 == *pFChunk->pRef) {
            if (pFChunk->is_intermediate) {
                kfcKey.offset = pFChunk->offset;
                avltree_delete(pFcc->cacheKey, &kfcKey);
            } else {
                nfcKey.offset = pFChunk->offset;
                rbtree_delete(pFcc->cacheNormal, &nfcKey);
            }
        }
    }
    LeaveCriticalSection(&pFcc->lock);
    vector_destroy(vFrameChunks);
}

static void _libtcas_frame_chunks_cache_proc_args_fin(TCAS_pFrameChunksCacheProcArgs pArgs) {
    if (pArgs->indexStreams)
        vector_destroy(pArgs->indexStreams);
    libtcas_fin_file_cache(&pArgs->fileCache);
    libtcas_close_file(&pArgs->file);
}

void libtcas_frame_chunks_cache_fin(TCAS_pFrameChunksCache pFcc) {
    tcas_u32 i, count;
    TCAS_QueuedFrameChunks qfc;
    pFcc->active = 0;
    ReleaseSemaphore(pFcc->semQueue, 1, NULL);   /* to let the thread quit */
    WaitForSingleObject(pFcc->tdWorker, INFINITE);
    CloseHandle(pFcc->tdWorker);
    _libtcas_frame_chunks_cache_proc_args_fin(&pFcc->fccpArgs);
    CloseHandle(pFcc->semFrames);
    CloseHandle(pFcc->semQueue);
    DeleteCriticalSection(&pFcc->lock);
    rbtree_destroy(pFcc->cacheNormal);
    avltree_destroy(pFcc->cacheKey);
    count = queue_get_count(pFcc->qFrameChunks);
    for (i = 0; i < count; i++) {
        queue_pop_front(pFcc->qFrameChunks, &qfc);
        vector_destroy(qfc.chunks);
    }
    queue_destroy(pFcc->qFrameChunks);
}

