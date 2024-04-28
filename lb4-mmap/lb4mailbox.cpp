#include "lb4mailbox.h"

#include <cstring>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <windows.h>

#define CreateFileR(name) CreateFile(name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)
#define CreateFileW(name) CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr)
#define CreateFileRW(name) CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)
#define CreateFileMappingR(fp) CreateFileMapping(fp, nullptr, PAGE_READONLY, 0, 0, nullptr)
#define CreateFileMappingW(fp, minSize) CreateFileMapping(fp, nullptr, PAGE_READWRITE, 0, minSize, nullptr)

char UINT_RW_ARR[4];

void openFileR(const std::string& path, void** file, void** mapping, char** data) {
    *file = CreateFileR(path.c_str());
    if (*file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");
    if (GetFileSize(*file, nullptr) < 8) {
        CloseHandle(*file);
        throw std::underflow_error("Size of mailbox is too small!");
    }

    *mapping = CreateFileMappingR(*file);
    if (*mapping == nullptr) {
        CloseHandle(*file);
        throw std::runtime_error("Failed to create file mapping");
    }

    *data = static_cast<char*>(MapViewOfFile(*mapping, FILE_MAP_READ, 0, 0, 0));
    if (*data == nullptr) {
        CloseHandle(*mapping);
        CloseHandle(*file);
        throw std::runtime_error("Failed to map view of file");
    }
}

void openFileW(const std::string& path, const uint32_t minSize, void** file, void** mapping, char** data) {
    *file = CreateFileW(path.c_str());
    if (*file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");

    *mapping = CreateFileMappingW(*file, minSize);
    if (*mapping == nullptr) {
        CloseHandle(*file);
        throw std::runtime_error("Failed to create file mapping, GetLastError(): "+std::to_string(GetLastError()));
    }

    *data = static_cast<char*>(MapViewOfFile(*mapping, FILE_MAP_WRITE, 0, 0, 0));
    if (*data == nullptr) {
        CloseHandle(*mapping);
        CloseHandle(*file);
        throw std::runtime_error("Failed to map view of file, GetLastError(): "+std::to_string(GetLastError()));
    }
}

void openFileRW(const std::string& path, const uint32_t minSize, void** file, void** mapping, char** data) {
    *file = CreateFileRW(path.c_str());
    if (*file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");
    if (GetFileSize(*file, nullptr) < 8) {
        CloseHandle(*file);
        throw std::underflow_error("Size of mailbox is too small!");
    }

    *mapping = CreateFileMappingW(*file, minSize);
    if (*mapping == nullptr) {
        CloseHandle(*file);
        throw std::runtime_error("Failed to create file mapping");
    }

    *data = static_cast<char*>(MapViewOfFile(*mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
    if (*data == nullptr) {
        CloseHandle(*mapping);
        CloseHandle(*file);
        throw std::runtime_error("Failed to map view of file");
    }
}

uint32_t crc32(const char* buf, uint32_t size) {
    uint32_t crc = 0xFFFFFFFF;
    while (size--) {
        crc ^= *buf++;
        for (int k = 0; k < 8; k++)
            crc = crc & 1 ? crc >> 1 ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}

uint32_t readUint32(std::fstream& file) {
    uint32_t result;
    file.read(UINT_RW_ARR, 4);
    memcpy(&result, UINT_RW_ARR, 4);

    return result;
}

uint32_t readUint32(std::ifstream& file) {
    return readUint32((std::fstream&)file);
}

uint32_t readUint32(const char* data) {
    return *reinterpret_cast<const uint32_t*>(data);
}

uint32_t readUint32(const char* data, uint32_t& pos) {
    const uint32_t value = readUint32(data);
    pos += 4;

    return value;
}


MailboxEntry::MailboxEntry(const char* content, const uint32_t size) {
    this->content_size = content[size - 1] == '\0' ? size - 1 : size;
    this->content = new char[size];
    memcpy(this->content, content, size);

    this->checksum = crc32(this->content, this->content_size);
}

MailboxEntry::MailboxEntry(const std::string& content) {
    this->content_size = content.size();
    this->content = new char[this->content_size];
    memcpy(this->content, content.c_str(), this->content_size);

    this->checksum = crc32(this->content, this->content_size);
}

std::string MailboxEntry::getContent() {
    return std::string(content);
}

void MailboxEntry::write(char* data) {
    memcpy(data, &content_size, 4);
    memcpy(data+4, &checksum, 4);
    memcpy(data+8, content, content_size);
}

MailBox::MailBox(const std::string& name) {
    uint32_t pos = 0;
    filename = name;

    void* file;
    void* mapping;
    char* data;
    openFileR(name, &file, &mapping, &data);

    max_size = readUint32(data, pos);
    current_index = readUint32(data, pos);

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);
}

MailBox::MailBox(const std::string& name, const uint32_t max_size) {
    filename = name;
    this->max_size = max_size;

    void* file;
    void* mapping;
    char* data;
    openFileW(name, 8 + 4 * max_size, &file, &mapping, &data);

    memcpy(data, &max_size, 4);
    memcpy(data + 4, &current_index, 4);
    memset(data + 8, 0, 4 * max_size);

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);
}

uint32_t MailBox::getMaxSize() {
    return max_size;
}

uint32_t MailBox::getEntriesCount() {
    return current_index;
}

uint64_t MailBox::getCurrentSize() {
    void* file;
    void* mapping;
    char* data;
    char* currentData;
    openFileR(filename, &file, &mapping, &data);
    memcpy(&currentData, &data, sizeof(char*));

    currentData += 8;
    uint32_t total_size = 0, pos = 0;
    for (uint32_t i = 0; i < current_index; i++) {
        uint32_t tmp = readUint32(currentData, pos);
        total_size += readUint32(data + max_size * 4 + 8 + tmp, pos);
    }

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);

    return total_size;
}

void MailBox::addEntry(MailboxEntry* entry) {
    if (current_index >= max_size)
        throw std::overflow_error("Mailbox is full!");

    void* file;
    void* mapping;
    char* data;

    openFileR(filename, &file, &mapping, &data);
    uint32_t newMinSize = GetFileSize(file, nullptr) + 8 + entry->getContent().size()+1;

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);

    openFileRW(filename, newMinSize, &file, &mapping, &data);

    uint32_t lastPtr = readUint32(data + 8 + current_index * 4);
    uint32_t ptr = lastPtr + readUint32(data + 8 + max_size * 4 + lastPtr);
    printf("Writing at %d\n", ptr);
    entry->write(data + 8 + max_size * 4 + ptr);

    memcpy(data + current_index * 4 + 8, &ptr, 4);
    current_index++;
    memcpy(data + 4, &current_index, 4);

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);
}

MailboxEntry* MailBox::readEntry(const uint32_t index, const bool del) {
    if (index >= current_index)
        throw std::range_error("Requested mail entry does not exist!");

    void* file;
    void* mapping;
    char* data;
    openFileR(filename, &file, &mapping, &data);

    const uint32_t ptr = readUint32(data + 8 + index * 4);
    const uint32_t size = readUint32(data + 8 + max_size * 4 + ptr);
    const uint32_t checksum = readUint32(data + 8 + max_size * 4 + ptr + 4);

    const auto content = new char[size + 1];
    memcpy(content, data, size);
    content[size] = '\0';

    if (crc32(content, size) != checksum)
        throw std::runtime_error("Mail entry checksum mismatch!");

    auto* entry = new MailboxEntry(content, size + 1);

    delete[] content;

    if (del)
        deleteEntry(index);

    return entry;
}

void MailBox::deleteEntry(const uint32_t index) {
    if (index >= current_index)
        throw std::range_error("Requested mail entry does not exist!");

    void* file;
    void* mapping;
    char* data;
    openFileRW(filename, 0, &file, &mapping, &data);

    uint32_t tmp = readUint32(data + 8 + index * 4);
    uint32_t bytesToMove = readUint32(data + 8 + max_size * 4 + tmp) + 8;

    uint32_t indexesToMove = current_index - index - 1;
    memcpy(data + 8 + index * 4, data + 8 + index * 4 + 4, indexesToMove * 4);

    current_index--;
    memcpy(data + 4, &current_index, 4);

    for (uint32_t i = index; i < current_index; i++) {
        tmp = readUint32(data + 8 + i * 4);
        uint32_t pos = 8 + max_size * 4 + tmp;
        uint32_t size_to_move = readUint32(data + pos) + 8;

        pos -= 4;
        memcpy(data + pos - bytesToMove, data + pos, size_to_move);

        tmp = readUint32(data + 8 + i * 4);
        tmp -= bytesToMove;
        memcpy(data + 8 + i * 4, &tmp, 4);
    }

    uint32_t file_size = GetFileSize(file, nullptr);

    std::filesystem::resize_file(filename, file_size - bytesToMove);
}

void MailBox::deleteAllEntries() {
    current_index = 0;

    void* file;
    void* mapping;
    char* data;
    openFileW(filename, 8 + 4 * max_size, &file, &mapping, &data);

    memcpy(data, &max_size, 4);
    memcpy(data + 4, &current_index, 4);
    memset(data + 8, 0, 4 * max_size);

    UnmapViewOfFile(data);
    CloseHandle(mapping);
    CloseHandle(file);

    std::filesystem::resize_file(filename, 8 + (max_size * 4));
}

uint32_t MailBox::getMailboxCount() {
    uint32_t count = 0;
    for (const auto& entry : std::filesystem::directory_iterator("."))
        if (entry.is_regular_file() && entry.path().extension() == ".mb")
            count++;

    return count;
}
