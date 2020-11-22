#include "portability.h"

static inline size_t BitSet_internal_min_length(const Array* restrict b1, const Array* restrict b2) {
  return b1->len < b2->len ? b1->len : b2->len;
}

Array BitSet_copy_internal(const Array* bitset) {
  Array res = {
    .len = bitset->len,
    .capacity = bitset->capacity,
    .data = (uint64_t*)CARP_MALLOC(bitset->len * sizeof(uint64_t))
  };
  memcpy(res.data, bitset->data, res.len * sizeof(uint64_t));
  return res;
}

bool BitSet_internal_resize(Array* bitset, size_t newlen, bool padwithzeroes) {
  size_t smallest = newlen < bitset->len ? newlen : bitset->len;
  if (bitset->capacity < newlen) {
    uint64_t *newarray;
    bitset->capacity = newlen * 2;
    if ((newarray = (uint64_t *)CARP_REALLOC(bitset->data, sizeof(uint64_t) * bitset->capacity)) == NULL) {
      CARP_FREE(bitset->data);
      return false;
    }
    bitset->data = newarray;
  }
  if (padwithzeroes && (newlen > smallest)) {
    memset(bitset->data + smallest, 0, sizeof(uint64_t) * (newlen - smallest));
  }
  bitset->len = newlen;
  return true;
}

void BitSet_internal_set(Array* bitset,  size_t i) {
  size_t shiftedi = i >> 6;
  if (shiftedi >= bitset->len) {
    size_t newlen = shiftedi+1;
    BitSet_internal_resize(bitset, newlen, true);
    assert(bitset->len == newlen);
  }
  ((uint64_t*)(bitset->data))[shiftedi] |= ((uint64_t)1) << (i % 64);
}

bool BitSet_internal_get(const Array* bitset,  size_t i) {
  size_t shiftedi = i >> 6;
  if (shiftedi >= bitset->len) {
    return false;
  }
  return (((uint64_t*)(bitset->data))[shiftedi] & (((uint64_t)1) << (i % 64))) != 0;
}

bool nextSetBit(const Array* bitset, size_t *i) {
  size_t x = *i >> 6;
  if (x >= bitset->len) {
    return false;
  }
  uint64_t w = ((uint64_t*)(bitset->data))[x];
  w >>= (*i & 63);
  if (w != 0) {
    *i += __builtin_ctzll(w);
    return true;
  }
  x++;
  while (x < bitset->len) {
    w = ((uint64_t*)(bitset->data))[x];
    if (w != 0) {
      *i = x * 64 + __builtin_ctzll(w);
      return true;
    }
    x++;
  }
  return false;
}

size_t BitSet_internal_str_size(const Array* b) {
  size_t size = strlen("BitSet{");
  for (size_t i=0; nextSetBit(b, &i); i++) {
    size += snprintf(NULL, 0, "%zu ", i);
  }
  size += strlen("}");
  return size;
}

String BitSet_internal_str(const Array* b) {
  size_t size = BitSet_internal_str_size(b);
  String res = CARP_MALLOC(size+1);
  memset(res, 0, size+1);
  strcpy(res, "BitSet{");
  size_t off = strlen(res);
  for (size_t i=0; nextSetBit(b, &i); i++) {
    size_t csize = snprintf(NULL, 0, "%zu ", i);
    snprintf(res+off, csize+1, "%zu ", i);
    off += csize;
  }
  char* v = strrchr(res, ' ');
  while (v && *v != '\0') { *v = '\0'; v++; }
  strcat(res, "}");
  return res;
}

size_t BitSet_internal_card(const Array* bitset) {
  size_t card = 0;
  size_t k = 0;
  const uint64_t* data = (uint64_t*)(bitset->data);
  for (; k + 7 < bitset->len; k+=8) {
    card += __builtin_popcountll(data[k]);
    card += __builtin_popcountll(data[k+1]);
    card += __builtin_popcountll(data[k+2]);
    card += __builtin_popcountll(data[k+3]);
    card += __builtin_popcountll(data[k+4]);
    card += __builtin_popcountll(data[k+5]);
    card += __builtin_popcountll(data[k+6]);
    card += __builtin_popcountll(data[k+7]);
  }
  for (; k + 3 < bitset->len; k+=4) {
    card += __builtin_popcountll(data[k]);
    card += __builtin_popcountll(data[k+1]);
    card += __builtin_popcountll(data[k+2]);
    card += __builtin_popcountll(data[k+3]);
  }
  for (; k < bitset->len; k++) {
    card += __builtin_popcountll(data[k]);
  }
  return card;
}

uint64_t BitSet_internal_minimum(const Array* bitset) {
  const uint64_t* data = (uint64_t*)bitset->data;
  for (size_t k = 0; k < bitset->len; k++) {
    uint64_t w = data[k];
    if (w != 0) {
      return __builtin_ctzll(w) + k * 64;
    }
  }
  return 0;
}

uint64_t BitSet_internal_maximum(const Array* bitset) {
  const uint64_t* data = (uint64_t*)bitset->data;
  for (size_t k = bitset->len; k > 0; k--) {
    uint64_t w = data[k-1];
    if ( w != 0 ) {
      return  63 - __builtin_clzll(w) + (k - 1) * 64;
    }
  }
  return 0;
}

bool BitSet_internal_intersect(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  for (size_t k = 0; k < minlength; k++) {
    if ((b1data[k] & b2data[k]) != 0) {
      return true;
    }
  }
  return false;
}

bool BitSet_internal_disjoint(const Array* restrict b1, const Array* restrict b2) {
  return !BitSet_internal_intersect(b1, b2);
}

