#include <coddle/coddle.hpp>
#include <coddle/config.hpp>

int main(int argc, char **argv)
{
  Config config(argc, argv);
  config.pkgs.insert(std::end(config.pkgs), { "sdl2" , "libavdevice", "libavformat", "libavutil", "libavcodec" });
  return coddle(&config);
}
