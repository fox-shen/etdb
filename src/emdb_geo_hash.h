#ifndef H_EMDB_GEO_HASH_H
#define H_EMDB_GEO_HASH_H

#define PI      3.1415926
#define DEG2RAD (PI/180)
#define RAD2DEG (180/PI)

#define MAX_LAT             90.0
#define MIN_LAT             -90.0

#define MAX_LONG            180.0
#define MIN_LONG            -180.0

#define NORTH               0
#define EAST                1
#define SOUTH               2
#define WEST                3

#define LENGTH_OF_DEGREE        111100      // meters

// Metric in meters
typedef struct emdb_geo_hash_box_dimension_s emdb_geo_hash_box_dimension_t;
struct emdb_geo_hash_box_dimension_s {
  double height;
  double width;
};

typedef struct emdb_geo_hash_corrd_s emdb_geo_hash_corrd_t;
struct emdb_geo_hash_corrd_s{
  double latitude;
  double longitude;
    
  double north;
  double east;
  double south;
  double west;
  emdb_geo_hash_box_dimension_t dimension;
};

/*
 * Creates a the hash at the specified precision. If precision is set to 0.
 * or less than it defaults to 12.
 */
extern char* 
emdb_geo_hash_encode(double lat, double lng, int precision);

/* 
 * Returns the latitude and longitude used to create the hash along with
 * the bounding box for the encoded coordinate.
 */
extern emdb_geo_hash_corrd_t 
emdb_geo_hash_decode(char* hash);

/* 
 * Return an array of geohashes that represent the neighbors of the passed
 * in value. The neighbors are indexed as followed:
 *
 *                  N, NE, E, SE, S, SW, W, NW
 * 					0, 1,  2,  3, 4,  5, 6, 7
 */ 
extern char** 
emdb_geo_hash_neighbors(char* hash);

/*
 * Returns the width and height of a precision value.
 */
extern emdb_geo_hash_box_dimension_t 
emdb_geo_hash_dimensions_for_precision(int precision);

#endif