Array BitSet_internal_union(const Array* restrict b1, const Array* restrict b2) {
  Array res = BitSet_copy_internal(b1);
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  uint64_t* resdata = (uint64_t*)res.data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t k = 0;
  for (size_t k = 0; k < minlength; ++k) {
    resdata[k] = b1data[k] | b2data[k];
  }
  if (b2->len > res.len) {
     size_t oldsize = res.len;
     BitSet_internal_resize(&res, b2->len, false);
     assert(res.len == b2->len);
     memcpy(resdata + oldsize, b2data + oldsize, (b2->len - oldsize) * sizeof(uint64_t));
  }
  return res;
}

Array BitSet_internal_intersection(const Array* restrict b1, const Array* restrict b2) {
  Array res = BitSet_copy_internal(b1);
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  uint64_t* resdata = (uint64_t*)res.data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t k = 0;
  for (; k < minlength; ++k) {
    resdata[k] = b1data[k] & b2data[k];
  }
  for (; k < b1->len; ++k) {
    resdata[k] = 0;
  }
  return res;
}

Array BitSet_internal_difference(const Array* restrict b1, const Array* restrict b2) {
  Array res = BitSet_copy_internal(b1);
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  uint64_t* resdata = (uint64_t*)res.data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t k = 0;
  for (; k < minlength; ++k) {
    resdata[k] = b1data[k] & ~(b2data[k]);
  }
  return res;
}

Array BitSet_internal_symmetric_difference(const Array* restrict b1, const Array* restrict b2) {
  Array res = BitSet_copy_internal(b1);
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  uint64_t* resdata = (uint64_t*)res.data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t k = 0;
  for (; k < minlength; ++k) {
    resdata[k] = b1data[k] ^ b2data[k];
  }
  if (b2->len > res.len) {
     size_t oldsize = b1->len;
     BitSet_internal_resize(&res, b2->len, false);
     memcpy(resdata + oldsize, b2data + oldsize, (b2->len - oldsize) * sizeof(uint64_t));
  }
  return res;
}

size_t BitSet_internal_union_card(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t answer = 0;
  size_t k = 0;
  for (; k + 3 < minlength; k += 4) {
    answer += __builtin_popcountll(b1data[k] | b2data[k]);
    answer += __builtin_popcountll(b1data[k+1] | b2data[k+1]);
    answer += __builtin_popcountll(b1data[k+2] | b2data[k+2]);
    answer += __builtin_popcountll(b1data[k+3] | b2data[k+3]);
  }
  for (; k < minlength; ++k) {
    answer += __builtin_popcountll ( b1data[k] | b2data[k]);
  }
  if (b2->len > b1->len) {
    for (; k + 3 < b2->len; k+=4) {
      answer += __builtin_popcountll(b2data[k]);
      answer += __builtin_popcountll(b2data[k+1]);
      answer += __builtin_popcountll(b2data[k+2]);
      answer += __builtin_popcountll(b2data[k+3]);
    }
    for (; k < b2->len; ++k) {
      answer += __builtin_popcountll(b2data[k]);
    }
  } else {
    for (; k + 3 < b1->len; k+=4) {
      answer += __builtin_popcountll(b1data[k]);
      answer += __builtin_popcountll(b1data[k+1]);
      answer += __builtin_popcountll(b1data[k+2]);
      answer += __builtin_popcountll(b1data[k+3]);
    }
    for (; k < b1->len; ++k) {
      answer += __builtin_popcountll(b1data[k]);
    }
  }
  return answer;
}

size_t BitSet_internal_intersection_card(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t answer = 0;
  for (size_t k = 0; k < minlength; ++k) {
    answer += __builtin_popcountll(b1data[k] & b2data[k]);
  }
  return answer;
}

size_t BitSet_internal_difference_card(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t answer = 0;
  size_t k = 0;
  for (; k < minlength; ++k) {
    answer += __builtin_popcountll(b1data[k] & ~ (b2data[k]));
  }
  for (; k < b1->len ; ++k) {
    answer += __builtin_popcountll(b1data[k]);
  }
  return answer;
}

size_t BitSet_internal_symmetric_difference_card(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t minlength = BitSet_internal_min_length(b1, b2);
  size_t answer = 0;
  size_t k = 0;
  for (; k < minlength; ++k) {
    answer += __builtin_popcountll(b1data[k] ^ b2data[k]);
  }
  if (b2->len > b1->len) {
    for (; k < b2->len; ++k) {
      answer += __builtin_popcountll(b2data[k]);
    }
  } else {
    for (; k < b1->len; ++k) {
      answer += __builtin_popcountll(b1data[k]);
    }
  }
  return answer;
}

static bool BitSet_internal_any_bits_set(const Array* b, size_t starting_loc) {
  const uint64_t* bdata = (uint64_t*)b->data;
  if (starting_loc >= b->len) {
    return false;
  }
  for (size_t k = starting_loc; k < b->len; k++) {
    if (bdata[k] != 0) {
      return true;
    }
  }
  return false;
}

bool BitSet_internal_contains_all(const Array* restrict b1, const Array* restrict b2) {
  const uint64_t* b1data = (uint64_t*)b1->data;
  const uint64_t* b2data = (uint64_t*)b2->data;
  size_t k = 0;
  for (k = 0; k < b1->len; k++) {
    if ((b1data[k] & b2data[k]) != b2data[k]) {
      return false;
    }
  }
  if (b2->len > b1->len) {
    return !BitSet_internal_any_bits_set(b2, b1->len);
  }
  return true;
}

Array BitSet_internal_from_array(const Array* in) {
  Array bitset = {.len = 0, .capacity = 0, .data = NULL};
  const uint64_t* data = (uint64_t*)in->data;
  for (size_t i = 0; i < in->len; i++) {
    BitSet_internal_set(&bitset, data[i]);
  }
  return bitset;
}

