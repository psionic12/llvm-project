#include "scene.h"

template <typename T>
void GetResource(std::string id) {
  int fd = open(id.c_str(), O_RDONLY);
  if (fd < 0) {
    std::cerr << "failed to open file " << file << std::endl;
    return -1;
  }
  google::protobuf::io::FileInputStream file_input_stream(fd);
  std::cout << file.extension() << std::endl;
  if (readable) {
    google::protobuf::TextFormat::Parse(&file_input_stream, &info);
  } else {
    info.ParseFromZeroCopyStream(&file_input_stream);
  }
}

int main() {
  me::Scene scene;
}