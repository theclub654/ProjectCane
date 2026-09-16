#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

constexpr std::size_t CHUNK_SIZE = 0x2000;
constexpr std::size_t MAX_MATCH_LENGTH = 10;

std::vector<uint8_t> Decompress(std::span<const uint8_t> input);
int main(int argc, char** argv);