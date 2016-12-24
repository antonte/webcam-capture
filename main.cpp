#include "sdl_check.hpp"
extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
}
#include <SDL.h>
#include <iostream>

static int clamp(int x)
{
  return std::min(255, std::max(x, 0));
}

int main()
{
  const auto Width = 320;
  const auto Height = 240;
  avdevice_register_all();
  av_register_all();
  AVFormatContext *formatContext = nullptr;
  auto fileName = "/dev/video0";
  auto format = "v4l2";
  auto inputFormat = av_find_input_format(format);
  if (!inputFormat)
  {
    std::cerr << "Unknown input format: " << format << std::endl;
    return 1;
  }

  AVDictionary *format_opts = nullptr;
  av_dict_set(&format_opts, "framerate", "1000", 0);;
  av_dict_set(&format_opts, "video_size", "320x240", 0);
  auto err = avformat_open_input(&formatContext, fileName, inputFormat, &format_opts);
  if (err != 0)
  {
    std::cout << "Could not open input " << fileName << std::endl;
    return 2;
  }
  
  av_dump_format(formatContext, 0, fileName, 0);
  av_dict_free(&format_opts);

  err = SDL_Init(SDL_INIT_EVERYTHING);
  SDL_CHECK(err == 0, "SDL_Init(SDL_INIT_EVERYTHING)");
  auto window = SDL_CreateWindow("Webcam", 65, 126, Width, Height, SDL_WINDOW_BORDERLESS);
  SDL_CHECK(window, "SDL_CreateWindow(\"Webcam\", 65, 126, Width, Height, SDL_WINDOW_BORDERLESS);");
  auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_CHECK(renderer, "SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)");
  auto texture = SDL_CreateTexture(renderer,
                               SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_STREAMING, Width, Height);
  SDL_CHECK(texture, "SDL_CreateTexture");
  
  auto done = false;
  while (!done)
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
      {
        done = true;
        break;
      }
    }
    AVPacket packet;
    err = av_read_frame(formatContext, &packet);
    if (err)
    {
      done = true;
      break;
    }
    uint8_t *pixels;
    int pitch;
    auto res = SDL_LockTexture(texture, nullptr, (void **)&pixels, &pitch);
    SDL_CHECK(res == 0, "SDL_LockTexture(texture, nullptr, &pixels, &pitch)");
    for (int j = 0; j < Height; ++j)
      for (int i = 0; i < Width; ++i)
      {
        auto y = packet.data[j * Width * 2 + i * 2];
        auto u = packet.data[j * Width * 2 + (i & 0xfffe) * 2 + 1];
        auto v = packet.data[j * Width * 2 + (i | 0x1) * 2 + 1];
        auto c = y - 16;
        auto d = u - 128;
        auto e = v - 128;
        auto r = clamp((298 * c + 409 * e + 128) >> 8);
        auto g = clamp((298 * c - 100 * d - 208 * e + 128) >> 8);
        auto b = clamp((298 * c + 516 * d + 128) >> 8);
        pixels[i * 4 + j * pitch] = 0xff;
        pixels[i * 4 + 1 + j * pitch] = b;
        pixels[i * 4 + 2 + j * pitch] = g;
        pixels[i * 4 + 3 + j * pitch] = r;
      }
    SDL_UnlockTexture(texture);
    
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    av_free_packet(&packet);
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  avformat_free_context(formatContext);
}
