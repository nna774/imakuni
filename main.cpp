#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <cctype>
#include <cassert>

using Byte = unsigned char;

class Chunk {
public:
  Chunk(std::string const& type) : _type{type} {}
  std::string type() { return _type; }
  bool isCritical() { return std::isupper(_type[0]); }
  bool isPublic() { return std::isupper(_type[1]); }
  bool isSafe() { return std::isupper(_type[3]); }
private:
  std::string const _type;
};

class IHDRChunk : public Chunk {
public:
  IHDRChunk() : Chunk{"IHDR"} {}
};

void _read(std::istream& fs, char* p, size_t size) {
  fs.read(p, size);
}

void _read(std::istream& fs, Byte* p, size_t size) {
  fs.read(reinterpret_cast<char*>(p), size);
}

#define read(fs, p) _read((fs), (p), sizeof(p))
#define readWithSize(fs, p, size) _read((fs), (p), (size))

bool readHeader(std::istream& fs) {
  Byte sig[8];
  read(fs, sig);
  if(!(sig[0] == 0x89 &&
       sig[1] == 0x50 &&
       sig[2] == 0x4e &&
       sig[3] == 0x47 &&
       sig[4] == 0x0d &&
       sig[5] == 0x0a &&
       sig[6] == 0x1a &&
       sig[7] == 0x0a)) {
    return false;
  }
  return true;
}

size_t readSize(std::istream& fs) {
  Byte sizes[4];
  read(fs, sizes);
  return (sizes[0] << 24) + (sizes[1] << 16) + (sizes[2] << 8) + sizes[3];
}

std::unique_ptr<Chunk> readIHDR(std::istream& fs) {
  int width = readSize(fs);
  int height = readSize(fs);
  std::cout << width << ' ' << height;
  Byte other[9];
  read(fs, other);
  return std::unique_ptr<Chunk>{new IHDRChunk{}};
}

std::unique_ptr<Chunk> readIDAT(std::istream& fs, size_t size) {
  std::vector<Byte> data(size);
  readWithSize(fs, data.data(), size);
  char crc[4];
  read(fs, crc);
  return std::unique_ptr<Chunk>{new Chunk{"IDAT"}};
}

std::unique_ptr<Chunk> readIEND(std::istream& fs) {
  char crc[4];
  read(fs, crc);
  return std::unique_ptr<Chunk>{new Chunk{"IEND"}};
}

std::unique_ptr<Chunk> readChunk(std::istream& fs) {
  size_t size = readSize(fs);
  char types[4];
  read(fs, types);
  std::string type{types};
  if(type == "IHDR") {
    return readIHDR(fs);
  } else if(type == "IDAT") {
    return readIDAT(fs, size);
  } else if(type == "IEND") {
    return readIEND(fs);
  }
  assert(!"never come");
}

std::vector<std::unique_ptr<Chunk>> readChunks(std::istream& fs) {
  std::vector<std::unique_ptr<Chunk>> v;
  std::unique_ptr<Chunk> c;
  std::string type;
  do {
    c = readChunk(fs);
    type = c->type();
    std::cout << type << std::endl;
    v.push_back(std::move(c));
  } while(type != "IEND");
  return v;
}

void load(std::istream& fs) {
  Byte sig[8];
  if(!readHeader(fs)) {
    std::cerr << "not png file" << std::endl;
    return ;
  }

  std::vector<std::unique_ptr<Chunk>> chunks = readChunks(fs);
}

int main(int argc, char** argv) {
  if(argc >= 2) {
    std::ifstream fs{argv[1], std::ifstream::binary};
    if (!fs.is_open()) {
      std::cerr << "failed to open" << std::endl;
      return -1;
    }
    load(fs);
  } else {
    load(std::cin);
  }
  return 0;
}
