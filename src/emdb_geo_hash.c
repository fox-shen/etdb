#include "emdb.h"

typedef struct emdb_geo_hash_interval_s emdb_geo_hash_interval_t;
struct emdb_geo_hash_interval_s{
  double high;
  double low;
};

/* Normal 32 characer map used for geohashing */
static char emdb_geo_hash_char_map[] =  "0123456789bcdefghjkmnpqrstuvwxyz";

/*
 *  The follow character maps were created by Dave Troy and used in his Javascript Geohashing
 *  library. http://github.com/davetroy/geohash-js
 */
static const char *emdb_geo_hash_even_neighbors[] = 
                               {"p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                                "bc01fg45238967deuvhjyznpkmstqrwx", 
                                "14365h7k9dcfesgujnmqp0r2twvyx8zb",
                                "238967debc01fg45kmstqrwxuvhjyznp"
                               };

static const char *emdb_geo_hash_odd_neighbors[] = 
                              {"bc01fg45238967deuvhjyznpkmstqrwx", 
                               "p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                               "238967debc01fg45kmstqrwxuvhjyznp",
                               "14365h7k9dcfesgujnmqp0r2twvyx8zb"    
                              };

static const char *emdb_geo_hash_even_borders[] = {"prxz", "bcfguvyz", "028b", "0145hjnp"};
static const char *emdb_geo_hash_odd_borders[] = {"bcfguvyz", "prxz", "0145hjnp", "028b"};

static int 
emdb_geo_hash_index_for_char(char c, const char *string)
{ 
  int index = -1;
  int string_amount = strlen(string);
  int i;
  for(i = 0; i < string_amount; i++) {  
    if(c == string[i]) {
      index = i; 
      break;
    }
  }
  return index;
}

static char* 
emdb_geo_hash_get_neighbor(char *hash, int direction)
{
  int hash_length = strlen(hash);
  char last_char = hash[hash_length - 1];
    
  int is_odd = hash_length % 2;
  const char **border = is_odd ? emdb_geo_hash_odd_borders : emdb_geo_hash_even_borders;
  const char **neighbor = is_odd ? emdb_geo_hash_odd_neighbors : emdb_geo_hash_even_neighbors; 
    
  char *base = (char*)malloc(sizeof(char) * 1);
  base[0] = '\0';
  strncat(base, hash, hash_length - 1);
    
  if(emdb_geo_hash_index_for_char(last_char, border[direction]) != -1)
     base = emdb_geo_hash_get_neighbor(base, direction);

  int neighbor_index = emdb_geo_hash_index_for_char(last_char, neighbor[direction]);
  last_char = emdb_geo_hash_char_map[neighbor_index];
        
  char *last_hash = (char*)malloc(sizeof(char) * 2);
  last_hash[0] = last_char;
  last_hash[1] = '\0';
  strcat(base, last_hash);
  free(last_hash);
    
  return base;
}

char* 
emdb_geo_hash_encode(double lat, double lng, int precision) {    
  if(precision < 1 || precision > 12)
    precision = 6;
    
  char* hash = NULL;
  if(lat <= 90.0 && lat >= -90.0 && lng <= 180.0 && lng >= -180.0) {
    hash = (char*)malloc(sizeof(char) * (precision + 1));
    hash[precision] = '\0';
    precision *= 5.0;
        
    emdb_geo_hash_interval_t lat_interval = {MAX_LAT, MIN_LAT};
    emdb_geo_hash_interval_t lng_interval = {MAX_LONG, MIN_LONG};

    emdb_geo_hash_interval_t *interval;
    double coord, mid;
    int is_even = 1;
    unsigned int hashChar = 0;
    int i;

    for(i = 1; i <= precision; i++) {
      if(is_even) {
        interval = &lng_interval;
        coord = lng;
      } else {
        interval = &lat_interval;
        coord = lat;   
      }
            
      mid = (interval->low + interval->high) / 2.0;
      hashChar = hashChar << 1;
            
      if(coord > mid) {
        interval->low = mid;
        hashChar |= 0x01;
      } else
        interval->high = mid;
            
      if(!(i % 5)) {
        hash[(i - 1) / 5] = emdb_geo_hash_char_map[hashChar];
        hashChar = 0;
      }
            
      is_even = !is_even;
    }
  }
  return hash;
}

emdb_geo_hash_corrd_t
emdb_geo_hash_decode(char *hash) 
{ 
  emdb_geo_hash_corrd_t coordinate = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {0.0, 0.0}};
  if(hash) {    
    int char_amount = strlen(hash);
    if(char_amount) {
       unsigned int char_mapIndex;
       emdb_geo_hash_interval_t lat_interval = {MAX_LAT, MIN_LAT};
       emdb_geo_hash_interval_t lng_interval = {MAX_LONG, MIN_LONG};
       emdb_geo_hash_interval_t *interval;
        
       int is_even = 1;
       double delta;
       int i, j;
       for(i = 0; i < char_amount; i++){
         char_mapIndex = emdb_geo_hash_index_for_char(hash[i], (char*)emdb_geo_hash_char_map);
         //if(char_mapIndex < 0)
         //    break;
            
         // Interpret the last 5 bits of the integer
         for(j = 0; j < 5; j++) {
           interval = is_even ? &lng_interval : &lat_interval;
                
           delta = (interval->high - interval->low) / 2.0;
                
           if((char_mapIndex << j) & 0x0010)
             interval->low += delta;
           else
             interval->high -= delta;
                
           is_even = !is_even;
         } 
       }
            
       coordinate.latitude  = lat_interval.high - ((lat_interval.high - lat_interval.low) / 2.0);
       coordinate.longitude = lng_interval.high - ((lng_interval.high - lng_interval.low) / 2.0);
            
       coordinate.north = lat_interval.high;
       coordinate.east  = lng_interval.high;
       coordinate.south = lat_interval.low;
       coordinate.west  = lng_interval.low;
     }
  }
  return coordinate;
}


char** 
emdb_geo_hash_neighbors(char *hash)
{
  char** neighbors = NULL;    
  if(hash) {
    // N, NE, E, SE, S, SW, W, NW
    neighbors = (char**)malloc(sizeof(char*) * 8);
        
    neighbors[0] = emdb_geo_hash_get_neighbor(hash, NORTH);
    neighbors[1] = emdb_geo_hash_get_neighbor(neighbors[0], EAST);
    neighbors[2] = emdb_geo_hash_get_neighbor(hash, EAST);
    neighbors[3] = emdb_geo_hash_get_neighbor(neighbors[2], SOUTH);
    neighbors[4] = emdb_geo_hash_get_neighbor(hash, SOUTH);
    neighbors[5] = emdb_geo_hash_get_neighbor(neighbors[4], WEST);                
    neighbors[6] = emdb_geo_hash_get_neighbor(hash, WEST);
    neighbors[7] = emdb_geo_hash_get_neighbor(neighbors[6], NORTH);        
  }
  return neighbors;
}

emdb_geo_hash_box_dimension_t
emdb_geo_hash_dimensions_for_precision(int precision) 
{
  emdb_geo_hash_box_dimension_t dimensions = {0.0, 0.0};
  if(precision > 0) {
     int lat_times_to_cut = precision * 5 / 2;
     int lng_times_to_cut = precision * 5 / 2 + (precision % 2 ? 1 : 0);
	
     double width = 360.0;
     double height = 180.0;
	
     int i;
     for(i = 0; i < lat_times_to_cut; i++)
       height /= 2.0;
     for(i = 0; i < lng_times_to_cut; i++)
       width /= 2.0;
     dimensions.width = width;
     dimensions.height = height;	
  }
  return dimensions;
}
