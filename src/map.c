#include <math.h>
#include <raylib.h>
#include <string.h>
#include "map.h"

// external
void fetch_map_tile(int x, int y, int zoom, const char *savedir);

#define MAX(_X, _Y) (_X) > (_Y) ? (_X) : (_Y)
#define ZOOM_FROM_SIZE(_W) (int)log2(360/_W)

#define CACHE ".cache"

Vector2 lon_lat_to_tile(float lon, float lat, int zoom) {
    float lat_rad = lat * M_PI / 180.0;
    int n = 1 << zoom;  // 2^zoom
    Vector2 v;
    v.x = (int)((lon + 180.0) / 360.0) * n;
    v.y = (int)(((1.0 - log(tan(lat_rad) + (1.0/cos(lat_rad))) / M_PI))/ 2.0) * n;
    return v;
}

// top left lon/lat of given tile
Vector2 tile_to_lon_lat(int x, int y, int zoom) {
    int n = 1 << zoom;  // 2^zoom
    Vector2 v;
    v.x = x / (float)n * 360.0 - 180.0;
    float lat_rad = atan(sinh(M_PI * (1 - 2 * y / (float)n)));
    v.y = lat_rad * 180.0 / M_PI;

    return v;
}

static MapBB get_gpx_map_bb(Vector2 *points, size_t num_points) {
    MapBB bb = {points[0], points[0]};

    // Find limits
    for (size_t i = 1; i < num_points; i++) {
        if (points[i].x < bb.min.x) bb.min.x = points[i].x;
        else if (points[i].x > bb.max.x) bb.max.x = points[i].x;
        if (points[i].y < bb.min.y) bb.min.y = points[i].y;
        else if (points[i].y > bb.min.y) bb.min.y = points[i].y;
    }

    // Scale to square bounding box
    float size;
    if ((bb.max.x - bb.min.x) > (bb.max.y - bb.min.y)) {
        size = (bb.max.x - bb.min.x);
        float height = (bb.max.y - bb.min.y);
        float diff = (size - height) / 2;
        bb.max.y += diff;
        bb.min.y -= diff;
    } else {
        size = (bb.max.y - bb.min.y);
        float width = (bb.max.x - bb.min.x);
        float diff = (size - width) / 2;
        bb.max.x += diff;
        bb.min.x -= diff;
    }
    float scaled_size = size * MAP_BB_LIMIT_FACTOR;
    float diff = (scaled_size - size) / 2;
    bb.min.x -= diff;
    bb.min.y -= diff;
    bb.max.x += diff;
    bb.max.y += diff;

    return bb;
}

void fetch_map_tiles(GpxData gpx_data) {
    MapBB bb = get_gpx_map_bb(gpx_data.points, gpx_data.num_points);

    const char *fname = GetFileNameWithoutExt(gpx_data.filename);
    const char *base_dir = TextFormat("%s/%s", CACHE, fname);
    MakeDirectory(base_dir);

    float width = bb.max.x - bb.max.x;
    int base_zoom = ZOOM_FROM_SIZE(width);

    for (int l = 0; l < LEVELS_OF_ZOOM; l++) {
        int zoom = base_zoom + l;
        Vector2 min_tile = lon_lat_to_tile(bb.min.x, bb.max.y, zoom);
        Vector2 max_tile = lon_lat_to_tile(bb.max.x, bb.max.y, zoom);

        const char *save_dir = TextFormat("%s/z%d_tiles", base_dir, zoom);
        MakeDirectory(save_dir);

        for (int y = min_tile.y; y < max_tile.y; y++) {
            for (int x = min_tile.x; x < max_tile.x; x++) {
                fetch_map_tile(x, y, zoom, base_dir);
            }
        }

    }
    
}

Texture *load_tiles(const char *gpx_dir, int zoom_level) {
}

GpxData get_gpx_data(const char *file_name) {
    GpxData data;
    data.filename = file_name;
    data.num_points = 4;
    data.hr_values = NULL;

    // test
    Vector2 tmp[4] = {
        (Vector2){-34.06f, 19.622f},
        (Vector2){-34.056f, 19.600f},
        (Vector2){-34.043f, 19.609f},
        (Vector2){-34.0617f, 19.603f},
    };

    size_t b = sizeof(Vector2) * data.num_points;
    data.points = malloc(b);
    memcpy(data.points, tmp, b);

    return data;
}

void free_gpx_data(GpxData data);
