#include "etdb.h"

typedef struct etdb_geo_hash_interval_s etdb_geo_hash_interval_t;
struct etdb_geo_hash_interval_s{
  double high;
  double low;
};

/* Normal 32 characer map used for geohashing */
static char etdb_geo_hash_char_map[] =  "0123456789bcdefghjkmnpqrstuvwxyz";

/*
 *  The follow character maps were created by Dave Troy and used in his Javascript Geohashing
 *  library. http://github.com/davetroy/geohash-js
 */
static const char *etdb_geo_hash_even_neighbors[] = 
                               {"p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                                "bc01fg45238967deuvhjyznpkmstqrwx", 
                                "14365h7k9dcfesgujnmqp0r2twvyx8zb",
                                "238967debc01fg45kmstqrwxuvhjyznp"
                               };

static const char *etdb_geo_hash_odd_neighbors[] = 
                              {"bc01fg45238967deuvhjyznpkmstqrwx", 
                               "p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                               "238967debc01fg45kmstqrwxuvhjyznp",
                               "14365h7k9dcfesgujnmqp0r2twvyx8zb"    
                              };

static const char *etdb_geo_hash_even_borders[] = {"prxz", "bcfguvyz", "028b", "0145hjnp"};
static const char *etdb_geo_hash_odd_borders[] = {"bcfguvyz", "prxz", "0145hjnp", "028b"};

static int 
etdb_geo_hash_index_for_char(char c, const char *string)
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

void 
etdb_geo_hash_get_neighbor(char *hash, int char_amount, int direction, char *nei)
{
  int hash_length = char_amount;
  char last_char  = hash[hash_length - 1];
    
  int is_odd = hash_length % 2;
  const char **border = is_odd ? etdb_geo_hash_odd_borders : etdb_geo_hash_even_borders;
  const char **neighbor = is_odd ? etdb_geo_hash_odd_neighbors : etdb_geo_hash_even_neighbors; 
 
  if(etdb_geo_hash_index_for_char(last_char, border[direction]) != -1)
     etdb_geo_hash_get_neighbor(hash, hash_length - 1, direction, nei);
  else
     memcpy(nei, hash, hash_length - 1);

  int neighbor_index = etdb_geo_hash_index_for_char(last_char, neighbor[direction]);
  last_char = etdb_geo_hash_char_map[neighbor_index];

  nei[hash_length - 1] = last_char;
}

char* 
etdb_geo_hash_encode(double lat, double lng, char *hash, int precision){    
  if(precision < 1 || precision > 12)
    precision = 6;

  if(lat <= 90.0 && lat >= -90.0 && lng <= 180.0 && lng >= -180.0) {
    //hash[precision] = '\0';
    precision *= 5;
        
    etdb_geo_hash_interval_t lat_interval = {MAX_LAT, MIN_LAT};
    etdb_geo_hash_interval_t lng_interval = {MAX_LONG, MIN_LONG};

    etdb_geo_hash_interval_t *interval;
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
        hash[(i - 1) / 5] = etdb_geo_hash_char_map[hashChar];
        hashChar = 0;
      }
            
      is_even = !is_even;
    }
  }
  return hash;
}

