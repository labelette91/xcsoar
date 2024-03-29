/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/OpenGL/Cache.hpp"
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/OpenGL/Debug.hpp"
#include "Screen/OpenGL/Point.hpp"
#include "Screen/Font.hpp"
#include "Screen/Color.hpp"
#include "Util/ListHead.hpp"
#include "Util/Cache.hpp"

#include <unordered_map>
#include <string>
#include <assert.h>

/**
 * A key type for the Cache template class.  It can be operated in two
 * modes: the zero-allocation mode is used by default; it stores a
 * pointer to the original string.  A key that is used to actually
 * store the element must contain a copy of the string, just in case
 * the original string goes out of scope.  The method Allocate()
 * creates that copy.
 */
struct TextCacheKey {
  const Font *font;
  UPixelScalar height;
  const char *text;
  char *allocated;
  size_t hash;

#if GCC_VERSION > 40500
  TextCacheKey(const TextCacheKey &other) = delete;
#else
  /* workaround for gcc version in the Android NDK */
  TextCacheKey(const TextCacheKey &other)
    :font(other.font), height(other.height),
     text(other.text), allocated(NULL),
     hash(other.hash) {
    if (other.allocated != NULL)
      Allocate();
  }
#endif

  TextCacheKey(TextCacheKey &&other)
    :font(other.font), height(other.height),
     text(other.text), allocated(other.allocated),
     hash(other.hash) {
    other.allocated = NULL;
  }

  TextCacheKey(const Font &_font, const char *_text)
    :font(&_font), height(_font.GetHeight()), text(_text), allocated(NULL) {}

  ~TextCacheKey() {
    free(allocated);
  }

  /**
   * Copy the "text" attribute.  This must be called before inserting
   * this key into the #Cache.
   */
  void Allocate() {
    assert(allocated == NULL);

    text = allocated = strdup(text);
  }

  TextCacheKey &operator=(const TextCacheKey &other) = delete;

  TextCacheKey &operator=(TextCacheKey &&other) {
    font = other.font;
    height = other.height;
    text = other.text;
    std::swap(allocated, other.allocated);
    hash = other.hash;
    return *this;
  }

  gcc_pure
  bool operator==(const TextCacheKey &other) const {
    return font == other.font && height == other.height &&
      strcmp(text, other.text) == 0;
  }

  struct StringHash {
    gcc_pure
    size_t operator()(const char *__s) const {
      /* code copied from libstdc++ backward/hash_fun.h */
      unsigned long __h = 0;
      for ( ; *__s; ++__s)
        __h = 5 * __h + *__s;
      return size_t(__h);
    }
  };

  struct Hash {
    StringHash string_hash;

    gcc_pure
    size_t operator()(const TextCacheKey &key) const {
      return (size_t)(const void *)key.font ^ key.height
        ^ string_hash(key.text);
    }
  };
};

struct RenderedText : public ListHead {
  GLTexture *texture;

  RenderedText(const RenderedText &other) = delete;
  RenderedText(RenderedText &&other)
    :texture(other.texture) {
    other.texture = NULL;
  }

#ifdef ANDROID
  RenderedText(int id, unsigned width, unsigned height)
    :texture(new GLTexture(id, width, height)) {}
#else
  RenderedText(SDL_Surface *surface)
    :texture(new GLTexture(surface)) {}
#endif

  ~RenderedText() {
    delete texture;
  }

  RenderedText &operator=(const RenderedText &other) = delete;

  RenderedText &operator=(RenderedText &&other) {
    std::swap(texture, other.texture);
    return *this;
  }
};

static Cache<TextCacheKey, PixelSize, 1024u, TextCacheKey::Hash> size_cache;
static Cache<TextCacheKey, RenderedText, 256u, TextCacheKey::Hash> text_cache;

PixelSize
TextCache::GetSize(const Font &font, const char *text)
{
  TextCacheKey key(font, text);
  const PixelSize *cached = size_cache.Get(key);
  if (cached != NULL)
    return *cached;

  PixelSize size = font.TextSize(text);

  key.Allocate();
  size_cache.Put(std::move(key), std::move(size));
  return size;
}

PixelSize
TextCache::LookupSize(const Font &font, const char *text)
{
  PixelSize size = { 0, 0 };

  if (*text == 0)
    return size;

  TextCacheKey key(font, text);
  const RenderedText *cached = text_cache.Get(key);
  if (cached == NULL)
    return size;

  return cached->texture->GetSize();
}

GLTexture *
TextCache::get(const Font *font, const char *text)
{
  assert(pthread_equal(pthread_self(), OpenGL::thread));
  assert(font != NULL);
  assert(text != NULL);

  if (*text == 0)
    return NULL;

  TextCacheKey key(*font, text);

  /* look it up */

  const RenderedText *cached = text_cache.Get(key);
  if (cached != NULL)
    return cached->texture;

  /* render the text into a OpenGL texture */

#ifdef ANDROID
  PixelSize size;
  int texture_id = font->TextTextureGL(text, size);
  if (texture_id == 0)
    return NULL;

  RenderedText rt(texture_id, size.cx, size.cy);
#else
  const Color background_color = COLOR_BLACK;
  const Color text_color = COLOR_WHITE;
  SDL_Surface *surface = ::TTF_RenderUTF8_Shaded(font->Native(), text,
                                                 text_color, background_color);
  if (surface == NULL)
    return NULL;

  /* insert into cache */

  RenderedText rt(surface);
  SDL_FreeSurface(surface);
#endif

  GLTexture *texture = rt.texture;

  key.Allocate();
  text_cache.Put(std::move(key), std::move(rt));

  /* done */

  return texture;
}

void
TextCache::flush()
{
  assert(pthread_equal(pthread_self(), OpenGL::thread));

  size_cache.Clear();
  text_cache.Clear();
}
