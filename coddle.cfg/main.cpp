#include <coddle/coddle.hpp>
#include <coddle/config.hpp>

int main(int argc, char **argv)
{
  Config config(argc, argv);
  config.cflags.push_back("$(pkg-config --cflags sdl2 libavdevice libavformat libavutil libavcodec)");
  config.ldflags.push_back("$(pkg-config --libs sdl2 libavdevice libavformat libavutil libavcodec)");
  return coddle(&config);
}