etdb_geo_hash_corrd_t
etdb_geo_hash_decode(char *hash, int char_amount) 
{ 
  etdb_geo_hash_corrd_t coordinate = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {0.0, 0.0}};
  if(hash) {    
    if(char_amount) {
       unsigned int char_mapIndex;
       etdb_geo_hash_interval_t lat_interval = {MAX_LAT, MIN_LAT};
       etdb_geo_hash_interval_t lng_interval = {MAX_LONG, MIN_LONG};
       etdb_geo_hash_interval_t *interval;
        
       int is_even = 1;
       double delta;
       int i, j;
       for(i = 0; i < char_amount; i++){
         char_mapIndex = etdb_geo_hash_index_for_char(hash[i], (char*)etdb_geo_hash_char_map);
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

void 
etdb_geo_hash_neighbors(char *hash, int char_amount, char **neighbors)
{ 
  if(hash) {
    // N, NE, E, SE, S, SW, W, NW       
    etdb_geo_hash_get_neighbor(hash, char_amount, NORTH, neighbors[0]);
    etdb_geo_hash_get_neighbor(neighbors[0], char_amount, EAST, neighbors[1]);
    etdb_geo_hash_get_neighbor(hash, char_amount, EAST, neighbors[2]);
    etdb_geo_hash_get_neighbor(neighbors[2], char_amount, SOUTH, neighbors[3]);
    etdb_geo_hash_get_neighbor(hash,char_amount, SOUTH, neighbors[4]);
    etdb_geo_hash_get_neighbor(neighbors[4],char_amount, WEST, neighbors[5]);                
    etdb_geo_hash_get_neighbor(hash,char_amount, WEST, neighbors[6]);
    etdb_geo_hash_get_neighbor(neighbors[6],char_amount, NORTH, neighbors[7]);        
  }
}

etdb_geo_hash_box_dimension_t
etdb_geo_hash_dimensions_for_precision(int precision) 
{
  etdb_geo_hash_box_dimension_t dimensions = {0.0, 0.0};
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

void
etdb_geo_hash_get_max_cover_interate_impl(etdb_geo_hash_interval_t *lat_range,
                                          etdb_geo_hash_interval_t *lng_range,
                                          etdb_geo_hash_interval_t *lat_interval,
                                          etdb_geo_hash_interval_t *lng_interval,
                                          int pos, 
                                          int precision_bits,
                                          unsigned char *hash,
                                          unsigned char hashChar 
                                          )
{
  if(pos > precision_bits)  return;

  etdb_geo_hash_interval_t *interval;
  etdb_geo_hash_interval_t *range;
  int is_even = pos & 0x01;
  double mid;
  int in_char = !(pos % 5);

  if(is_even){
    interval = lng_interval;
    range    = lng_range;
  }else{
    interval = lat_interval;
    range    = lat_range;
  }
  mid      = (interval->low + interval->high) / 2.0;
  hashChar = hashChar << 1;

  if(range->low > mid){
    interval->low = mid;
    hashChar     |= 0x01;

    if(in_char){
      hash[(pos - 1)/5]  = etdb_geo_hash_char_map[hashChar];
      hashChar           = 0;

      if(lat_range->low <= lat_interval->low && lat_interval->high <= lat_range->high &&
          lng_range->low <= lng_interval->low && lng_interval->high <= lng_range->high)
      {
        /// TODO: add handler
        snprintf("1:%d %s \n", (pos - 1)/5 + 1, hash);
        return;
      }else if(pos == precision_bits){
        snprintf("0:%d %s \n", (pos - 1)/5 + 1, hash);
      }
    }
    etdb_geo_hash_get_max_cover_interate_impl(lat_range, lng_range, lat_interval, lng_interval,
                                              pos + 1, precision_bits, hash, hashChar);
  }else if(range->high < mid){
    interval->high = mid;
    
    if(in_char){
      hash[(pos - 1)/5]  = etdb_geo_hash_char_map[hashChar];
      hashChar           = 0;
  
      if(lat_range->low <= lat_interval->low && lat_interval->high <= lat_range->high &&
         lng_range->low <= lng_interval->low && lng_interval->high <= lng_range->high)
      {
        /// TODO: add handler
        printf("1:%d %s \n", (pos - 1)/5 + 1, hash);
        return;
      }else if(pos == precision_bits){
        printf("0:%d %s \n", (pos - 1)/5 + 1, hash);
      }
    }
    etdb_geo_hash_get_max_cover_interate_impl(lat_range, lng_range, lat_interval, lng_interval,
                                              pos + 1, precision_bits, hash, hashChar);
  }else{
    interval->low              = mid;
    unsigned char hashCharTemp = hashChar;
    hashChar                  |= 0x01;

    if(in_char){
      hash[(pos - 1)/5] = etdb_geo_hash_char_map[hashChar];
      hashChar          = 0;
  
      if(lat_range->low <= lat_interval->low && lat_interval->high <= lat_range->high &&
         lng_range->low <= lng_interval->low && lng_interval->high <= lng_range->high)
      {
        /// TODO: add handler
        printf("1:%d %s \n", (pos - 1)/5 + 1, hash);
        goto Next;
      }else if(pos == precision_bits){
        printf("0:%d %s \n", (pos - 1)/5 + 1, hash);
      }
    }
    etdb_geo_hash_get_max_cover_interate_impl(lat_range, lng_range, lat_interval, lng_interval,
                                              pos + 1, precision_bits, hash, hashChar);
Next:
    interval->high = mid;
    hashChar       = hashCharTemp; 

    if(in_char){
      hash[(pos - 1)/5] = etdb_geo_hash_char_map[hashChar];
      hashChar          = 0;

      if(lat_range->low <= lat_interval->low && lat_interval->high <= lat_range->high &&
         lng_range->low <= lng_interval->low && lng_interval->high <= lng_range->high)
      {
        /// TODO: add handler
        printf("1:%d %s \n", (pos - 1)/5 + 1, hash);
        return;
      }else if(pos == precision_bits){
        printf("0:%d %s \n", (pos - 1)/5 + 1, hash);
      }
    }
    etdb_geo_hash_get_max_cover_interate_impl(lat_range, lng_range, lat_interval, lng_interval,
                                              pos + 1, precision_bits, hash, hashChar);
  }
}

extern void
etdb_geo_hash_get_max_cover_iterate(double lat1, double lat2, double lng1, double lng2, int precision)
{
  if(lat1 > lat2){
    etdb_swap(lat1, lat2);
  }
  if(lng1 > lng2){
    etdb_swap(lng1, lng2);
  }
  
  if(precision < 6 || precision > 12)  precision = 12;
  
  precision                            *= 5;
  etdb_geo_hash_interval_t lat_range    = {lat2, lat1};
  etdb_geo_hash_interval_t lng_range    = {lng2, lng1};

  etdb_geo_hash_interval_t lat_interval = {MAX_LAT, MIN_LAT};
  etdb_geo_hash_interval_t lng_interval = {MAX_LONG, MIN_LONG};

  unsigned char hash[13] = "\0";
  etdb_geo_hash_get_max_cover_interate_impl(&lat_range, &lng_range, &lat_interval, &lng_interval, 1, precision, hash, 0);
}

